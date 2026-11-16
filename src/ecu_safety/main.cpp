// ATtiny1616 safety supervisor firmware. Logic lives in supervisor.h (host
// tested); this file is only the pins and timers.
//
//   PA1 NE_DIG   in   pump teeth (MAX9926 output)
//   PA2 SPV_IN   in   gated SPV command (after the ARM AND gate)
//   PA3 SPV_CMP  in   harness SPV comparator, high = valve energized
//   PA4 HB       in   STM32 heartbeat, toggled by its control loop
//   PA5 TDC_DIG  in   crank TDC (logged only)
//   PA6 ARM      in   ARM line
//   PA7 SAFE_OK  out  to the ARM AND gate (1k series, 100k pull-down)
//   PB5 LED      out  low = on: steady = OK, blinking = tripped (code)
//   PB2/PB3      USART0 to the ESP32 (status line, 115200)
//
// Build: pio run -e ecu-safety   Flash over UPDI from the ESP32 (or J12).
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>

#include "supervisor.h"

static safety::Supervisor sup;
static volatile uint8_t tick_flag;

ISR(PORTA_PORT_vect) {
    uint8_t f = PORTA.INTFLAGS;
    PORTA.INTFLAGS = f;
    if (f & PIN1_bm) sup.ne_edge();
    if (f & PIN4_bm) sup.hb_edge();
    if (f & PIN2_bm) sup.spv_level(PORTA.IN & PIN2_bm);
}

ISR(TCB0_INT_vect) {
    TCB0.INTFLAGS = TCB_CAPT_bm;
    tick_flag = 1;
}

static void uart_put(char c) {
    while (!(USART0.STATUS & USART_DREIF_bm)) {}
    USART0.TXDATAL = c;
}

int main() {
    // 20 MHz internal, no prescaler (fuse OSCCFG = 20 MHz).
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0);
    // SAFE_OK low until the supervisor says otherwise.
    PORTA.DIRSET = PIN7_bm;
    PORTA.OUTCLR = PIN7_bm;
    PORTB.DIRSET = PIN5_bm | PIN2_bm;
    PORTB.OUTSET = PIN5_bm;
    // Edges: NE rising, HB both, SPV_IN both.
    PORTA.PIN1CTRL = PORT_ISC_RISING_gc;
    PORTA.PIN4CTRL = PORT_ISC_BOTHEDGES_gc;
    PORTA.PIN2CTRL = PORT_ISC_BOTHEDGES_gc;
    // 1 kHz tick on TCB0 (periodic interrupt mode, CLK_PER/2).
    TCB0.CCMP = 10000 - 1;
    TCB0.INTCTRL = TCB_CAPT_bm;
    TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
    // USART0 115200 8N1, TX only for status.
    USART0.BAUD = (uint16_t)((64UL * 20000000UL) / (16UL * 115200UL));
    USART0.CTRLB = USART_TXEN_bm;
    // Watchdog: if this loop ever hangs, the tiny resets and SAFE_OK goes low.
    _PROTECTED_WRITE(WDT.CTRLA, WDT_PERIOD_16CLK_gc);
    sei();
    uint16_t ms = 0;
    for (;;) {
        if (!tick_flag) continue;
        tick_flag = 0;
        __asm__ __volatile__("wdr");
        bool spv_cmp = PORTA.IN & PIN3_bm;
        bool armed = PORTA.IN & PIN6_bm;
        if (sup.tick(spv_cmp, armed)) PORTA.OUTSET = PIN7_bm;
        else PORTA.OUTCLR = PIN7_bm;
        ms++;
        // LED: steady on when OK, otherwise blink the trip code.
        if (sup.safe_ok()) PORTB.OUTCLR = PIN5_bm;
        else if ((ms & 0x7F) == 0) PORTB.OUTTGL = PIN5_bm;
        if (ms >= 100) {   // 10 Hz status line: "S <ok> <trip> <rpm>"
            ms = 0;
            char buf[24];
            int n = snprintf(buf, sizeof buf, "S %u %u %u\n", sup.safe_ok(), sup.last_trip(), sup.rpm());
            for (int i = 0; i < n; i++) uart_put(buf[i]);
        }
    }
}
