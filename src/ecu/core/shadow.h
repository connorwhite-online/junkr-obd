// Shadow mode: the stock ECU drives the engine, and this board watches.
//
// The stock SPV and TCV outputs reach us through the tails (S_SPV, S_TCV,
// via the LM2903 edge comparators). For every segment we record where the
// stock ECU opened the SPV (NE angle), what TCV duty it ran and the advance
// we measured, binned by rpm and pedal. That gives two things:
//  1. a stock-equivalent base map, learned by observation, so takeover
//     starts from "exactly what Toyota did" instead of a guess;
//  2. a live comparison of our own plan against the stock one, which has to
//     agree for a long run before takeover is unlocked.
#pragma once
#include <math.h>
#include <stdint.h>
#include "calib.h"

namespace ecu {

class ShadowLearner {
public:
    static constexpr int NX = 6, NY = 6;   // same axes as Calibration::q_driver

    explicit ShadowLearner(const Calibration &c) : c_(c) {}

    // One stock injection observed: SPV opened at `stock_open_deg`.
    void observe(float rpm, float pedal, float stock_open_deg, float stock_tcv_duty, float measured_adv,
                 float our_open_deg) {
        int i = nearest(c_.q_driver.x, NX, pedal), j = nearest(c_.q_driver.y, NY, rpm);
        Cell &k = cell_[j][i];
        k.n++;
        float w = k.n < 50 ? 1.0f / k.n : 0.02f;   // mean, then slow EWMA
        k.open_deg += w * (stock_open_deg - k.open_deg);
        k.tcv += w * (stock_tcv_duty - k.tcv);
        k.adv += w * (measured_adv - k.adv);
        float d = our_open_deg - stock_open_deg;
        k.err += w * (d - k.err);
        n_total_++;
        if (fabsf(d) < tol_deg_) agree_run_++; else agree_run_ = 0;
    }

    // Takeover is offered only after this many consecutive agreeing segments.
    bool agrees(uint32_t needed = 20000) const { return agree_run_ >= needed; }
    uint32_t agree_run() const { return agree_run_; }
    uint32_t samples(int j, int i) const { return cell_[j][i].n; }
    float stock_open_deg(int j, int i) const { return cell_[j][i].open_deg; }
    float stock_adv(int j, int i) const { return cell_[j][i].adv; }

    // Write the learned map into a calibration: Q for each (pedal, rpm) cell
    // is what our spill model says the stock spill angle delivers. Cells with
    // too few samples keep their old value. Returns the cells written.
    int export_to(Calibration &out, uint32_t min_samples = 30) const {
        int written = 0;
        for (int j = 0; j < NY; j++)
            for (int i = 0; i < NX; i++) {
                const Cell &k = cell_[j][i];
                if (k.n < min_samples) continue;
                out.q_driver.z[j][i] = c_.spill.at(k.open_deg, c_.q_driver.y[j]);
                written++;
            }
        return written;
    }

private:
    struct Cell { uint32_t n = 0; float open_deg = 0, tcv = 0, adv = 0, err = 0; };
    static int nearest(const float *ax, int n, float v) {
        int best = 0;
        for (int i = 1; i < n; i++)
            if (fabsf(ax[i] - v) < fabsf(ax[best] - v)) best = i;
        return best;
    }
    const Calibration &c_;
    Cell cell_[NY][NX];
    uint32_t n_total_ = 0, agree_run_ = 0;
    float tol_deg_ = 1.5f;
};

}  // namespace ecu
