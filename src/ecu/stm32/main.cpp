// STM32H743 engine firmware for the JUNKR standalone ECU (rev A board).
// Also runs on a NUCLEO-H743ZI (same chip) for bench work: pio run -e ecu
//
// Timing:
//   TIM2  32-bit, 1 MHz free-running. CH1 NE, CH2 TDC, CH3 harness SPV edge,
//         CH4 harness TCV edge, all input capture: every edge has a hardware
//         timestamp, so ISR latency never shows up as angle error.
//   TIM5  32-bit, 1 MHz, output compare: SPV close / peak end / open events.
//   TIM1  CH1 (PE9) SPV gate, 20 kHz PWM: 100 % for the peak, then hold duty.
//   TIM8  CH1 (PC6) TCV PWM.
// Everything else (fuel, timing loop, A343F, safety) is ecu::Engine, the same
// code the host tests run against the simulator.
//
// Nothing here can fuel the engine on its own: SPV_CMD, TCV_CMD and SVR_CMD
// pass through the ARM AND gate (ARM_REQ & SAFE_OK & RUN switch).
#include <Arduino.h>
#include <HardwareTimer.h>

#include "board_pins.h"
#include "core/engine.h"
#include "core/table.h"

using namespace ecu;

static Calibration cal;
static Engine eng(cal);
static Sensors sens;

static HardwareTimer t2(TIM2), t5(TIM5), t1(TIM1), t8(TIM8);
static Uart esp_link(PA10, PA9);   // USART1 to the ESP32 (also the ROM bootloader)

constexpr float SPV_PEAK_US = 450;    // full current until the valve has pulled in [VERIFY]
constexpr float SPV_HOLD_PCT = 35;    // hold duty at 20 kHz [VERIFY on the bench, ISPV]
constexpr uint32_t TCV_HZ = 300;      // [VERIFY] stock TCV frequency, scope it in shadow mode
constexpr float SPD_PULSES_PER_KM = 2548;   // 637 rev/km x 4 pulses [VERIFY]

// ---------------------------------------------------------------- SPV events
static volatile uint32_t spv_close_at, spv_open_at;
static volatile bool spv_close_armed, spv_open_armed, spv_on;

static inline uint32_t t5_now() { return TIM5->CNT; }
static inline uint32_t t2_now() { return TIM2->CNT; }

static void spv_set(bool on) {
    spv_on = on;
    t1.setCaptureCompare(1, on ? 100 : 0, PERCENT_COMPARE_FORMAT);
    digitalWrite(PE11, on ? HIGH : LOW);   // SPV_REC: recirculate while held, fast decay at open
}

static void t5_ch1_isr() {   // SPV close (start of the injection window)
    if (!spv_close_armed) return;
    spv_close_armed = false;
    spv_set(true);
    t5.setCaptureCompare(2, t5_now() + (uint32_t)SPV_PEAK_US, TICK_COMPARE_FORMAT);
}
static void t5_ch2_isr() {   // peak over: drop to hold
    if (spv_on) t1.setCaptureCompare(1, (uint32_t)SPV_HOLD_PCT, PERCENT_COMPARE_FORMAT);
}
static void t5_ch3_isr() {   // SPV open: end of injection
    if (!spv_open_armed) return;
    spv_open_armed = false;
    spv_set(false);
}

// Edge timestamps are TIM2 ticks; events are scheduled on TIM5. Both run at
// 1 MHz from the same clock, so one offset converts between them.
static inline uint32_t to_t5(uint32_t t2_tick) { return t2_tick + (t5_now() - t2_now()); }

