// Exhaust sensors on the AUX plug: EGT protection and wideband live tuning.
//
// EGT (first): a K-type probe before the turbo through a 0-5 V amplifier.
//   Above egt_derate_start_c the fuel limit comes down linearly, so the
//   driver can still drive but the head and turbo can't be cooked. A fitted
//   probe that fails holds the limit at egt_fault_scale until it is fixed.
//
// Wideband (second): a controller's 0-5 V lambda output. Diesel smoke is
//   the limit worth tuning, so the wideband trims the smoke map (q_smoke)
//   toward a target lambda at the fuel limit, cell by cell, while you drive:
//     - too rich at the limit: the cell's trim comes down (quickly)
//     - lean with margin: it goes back up, slowly, never past trim_max, and
//       only while the EGT is healthy and well below its derate point
//     - richer than lambda_floor anywhere: fuel is pulled at once
//   It only learns while this ECU is actually fueling (takeover). In shadow
//   mode the stock ECU is fueling, so the readings are just logged.
//
// Nothing here can raise fuel above q_max: the trims only scale the smoke
// limit, and the final limit is still min(q_max, smoke) times the EGT scale.
#pragma once
#include <stdint.h>
#include "calib.h"

namespace ecu {

struct ExhaustInputs {
    float aux_v[2];      // AUX_AN1, AUX_AN2 at the harness, volts
    float rpm, map_kpa;
    bool running;        // engine running (not cranking, not stalled)
    float run_s;         // seconds since it started
    bool fueling;        // this ECU's fuel is reaching the engine (takeover)
    bool at_limit;       // last loop's driver Q was held by the smoke limit
    float dt;
};

class Exhaust {
public:
    explicit Exhaust(const Calibration &c) : c_(c) {
        for (size_t i = 0; i < 5; i++) trim_.x[i] = c.q_smoke.x[i];
        for (size_t j = 0; j < 3; j++) trim_.y[j] = c.q_smoke.y[j];
        reset_trims();
    }

    void update(const ExhaustInputs &in) {
        dtc_egt_ = dtc_wb_ = false;
        egt_fitted_ = wb_fitted_ = false;
        int ie = find(AuxUse::Egt), iw = find(AuxUse::Wideband);

        // ---- EGT
        egt_scale_ = 1;
        egt_ok_ = false;
        if (ie >= 0) {
            egt_fitted_ = true;
            egt_v_ = in.aux_v[ie];
            egt_c_ = lin(egt_v_, c_.egt_v0, c_.egt_c0, c_.egt_v1, c_.egt_c1);
            // Pegged output (open probe on most amps, or the 100k pull-down
            // in the pigtail when unplugged), or stone cold after a minute
            // of running.
            bool bad = egt_v_ < 0.05f || egt_v_ > 4.95f || (in.running && in.run_s > 60 && egt_c_ < 60);
            if (bad) egt_bad_s_ += in.dt; else egt_bad_s_ = 0;
            if (egt_bad_s_ > 0.5f) {
                dtc_egt_ = true;
                egt_scale_ = c_.egt_fault_scale;
            } else {
                egt_ok_ = true;
                float span = c_.egt_derate_end_c - c_.egt_derate_start_c;
                float f = clampf((egt_c_ - c_.egt_derate_start_c) / span, 0, 1);
                egt_scale_ = 1 - f * (1 - c_.egt_min_scale);
            }
        }

        // ---- Wideband
        wb_valid_ = false;
        learning_ = false;
        if (iw >= 0) {
            wb_fitted_ = true;
            wb_v_ = in.aux_v[iw];
            lambda_ = lin(wb_v_, c_.wb_v0, c_.wb_l0, c_.wb_v1, c_.wb_l1);
            bool range = wb_v_ > 0.05f && wb_v_ < 4.95f;
            if (!range && in.running && in.run_s > c_.wb_warmup_s) wb_bad_s_ += in.dt; else wb_bad_s_ = 0;
            dtc_wb_ = wb_bad_s_ > 2.0f;
            wb_valid_ = range && in.running && in.run_s > c_.wb_warmup_s;
        }
        // Floor: richer than lambda_floor anywhere pulls fuel now.
        if (wb_valid_ && in.fueling && lambda_ < c_.lambda_floor)
            rich_cut_ = clampf(rich_cut_ + c_.rich_cut_rate * (c_.lambda_floor - lambda_) * 10 * in.dt, 0,
                               c_.rich_cut_max);
        else
            rich_cut_ = clampf(rich_cut_ - 0.2f * in.dt, 0, c_.rich_cut_max);
        // Smoke-limit trim, only at the limit and only on our own fuel.
        if (wb_valid_ && in.fueling && in.at_limit) {
            float err = lambda_ - c_.lambda_target.at(in.rpm);   // + lean, - rich
            float rate = 0;
            if (err < 0) rate = c_.trim_rate_rich * err;
            else if (egt_ok_ && egt_c_ < c_.egt_derate_start_c - 50) rate = c_.trim_rate_lean * err;
            if (rate != 0) {
                learning_ = true;
                float d = rate * in.dt;
                trim_.each_weight(in.map_kpa, in.rpm, [&](size_t j, size_t i, float w) {
                    trim_.z[j][i] = clampf(trim_.z[j][i] + d * w, c_.trim_min, c_.trim_max);
                });
            }
        }
    }

    // Multiplier for the smoke limit at this point (1 + learned trim).
    float smoke_scale(float map_kpa, float rpm) const { return 1 + trim_.at(map_kpa, rpm); }
    // Multiplier for the final fuel limit: EGT derate and the rich floor.
    float limit_scale() const { return egt_scale_ * (1 - rich_cut_); }

    void reset_trims() {
        for (auto &row : trim_.z)
            for (float &v : row) v = 0;
    }
    const Table2D<5, 3> &trims() const { return trim_; }
    Table2D<5, 3> &trims() { return trim_; }   // the app can load saved trims here

    bool egt_fitted() const { return egt_fitted_; }
    bool wb_fitted() const { return wb_fitted_; }
    float egt_c() const { return egt_fitted_ ? egt_c_ : 0; }
    float egt_v() const { return egt_v_; }
    float lambda() const { return wb_fitted_ ? lambda_ : 0; }
    float wb_v() const { return wb_v_; }
    float egt_scale() const { return egt_scale_; }
    float rich_cut() const { return rich_cut_; }
    bool egt_limiting() const { return egt_scale_ < 0.999f; }
    bool wb_valid() const { return wb_valid_; }
    bool learning() const { return learning_; }
    bool dtc_egt() const { return dtc_egt_; }
    bool dtc_wb() const { return dtc_wb_; }

private:
    int find(AuxUse u) const {
        for (int i = 0; i < 2; i++)
            if (c_.aux_use[i] == u) return i;
        return -1;
    }
    static float lin(float v, float v0, float y0, float v1, float y1) {
        return v1 != v0 ? y0 + (v - v0) * (y1 - y0) / (v1 - v0) : y0;
    }
    const Calibration &c_;
    Table2D<5, 3> trim_{};
    float egt_v_ = 0, egt_c_ = 0, egt_scale_ = 1, egt_bad_s_ = 0;
    float wb_v_ = 0, lambda_ = 0, wb_bad_s_ = 0, rich_cut_ = 0;
    bool egt_fitted_ = false, wb_fitted_ = false, egt_ok_ = false, wb_valid_ = false, learning_ = false;
    bool dtc_egt_ = false, dtc_wb_ = false;
};

}  // namespace ecu
