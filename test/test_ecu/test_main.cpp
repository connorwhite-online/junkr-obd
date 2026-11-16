// Standalone ECU core against the pump/engine simulator: pio test -e native -f test_ecu
#include <math.h>
#include <unity.h>

#include "core/engine.h"
#include "core/sim.h"
#include "ecu_safety/supervisor.h"

using namespace ecu;

void setUp() {}
void tearDown() {}

// Wires the simulator's edges into the engine, like the ISRs do.
struct Bench : Sim::Sink {
    Calibration cal;
    Engine eng{cal};
    Sim sim;
    Sensors s;
    Commands cmd;
    bool ignore_plan = false;
    explicit Bench(SimParams p = SimParams()) : sim(cal, p) {}
    void ne(uint32_t t) override { eng.on_ne(t); }
    void tdc(uint32_t t) override { eng.on_tdc(t); }
    SpvPlan plan() override { return ignore_plan ? SpvPlan{} : eng.spv_plan(); }
    // Run for `sec`, control loop at 1 kHz, sim at 20 us steps.
    void run(float sec) {
        int ms = (int)(sec * 1000);
        for (int i = 0; i < ms; i++) {
            for (int k = 0; k < 50; k++) sim.step(*this, 20);
            sim.starter = s.starter;
            cmd = eng.update(sim.now_us(), s, 0.001f);
            sim.tcv_duty = cmd.tcv_duty;
        }
    }
};

static SimParams steady() {
    SimParams p;
    p.inertia = 1e7f;   // speed held by the "dyno"
    return p;
}

void test_tables() {
    Table1D<3> t{{0, 10, 20}, {0, 100, 120}};
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 50, t.at(5));
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 0, t.at(-5));
    TEST_ASSERT_FLOAT_WITHIN(1e-4, 120, t.at(99));
    Calibration c;
    float a = c.spill.inv_x(30, 2000);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 30, c.spill.at(a, 2000));
}

void test_decoder_sync_and_rpm() {
    const float rpms[] = {150.0f, 750.0f, 2500.0f, 4500.0f};
    for (float rpm : rpms) {
        Bench b(steady());
        b.ignore_plan = true;
        b.sim.set_rpm(rpm);
        b.run(0.5f);
        TEST_ASSERT_TRUE(b.eng.ne().synced());
        TEST_ASSERT_EQUAL_UINT32(0, b.eng.ne().sync_losses());
        TEST_ASSERT_FLOAT_WITHIN(rpm * 0.01f, rpm, b.eng.ne().rpm());
    }
}

void test_decoder_rejects_noise_and_resyncs() {
    Bench b(steady());
    b.ignore_plan = true;
    b.sim.set_rpm(1500);
    b.run(0.2f);
    TEST_ASSERT_TRUE(b.eng.ne().synced());
    // A spurious edge right after a real tooth.
    b.eng.on_ne(b.eng.ne().last_tooth_us() + 30);
    TEST_ASSERT_FALSE(b.eng.ne().synced());
    TEST_ASSERT_EQUAL_UINT32(1, b.eng.ne().sync_losses());
    b.run(0.1f);   // 0.1 s at 1500 rpm is 5 gaps
    TEST_ASSERT_TRUE(b.eng.ne().rpm_valid());
}

void test_tdc_phase_measures_advance() {
    Bench b(steady());
    b.ignore_plan = true;
    b.sim.set_rpm(2000);
    b.run(2.0f);
    TEST_ASSERT_TRUE(b.eng.tdc().valid());
    float meas = b.eng.tdc().advance_deg(b.cal.tdc_cal_offset);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, b.sim.advance(), meas);
}

void test_spv_delivers_requested_q() {
    // Fuel on a dyno: the SPV schedule (with its response-time lead) must
    // deliver what the fuel model asked for.
    Bench b(steady());
    b.sim.set_rpm(2000);
    b.s.pedal = 50; b.s.idl = false; b.s.va_v = 2.5f;
    b.run(1.0f);
    TEST_ASSERT_TRUE(b.sim.injections() > 50);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, b.eng.commands().q, b.sim.last_q());
}

