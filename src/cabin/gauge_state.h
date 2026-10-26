// =====================================================================
//  Gauge model: CAN frames in, engineering values and alerts out.
//
//  Everything the round gauge decides lives here, away from LVGL and
//  Arduino, so the host tests (test/test_gauge) exercise the same code
//  that runs in the dash. Internally kPa and °C like the bus; the UI
//  converts to °F / psi for display.
//
//  Alert rules and levels: docs/SENSORS_GAUGE_LOGGING.md §5.
// =====================================================================
#pragma once
#include <math.h>
#include <stdint.h>

#include "can_protocol.h"

namespace gauge {

// ---- alert levels (pre-turbo EGT) --------------------------------------
constexpr float EGT_WARN_F = 1150, EGT_RED_F = 1250;
constexpr float THW_WARN_C = 105, THW_RED_C = 110;          // 221 / 230 °F
constexpr float BOOST_WARN_PSI = 15, BOOST_RED_PSI = 17;    // ECU cuts ~15
constexpr float THO_WARN_C = 110, THO_RED_C = 126.7f;       // 230 / 260 °F
constexpr float VBAT_LOW_V = 12.4f, VBAT_OK_V = 12.8f;
constexpr float RUNNING_RPM = 400;

constexpr uint32_t EGT_HOLD_MS = 2000;
constexpr uint32_t THW_HOLD_MS = 2000;
constexpr uint32_t BOOST_HOLD_MS = 500;
constexpr uint32_t THO_HOLD_MS = 2000;
constexpr uint32_t VBAT_HOLD_MS = 10000;
constexpr uint32_t CEL_HOLD_MS = 1000;
constexpr uint32_t TAP_LOST_MS = 1000;      // no 0x110 for this long
constexpr uint32_t NOTHING_MS = 3000;       // no frames at all after boot
constexpr uint32_t SNOOZE_MS = 30000;       // button hides an overlay this long
constexpr uint32_t TAP_ALERT_MS = 3000;     // a 0x115 stays active this long

constexpr uint32_t FAST_STALE_MS = 500;     // 20-50 Hz frames
constexpr uint32_t SLOW_STALE_MS = 2500;    // 2-5 Hz frames

constexpr int TREND_N = 60;                 // coolant trend, one sample / 10 s
constexpr uint32_t TREND_MS = 10000;

// Numbering is shared with the tap's 0x115 frames. Lower = higher priority.
enum class Alert : uint8_t {
    NONE = 0,
    EGT_HIGH = 1,
    COOLANT_HIGH = 2,
    OVERBOOST = 3,
    ATF_HIGH = 4,
    CHECK_ENGINE = 5,
    LOW_VOLTS = 6,
    BUS_LOST = 7,
};
constexpr int N_ALERTS = 8;

inline bool is_red(Alert a) {
    return a == Alert::EGT_HIGH || a == Alert::COOLANT_HIGH || a == Alert::OVERBOOST;
}
// BUS_LOST never takes over the screen: the values turn to dashes instead.
inline bool takes_over(Alert a) { return a != Alert::NONE && a != Alert::BUS_LOST; }

enum class Page : uint8_t { DRIVE = 0, TEMPS, ENGINE, TUNE };
constexpr int N_PAGES = 4;

inline float c_to_f(float c) { return c * 9.0f / 5.0f + 32.0f; }
inline float f_to_c(float f) { return (f - 32.0f) * 5.0f / 9.0f; }
constexpr float KPA_PER_PSI = 6.894757f;

// What the UI reads. NaN = unknown, stale, or not fitted.
struct Readings {
    float egt_f = NAN;
    float boost_psi = NAN;        // real boost (tap PIM, else engine-bay MAP)
    float boost_seen_psi = NAN;   // what the ECU is told
    float map_abs_kpa = NAN, baro_kpa = NAN;
    float rpm = NAN, timing_btdc = NAN, spv_deg = NAN, tcv_pct = NAN;
    float va_pct = NAN, va_seen_pct = NAN;
    float thw_c = NAN, thw_seen_c = NAN, tha_c = NAN, thf_c = NAN, tho_c = NAN;
    float vbat_v = NAN, speed_kmh = NAN;
    float lambda = NAN;
    bool dig_valid = false;
    uint32_t dig = 0;
    bool pb_valid = false, pb_armed = false, pb_learning = false;
    uint8_t pb_authority_pct = 0, pb_extra_fuel_pct = 0;
    bool tap_alive = false, bay_alive = false;
};

struct ActiveAlert {
    Alert code = Alert::NONE;
    float value = NAN;   // display units: °F, psi, V
    float limit = NAN;
};

class State {
  public:
    // ---------------------------------------------------------------- input
    void ingest(uint32_t id, const uint8_t* d, uint8_t len, uint32_t now) {
        using namespace can_proto;
        if (len < min_len(id)) return;
        switch (id) {
            case ID_EGT: {
                const int16_t v = get_i16(d);
                egt_f_ = v == SENTINEL_INVALID ? NAN : v;
                t_egt_ = now;
                t_bay_ = now;
                break;
            }
            case ID_MAP_PSI: {
                const int16_t v = get_i16(d);
                bay_psi_ = v == SENTINEL_INVALID ? NAN : v / 10.0f;
                t_bay_psi_ = now;
                t_bay_ = now;
                break;
            }
            case ID_MAP_ABS: {
                const MapAbs m = unpack_map_abs(d);
                map_abs_ = u16f(m.map_kpa_x10, 0.1f);
                baro_ = u16f(m.baro_kpa_x10, 0.1f);
                t_map_abs_ = now;
                t_bay_ = now;
                break;
            }
            case ID_TAP_ENGINE: {
                const TapEngine m = unpack_tap_engine(d);
                rpm_ = u16f(m.rpm, 1);
                timing_ = i16f(m.timing_btdc_x10, 0.1f);
                tcv_ = u16f(m.tcv_duty_x10, 0.1f);
                t_engine_ = now;
                t_tap_ = now;
                tap_seen_ = true;
                break;
            }
            case ID_TAP_FUEL: {
                const TapFuel m = unpack_tap_fuel(d);
                spv_ = u16f(m.spv_deg_x10, 0.1f);
                va_ = u16f(m.va_pct_x10, 0.1f);
                pim_ = u16f(m.pim_kpa, 1);
                t_fuel_ = now;
                t_tap_ = now;
                break;
            }
            case ID_TAP_TEMPS: {
                const TapTemps m = unpack_tap_temps(d);
                thw_ = i16f(m.thw, 1);
                tha_ = i16f(m.tha, 1);
                thf_ = i16f(m.thf, 1);
                tho_ = i16f(m.tho, 1);
                t_temps_ = now;
                t_tap_ = now;
                break;
            }
            case ID_TAP_PB: {
                pb_ = unpack_piggyback(d);
                t_pb_ = now;
                t_tap_ = now;
                break;
            }
            case ID_TAP_STATUS: {
                status_ = unpack_tap_status(d);
                t_status_ = now;
                t_tap_ = now;
                break;
            }
            case ID_TAP_ALERT: {
                const AlertMsg m = unpack_alert(d);
                if (m.code > 0 && m.code < N_ALERTS) {
                    tap_alert_ = m;
                    t_tap_alert_ = now;
                    have_tap_alert_ = true;
                }
                break;
            }
            case ID_WIDEBAND: {
                // rusEFI wideband: byte 2..3 little-endian lambda ×10000.
                if (len >= 4) {
                    lambda_ = (d[2] | (d[3] << 8)) / 10000.0f;
                    t_lambda_ = now;
                }
                break;
            }
            default:
                return;
        }
        any_seen_ = true;
        t_any_ = now;
    }

