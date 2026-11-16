// Fuel quantity and SPV spill scheduling.
//
// The SPV is a normally open spill valve: de-energized means fuel spills and
// nothing is injected. Each 180 deg segment we close it before the plunger
// lifts and open it at the spill angle that ends injection at the requested Q.
// Every failure path (no sync, no ARM, cut) simply leaves it open.
#pragma once
#include <stdint.h>
#include "calib.h"
#include "trigger.h"

namespace ecu {

struct FuelInputs {
    float rpm;
    float pedal_pct;
    float map_kpa;
    float ect_c;
    bool cranking;
    bool pedal_fault;   // limp: idle plus a little
};

class FuelController {
public:
    explicit FuelController(const Calibration &c) : c_(c) {}

    float idle_target(float ect) const {
        float f = clampf((ect + 20.0f) / 100.0f, 0, 1);
        return c_.idle_rpm_cold + (c_.idle_rpm_warm - c_.idle_rpm_cold) * f;
    }

    // Requested Q in mm3/stroke. dt in seconds.
    float update(const FuelInputs &in, float dt) {
        if (in.cranking) { integ_ = 0; q_ = c_.q_start.at(in.ect_c); return q_; }
        float pedal = in.pedal_fault ? clampf(in.pedal_pct, 0, 8) : in.pedal_pct;
        float q = c_.q_driver.at(pedal, in.rpm);
        // Idle governor: only near idle with the pedal up.
        float tgt = idle_target(in.ect_c);
        if (pedal < 3 && in.rpm < tgt + 400) {
            float err = tgt - in.rpm;
            integ_ = clampf(integ_ + err * c_.idle_ki * dt, -c_.idle_q_trim_max, c_.idle_q_trim_max);
            q += clampf(err * c_.idle_kp + integ_, -c_.idle_q_trim_max, c_.idle_q_trim_max);
        } else {
            integ_ *= 0.98f;
        }
        float lim = c_.q_max.at(in.rpm);
        float smoke = c_.q_smoke.at(in.map_kpa, in.rpm);
        if (smoke < lim) lim = smoke;
        if (q > lim) q = lim;
        // Overspeed taper, then cut.
        if (in.rpm > c_.rpm_hard_cut) q = 0;
        else if (in.rpm > c_.rpm_soft_cut)
            q *= (c_.rpm_hard_cut - in.rpm) / (c_.rpm_hard_cut - c_.rpm_soft_cut);
        q_ = clampf(q, 0, 80);
        return q_;
    }
    float q() const { return q_; }

private:
    const Calibration &c_;
    float integ_ = 0, q_ = 0;
};

// One segment's SPV events, in NE terms the timer ISR can act on directly.
struct SpvPlan {
    bool inject;          // false: keep the SPV open (no fuel)
    float close_deg;      // energize at this NE angle
    float open_deg;       // de-energize (end of injection) at this angle
    int open_tooth;       // schedule from this tooth...
    uint32_t open_delay_us;  // ...plus this delay
};

inline SpvPlan plan_spv(const Calibration &c, float q, float rpm, uint32_t pitch_us) {
    SpvPlan p{};
    if (q <= 0.3f || pitch_us == 0) { p.inject = false; return p; }
    p.inject = true;
    p.close_deg = c.spv_close_deg;
    float a = c.spill.inv_x(q, rpm);
    // Command the valve early by its response time.
    float lead_deg = c.spv_open_delay_us / (float)pitch_us * NE_PITCH_DEG;
    float cmd = a - lead_deg;
    if (cmd < p.close_deg + 1) cmd = p.close_deg + 1;
    int tooth = (int)(cmd / NE_PITCH_DEG);
    if (tooth > NE_TEETH_PER_SEG - 1) tooth = NE_TEETH_PER_SEG - 1;
    p.open_deg = a;
    p.open_tooth = tooth;
    p.open_delay_us = (uint32_t)((cmd - tooth * NE_PITCH_DEG) / NE_PITCH_DEG * (float)pitch_us);
    return p;
}

}  // namespace ecu
