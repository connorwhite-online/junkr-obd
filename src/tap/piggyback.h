// =====================================================================
//  Piggyback core for the ECU tap board (ESP32-S3).
//
//  The same math as tuner/sim.py Piggyback.apply and tuner/tables.py
//  lookup, so a table learned on the laptop behaves identically on the
//  truck. Header-only, no Arduino dependencies: it is unit-tested on the
//  host (tuner/tests/test_firmware.py compiles it with g++).
//
//  Tables come from `python -m tuner export --out src/tap/tables_gen.h`.
//
//  Safety ordering inside apply(), every sample:
//    1. table value, clamped to the actuator's hard limits
//    2. optional learning dither
//    3. PIM: authority cap (fitted sensors), learned lambda ratio cap,
//       then the boost-cut margin
//    4. THW: zero below 75 C (warm-up behaviour is never touched)
//  The bypass relay (normally closed) is outside this code: any fault
//  drops it and the ECU sees the real sensors again.
// =====================================================================
#pragma once

#include <math.h>
#include <stdint.h>

namespace pb {

constexpr int NB = 3, NI = 3, NR = 10, NV = 8;
constexpr int NCELLS = NB * NI * NR * NV;

struct Axes {
    float baro[NB];
    float iat[NI];
    float rpm[NR];
    float va[NV];
};

// Grid flattened in C order over (baro, iat, rpm, va), like numpy ravel().
struct Tables {
    float pim_off[NCELLS];
    float thw_off[NCELLS];
    float pim_rcap[NCELLS];
};

struct Limits {
    float pim_lo = -30.0f, pim_hi = 25.0f, pim_probe = 2.0f, pim_period = 2.0f;
    float thw_lo = -15.0f, thw_hi = 0.0f,  thw_probe = 3.0f, thw_period = 24.0f;
    float boost_cut_kpa = 204.0f;   // ECU fuel cut, absolute
    float margin_kpa = 6.0f;        // stay this far under it
    float thw_min_c = 75.0f;        // below this, THW passes through untouched
    float max_fuel_gain = 0.0f;     // 0.20 with wideband + EGT, 0.08 with one, 0 with neither
};

struct Output {
    float pim_seen, thw_seen;       // what the ECU gets
    float pim_off, thw_off;         // applied offsets (after all caps)
    float pim_tbl, thw_tbl;         // table values (same caps, no dither)
    int8_t s_pim, s_thw;            // dither sign this sample
};

inline float clampf(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }

// index and fraction of x on a monotonic axis (clamped at the ends)
inline void locate(const float* ax, int n, float x, int& i, float& f) {
    x = clampf(x, ax[0], ax[n - 1]);
    i = 0;
    while (i < n - 2 && x >= ax[i + 1]) ++i;
    f = (x - ax[i]) / (ax[i + 1] - ax[i]);
}

// 4-D multilinear interpolation: 16 corners.
inline float lookup(const Axes& A, const float* grid, float baro, float iat, float rpm, float va) {
    int ib, ii, ir, iv;
    float fb, fi, fr, fv;
    locate(A.baro, NB, baro, ib, fb);
    locate(A.iat, NI, iat, ii, fi);
    locate(A.rpm, NR, rpm, ir, fr);
    locate(A.va, NV, va, iv, fv);
    float out = 0.0f;
    for (int c = 0; c < 16; ++c) {
        const int b = c & 1, i = (c >> 1) & 1, r = (c >> 2) & 1, v = (c >> 3) & 1;
        const float w = (b ? fb : 1 - fb) * (i ? fi : 1 - fi) * (r ? fr : 1 - fr) * (v ? fv : 1 - fv);
        if (w == 0.0f) continue;
        const int idx = (((ib + b) * NI + (ii + i)) * NR + (ir + r)) * NV + (iv + v);
        out += grid[idx] * w;
    }
    return out;
}

inline int8_t dither_sign(float t, float period) {
    return fmodf(t, period) < period / 2 ? 1 : -1;
}

// t: seconds since start; pim, baro_est in kPa abs; tha, thw in C; va pedal %.
inline Output apply(const Axes& A, const Tables& T, const Limits& L, bool learning,
                    float t, float rpm, float va, float pim, float tha, float thw, float baro_est) {
    Output o;
    o.s_pim = dither_sign(t, L.pim_period);
    o.s_thw = dither_sign(t, L.thw_period);

    float pim_tbl = clampf(lookup(A, T.pim_off, baro_est, tha, rpm, va), L.pim_lo, L.pim_hi);
    float thw_tbl = clampf(lookup(A, T.thw_off, baro_est, tha, rpm, va), L.thw_lo, L.thw_hi);
    float pim_u = pim_tbl, thw_u = thw_tbl;
    if (learning) {
        pim_u = clampf(pim_u + o.s_pim * L.pim_probe, L.pim_lo, L.pim_hi);
        thw_u = clampf(thw_u + o.s_thw * L.thw_probe, L.thw_lo, L.thw_hi);
    }

    const float rcap = L.max_fuel_gain > 0 ? lookup(A, T.pim_rcap, baro_est, tha, rpm, va) : 1.0f;
    auto cap_pim = [&](float off) {
        off = fminf(off, L.max_fuel_gain * pim);
        if (off > 0) off = fminf(off, pim * (fmaxf(rcap, 1.0f) - 1.0f));
        return fminf(pim + off, L.boost_cut_kpa - L.margin_kpa) - pim;
    };
    o.pim_off = cap_pim(pim_u);
    o.pim_tbl = cap_pim(pim_tbl);
    o.pim_seen = pim + o.pim_off;

    if (thw < L.thw_min_c) { thw_u = 0; thw_tbl = 0; }
    o.thw_off = thw_u;
    o.thw_tbl = thw_tbl;
    o.thw_seen = thw + thw_u;
    return o;
}

}  // namespace pb
