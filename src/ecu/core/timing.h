// Injection timing: closed loop on measured advance (TDC phase in the NE
// segment), driving the TCV duty. Cranking, stall or no TDC signal falls
// back to a fixed duty, which is what the stock ECU does (DTC 14 on fault).
#pragma once
#include "calib.h"

namespace ecu {

struct TimingInputs {
    float rpm, q, ect_c;
    bool cranking;
    bool measured_valid;
    float measured_adv;
};

class TimingController {
public:
    explicit TimingController(const Calibration &c) : c_(c) {}

    float update(const TimingInputs &in, float dt) {
        target_ = c_.adv_target.at(in.q, in.rpm) + c_.adv_ect_add.at(in.ect_c);
        if (in.cranking || !in.measured_valid) {
            integ_ = 0;
            open_loop_ = true;
            duty_ = c_.tcv_fixed_duty;
            if (!in.cranking && !in.measured_valid) no_signal_s_ += dt; else no_signal_s_ = 0;
            return duty_;
        }
        open_loop_ = false;
        no_signal_s_ = 0;
        float err = target_ - in.measured_adv;
        float p = c_.tcv_kp * err;
        float trial = integ_ + c_.tcv_ki * err * dt;
        float d = c_.tcv_ff + p + trial;
        // Anti-windup: hold the integrator when the output saturates further.
        if ((d > 95 && err > 0) || (d < 0 && err < 0)) trial = integ_;
        integ_ = clampf(trial, -60, 60);
        duty_ = clampf(c_.tcv_ff + p + integ_, 0, 95);
        last_err_ = err;
        return duty_;
    }
    float target() const { return target_; }
    float duty() const { return duty_; }
    float error() const { return last_err_; }
    bool open_loop() const { return open_loop_; }
    // DTC 14 equivalent: running with no usable timing feedback for 3 s.
    bool timing_fault() const { return no_signal_s_ > 3.0f; }

private:
    const Calibration &c_;
    float integ_ = 0, duty_ = 0, target_ = 0, last_err_ = 0, no_signal_s_ = 0;
    bool open_loop_ = true;
};

}  // namespace ecu