// ---------------------------------------------------------------- edge ISRs
static void ne_isr() {
    uint32_t t = t2.getCaptureCompare(1, TICK_COMPARE_FORMAT);
    eng.on_ne(t);
    const NeDecoder &ne = eng.ne();
    const SpvPlan &p = eng.spv_plan();
    if (!ne.synced() || !p.inject) {
        if (spv_on) spv_set(false);
        spv_close_armed = spv_open_armed = false;
        return;
    }
    uint32_t pitch = ne.pitch_us();
    if (ne.tooth_index() == 0) {
        spv_close_at = to_t5(t) + (uint32_t)(p.close_deg / NE_PITCH_DEG * pitch);
        spv_close_armed = true;
        t5.setCaptureCompare(1, spv_close_at, TICK_COMPARE_FORMAT);
    }
    if (ne.tooth_index() == p.open_tooth) {
        spv_open_at = to_t5(t) + p.open_delay_us;
        spv_open_armed = true;
        t5.setCaptureCompare(3, spv_open_at, TICK_COMPARE_FORMAT);
    }
}
static void tdc_isr() { eng.on_tdc(t2.getCaptureCompare(2, TICK_COMPARE_FORMAT)); }

// Harness SPV edge: in shadow mode this is the stock ECU's valve; the falling
// edge (valve opens) is the end of its injection.
static volatile uint32_t stock_tcv_rise, stock_tcv_period, stock_tcv_high;
static volatile float stock_tcv_duty;
static void spv_cmp_isr() {
    if (eng.mode() == Mode::Shadow) eng.on_stock_spv_open(t2.getCaptureCompare(3, TICK_COMPARE_FORMAT), stock_tcv_duty, sens);
}
static void tcv_cmp_isr() {
    uint32_t t = t2.getCaptureCompare(4, TICK_COMPARE_FORMAT);
    bool high = digitalRead(PB11);
    if (high) { stock_tcv_period = t - stock_tcv_rise; stock_tcv_rise = t; }
    else stock_tcv_high = t - stock_tcv_rise;
    if (stock_tcv_period) stock_tcv_duty = 100.0f * stock_tcv_high / stock_tcv_period;
}

// ---------------------------------------------------------------- vehicle speed
static volatile uint32_t spd_last, spd_period;
static void spd_isr() {
    uint32_t t = micros();
    spd_period = t - spd_last;
    spd_last = t;
}

// ---------------------------------------------------------------- analog
// ADC reference is REF3030 (3.000 V); every input arrives through a follower
// and a 10k/15k divider, so pin volts x 1/0.6 = harness volts.
static float harness_v(uint32_t pin) { return analogRead(pin) * (3.0f / 4095.0f) / 0.6f; }

// Toyota NTC (THW, THA, THF, THO) against the ECU's 2.49k pull-up to VC.
static Table1D<7> ntc{{0.0f, 0.37f, 0.85f, 1.6f, 2.5f, 3.4f, 4.1f}, {140, 100, 75, 50, 25, 0, -20}};
// PIM (MAP) volts -> kPa abs. [VERIFY] against the stock sensor in shadow mode.
static Table1D<2> pim{{0.5f, 4.5f}, {20, 250}};
// VA volts -> pedal %. [VERIFY] closed / wide-open volts on the car.
static Table1D<2> va{{0.6f, 4.0f}, {0, 100}};

// Slow inputs through three CD74HC4067 muxes (MUX_S0..3), 100k/22k dividers.
enum MuxA { IDL, STP, AC, STA, IGSW, PATTERN, OD2, IMI, ACT, HSW, NSW, L4, TFN, SHIFT_L, SHIFT_2, SNW };
static uint16_t mux_bits[3];
static void scan_muxes() {
    const uint32_t sel[4] = {PF0, PF1, PF2, PF15};
    const uint32_t in[3] = {PF5, PF6, PF7};
    uint16_t b[3] = {0, 0, 0};
    for (int ch = 0; ch < 16; ch++) {
        for (int k = 0; k < 4; k++) digitalWrite(sel[k], (ch >> k) & 1);
        delayMicroseconds(5);
        for (int m = 0; m < 3; m++)
            if (analogRead(in[m]) * (3.0f / 4095.0f) > 1.3f) b[m] |= 1u << ch;   // > ~7 V at the harness
    }
    for (int m = 0; m < 3; m++) mux_bits[m] = b[m];
}
static bool mux(int m, int ch) { return mux_bits[m] >> ch & 1; }