    // Button: short press acknowledges an overlay, or pages forward.
    void short_press(uint32_t now) {
        if (overlay(now)) {
            snooze_[(int)current_.code] = now;
            snoozed_[(int)current_.code] = true;
            if (current_.code == Alert::CHECK_ENGINE) cel_acked_ = true;
            return;
        }
        page_ = (Page)(((int)page_ + 1) % N_PAGES);
    }
    void long_press() { page_ = Page::DRIVE; }

    // ---------------------------------------------------------------- tick
    void tick(uint32_t now) {
        Readings& r = r_;
        const bool bay = fresh(t_bay_, now, SLOW_STALE_MS);
        const bool tap = fresh(t_tap_, now, TAP_LOST_MS);
        r.bay_alive = bay;
        r.tap_alive = tap;

        r.egt_f = fresh(t_egt_, now, FAST_STALE_MS) ? egt_f_ : NAN;
        r.map_abs_kpa = fresh(t_map_abs_, now, SLOW_STALE_MS) ? map_abs_ : NAN;
        r.baro_kpa = fresh(t_map_abs_, now, SLOW_STALE_MS) && !isnan(baro_) ? baro_ : NAN;
        const float baro = isnan(r.baro_kpa) ? 101.3f : r.baro_kpa;

        const bool eng = fresh(t_engine_, now, FAST_STALE_MS);
        r.rpm = eng ? rpm_ : NAN;
        r.timing_btdc = eng ? timing_ : NAN;
        r.tcv_pct = eng ? tcv_ : NAN;

        const bool fuel = fresh(t_fuel_, now, FAST_STALE_MS);
        r.spv_deg = fuel ? spv_ : NAN;
        r.va_pct = fuel ? va_ : NAN;
        // Real boost: the stock sensor via the tap first (it is what the
        // ECU reacts to), the engine-bay 3-bar as the fallback.
        if (fuel && !isnan(pim_)) r.boost_psi = (pim_ - baro) / KPA_PER_PSI;
        else if (fresh(t_bay_psi_, now, FAST_STALE_MS)) r.boost_psi = bay_psi_;
        else r.boost_psi = NAN;

        const bool temps = fresh(t_temps_, now, SLOW_STALE_MS);
        r.thw_c = temps ? thw_ : NAN;
        r.tha_c = temps ? tha_ : NAN;
        r.thf_c = temps ? thf_ : NAN;
        r.tho_c = temps ? tho_ : NAN;

        r.pb_valid = fresh(t_pb_, now, SLOW_STALE_MS);
        if (r.pb_valid) {
            r.pb_armed = pb_.armed;
            r.pb_learning = pb_.learning;
            r.pb_authority_pct = pb_.authority_pct;
            r.pb_extra_fuel_pct = pb_.armed ? pb_.extra_fuel_pct : 0;
            r.boost_seen_psi = pb_.pim_seen_kpa == can_proto::U16_INVALID
                                   ? NAN : (pb_.pim_seen_kpa - baro) / KPA_PER_PSI;
            r.va_seen_pct = u16f(pb_.va_seen_pct_x10, 0.1f);
            r.thw_seen_c = i16f(pb_.thw_seen, 1);
        } else {
            r.pb_armed = r.pb_learning = false;
            r.pb_authority_pct = r.pb_extra_fuel_pct = 0;
            r.boost_seen_psi = r.va_seen_pct = r.thw_seen_c = NAN;
        }

        r.dig_valid = fresh(t_status_, now, SLOW_STALE_MS);
        r.vbat_v = r.dig_valid ? u16f(status_.vbat_mv, 0.001f) : NAN;
        r.speed_kmh = r.dig_valid ? u16f(status_.speed_kmh_x10, 0.1f) : NAN;
        r.dig = r.dig_valid ? status_.dig : 0;
        r.lambda = fresh(t_lambda_, now, FAST_STALE_MS) ? lambda_ : NAN;

        if (now - t_trend_ >= TREND_MS || !trend_started_) {
            t_trend_ = now;
            trend_started_ = true;
            trend_[trend_head_] = r.thw_c;
            trend_head_ = (trend_head_ + 1) % TREND_N;
            if (trend_count_ < TREND_N) ++trend_count_;
        }

        evaluate(now);
    }

