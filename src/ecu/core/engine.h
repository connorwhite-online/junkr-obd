// The engine controller: everything the STM32 runs, minus the HAL.
//
//   on_ne()/on_tdc()     from the input-capture ISRs (MAX9926 outputs)
//   on_stock_spv_open()  from the S_SPV comparator, shadow mode only
//   update()             from the 1 kHz control loop
//   spv_plan()           read by the NE ISR at tooth 0 to arm the SPV timer
//
// The HAL layer (src/ecu/stm32) maps Commands onto pins; this file never
// touches hardware, so the host tests run the same code.
#pragma once
#include <stdint.h>
#include "calib.h"
#include "fuel.h"
#include "safety.h"
#include "shadow.h"
#include "timing.h"
#include "trans.h"
#include "trigger.h"

namespace ecu {

enum class Mode : uint8_t { Shadow, Takeover };

struct Sensors {
    float pedal = 0, va_v = 0.8f;     // accelerator %, raw volts
    bool idl = true;                  // idle switch closed
    float map_kpa = 100, map_v = 1.5f;
    float ect_c = 80, ect_v = 1.0f;
    float vbatt = 13.8f;
    float speed_kmh = 0;
    Range range = Range::P;
    bool od_switch_on = true, brake = false, starter = false;
    bool supervisor_ok = true;
    bool arm_switch = false;          // RUN switch + user arm in the app
};

struct Commands {
    SpvPlan spv{};
    float tcv_duty = 0;
    bool svr_on = false;
    bool s1 = false, s2 = false, sl = false;
    bool takeover = false;            // energize the takeover relays
    bool arm_req = false;             // to the ARM AND gate
    float q = 0;
    uint32_t dtc = 0;
};

class Engine {
public:
    explicit Engine(Calibration &c) : cal_(c), fuel_(c), timing_(c), trans_(c), safety_(c), learn_(c) {}

    void set_mode(Mode m) { mode_ = m; }
    Mode mode() const { return mode_; }

    void on_ne(uint32_t t_us) {
        ne_.tooth(t_us);
        if (ne_.synced() && ne_.tooth_index() == 0) {
            // Plan this segment from the latest Q. The plan is only acted on
            // when fuel is allowed and ARM is up; otherwise the SPV stays open.
            plan_ = (fuel_ok_ && ne_.rpm_valid()) ? plan_spv(cal_, cmd_.q, ne_.rpm(), ne_.pitch_us())
                                                   : SpvPlan{};
            seg_planned_ = true;
        }
    }
    void on_tdc(uint32_t t_us) { tdc_.edge(t_us, ne_); }
    void on_stock_spv_open(uint32_t t_us, float stock_tcv_duty, const Sensors &s) {
        if (!ne_.rpm_valid()) return;
        float a = ne_.angle_at(t_us);
        float our = plan_spv(cal_, fuel_.q(), ne_.rpm(), ne_.pitch_us()).open_deg;
        learn_.observe(ne_.rpm(), pedal(s), a, stock_tcv_duty, tdc_.advance_deg(cal_.tdc_cal_offset), our);
    }

    const Commands &update(uint32_t now_us, const Sensors &s, float dt) {
        float rpm = ne_.rpm_valid() ? ne_.rpm() : ne_.rpm_inst();
        bool stalled = ne_.stalled(now_us);
        // Cranking until the engine first passes crank-exit rpm; back to cranking only after a stall.
        if (stalled || rpm < 150) running_ = false;
        if (rpm > cal_.rpm_crank_exit) running_ = true;
        bool cranking = !running_;
        SafetyInputs si{rpm, ne_.synced(), stalled, ne_.sync_losses(), s.pedal, s.va_v, s.idl,
                        s.ect_v, s.map_v, s.vbatt, s.supervisor_ok, timing_.timing_fault()};
        SafetyOutputs so = safety_.update(si, dt);
        float ect = so.ect_fault ? 80.0f : s.ect_c;      // limp defaults
        float map = so.map_fault ? 100.0f : s.map_kpa;   // no boost credit
        FuelInputs fi{rpm, pedal(s), map, ect, cranking, so.pedal_fault};
        cmd_.q = fuel_.update(fi, dt);
        fuel_ok_ = so.fuel_allowed;
        TimingInputs ti{rpm, cmd_.q, ect, cranking || stalled, tdc_.valid() && !stalled,
                        tdc_.advance_deg(cal_.tdc_cal_offset)};
        cmd_.tcv_duty = stalled ? cal_.tcv_fixed_duty : timing_.update(ti, dt);
        TransInputs tri{s.range, s.speed_kmh, pedal(s), s.od_switch_on, s.brake, ect < 40, false};
        TransOutputs to = trans_.update(tri);
        cmd_.s1 = to.s1; cmd_.s2 = to.s2; cmd_.sl = to.sl;
        cmd_.svr_on = so.svr_on;
        cmd_.takeover = mode_ == Mode::Takeover && s.arm_switch && s.supervisor_ok;
        cmd_.arm_req = cmd_.takeover;
        cmd_.spv = plan_;
        cmd_.dtc = so.dtc;
        return cmd_;
    }

    const SpvPlan &spv_plan() const { return plan_; }
    const NeDecoder &ne() const { return ne_; }
    const TdcPhase &tdc() const { return tdc_; }
    const TimingController &timing() const { return timing_; }
    const FuelController &fuel() const { return fuel_; }
    const ShadowLearner &learner() const { return learn_; }
    const A343F &trans() const { return trans_; }
    const Commands &commands() const { return cmd_; }

private:
    static float pedal(const Sensors &s) { return s.pedal; }
    Calibration &cal_;
    NeDecoder ne_;
    TdcPhase tdc_;
    FuelController fuel_;
    TimingController timing_;
    A343F trans_;
    Safety safety_;
    ShadowLearner learn_;
    Commands cmd_;
    SpvPlan plan_{};
    Mode mode_ = Mode::Shadow;
    bool fuel_ok_ = false, seg_planned_ = false, running_ = false;
};

}  // namespace ecu
