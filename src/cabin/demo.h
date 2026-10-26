// Synthetic bus traffic for bench work without the truck (build env
// cabin-demo). One 90 s loop: idle, cruise with the piggyback armed, a
// hill pull that runs EGT past the red line for a few seconds, then
// back to cruise. Frames go through the same State::ingest as real ones.
#pragma once
#include <math.h>
#include <stdint.h>

#include "can_protocol.h"

namespace demo {

constexpr uint32_t LOOP_MS = 90000;

inline float ramp(float t, float t0, float t1) {
    return t <= t0 ? 0.0f : t >= t1 ? 1.0f : (t - t0) / (t1 - t0);
}

// Fills `out` with one of every frame; returns how many. Call at ~20 Hz.
inline int frames(uint32_t now_ms, can_proto::Frame* out) {
    using namespace can_proto;
    const float t = (now_ms % LOOP_MS) / 1000.0f;
    const float wob = sinf(now_ms / 700.0f);

    // load: 0 idle, ~0.35 cruise, 1 full pull
    const float pull = ramp(t, 40, 44) * (1 - ramp(t, 56, 59));
    const float load = 0.35f * ramp(t, 8, 14) + 0.65f * pull;
    const float rpm = 750 + 1500 * ramp(t, 8, 14) + 900 * pull + 25 * wob;
    const float boost_psi = 1.5f + 11.0f * (load - 0.35f > 0 ? (load - 0.35f) / 0.65f : 0) + 2.0f * load + 0.3f * wob;
    const float baro = 101.3f;
    const float pim = baro + boost_psi * 6.894757f;
    const bool armed = t > 18;
    const float extra = armed ? 0.06f : 0.0f;
    const float pim_seen = armed ? pim * (1 + extra) : pim;
    // EGT lags load; the pull overshoots 1250 °F for ~4 s around t = 52 s.
    const float egt_f = 420 + 560 * ramp(t, 9, 20) + 330 * ramp(t, 44, 52) * (1 - ramp(t, 55, 62)) + 8 * wob;
    // Coolant climbs slowly over the first 10 minutes (not per loop), so the
    // Temps trend has something to show.
    const float thw = 80 + 8 * ramp(now_ms / 1000.0f, 0, 600) + 2.5f * ramp(t, 44, 60) * (1 - ramp(t, 70, 90));
    const float tho = 70 + 18 * ramp(t, 10, 80);

    int n = 0;
    out[n++] = pack_i16(ID_EGT, to_i16(egt_f));
    out[n++] = pack_i16(ID_MAP_PSI, to_i16(boost_psi * 10));
    out[n++] = pack(MapAbs{to_u16(pim * 10), to_u16(baro * 10), 5010, U16_INVALID});
    out[n++] = pack(TapEngine{to_u16(rpm), to_i16((4 + 6 * load) * 10), to_u16((30 + 25 * load) * 10)});
    out[n++] = pack(TapFuel{to_u16((8 + 16 * load) * 10), to_u16(load * 100 * 10), to_u16(pim)});
    out[n++] = pack(TapTemps{to_i16(thw), to_i16(28 + 14 * load), 31, to_i16(tho)});
    out[n++] = pack(Piggyback{(uint8_t)armed, to_u16(pim_seen), to_u16(load * 1000), to_i16(thw),
                              (uint8_t)(extra * 100)},
                    false, 20);
    const uint32_t dig = (load > 0.5f ? (uint32_t)DIG_LOCKUP : 0u) | DIG_S1 | DIG_S2;
    out[n++] = pack(TapStatus{to_u16(13900 + 60 * wob), to_u16(load * 900 + (rpm > 900 ? 300 : 0)), dig});
    return n;
}

}  // namespace demo