static void read_sensors() {
    scan_muxes();
    sens.va_v = harness_v(PA3);
    sens.pedal = clampf(va.at(sens.va_v), 0, 100);
    sens.idl = !mux(0, IDL);   // IDL pulls the line to E2 when the pedal is released [VERIFY polarity]
    sens.ect_v = harness_v(PA4);
    sens.ect_c = ntc.at(sens.ect_v);
    sens.map_v = harness_v(PA0);
    sens.map_kpa = pim.at(sens.map_v);
    sens.vbatt = harness_v(PF13) * (18.0f / 5.0f);   // VB_A has its own 100k/22k first
    sens.starter = mux(0, STA);
    sens.brake = mux(0, STP);
    sens.od_switch_on = !mux(0, OD2);
    sens.range = mux(0, NSW) ? Range::P : mux(0, SHIFT_L) ? Range::L : mux(0, SHIFT_2) ? Range::Two : Range::D;
    uint32_t per = spd_period;
    sens.speed_kmh = (per && micros() - spd_last < 500000) ? 3.6e9f / (per * SPD_PULSES_PER_KM) : 0;
    sens.arm_switch = mux(2, 14);   // RUN_SW
    sens.supervisor_ok = digitalRead(PE12);
}

// ---------------------------------------------------------------- outputs
static void write_outputs(const Commands &c) {
    t8.setCaptureCompare(1, (uint32_t)c.tcv_duty, PERCENT_COMPARE_FORMAT);
    digitalWrite(PD7, c.svr_on);                 // SVR_CMD (gated by ARM)
    digitalWrite(PG0, c.s1);                     // DRV_S1
    digitalWrite(PG4, c.s2);                     // DRV_S2
    digitalWrite(PG8, c.sl);                     // DRV_SL
    bool k = c.takeover;
    digitalWrite(PD3, k);                        // K_FUEL
    digitalWrite(PD4, k);                        // K_TIME
    digitalWrite(PD5, k);                        // K_TRANS
    digitalWrite(PD6, k);                        // K_AIR
    digitalWrite(PD11, c.arm_req);               // ARM_REQ
    digitalWrite(PD8, eng.ne().rpm() > 0 || sens.starter);   // MREL_CMD: hold main relay while turning
    digitalWrite(PB5, c.dtc != 0);               // LED_FAULT
}

// ---------------------------------------------------------------- link to the ESP32
// 50 Hz CSV line the ESP32 logs and serves to the tuner app:
// D,ms,rpm,q,adv,adv_tgt,tcv,ped,map,ect,gear,dtc,mode,agree
static void report() {
    char b[160];
    const Commands &c = eng.commands();
    snprintf(b, sizeof b, "D,%lu,%.0f,%.1f,%.1f,%.1f,%.0f,%.0f,%.0f,%.0f,%d,%lu,%d,%lu\n",
             (unsigned long)millis(), eng.ne().rpm(), c.q, eng.tdc().advance_deg(cal.tdc_cal_offset),
             eng.timing().target(), c.tcv_duty, sens.pedal, sens.map_kpa, sens.ect_c, eng.trans().gear(),
             (unsigned long)c.dtc, (int)eng.mode(), (unsigned long)eng.learner().agree_run());
    esp_link.print(b);
}

// ---------------------------------------------------------------- setup / loop
static bool standalone = false;

