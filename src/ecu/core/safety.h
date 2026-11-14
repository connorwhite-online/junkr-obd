// In-MCU safety rules. The ATtiny supervisor (src/ecu_safety) enforces the
// hard ones again in hardware: it alone can drop SAFE_OK, which removes ARM,
// opens the SPV (no fuel) and drops relay coil power.
#pragma once
#include <stdint.h>
#include "calib.h"

namespace ecu {

enum Dtc : uint32_t {
    DTC_NONE = 0,
    DTC_NE = 1u << 0,          // NE signal / sync (Toyota 12/13 territory)
    DTC_TIMING = 1u << 1,      // timing control (Toyota 14)
    DTC_PEDAL = 1u << 2,       // VA out of range or disagrees with IDL (Toyota 19)
    DTC_OVERSPEED = 1u << 3,
    DTC_ECT = 1u << 4,         // coolant sensor out of range (Toyota 22)
    DTC_MAP = 1u << 5,         // MAP out of range (Toyota 31/32)
    DTC_VBATT = 1u << 6,
    DTC_SUPERVISOR = 1u << 7,  // ATtiny withdrew SAFE_OK
};

struct SafetyInputs {
    float rpm;
    bool synced, stalled;
    uint32_t sync_losses;
    float pedal_pct, va_v;     // accelerator position, % and raw volts
    bool idl;                  // idle switch closed (pedal released)
    float ect_v, map_v, vbatt; // raw volts where range checks matter
    bool supervisor_ok;
    bool timing_fault;
};

struct SafetyOutputs {
    bool fuel_allowed;   // false: SPV stays open
    bool svr_on;         // SVR relay (SPV supply)
    bool pedal_fault, ect_fault, map_fault;
    uint32_t dtc;
};

class Safety {
public:
    explicit Safety(const Calibration &c) : c_(c) {}

    SafetyOutputs update(const SafetyInputs &in, float dt) {
        SafetyOutputs o{};
        // Overspeed latches the SVR off until the engine is back below 1000 rpm.
        if (in.rpm > c_.rpm_svr_cut) { overspeed_ = true; dtc_ |= DTC_OVERSPEED; }
        if (overspeed_ && (in.rpm < 1000 || in.stalled)) overspeed_ = false;
        // Repeated sync loss: cut fuel and hold it until NE is stable for 0.5 s.
        if (in.sync_losses != last_losses_) {
            last_losses_ = in.sync_losses;
            loss_hold_s_ = 0.5f;
            if (++loss_count_ >= 3) dtc_ |= DTC_NE;
        }
        if (in.synced && loss_hold_s_ > 0) loss_hold_s_ -= dt;
        // The 1KZ-TE pedal has one track (VA) plus the IDL switch: VA must be in
        // range, and IDL closed means the pedal is (nearly) released.
        bool va_range = in.va_v < 0.2f || in.va_v > 4.8f;
        bool idl_disagree = in.idl && in.pedal_pct > c_.pedal_mismatch_pct;
        if (idl_disagree) idl_bad_s_ += dt; else idl_bad_s_ = 0;
        o.pedal_fault = va_range || idl_bad_s_ > 0.2f;
        if (o.pedal_fault) dtc_ |= DTC_PEDAL;
        o.ect_fault = in.ect_v < 0.1f || in.ect_v > 4.9f;
        if (o.ect_fault) dtc_ |= DTC_ECT;
        o.map_fault = in.map_v < 0.1f || in.map_v > 4.9f;
        if (o.map_fault) dtc_ |= DTC_MAP;
        if (in.vbatt < 8.0f && in.rpm > 500) dtc_ |= DTC_VBATT;
        if (!in.supervisor_ok) dtc_ |= DTC_SUPERVISOR;
        if (in.timing_fault) dtc_ |= DTC_TIMING;
        o.svr_on = !overspeed_ && in.supervisor_ok;
        o.fuel_allowed = o.svr_on && in.synced && !in.stalled && loss_hold_s_ <= 0;
        o.dtc = dtc_;
        return o;
    }
    uint32_t dtc() const { return dtc_; }
    void clear_dtc() { dtc_ = 0; loss_count_ = 0; }

private:
    const Calibration &c_;
    bool overspeed_ = false;
    uint32_t dtc_ = 0, last_losses_ = 0;
    int loss_count_ = 0;
    float loss_hold_s_ = 0, idl_bad_s_ = 0;
};

}  // namespace ecu