void test_cold_start_to_idle() {
    Bench b;
    b.s.starter = true;
    b.run(1.5f);
    b.s.starter = false;
    b.run(8.0f);
    float idle = b.eng.fuel().idle_target(b.s.ect_c);
    TEST_ASSERT_FLOAT_WITHIN(40, idle, b.sim.rpm());
    TEST_ASSERT_FALSE(b.eng.timing().open_loop());
    TEST_ASSERT_FLOAT_WITHIN(1.0f, b.eng.timing().target(), b.sim.advance());
    TEST_ASSERT_EQUAL_UINT32(0, b.cmd.dtc);
}

void test_rev_limiter_holds() {
    Bench b;
    b.s.starter = true;
    b.run(1.5f);
    b.s.starter = false;
    b.run(2.0f);
    b.s.pedal = 100; b.s.idl = false; b.s.va_v = 4.2f;
    float peak = 0;
    for (int i = 0; i < 60; i++) {
        b.run(0.1f);
        if (b.sim.rpm() > peak) peak = b.sim.rpm();
    }
    TEST_ASSERT_TRUE(peak > 4400);
    TEST_ASSERT_TRUE(peak < 4950);
    TEST_ASSERT_TRUE(b.cmd.svr_on);
}

void test_supervisor_cut_stops_fuel() {
    Bench b;
    b.s.starter = true;
    b.run(1.5f);
    b.s.starter = false;
    b.run(3.0f);
    uint64_t n = b.sim.injections();
    TEST_ASSERT_TRUE(n > 0);
    b.s.supervisor_ok = false;
    b.run(0.05f);
    n = b.sim.injections();
    b.run(3.0f);
    TEST_ASSERT_TRUE(b.sim.injections() - n <= 1);
    TEST_ASSERT_FALSE(b.cmd.svr_on);
    TEST_ASSERT_TRUE(b.cmd.dtc & DTC_SUPERVISOR);
    TEST_ASSERT_TRUE(b.sim.rpm() < 300);   // engine winding down
}

void test_stall_goes_open_loop() {
    Bench b;
    b.s.starter = true;
    b.run(1.5f);
    b.s.starter = false;
    b.run(2.0f);
    b.s.supervisor_ok = false;   // kill it
    b.run(5.0f);
    TEST_ASSERT_TRUE(b.eng.ne().stalled(b.sim.now_us()));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, b.cal.tcv_fixed_duty, b.cmd.tcv_duty);
    TEST_ASSERT_FALSE(b.eng.spv_plan().inject);
}

void test_pedal_mismatch_limps() {
    Bench b(steady());
    b.sim.set_rpm(2000);
    b.s.pedal = 80;
    b.s.va_v = 3.5f;
    b.s.idl = true;   // switch says released: VA can't be trusted
    b.run(0.5f);
    TEST_ASSERT_TRUE(b.cmd.dtc & DTC_PEDAL);
    Calibration c;
    TEST_ASSERT_TRUE(b.cmd.q <= c.q_driver.at(8, 2000) + 0.01f);
}

void test_sensor_faults_use_defaults() {
    Bench b(steady());
    b.sim.set_rpm(2000);
    b.s.pedal = 100; b.s.idl = false; b.s.va_v = 4.2f;
    b.s.map_kpa = 220;   // claims full boost...
    b.s.map_v = 0.0f;    // ...but the sensor is open circuit
    b.run(0.3f);
    TEST_ASSERT_TRUE(b.cmd.dtc & DTC_MAP);
    Calibration c;
    TEST_ASSERT_TRUE(b.cmd.q <= c.q_smoke.at(100, 2000) + 0.01f);
}