    // --------------------------------------------------------------- output
    const Readings& readings() const { return r_; }
    Page page() const { return page_; }
    const ActiveAlert& alert() const { return current_; }
    // True when the current alert should cover the page.
    bool overlay(uint32_t now) const {
        if (!takes_over(current_.code)) return false;
        const int i = (int)current_.code;
        if (current_.code == Alert::CHECK_ENGINE && cel_acked_) return false;
        return !(snoozed_[i] && now - snooze_[i] < SNOOZE_MS);
    }
    bool alert_active(Alert a) const { return active_[(int)a]; }
    bool booted_quiet(uint32_t now) const { return !any_seen_ && now >= NOTHING_MS; }

    // Coolant trend, oldest first; NaN where nothing was known.
    int trend_count() const { return trend_count_; }
    float trend_at(int i) const {
        const int start = (trend_head_ - trend_count_ + TREND_N) % TREND_N;
        return trend_[(start + i) % TREND_N];
    }

  private:
    static bool fresh(uint32_t t, uint32_t now, uint32_t max_age) {
        return t != 0 && now - t <= max_age;
    }
    static float u16f(uint16_t v, float s) { return v == can_proto::U16_INVALID ? NAN : v * s; }
    static float i16f(int16_t v, float s) { return v == can_proto::SENTINEL_INVALID ? NAN : v * s; }

