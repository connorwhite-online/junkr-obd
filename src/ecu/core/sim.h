// Host-side pump + engine simulator, used by the tests and by the bench
// stimulator. It produces NE and TDC edge times from a crank model, and
// turns each segment's SPV plan into delivered fuel and torque.
//
// The plant is deliberately simple (one inertia, mean torque per segment,
// first-order timer piston) but it closes every loop the firmware has.
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "calib.h"
#include "fuel.h"
#include "trigger.h"

namespace ecu {

struct SimParams {
    float inertia = 0.30f;          // kg m2 at the crank (engine + converter)
    float nm_per_mm3 = 4.2f;        // mean torque per mm3/stroke at best timing
    float best_adv = 8.0f;          // deg BTDC with the best efficiency
    float friction_nm = 12.0f, friction_per_krpm = 6.0f;
    float load_nm = 0;
    float starter_nm = 70, starter_rpm = 250;
    float ne_base_deg = -30.0f;     // NE tooth 0 vs TDC with zero advance (sets tdc offset 30)
    float timer_tau_s = 0.25f;      // timer piston response
    float adv_per_duty = 0.25f, adv_at_zero_duty = -5.0f;  // TCV duty -> advance (deg)
    float adv_per_krpm = 1.5f;      // hydraulic advance with pump speed
    float spv_true_delay_us = 300;  // actual valve response
    float spill_scale = 1.0f;       // true Q / modelled Q (calibration error)
    float ne_jitter_us = 0;         // edge noise
};

class Sim {
public:
    Sim(const Calibration &c, SimParams p = SimParams()) : c_(c), p_(p) {}

    // Event sink the test supplies.
    struct Sink {
        virtual void ne(uint32_t t) = 0;
        virtual void tdc(uint32_t t) = 0;
        virtual SpvPlan plan() = 0;   // read at tooth 0, like the ISR
        virtual ~Sink() {}
    };

    void set_rpm(float rpm) { omega_ = rpm * 2 * (float)M_PI / 60; }
    float rpm() const { return omega_ * 60 / (2 * (float)M_PI); }
    float advance() const { return adv_; }
    float last_q() const { return q_seg_; }
    uint32_t now_us() const { return (uint32_t)t_us_; }
    uint64_t injections() const { return injections_; }
    bool starter = false;
    float tcv_duty = 0;

    // Advance the model by dt_us, calling the sink for each edge.
    void step(Sink &s, double dt_us) {
        double dt = dt_us * 1e-6;
        // Torque: mean of the segment's combustion, at the current timing.
        float eff = 1.0f - 0.004f * (adv_ - p_.best_adv) * (adv_ - p_.best_adv);
        if (eff < 0.3f) eff = 0.3f;
        float rpm_now = rpm();
        float tq = p_.nm_per_mm3 * q_seg_ * eff - p_.friction_nm - p_.friction_per_krpm * rpm_now / 1000 -
                   p_.load_nm;
        if (starter && rpm_now < p_.starter_rpm * 1.6f) tq += p_.starter_nm;
        if (rpm_now < 1 && tq < 0) tq = 0;
        omega_ += (float)(tq / p_.inertia * dt);
        if (omega_ < 0) omega_ = 0;
        // Timer piston.
        float adv_ss = p_.adv_at_zero_duty + p_.adv_per_duty * tcv_duty + p_.adv_per_krpm * rpm_now / 1000;
        adv_ += (float)((adv_ss - adv_) * dt / p_.timer_tau_s);

        double th0 = theta_, th1 = theta_ + omega_ * 180.0 / M_PI * dt;   // crank deg
        // NE teeth sit at crank angle seg*180 + base + k*pitch - advance.
        // NE angle is tracked continuously so a moving timer piston never skips or repeats a tooth.
        double ne0 = have_ne_ ? ne_prev_ : th0 + adv_ - p_.ne_base_deg, ne1 = th1 + adv_ - p_.ne_base_deg;
        if (ne1 < ne0) ne1 = ne0;   // the ring can retard a little, but the teeth never run backwards here
        ne_prev_ = ne1;
        have_ne_ = true;
        long k0 = (long)floor(ne0 / NE_PITCH_DEG), k1 = (long)floor(ne1 / NE_PITCH_DEG);
        for (long k = k0 + 1; k <= k1; k++) {
            int slot = (int)(((k % 16) + 16) % 16);
            double t = t_us_ + (k * NE_PITCH_DEG - ne0) / (ne1 - ne0) * dt_us;
            if (slot < NE_TEETH_PER_SEG) {
                double j = p_.ne_jitter_us ? ((double)rand() / RAND_MAX - 0.5) * 2 * p_.ne_jitter_us : 0;
                s.ne((uint32_t)(t + j));
                if (slot == 0) on_segment_start(s, t, k);
                if (pending_ && slot == plan_.open_tooth) {
                    open_t_ = t + plan_.open_delay_us + p_.spv_true_delay_us;
                    pending_ = false;
                    opening_ = true;
                }
            }
        }
        // TDC: one per cylinder (every 180 deg CA). [VERIFY tooth count]
        long d0 = (long)floor(th0 / 180.0), d1 = (long)floor(th1 / 180.0);
        for (long d = d0 + 1; d <= d1; d++) s.tdc((uint32_t)(t_us_ + (d * 180.0 - th0) / (th1 - th0) * dt_us));
        if (opening_ && t_us_ + dt_us >= open_t_) {
            // Angle at which the valve actually opened, in NE degrees after tooth 0.
            double ne_at = ne0 + (open_t_ - t_us_) / dt_us * (ne1 - ne0);
            float a = (float)(ne_at - seg_ne0_);
            q_next_ = p_.spill_scale * c_.spill.at(a, rpm_now);
            if (q_next_ < 0) q_next_ = 0;
            opening_ = false;
            injections_++;
            last_open_deg_ = a;
        }
        theta_ = th1;
        t_us_ += dt_us;
    }
    float last_open_deg() const { return last_open_deg_; }

private:
    void on_segment_start(Sink &s, double t, long k) {
        // The previous segment's fuel burns now.
        q_seg_ = q_next_;
        q_next_ = 0;
        plan_ = s.plan();
        seg_ne0_ = k * NE_PITCH_DEG;
        pending_ = plan_.inject;
        opening_ = false;
        (void)t;
    }
    const Calibration &c_;
    SimParams p_;
    double theta_ = 0, t_us_ = 0, open_t_ = 0, seg_ne0_ = 0;
    float omega_ = 0, adv_ = 0, q_seg_ = 0, q_next_ = 0, last_open_deg_ = 0;
    SpvPlan plan_{};
    bool pending_ = false, opening_ = false, have_ne_ = false;
    double ne_prev_ = 0;
    uint64_t injections_ = 0;
};

}  // namespace ecu
