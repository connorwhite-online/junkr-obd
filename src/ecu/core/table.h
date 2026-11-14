// Calibration tables with clamped linear / bilinear interpolation.
// Header-only, no heap: the same code runs on the STM32H7 and in host tests.
#pragma once
#include <stddef.h>

namespace ecu {

inline float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

// Index + fraction of v in an ascending axis, clamped to the ends.
inline void axis_pos(const float *ax, size_t n, float v, size_t &i, float &f) {
    if (n < 2 || v <= ax[0]) { i = 0; f = 0; return; }
    if (v >= ax[n - 1]) { i = n - 2; f = 1; return; }
    i = 0;
    while (i + 2 < n && v >= ax[i + 1]) i++;
    f = (v - ax[i]) / (ax[i + 1] - ax[i]);
}

template <size_t N>
struct Table1D {
    float x[N];
    float y[N];
    float at(float v) const {
        size_t i; float f;
        axis_pos(x, N, v, i, f);
        return y[i] + (y[i + 1] - y[i]) * f;
    }
};

// z[row][col]: rows follow axis y, columns follow axis x.
template <size_t NX, size_t NY>
struct Table2D {
    float x[NX];
    float y[NY];
    float z[NY][NX];
    float at(float vx, float vy) const {
        size_t i, j; float fx, fy;
        axis_pos(x, NX, vx, i, fx);
        axis_pos(y, NY, vy, j, fy);
        float a = z[j][i] + (z[j][i + 1] - z[j][i]) * fx;
        float b = z[j + 1][i] + (z[j + 1][i + 1] - z[j + 1][i]) * fx;
        return a + (b - a) * fy;
    }
    // Inverse lookup along x for a fixed y (z must rise with x): the x that gives z.
    float inv_x(float vz, float vy) const {
        size_t j; float fy;
        axis_pos(y, NY, vy, j, fy);
        auto zz = [&](size_t i) { return z[j][i] + (z[j + 1][i] - z[j][i]) * fy; };
        if (vz <= zz(0)) return x[0];
        for (size_t i = 0; i + 1 < NX; i++) {
            float a = zz(i), b = zz(i + 1);
            if (vz <= b) return b > a ? x[i] + (x[i + 1] - x[i]) * (vz - a) / (b - a) : x[i];
        }
        return x[NX - 1];
    }
};

}  // namespace ecu
