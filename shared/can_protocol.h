// =====================================================================
//  CAN bus contract shared by every node (engine-bay, tap, gauge).
//
//  500 kbit/s, 11-bit IDs, all multi-byte fields big-endian.
//  Invalid / not fitted: INT16_MIN for signed fields, 0xFFFF for
//  unsigned ones. New frames are in kPa / °C; the gauge converts.
//  The table lives in docs/SENSORS_GAUGE_LOGGING.md §4.
//
//  Header-only and Arduino-free so the host tests (pio test -e native)
//  check the exact packing the firmware uses.
// =====================================================================
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace can_proto {

// ---- engine-bay node (ESP32-C3) ---------------------------------------
constexpr uint32_t ID_EGT     = 0x100;  // 20 Hz  EGT °F (i16)
constexpr uint32_t ID_COOLANT = 0x101;  // retired: coolant is THW from the tap (0x112)
constexpr uint32_t ID_MAP_PSI = 0x102;  // 20 Hz  MAP gauge psi ×10 (i16)
constexpr uint32_t ID_MAP_ABS = 0x103;  //  2 Hz  MapAbs below

// ---- ECU tap box (ESP32-S3) -------------------------------------------
constexpr uint32_t ID_TAP_ENGINE = 0x110;  // 50 Hz  TapEngine
constexpr uint32_t ID_TAP_FUEL   = 0x111;  // 50 Hz  TapFuel
constexpr uint32_t ID_TAP_TEMPS  = 0x112;  //  5 Hz  TapTemps
constexpr uint32_t ID_TAP_PB     = 0x113;  // 10 Hz  Piggyback
constexpr uint32_t ID_TAP_STATUS = 0x114;  //  5 Hz  TapStatus
constexpr uint32_t ID_TAP_ALERT  = 0x115;  // on change  Alert

// ---- optional ---------------------------------------------------------
constexpr uint32_t ID_WIDEBAND = 0x190;    // rusEFI-style wideband, 50 Hz

constexpr uint32_t CAN_BITRATE_BPS = 500000;
constexpr uint32_t TX_PERIOD_MS = 50;      // engine-bay fast frames, 20 Hz
constexpr uint32_t SLOW_PERIOD_MS = 500;   // engine-bay 0x103, 2 Hz

constexpr int16_t  SENTINEL_INVALID = INT16_MIN;
constexpr uint16_t U16_INVALID = 0xFFFF;

// ---- byte helpers -----------------------------------------------------
inline void put_u16(uint8_t* p, uint16_t v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
inline void put_i16(uint8_t* p, int16_t v) { put_u16(p, (uint16_t)v); }
inline void put_u32(uint8_t* p, uint32_t v) { put_u16(p, (uint16_t)(v >> 16)); put_u16(p + 2, (uint16_t)v); }
inline uint16_t get_u16(const uint8_t* p) { return (uint16_t)((p[0] << 8) | p[1]); }
inline int16_t get_i16(const uint8_t* p) { return (int16_t)get_u16(p); }
inline uint32_t get_u32(const uint8_t* p) { return ((uint32_t)get_u16(p) << 16) | get_u16(p + 2); }

// Clamp-and-round into the wire types, mapping NaN to the sentinel.
inline int16_t to_i16(float x) {
    if (x != x) return SENTINEL_INVALID;
    if (x > 32767.0f) return 32767;
    if (x < -32767.0f) return -32767;
    return (int16_t)(x < 0 ? x - 0.5f : x + 0.5f);
}
inline uint16_t to_u16(float x) {
    if (x != x) return U16_INVALID;
    if (x > 65534.0f) return 65534;
    if (x < 0.0f) return 0;
    return (uint16_t)(x + 0.5f);
}

struct Frame {
    uint32_t id;
    uint8_t len;
    uint8_t data[8];
};

// ---- payloads ---------------------------------------------------------
// 0x100 / 0x102: one i16 (legacy two-byte frames, unchanged).
inline Frame pack_i16(uint32_t id, int16_t v) {
    Frame f{id, 2, {}};
    put_i16(f.data, v);
    return f;
}

// 0x103
struct MapAbs {
    uint16_t map_kpa_x10;   // absolute manifold pressure
    uint16_t baro_kpa_x10;  // latched with the engine stopped
    uint16_t v5_mv;         // sensor supply, for ratiometric MAP
    uint16_t aux_mv;        // ADS1115 A2 at the connector (oil pressure later)
};
inline Frame pack(const MapAbs& m) {
    Frame f{ID_MAP_ABS, 8, {}};
    put_u16(f.data, m.map_kpa_x10);
    put_u16(f.data + 2, m.baro_kpa_x10);
    put_u16(f.data + 4, m.v5_mv);
    put_u16(f.data + 6, m.aux_mv);
    return f;
}
inline MapAbs unpack_map_abs(const uint8_t* d) {
    return {get_u16(d), get_u16(d + 2), get_u16(d + 4), get_u16(d + 6)};
}

// 0x110
struct TapEngine {
    uint16_t rpm;
    int16_t timing_btdc_x10;  // actual injection timing
    uint16_t tcv_duty_x10;    // %
};
inline Frame pack(const TapEngine& m) {
    Frame f{ID_TAP_ENGINE, 6, {}};
    put_u16(f.data, m.rpm);
    put_i16(f.data + 2, m.timing_btdc_x10);
    put_u16(f.data + 4, m.tcv_duty_x10);
    return f;
}
inline TapEngine unpack_tap_engine(const uint8_t* d) {
    return {get_u16(d), get_i16(d + 2), get_u16(d + 4)};
}

// 0x111
struct TapFuel {
    uint16_t spv_deg_x10;   // spill-valve close→open, pump degrees
    uint16_t va_pct_x10;    // pedal, harness side (real)
    uint16_t pim_kpa;       // stock boost sensor, harness side, kPa ABSOLUTE
};
inline Frame pack(const TapFuel& m) {
    Frame f{ID_TAP_FUEL, 6, {}};
    put_u16(f.data, m.spv_deg_x10);
    put_u16(f.data + 2, m.va_pct_x10);
    put_u16(f.data + 4, m.pim_kpa);
    return f;
}
inline TapFuel unpack_tap_fuel(const uint8_t* d) {
    return {get_u16(d), get_u16(d + 2), get_u16(d + 4)};
}

// 0x112, all °C, harness side (real)
struct TapTemps {
    int16_t thw, tha, thf, tho;
};
inline Frame pack(const TapTemps& m) {
    Frame f{ID_TAP_TEMPS, 8, {}};
    put_i16(f.data, m.thw);
    put_i16(f.data + 2, m.tha);
    put_i16(f.data + 4, m.thf);
    put_i16(f.data + 6, m.tho);
    return f;
}
inline TapTemps unpack_tap_temps(const uint8_t* d) {
    return {get_i16(d), get_i16(d + 2), get_i16(d + 4), get_i16(d + 6)};
}

// 0x113: what the ECU is being told. armed = bypass relays energised.
struct Piggyback {
    uint8_t armed;
    uint16_t pim_seen_kpa;     // absolute
    uint16_t va_seen_pct_x10;
    int16_t thw_seen;          // °C
    uint8_t extra_fuel_pct;    // extra fuel in use now, %
};
// Byte 7 is the extra fuel in use (0..100 %). The authority the fitted
// sensors allow (0 / 8 / 20 %) rides in the top bits of byte 0:
//   byte0 bit0 = armed, bit1 = learning, bits 2..7 = authority %.
inline Frame pack(const Piggyback& m, bool learning, uint8_t authority_pct) {
    Frame f{ID_TAP_PB, 8, {}};
    f.data[0] = (uint8_t)((m.armed ? 1 : 0) | (learning ? 2 : 0) | ((authority_pct & 0x3F) << 2));
    put_u16(f.data + 1, m.pim_seen_kpa);
    put_u16(f.data + 3, m.va_seen_pct_x10);
    put_i16(f.data + 5, m.thw_seen);
    f.data[7] = m.extra_fuel_pct;
    return f;
}
struct PiggybackRx {
    bool armed, learning;
    uint8_t authority_pct;
    uint16_t pim_seen_kpa, va_seen_pct_x10;
    int16_t thw_seen;
    uint8_t extra_fuel_pct;
};
inline PiggybackRx unpack_piggyback(const uint8_t* d) {
    return {(d[0] & 1) != 0, (d[0] & 2) != 0, (uint8_t)(d[0] >> 2),
            get_u16(d + 1), get_u16(d + 3), get_i16(d + 5), d[7]};
}

// 0x114
enum DigBit : uint32_t {
    DIG_W_LAMP   = 1u << 0,   // check engine
    DIG_GLOW_IND = 1u << 1,
    DIG_GLOW_REL = 1u << 2,
    DIG_STA      = 1u << 3,   // starter
    DIG_LOCKUP   = 1u << 4,   // torque-converter lock-up solenoid
    DIG_S1       = 1u << 5,
    DIG_S2       = 1u << 6,
    DIG_AC       = 1u << 7,
    DIG_AC_CUT   = 1u << 8,
    DIG_STOP     = 1u << 9,
    DIG_IDL      = 1u << 10,
    DIG_NSW      = 1u << 11,  // park/neutral
    DIG_SW_2     = 1u << 12,
    DIG_SW_L     = 1u << 13,
    DIG_OD2      = 1u << 14,
};
struct TapStatus {
    uint16_t vbat_mv;
    uint16_t speed_kmh_x10;
    uint32_t dig;
};
inline Frame pack(const TapStatus& m) {
    Frame f{ID_TAP_STATUS, 8, {}};
    put_u16(f.data, m.vbat_mv);
    put_u16(f.data + 2, m.speed_kmh_x10);
    put_u32(f.data + 4, m.dig);
    return f;
}
inline TapStatus unpack_tap_status(const uint8_t* d) {
    return {get_u16(d), get_u16(d + 2), get_u32(d + 4)};
}

// 0x115: the tap's own alert decisions (the gauge also decides on its own).
struct AlertMsg {
    uint8_t code;     // gauge::Alert numbering
    int16_t value;    // in the alert's bus unit (°C, kPa, mV)
    int16_t limit;
};
inline Frame pack(const AlertMsg& m) {
    Frame f{ID_TAP_ALERT, 5, {}};
    f.data[0] = m.code;
    put_i16(f.data + 1, m.value);
    put_i16(f.data + 3, m.limit);
    return f;
}
inline AlertMsg unpack_alert(const uint8_t* d) { return {d[0], get_i16(d + 1), get_i16(d + 3)}; }

// Minimum payload length per ID; shorter frames are dropped by receivers.
inline uint8_t min_len(uint32_t id) {
    switch (id) {
        case ID_EGT: case ID_MAP_PSI: return 2;
        case ID_MAP_ABS: case ID_TAP_TEMPS: case ID_TAP_PB: case ID_TAP_STATUS: return 8;
        case ID_TAP_ENGINE: case ID_TAP_FUEL: return 6;
        case ID_TAP_ALERT: return 5;
        default: return 0;
    }
}

}  // namespace can_proto