    // Hold-then-latch with hysteresis: trips after `hold` ms at or above
    // `trip`, clears once below `clear`. NaN never trips and clears.
    bool hyst(Alert a, float v, float trip, float clear, uint32_t hold, uint32_t now) {
        const int i = (int)a;
        if (isnan(v)) { since_[i] = 0; return own_[i] = false; }
        if (own_[i]) {
            if (v < clear) { own_[i] = false; since_[i] = 0; }
            return own_[i];
        }
        if (v >= trip) {
            if (since_[i] == 0) since_[i] = now ? now : 1;
            if (now - since_[i] >= hold) own_[i] = true;
        } else {
            since_[i] = 0;
        }
        return own_[i];
    }
    bool hold_bool(Alert a, bool cond, uint32_t hold, uint32_t now) {
        return hyst(a, cond ? 1.0f : 0.0f, 1.0f, 0.5f, hold, now);
    }

    void evaluate(uint32_t now) {
        const Readings& r = r_;
        const bool running = !isnan(r.rpm) && r.rpm >= RUNNING_RPM;
        float value[N_ALERTS], limit[N_ALERTS];
        for (int i = 0; i < N_ALERTS; ++i) value[i] = limit[i] = NAN;

        hyst(Alert::EGT_HIGH, r.egt_f, EGT_RED_F, EGT_WARN_F, EGT_HOLD_MS, now);
        value[1] = r.egt_f; limit[1] = EGT_RED_F;
        hyst(Alert::COOLANT_HIGH, r.thw_c, THW_RED_C, THW_WARN_C, THW_HOLD_MS, now);
        value[2] = c_to_f(r.thw_c); limit[2] = c_to_f(THW_RED_C);
        hyst(Alert::OVERBOOST, r.boost_psi, BOOST_RED_PSI, BOOST_WARN_PSI, BOOST_HOLD_MS, now);
        value[3] = r.boost_psi; limit[3] = BOOST_RED_PSI;
        hyst(Alert::ATF_HIGH, r.tho_c, THO_RED_C, THO_WARN_C, THO_HOLD_MS, now);
        value[4] = c_to_f(r.tho_c); limit[4] = c_to_f(THO_RED_C);

        const bool w = r.dig_valid && (r.dig & can_proto::DIG_W_LAMP) && running;
        hold_bool(Alert::CHECK_ENGINE, w, CEL_HOLD_MS, now);
        if (!own_[5]) cel_acked_ = false;   // lamp went out: the next one shows again

        const float vbat = running ? r.vbat_v : NAN;
        hyst(Alert::LOW_VOLTS, isnan(vbat) ? NAN : -vbat, -VBAT_LOW_V, -VBAT_OK_V, VBAT_HOLD_MS, now);
        value[6] = r.vbat_v; limit[6] = VBAT_LOW_V;

        // Tap lost after having been seen, or nothing at all since boot.
        own_[7] = (tap_seen_ && !r.tap_alive) || booted_quiet(now);

        // The tap's own verdicts count as well, for a few seconds each.
        const bool tap_on = have_tap_alert_ && now - t_tap_alert_ <= TAP_ALERT_MS;
        for (int i = 0; i < N_ALERTS; ++i) active_[i] = own_[i];
        if (tap_on && !own_[tap_alert_.code]) {
            active_[tap_alert_.code] = true;
            tap_value(tap_alert_.code, value[tap_alert_.code], limit[tap_alert_.code]);
        }

        current_ = ActiveAlert{};
        for (int i = 1; i < N_ALERTS; ++i) {
            if (!active_[i]) {
                snoozed_[i] = false;
                continue;
            }
            if (current_.code == Alert::NONE) current_ = {(Alert)i, value[i], limit[i]};
        }
    }

