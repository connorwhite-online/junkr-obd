// Pure math for the engine-bay node: the 3-bar MAP transfer curve and the
// barometric-pressure latch. No Arduino dependencies, so the host tests
// (test/test_engine_bay) run it as-is.
#pragma once
#include <math.h>

namespace bay {

// GM 3-bar MAP (12569240 / 16254609), ratiometric to its 5 V supply.
// End points as used by MegaSquirt/rusEFI for the GM 3-bar: 1.1 kPa at
// 0 V and 315.5 kPa at Vsupply. [VERIFY on the bench with a hand pump,
// BENCH_TESTING.md stage 6; adjust these two numbers only.]
constexpr float GM3BAR_KPA_AT_0V = 1.1f;
constexpr float GM3BAR_KPA_AT_VS = 315.5f;

// The supply monitor (ADS A3) can be absent on an older board. Outside
// this window, assume the nominal rail instead of trusting the reading.
constexpr float V5_NOMINAL = 5.0f;
constexpr float V5_MIN_OK = 4.5f;
constexpr float V5_MAX_OK = 5.5f;

constexpr float KPA_PER_PSI = 6.894757f;
constexpr float STD_BARO_KPA = 101.3f;

inline float supply_or_nominal(float v5) {
    return (v5 >= V5_MIN_OK && v5 <= V5_MAX_OK) ? v5 : V5_NOMINAL;
}

// v_sig and v5 are at the sensor connector (after undoing the 2:1 dividers).
// NaN when the signal is outside the sensor's 0.1..4.9 V-ish useful band,
// which is what a disconnected or shorted sensor looks like.
inline float gm3bar_kpa(float v_sig, float v5) {
    const float vs = supply_or_nominal(v5);
    const float r = v_sig / vs;
    if (r < 0.02f || r > 0.98f) return NAN;
    return GM3BAR_KPA_AT_0V + (GM3BAR_KPA_AT_VS - GM3BAR_KPA_AT_0V) * r;
}

inline float gauge_psi(float map_kpa, float baro_kpa) {
    return (map_kpa - baro_kpa) / KPA_PER_PSI;
}

// Barometric pressure from the same absolute sensor, read while the engine
// is stopped: averaged over the first moments after key-on, then refreshed
// whenever the tap reports rpm == 0. A diesel has no throttle, so this is
// the one time the manifold is reliably at ambient.
class BaroTracker {
  public:
    static constexpr float MIN_KPA = 60.0f;    // ~4,000 m
    static constexpr float MAX_KPA = 108.0f;   // deep-low sea level + margin
    static constexpr float TAU_S = 5.0f;       // smoothing while stopped

    void boot_sample(float kpa) {
        if (isnan(kpa)) return;
        sum_ += kpa;
        ++n_;
    }

    // Call once at the end of the key-on window.
    void finish_boot() {
        if (n_ == 0) return;
        const float avg = sum_ / n_;
        if (avg >= MIN_KPA && avg <= MAX_KPA) {
            baro_ = avg;
            latched_ = true;
        }
    }

    // engine_stopped: true only when the tap has said rpm < 50 recently.
    void update(float kpa, bool engine_stopped, float dt_s) {
        if (!engine_stopped || isnan(kpa) || kpa < MIN_KPA || kpa > MAX_KPA) return;
        if (!latched_) {
            baro_ = kpa;
            latched_ = true;
            return;
        }
        const float a = dt_s / (TAU_S + dt_s);
        baro_ += a * (kpa - baro_);
    }

    float baro() const { return latched_ ? baro_ : STD_BARO_KPA; }
    bool latched() const { return latched_; }

  private:
    float sum_ = 0;
    int n_ = 0;
    float baro_ = STD_BARO_KPA;
    bool latched_ = false;
};

}  // namespace bay
