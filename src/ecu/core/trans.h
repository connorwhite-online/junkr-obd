// A343F 4-speed automatic: shift solenoids S1/S2 and lock-up SL.
//   1st S1 on  S2 off | 2nd on on | 3rd off on | OD off off
// With both solenoids off the valve body gives OD in D, 3rd in 2, 1st in L,
// so "all off" is the limp state.
#pragma once
#include "calib.h"

namespace ecu {

enum class Range : uint8_t { P, R, N, D, Two, L };

struct TransInputs {
    Range range;
    float speed_kmh;
    float pedal_pct;
    bool od_switch_on;   // OD allowed (button not pressed)
    bool brake;
    bool ect_cold;       // no OD / lock-up when cold
    bool fault;          // speed sensor or solenoid fault: limp
};

struct TransOutputs { bool s1, s2, sl; int gear; };

class A343F {
public:
    explicit A343F(const Calibration &c) : c_(c) {}

    TransOutputs update(const TransInputs &in) {
        TransOutputs o{false, false, false, gear_};
        if (in.fault) { o.gear = 0; return o; }   // limp: hydraulic default
        int maxg = 4;
        if (in.range == Range::Two) maxg = 3;   // [VERIFY] range limits against the owner manual
        if (in.range == Range::L) maxg = 2;
        if (!in.od_switch_on || in.ect_cold) maxg = maxg < 3 ? maxg : 3;
        if (in.range == Range::P || in.range == Range::N || in.range == Range::R) {
            gear_ = 1;
        } else {
            float v = in.speed_kmh, p = in.pedal_pct, h = c_.down_hyst_kmh;
            int g = gear_;
            if (g == 1 && v > c_.up12.at(p)) g = 2;
            if (g == 2 && v > c_.up23.at(p)) g = 3;
            if (g == 3 && v > c_.up3o.at(p)) g = 4;
            if (g == 4 && v < c_.up3o.at(p) - h) g = 3;
            if (g == 3 && v < c_.up23.at(p) - h) g = 2;
            if (g == 2 && v < c_.up12.at(p) - h) g = 1;
            if (g > maxg) g = maxg;
            gear_ = g;
        }
        o.gear = gear_;
        switch (gear_) {
            case 1: o.s1 = true;  o.s2 = false; break;
            case 2: o.s1 = true;  o.s2 = true;  break;
            case 3: o.s1 = false; o.s2 = true;  break;
            default: o.s1 = false; o.s2 = false; break;
        }
        if (gear_ == 4 && !in.brake && !in.ect_cold && in.pedal_pct < c_.lockup_max_pedal) {
            if (in.speed_kmh > c_.lockup_on_kmh) lock_ = true;
        }
        if (gear_ != 4 || in.brake || in.speed_kmh < c_.lockup_off_kmh || in.pedal_pct >= c_.lockup_max_pedal)
            lock_ = false;
        o.sl = lock_;
        return o;
    }
    int gear() const { return gear_; }

private:
    const Calibration &c_;
    int gear_ = 1;
    bool lock_ = false;
};

}  // namespace ecu
