// ATtiny1616 safety supervisor: the logic, separate from the AVR registers so
// the host tests run it. main.cpp feeds it edges and a 1 kHz tick.
//
// It owns SAFE_OK, one of the three inputs of the ARM AND gate. Dropping it
// removes ARM, which (in hardware, no firmware involved):
//   - gates SPV_IN low: the spill valve opens, so no fuel is injected
//   - gates TCV_IN and SVR_G low
//   - drops VRLY: the takeover relays fall back to the stock ECU
//
// It drops SAFE_OK when:
//   - the STM32 heartbeat (HB, toggled from the control loop) stops for 50 ms
//   - its own NE count says the engine is over the hard limit
//   - the gated SPV command stays on for more NE teeth than any real
//     injection can (angle, not time, so slow cranking is fine)
//   - the SPV harness comparator says the valve is energized while the gated
//     command is off (a shorted driver)
// and it latches until the engine has stopped and the STM32 heartbeats again.
#pragma once
#include <stdint.h>

namespace safety {

enum Trip : uint8_t {
    TRIP_NONE = 0,
    TRIP_HEARTBEAT = 1,
    TRIP_OVERSPEED = 2,
    TRIP_SPV_STUCK_ON = 4,
    TRIP_SPV_SHORT = 8,
};

struct Config {
    uint16_t hb_timeout_ms = 50;
    uint16_t overspeed_rpm = 5300;   // above the STM32's own cuts (4900 fuel, 5200 SVR)
    uint8_t spv_max_teeth = 8;       // 90 deg CA: the SPV is never closed that long
    uint16_t spv_max_on_ms = 400;    // and never longer than this, even cranking
    uint16_t short_ms = 5;
};

class Supervisor {
public:
    explicit Supervisor(Config c = Config()) : c_(c) {}

    // Call from the pin-change / event ISRs.
    void ne_edge() {
        ne_count_++;
        if (spv_on_ && spv_teeth_ < 255) spv_teeth_++;
    }
    void spv_level(bool on) {
        if (on && !spv_on_) { spv_teeth_ = 0; spv_ms_ = 0; }
        spv_on_ = on;
    }
    void hb_edge() { hb_age_ms_ = 0; hb_seen_ = true; }

    // 1 kHz tick. spv_cmp: the harness comparator says the valve is energized.
    // armed: the ARM line is up (our drivers own the harness). In shadow mode
    // the stock ECU drives the harness SPV, so the short check waits for ARM
    // plus the relays' switching time.
    bool tick(bool spv_cmp, bool armed = true) {
        if (!armed) armed_ms_ = 0; else if (armed_ms_ < 0xFFFF) armed_ms_++;
        bool owns = armed_ms_ > 30;
        bool spv_in = spv_on_;
        if (spv_in && spv_ms_ < 0xFFFF) spv_ms_++;
        if (hb_age_ms_ < 0xFFFF) hb_age_ms_++;
        // Engine speed from NE: 52 teeth per 720 deg CA, counted over 100 ms.
        if (++win_ms_ >= 100) {
            // rpm = teeth/0.1s / 52 per 2 rev * 60 = teeth * 600 / 26
            rpm_ = (uint16_t)((uint32_t)ne_count_ * 600 / 26);
            ne_count_ = 0;
            win_ms_ = 0;
        }
        if (hb_seen_ && hb_age_ms_ > c_.hb_timeout_ms) trip_ |= TRIP_HEARTBEAT;
        if (!hb_seen_) trip_ |= TRIP_HEARTBEAT;   // never armed before the STM32 runs
        if (rpm_ > c_.overspeed_rpm) trip_ |= TRIP_OVERSPEED;
        if (spv_in && (spv_teeth_ > c_.spv_max_teeth || spv_ms_ > c_.spv_max_on_ms)) trip_ |= TRIP_SPV_STUCK_ON;
        if (owns && !spv_in && spv_cmp) { if (++short_ms_ > c_.short_ms) trip_ |= TRIP_SPV_SHORT; }
        else short_ms_ = 0;
        // Re-arm only when stopped and the STM32 is alive again.
        if (trip_ && rpm_ == 0 && hb_seen_ && hb_age_ms_ < c_.hb_timeout_ms) {
            if (++clear_ms_ > 500) { trip_ = TRIP_NONE; clear_ms_ = 0; }
        } else {
            clear_ms_ = 0;
        }
        if (trip_) last_trip_ = trip_;
        return safe_ok();
    }
    bool safe_ok() const { return trip_ == TRIP_NONE; }
    uint8_t trip() const { return trip_; }
    uint8_t last_trip() const { return last_trip_; }
    uint16_t rpm() const { return rpm_; }

private:
    Config c_;
    uint16_t ne_count_ = 0, win_ms_ = 0, rpm_ = 0, hb_age_ms_ = 0xFFFF, short_ms_ = 0, clear_ms_ = 0;
    bool hb_seen_ = false, spv_on_ = false;
    uint8_t spv_teeth_ = 0;
    uint16_t spv_ms_ = 0, armed_ms_ = 0;
    uint8_t trip_ = TRIP_HEARTBEAT, last_trip_ = 0;
};

}  // namespace safety