void test_a343f_shifts() {
    Calibration c;
    A343F t(c);
    TransInputs in{Range::D, 0, 30, true, false, false, false};
    TransOutputs o = t.update(in);
    TEST_ASSERT_EQUAL_INT(1, o.gear);
    TEST_ASSERT_TRUE(o.s1 && !o.s2);
    int seen = 1;
    for (float v = 0; v < 100; v += 1) {
        in.speed_kmh = v;
        o = t.update(in);
        TEST_ASSERT_TRUE(o.gear >= seen);   // never downshifts while accelerating
        seen = o.gear;
    }
    TEST_ASSERT_EQUAL_INT(4, o.gear);
    TEST_ASSERT_TRUE(!o.s1 && !o.s2);
    TEST_ASSERT_TRUE(o.sl);
    in.brake = true;
    TEST_ASSERT_FALSE(t.update(in).sl);
    in.brake = false;
    in.od_switch_on = false;
    o = t.update(in);
    TEST_ASSERT_EQUAL_INT(3, o.gear);
    TEST_ASSERT_TRUE(!o.s1 && o.s2);
    in.fault = true;
    o = t.update(in);
    TEST_ASSERT_TRUE(!o.s1 && !o.s2 && !o.sl);   // hydraulic limp
}

void test_a343f_hysteresis() {
    Calibration c;
    A343F t(c);
    TransInputs in{Range::D, 0, 30, true, false, false, false};
    float up = c.up12.at(30);
    in.speed_kmh = up + 1;
    TEST_ASSERT_EQUAL_INT(2, t.update(in).gear);
    in.speed_kmh = up - 2;   // inside the hysteresis band
    TEST_ASSERT_EQUAL_INT(2, t.update(in).gear);
    in.speed_kmh = up - c.down_hyst_kmh - 1;
    TEST_ASSERT_EQUAL_INT(1, t.update(in).gear);
}

void test_shadow_learns_stock_map() {
    // The "stock ECU" runs 15% richer than our starting map. Watching its SPV
    // edges must pull our map onto it, and our plan must then agree with it.
    Calibration stock;
    for (auto &row : stock.q_driver.z)
        for (auto &z : row) z *= 1.15f;
    Calibration ours;
    Engine eng(ours);
    NeDecoder ne;
    Sensors s;
    float rpm = 1800, pedal = 50;
    s.pedal = pedal;
    s.idl = false;
    for (int seg = 0; seg < 400; seg++) {
        // Fake the timing: segments at 1800 rpm, tooth edges synthesized directly.
        uint32_t t0 = 1000 + seg * (uint32_t)(30e6 / rpm);
        uint32_t pitch = (uint32_t)(30e6 / rpm / 16);
        for (int k = 0; k < NE_TEETH_PER_SEG; k++) eng.on_ne(t0 + k * pitch);
        eng.update(t0 + 12 * pitch, s, 0.001f);
        float q_stock = stock.q_driver.at(pedal, rpm);
        float a = stock.spill.inv_x(q_stock, rpm);
        uint32_t t_open = t0 + (uint32_t)(a / NE_PITCH_DEG * pitch);
        if (a / NE_PITCH_DEG < 12) eng.on_stock_spv_open(t_open, 40, s);
    }
    int i = 3, j = 2;   // pedal 50, rpm 1800
    TEST_ASSERT_TRUE(eng.learner().samples(j, i) > 300);
    int n = eng.learner().export_to(ours);
    TEST_ASSERT_EQUAL_INT(1, n);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, stock.q_driver.at(pedal, rpm), ours.q_driver.z[j][i]);
}

// ---- ATtiny supervisor ----
static void ticks(safety::Supervisor &s, int ms, bool hb, bool cmp = false, bool armed = true, int ne_per_ms = 0) {
    for (int i = 0; i < ms; i++) {
        if (hb && i % 5 == 0) s.hb_edge();
        for (int k = 0; k < ne_per_ms; k++) s.ne_edge();
        s.tick(cmp, armed);
    }
}