    // 0x115 carries bus units; convert to what the overlay shows.
    void tap_value(int code, float& v, float& lim) const {
        const float a = tap_alert_.value, b = tap_alert_.limit;
        switch ((Alert)code) {
            case Alert::EGT_HIGH: case Alert::COOLANT_HIGH: case Alert::ATF_HIGH:
                v = c_to_f(a); lim = c_to_f(b); break;
            case Alert::OVERBOOST: v = a / KPA_PER_PSI; lim = b / KPA_PER_PSI; break;
            case Alert::LOW_VOLTS: v = a / 1000.0f; lim = b / 1000.0f; break;
            default: v = a; lim = b; break;
        }
    }

    Readings r_;
    Page page_ = Page::DRIVE;
    ActiveAlert current_;

    float egt_f_ = NAN, bay_psi_ = NAN, map_abs_ = NAN, baro_ = NAN;
    float rpm_ = NAN, timing_ = NAN, tcv_ = NAN, spv_ = NAN, va_ = NAN, pim_ = NAN;
    float thw_ = NAN, tha_ = NAN, thf_ = NAN, tho_ = NAN, lambda_ = NAN;
    can_proto::PiggybackRx pb_{};
    can_proto::TapStatus status_{};
    can_proto::AlertMsg tap_alert_{};
    bool have_tap_alert_ = false;

    uint32_t t_egt_ = 0, t_bay_psi_ = 0, t_map_abs_ = 0, t_bay_ = 0;
    uint32_t t_engine_ = 0, t_fuel_ = 0, t_temps_ = 0, t_pb_ = 0, t_status_ = 0;
    uint32_t t_tap_ = 0, t_tap_alert_ = 0, t_lambda_ = 0, t_any_ = 0;
    bool tap_seen_ = false, any_seen_ = false;

    bool active_[N_ALERTS] = {};
    bool own_[N_ALERTS] = {};
    uint32_t since_[N_ALERTS] = {};
    bool snoozed_[N_ALERTS] = {};
    uint32_t snooze_[N_ALERTS] = {};
    bool cel_acked_ = false;

    float trend_[TREND_N] = {};
    int trend_head_ = 0, trend_count_ = 0;
    uint32_t t_trend_ = 0;
    bool trend_started_ = false;
};

}  // namespace gauge