void setup() {
    esp_link.begin(921600);
    // VC_OFF_MCU high first: our sensor 5 V stays off until we know the
    // stock ECU is not there (it has a 100k pull-up, so it is off at reset too).
    pinMode(PD9, OUTPUT);
    digitalWrite(PD9, HIGH);
    const uint32_t outs[] = {PE11, PD7, PG0, PG4, PG8, PD3, PD4, PD5, PD6, PD11, PD8, PD10,
                             PE8, PB4, PB5, PF0, PF1, PF2, PF15, PE4, PE6};
    for (uint32_t p : outs) { pinMode(p, OUTPUT); digitalWrite(p, LOW); }
    pinMode(PE12, INPUT);   // SAFE_OK
    pinMode(PE10, INPUT);   // ARM (readback)
    analogReadResolution(12);

    // TIM2: 1 MHz timestamps on all four harness timing inputs.
    t2.setPrescaleFactor(t2.getTimerClkFreq() / 1000000);
    t2.setOverflow(0xFFFFFFFF, TICK_FORMAT);
    t2.setMode(1, TIMER_INPUT_CAPTURE_RISING, PA15);
    t2.setMode(2, TIMER_INPUT_CAPTURE_RISING, PB3);
    t2.setMode(3, TIMER_INPUT_CAPTURE_FALLING, PB10);
    t2.setMode(4, TIMER_INPUT_CAPTURE_BOTHEDGE, PB11);
    t2.attachInterrupt(1, ne_isr);
    t2.attachInterrupt(2, tdc_isr);
    t2.attachInterrupt(3, spv_cmp_isr);
    t2.attachInterrupt(4, tcv_cmp_isr);
    t2.setInterruptPriority(0, 0);

    // TIM5: SPV event scheduler, same 1 MHz.
    t5.setPrescaleFactor(t5.getTimerClkFreq() / 1000000);
    t5.setOverflow(0xFFFFFFFF, TICK_FORMAT);
    t5.setMode(1, TIMER_OUTPUT_COMPARE);
    t5.setMode(2, TIMER_OUTPUT_COMPARE);
    t5.setMode(3, TIMER_OUTPUT_COMPARE);
    t5.attachInterrupt(1, t5_ch1_isr);
    t5.attachInterrupt(2, t5_ch2_isr);
    t5.attachInterrupt(3, t5_ch3_isr);
    t5.setInterruptPriority(0, 0);

    t1.setPWM(1, PE9, 20000, 0);       // SPV gate
    t8.setPWM(1, PC6, TCV_HZ, 0);      // TCV
    t2.resume();
    t5.resume();
    attachInterrupt(digitalPinToInterrupt(PD13), spd_isr, RISING);   // SP1 (vehicle speed)

    // Shadow mode when the stock ECU is present (its VC 5 V shows up on VC_A
    // before ours is switched on); standalone takeover needs the app's unlock.
    delay(50);
    eng.set_mode(Mode::Shadow);
    if (harness_v(PC1) < 1.0f) {
        // No stock ECU: we are the only ECU, so power the sensors (VC) and
        // switch in our own 2.49k sensor pull-ups (PU_EN).
        digitalWrite(PD9, LOW);
        digitalWrite(PD10, HIGH);
        standalone = true;
    }
}

void loop() {
    static uint32_t last_us = micros(), n = 0;
    uint32_t now = micros();
    if (now - last_us < 1000) return;
    last_us += 1000;
    read_sensors();
    const Commands &c = eng.update(t2_now(), sens, 0.001f);
    write_outputs(c);
    digitalWrite(PE8, ++n & 1);   // HB to the ATtiny: it trips if this stops
    if (n % 20 == 0) report();
    if (esp_link.available()) {
        // Minimal command set from the ESP32: "T" request takeover, "S" shadow.
        int ch = esp_link.read();
        if (ch == 'S') eng.set_mode(Mode::Shadow);
        // Takeover needs a long run of agreement with the stock ECU, unless
        // there is no stock ECU at all (standalone, bench or engine stand).
        if (ch == 'T' && (standalone || eng.learner().agrees())) eng.set_mode(Mode::Takeover);
    }
}