void test_supervisor_arms_after_heartbeat() {
    safety::Supervisor s;
    TEST_ASSERT_FALSE(s.safe_ok());   // low from power-up
    ticks(s, 300, true);
    TEST_ASSERT_FALSE(s.safe_ok());   // needs 500 ms of heartbeat
    ticks(s, 400, true);
    TEST_ASSERT_TRUE(s.safe_ok());
}

void test_supervisor_heartbeat_loss_latches() {
    safety::Supervisor s;
    ticks(s, 700, true);
    TEST_ASSERT_TRUE(s.safe_ok());
    ticks(s, 200, true, false, true, 1);   // running: 1 tooth/ms = 2308 rpm
    ticks(s, 60, false, false, true, 1);   // STM32 hangs
    TEST_ASSERT_FALSE(s.safe_ok());
    TEST_ASSERT_TRUE(s.trip() & safety::TRIP_HEARTBEAT);
    ticks(s, 2000, true, false, true, 1);  // it recovers, but the engine still turns
    TEST_ASSERT_FALSE(s.safe_ok());
    ticks(s, 1000, true);                  // stopped: re-arms
    TEST_ASSERT_TRUE(s.safe_ok());
}

void test_supervisor_overspeed() {
    safety::Supervisor s;
    ticks(s, 700, true);
    ticks(s, 300, true, false, true, 2);   // 2 teeth/ms = 4615 rpm
    TEST_ASSERT_TRUE(s.safe_ok());
    TEST_ASSERT_UINT_WITHIN(20, 4615, s.rpm());
    ticks(s, 300, true, false, true, 3);   // 6923 rpm
    TEST_ASSERT_TRUE(s.trip() & safety::TRIP_OVERSPEED);
}

void test_supervisor_spv_stuck_and_short() {
    safety::Supervisor s;
    ticks(s, 700, true);
    s.spv_level(true);
    for (int i = 0; i < 6; i++) s.ne_edge();   // a normal injection
    s.spv_level(false);
    ticks(s, 1, true);
    TEST_ASSERT_TRUE(s.safe_ok());
    s.spv_level(true);
    for (int i = 0; i < 9; i++) s.ne_edge();   // closed for > 90 deg
    ticks(s, 1, true);
    TEST_ASSERT_TRUE(s.trip() & safety::TRIP_SPV_STUCK_ON);

    safety::Supervisor t;
    ticks(t, 700, true);
    ticks(t, 50, true, true, false);   // shadow mode: stock ECU drives the SPV, fine
    TEST_ASSERT_TRUE(t.safe_ok());
    ticks(t, 50, true, true, true);    // armed, our command off, valve energized: short
    TEST_ASSERT_TRUE(t.trip() & safety::TRIP_SPV_SHORT);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_tables);
    RUN_TEST(test_decoder_sync_and_rpm);
    RUN_TEST(test_decoder_rejects_noise_and_resyncs);
    RUN_TEST(test_tdc_phase_measures_advance);
    RUN_TEST(test_spv_delivers_requested_q);
    RUN_TEST(test_cold_start_to_idle);
    RUN_TEST(test_rev_limiter_holds);
    RUN_TEST(test_supervisor_cut_stops_fuel);
    RUN_TEST(test_stall_goes_open_loop);
    RUN_TEST(test_pedal_mismatch_limps);
    RUN_TEST(test_sensor_faults_use_defaults);
    RUN_TEST(test_a343f_shifts);
    RUN_TEST(test_a343f_hysteresis);
    RUN_TEST(test_shadow_learns_stock_map);
    RUN_TEST(test_supervisor_arms_after_heartbeat);
    RUN_TEST(test_supervisor_heartbeat_loss_latches);
    RUN_TEST(test_supervisor_overspeed);
    RUN_TEST(test_supervisor_spv_stuck_and_short);
    return UNITY_END();
}
