// =====================================================================
//  JUNKR ECU log format ("jlog"), version 1.
//
//  One byte stream, used unchanged on three hops:
//    STM32 --UART--> ESP32 --microSD file--> tools/junkrlog.py
//
//  Frame:  A5 5A | type u8 | len u16 | payload[len] | crc u16
//          little-endian; crc is CRC-16/CCITT-FALSE over type, len and
//          payload. A reader that loses sync scans for A5 5A and keeps
//          going, so a torn write or a dropped UART byte costs one frame.
//
//  The stream describes itself: a SCHEMA frame lists every sample
//  channel (name, unit, wire type, scale) and is repeated every second
//  and at the start of every file. Adding a channel never breaks an old
//  decoder, and old logs stay readable forever.
//
//  Spec and channel table: docs/standalone-ecu/LOGGING.md.
//  Header-only and Arduino-free so the host tests run the same packing.
// =====================================================================
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace jlog {

constexpr uint8_t SYNC0 = 0xA5, SYNC1 = 0x5A;
constexpr uint8_t FORMAT_VERSION = 1;
constexpr size_t HEADER = 5, TRAILER = 2, MAX_PAYLOAD = 1024;

enum Type : uint8_t {
    T_META = 1,     // text "key=value\n" lines: firmware, calibration hash, board
    T_SCHEMA = 2,   // sample channel list
    T_SAMPLE = 3,   // u32 t_us, u32 seq, then one value per schema channel
    T_INJ = 4,      // one per injection segment (180 crank deg)
    T_EVENT = 5,    // mode change, DTC change, limiter, drop counts...
    T_FILE = 6,     // written by the ESP32 at the top of each file
    T_CMD = 0x10,   // ESP32 -> STM32 (reserved; today's commands are single bytes)
};

// ---- CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) -------------------
inline uint16_t crc16(const uint8_t *p, size_t n, uint16_t crc = 0xFFFF) {
    while (n--) {
        crc ^= (uint16_t)(*p++) << 8;
        for (int i = 0; i < 8; i++) crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

// CRC-32 (IEEE), used for the calibration hash in META.
inline uint32_t crc32(const void *data, size_t n, uint32_t crc = 0) {
    const uint8_t *p = (const uint8_t *)data;
    crc = ~crc;
    while (n--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++) crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
    }
    return ~crc;
}

// ---- little-endian helpers ------------------------------------------
inline void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
inline void put32(uint8_t *p, uint32_t v) { put16(p, (uint16_t)v); put16(p + 2, (uint16_t)(v >> 16)); }
inline uint16_t get16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline uint32_t get32(const uint8_t *p) { return (uint32_t)get16(p) | ((uint32_t)get16(p + 2) << 16); }
inline void putf(uint8_t *p, float f) { uint32_t u; memcpy(&u, &f, 4); put32(p, u); }
inline float getf(const uint8_t *p) { uint32_t u = get32(p); float f; memcpy(&f, &u, 4); return f; }

// Wrap a payload into a frame. Returns the frame length, 0 if it won't fit.
inline size_t frame(uint8_t *out, size_t cap, uint8_t type, const uint8_t *payload, size_t len) {
    if (len > MAX_PAYLOAD || cap < len + HEADER + TRAILER) return 0;
    out[0] = SYNC0; out[1] = SYNC1; out[2] = type;
    put16(out + 3, (uint16_t)len);
    if (len) memmove(out + HEADER, payload, len);
    put16(out + HEADER + len, crc16(out + 2, len + 3));
    return len + HEADER + TRAILER;
}

// ---- sample channels --------------------------------------------------
// Wire types: 'h' i16, 'H' u16, 'i' i32, 'I' u32. value_on_wire = round(value * scale).
struct Channel { const char *name, *unit; char type; float scale; };

inline size_t type_size(char t) { return (t == 'i' || t == 'I') ? 4 : 2; }

// Bits of the "flags" channel.
enum Flag : uint32_t {
    F_SYNCED = 1u << 0, F_RPM_VALID = 1u << 1, F_INJECT = 1u << 2, F_TAKEOVER = 1u << 3,
    F_ARM_REQ = 1u << 4, F_ARM_SW = 1u << 5, F_SUPERVISOR_OK = 1u << 6, F_ARM = 1u << 7,
    F_IDL = 1u << 8, F_BRAKE = 1u << 9, F_STARTER = 1u << 10, F_OD_SW = 1u << 11,
    F_S1 = 1u << 12, F_S2 = 1u << 13, F_SL = 1u << 14, F_SVR = 1u << 15,
    F_TIMING_OPEN_LOOP = 1u << 16, F_TDC_VALID = 1u << 17, F_STANDALONE = 1u << 18,
    F_TIMING_FAULT = 1u << 19, F_EGT_LIMIT = 1u << 20, F_WB_VALID = 1u << 21, F_WB_LEARN = 1u << 22,
};

// Event codes (T_EVENT payload: u32 t_us, u8 code, u32 a, u32 b).
enum Event : uint8_t {
    E_BOOT = 1,          // a = reset cause bits
    E_MODE = 2,          // a = new mode (0 shadow, 1 takeover), b = old
    E_DTC = 3,           // a = new DTC mask, b = old
    E_TAKEOVER_DENIED = 4,  // a = agree_run at the time
    E_SYNC_LOST = 5,     // a = total sync losses
    E_DROPPED = 6,       // a = frames dropped since last report (TX buffer full), b = which type
    E_ARM = 7,           // a = ARM readback, b = SAFE_OK
    E_LINK = 8,          // written by the ESP32: a = 1 link up, 0 link lost
    E_OVERLOAD = 9,      // a = control loop time in us that overran 1 ms
};

// The channel table the STM32 sends. Order is the sample order. Append
// only: the decoder reads the SCHEMA frame, so reordering is safe too,
// but appending keeps diffs readable.
enum Ch {
    C_RPM, C_RPM_INST, C_Q, C_SPV_CLOSE, C_SPV_OPEN, C_ADV, C_ADV_TARGET, C_ADV_ERR, C_TCV,
    C_PEDAL, C_VA_V, C_MAP, C_MAP_V, C_ECT, C_ECT_V, C_VBATT, C_SPEED, C_RANGE, C_GEAR, C_MODE,
    C_FLAGS, C_DTC, C_SYNC_LOSSES, C_AGREE_RUN, C_STOCK_TCV, C_MUX0, C_MUX1, C_MUX2, C_LOOP_US,
    C_SEGMENTS, C_EGT, C_EGT_V, C_LAMBDA, C_WB_V, C_EGT_SCALE, C_RICH_CUT, C_SMOKE_TRIM, C_Q_LIMIT,
    C_COUNT
};

constexpr Channel CHANNELS[C_COUNT] = {
    {"rpm", "rpm", 'h', 1},
    {"rpm_inst", "rpm", 'h', 1},
    {"q", "mm3/st", 'H', 100},
    {"spv_close", "deg", 'h', 100},
    {"spv_open", "deg", 'h', 100},
    {"adv", "degBTDC", 'h', 100},
    {"adv_target", "degBTDC", 'h', 100},
    {"adv_err", "deg", 'h', 100},
    {"tcv_duty", "%", 'H', 100},
    {"pedal", "%", 'H', 100},
    {"va_v", "V", 'H', 1000},
    {"map", "kPa", 'H', 10},
    {"map_v", "V", 'H', 1000},
    {"ect", "C", 'h', 10},
    {"ect_v", "V", 'H', 1000},
    {"vbatt", "V", 'H', 1000},
    {"speed", "km/h", 'H', 10},
    {"range", "enum", 'h', 1},
    {"gear", "", 'h', 1},
    {"mode", "enum", 'h', 1},
    {"flags", "bits", 'I', 1},
    {"dtc", "bits", 'I', 1},
    {"sync_losses", "", 'I', 1},
    {"agree_run", "seg", 'I', 1},
    {"stock_tcv_duty", "%", 'H', 100},
    {"mux0", "bits", 'H', 1},
    {"mux1", "bits", 'H', 1},
    {"mux2", "bits", 'H', 1},
    {"loop_us", "us", 'H', 1},
    {"segments", "", 'I', 1},
    {"egt", "C", 'h', 10},
    {"egt_v", "V", 'H', 1000},
    {"lambda", "", 'H', 1000},
    {"wb_v", "V", 'H', 1000},
    {"egt_scale", "%", 'H', 100},
    {"rich_cut", "%", 'H', 100},
    {"smoke_trim", "%", 'h', 100},
    {"q_limit", "mm3/st", 'H', 100},
};

constexpr size_t wire_size(char t) { return (t == 'i' || t == 'I') ? 4 : 2; }
constexpr size_t sample_payload_size() {
    size_t s = 8;
    for (int i = 0; i < C_COUNT; i++) s += wire_size(CHANNELS[i].type);
    return s;
}

// SCHEMA payload: u8 version, u8 count, then per channel:
//   char type, f32 scale, name\0, unit\0
inline size_t schema_payload(uint8_t *out, size_t cap, const Channel *ch = CHANNELS, int n = C_COUNT) {
    size_t k = 0;
    if (cap < 2) return 0;
    out[k++] = FORMAT_VERSION;
    out[k++] = (uint8_t)n;
    for (int i = 0; i < n; i++) {
        size_t ln = strlen(ch[i].name) + 1, lu = strlen(ch[i].unit) + 1;
        if (k + 5 + ln + lu > cap) return 0;
        out[k++] = (uint8_t)ch[i].type;
        putf(out + k, ch[i].scale); k += 4;
        memcpy(out + k, ch[i].name, ln); k += ln;
        memcpy(out + k, ch[i].unit, lu); k += lu;
    }
    return k;
}

// Builds one SAMPLE payload in the CHANNELS order.
class Sample {
public:
    void begin(uint32_t t_us, uint32_t seq) {
        put32(buf_, t_us); put32(buf_ + 4, seq);
        memset(buf_ + 8, 0, sizeof buf_ - 8);
    }
    void set(int ch, float v) {
        const Channel &c = CHANNELS[ch];
        uint8_t *p = buf_ + offset(ch);
        float x = v * c.scale;
        if (x != x) x = 0;
        switch (c.type) {
            case 'h': put16(p, (uint16_t)clampi(x, -32767, 32767)); break;
            case 'H': put16(p, (uint16_t)clampi(x, 0, 65535)); break;
            case 'i': put32(p, (uint32_t)(int32_t)clampl(x, -2147483647.0, 2147483647.0)); break;
            default: put32(p, (uint32_t)clampl(x, 0, 4294967295.0)); break;
        }
    }
    void set_u32(int ch, uint32_t v) { put32(buf_ + offset(ch), v); }   // bitfields, counters: exact
    const uint8_t *data() const { return buf_; }
    size_t size() const { return SIZE; }

    static constexpr size_t SIZE = sample_payload_size();

private:
    static size_t offset(int ch) {
        size_t o = 8;
        for (int i = 0; i < ch; i++) o += type_size(CHANNELS[i].type);
        return o;
    }
    static int32_t clampi(float x, int32_t lo, int32_t hi) {
        if (x < lo) return lo;
        if (x > hi) return hi;
        return (int32_t)(x < 0 ? x - 0.5f : x + 0.5f);
    }
    static double clampl(double x, double lo, double hi) {
        if (x < lo) return lo;
        if (x > hi) return hi;
        return x < 0 ? x - 0.5 : x + 0.5;
    }
    uint8_t buf_[SIZE];
};

// INJ payload (21 bytes): one per 180 crank degrees, written from the NE ISR
// at tooth 0, so the segment period is the cylinder-by-cylinder crank speed.
struct Injection {
    uint32_t t_us;          // segment start (tooth 0), TIM2 ticks
    uint32_t period_us;     // previous segment's period: 180 deg of crank
    uint32_t segment;       // running segment count (cylinder = segment % 4)
    float q;                // planned mm3/stroke
    float close_deg, open_deg;  // planned SPV window, NE degrees
    bool inject;
    static constexpr size_t SIZE = 21;
    size_t pack(uint8_t *p) const {
        put32(p, t_us); put32(p + 4, period_us); put32(p + 8, segment);
        put16(p + 12, (uint16_t)(q * 100 + 0.5f));
        put16(p + 14, (uint16_t)(int16_t)(close_deg * 100));
        put16(p + 16, (uint16_t)(int16_t)(open_deg * 100));
        put16(p + 18, 0);
        p[20] = inject ? 1 : 0;
        return SIZE;
    }
};

inline size_t event_payload(uint8_t *p, uint32_t t_us, uint8_t code, uint32_t a, uint32_t b) {
    put32(p, t_us); p[4] = code; put32(p + 5, a); put32(p + 9, b);
    return 13;
}

// ---- stream parser ------------------------------------------------------
// Feed bytes; returns true when a complete, CRC-checked frame is ready in
// type()/payload()/len(). Bad frames are counted and skipped.
class Parser {
public:
    bool push(uint8_t b) {
        switch (st_) {
            case 0: st_ = (b == SYNC0) ? 1 : 0; return false;
            case 1: st_ = (b == SYNC1) ? 2 : (b == SYNC0 ? 1 : 0); return false;
            case 2: hdr_[0] = b; st_ = 3; return false;
            case 3: hdr_[1] = b; st_ = 4; return false;
            case 4:
                hdr_[2] = b;
                len_ = get16(hdr_ + 1);
                if (len_ > MAX_PAYLOAD) { bad_++; st_ = 0; return false; }
                n_ = 0;
                st_ = len_ ? 5 : 6;
                return false;
            case 5:
                buf_[n_++] = b;
                if (n_ == len_) st_ = 6;
                return false;
            case 6: crc_ = b; st_ = 7; return false;
            default: {
                st_ = 0;
                uint16_t got = (uint16_t)(crc_ | (b << 8));
                uint16_t c = crc16(hdr_, 3);
                c = crc16(buf_, len_, c);
                if (c != got) { bad_++; return false; }
                good_++;
                return true;
            }
        }
    }
    uint8_t type() const { return hdr_[0]; }
    const uint8_t *payload() const { return buf_; }
    size_t len() const { return len_; }
    uint32_t good() const { return good_; }
    uint32_t bad() const { return bad_; }

private:
    uint8_t st_ = 0, crc_ = 0, hdr_[3] = {0, 0, 0};
    uint8_t buf_[MAX_PAYLOAD];
    size_t len_ = 0, n_ = 0;
    uint32_t good_ = 0, bad_ = 0;
};

}  // namespace jlog
