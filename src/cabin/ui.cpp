// Round gauge pages, laid out after docs/gauge/mockups.py (480 x 480).
// Positions below are in panel pixels from the top-left; C is the centre.
#include "ui.h"

#include <lvgl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

LV_FONT_DECLARE(inter_bold_104);
LV_FONT_DECLARE(inter_bold_60);
LV_FONT_DECLARE(inter_bold_32);
LV_FONT_DECLARE(inter_semi_19);
LV_FONT_DECLARE(inter_bold_15);

using gauge::Alert;
using gauge::Page;
using gauge::Readings;

namespace {

constexpr int W = 480;
constexpr int C = W / 2;
constexpr int R = 212;          // split-ring centre radius
constexpr int RING_W = 22;

// colours (same as the mockups)
constexpr uint32_t FG = 0xf2f2f2, DIM = 0x8a8a8a, TRACK = 0x262626, DOT_OFF = 0x444444;
constexpr uint32_t BOOST = 0x22c7e8, EGT = 0xff8a1f, WARN = 0xffb000, RED = 0xff3b30;
constexpr uint32_t OK = 0x34c759, LEARN = 0x4da3ff, RPM = 0x7d7dff;

// gauge scales
constexpr float BOOST_MAX_PSI = 25, BOOST_CUT_PSI = 15;
constexpr float EGT_MAX_F = 1600;
constexpr float RPM_MAX = 4500, RPM_RED = 4000;

lv_color_t col(uint32_t c) { return lv_color_hex(c); }

// Point on a circle, angle in degrees clockwise from 12 o'clock.
lv_point_t at(float r, float a) {
    const float t = a * 3.14159265f / 180.0f;
    return {(lv_coord_t)lroundf(C + r * sinf(t)), (lv_coord_t)lroundf(C - r * cosf(t))};
}

// ---------------------------------------------------------------- builders
lv_obj_t* page_obj(lv_obj_t* parent) {
    lv_obj_t* o = lv_obj_create(parent);
    lv_obj_remove_style_all(o);
    lv_obj_set_size(o, W, W);
    lv_obj_clear_flag(o, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    return o;
}

// Label whose centre sits at (cx, cy).
lv_obj_t* label_c(lv_obj_t* p, const lv_font_t* f, uint32_t c, const char* txt, int cx, int cy) {
    lv_obj_t* l = lv_label_create(p);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, col(c), 0);
    lv_label_set_text(l, txt);
    lv_obj_align(l, LV_ALIGN_CENTER, cx - C, cy - C);
    return l;
}
// Label with its top-left (left = true) or top-right corner at (x, y).
lv_obj_t* label_at(lv_obj_t* p, const lv_font_t* f, uint32_t c, const char* txt, int x, int y, bool left) {
    lv_obj_t* l = lv_label_create(p);
    lv_obj_set_style_text_font(l, f, 0);
    lv_obj_set_style_text_color(l, col(c), 0);
    lv_label_set_text(l, txt);
    if (left) lv_obj_align(l, LV_ALIGN_TOP_LEFT, x, y);
    else lv_obj_align(l, LV_ALIGN_TOP_RIGHT, x - W, y);
    return l;
}

// LVGL angles start at 3 o'clock; the layout uses 12 o'clock.
int16_t lv_angle(float clock_deg) {
    int a = (int)lroundf(clock_deg - 90);
    while (a < 0) a += 360;
    return (int16_t)(a % 360);
}

lv_obj_t* ring_arc(lv_obj_t* p, int r, int width, float a_from, float a_to, uint32_t track, uint32_t ind,
                   int range_max, bool reverse) {
    lv_obj_t* a = lv_arc_create(p);
    const int size = 2 * (r + width / 2);
    lv_obj_set_size(a, size, size);
    lv_obj_center(a);
    lv_obj_remove_style(a, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(a, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_pad_all(a, 0, 0);
    lv_arc_set_rotation(a, 0);
    lv_arc_set_bg_angles(a, lv_angle(a_from), lv_angle(a_to));
    lv_arc_set_mode(a, reverse ? LV_ARC_MODE_REVERSE : LV_ARC_MODE_NORMAL);
    lv_arc_set_range(a, 0, range_max);
    lv_arc_set_value(a, 0);
    lv_obj_set_style_arc_width(a, width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(a, width, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(a, col(track), LV_PART_MAIN);
    lv_obj_set_style_arc_color(a, col(ind), LV_PART_INDICATOR);
    if (track == 0) lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_MAIN);
    return a;
}

struct Tick {
    lv_obj_t* line = nullptr;
    lv_point_t pts[2];
    void make(lv_obj_t* p, uint32_t c, int width) {
        line = lv_line_create(p);
        lv_obj_set_style_line_color(line, col(c), 0);
        lv_obj_set_style_line_width(line, width, 0);
        lv_obj_set_pos(line, 0, 0);
    }
    void place(float r1, float r2, float a) {
        pts[0] = at(r1, a);
        pts[1] = at(r2, a);
        lv_line_set_points(line, pts, 2);
    }
};

struct Pill {
    lv_obj_t* box;
    lv_obj_t* text;
    void make(lv_obj_t* p, int cx, int cy, int w, int h, const lv_font_t* f) {
        box = lv_obj_create(p);
        lv_obj_remove_style_all(box);
        lv_obj_set_size(box, w, h);
        lv_obj_align(box, LV_ALIGN_CENTER, cx - C, cy - C);
        lv_obj_set_style_radius(box, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(box, 2, 0);
        lv_obj_clear_flag(box, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        text = lv_label_create(box);
        lv_obj_set_style_text_font(text, f, 0);
        lv_obj_center(text);
    }
    void set(const char* s, uint32_t c, bool filled) {
        lv_obj_set_style_border_color(box, col(c), 0);
        lv_obj_set_style_bg_color(box, col(c), 0);
        lv_obj_set_style_bg_opa(box, filled ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_text_color(text, col(filled ? 0x000000 : c), 0);
        if (strcmp(lv_label_get_text(text), s) != 0) lv_label_set_text(text, s);
    }
};

// ---------------------------------------------------------------- updaters
void set_text(lv_obj_t* l, const char* s) {
    if (strcmp(lv_label_get_text(l), s) != 0) lv_label_set_text(l, s);
}
void set_color(lv_obj_t* l, uint32_t c) { lv_obj_set_style_text_color(l, col(c), 0); }

// "--" for unknown values.
void fmt(char* b, size_t n, float v, int decimals, const char* suffix = "") {
    if (isnan(v)) snprintf(b, n, "--%s", suffix);
    else snprintf(b, n, "%.*f%s", decimals, (double)v, suffix);
}
uint32_t level(float v, float warn, float red, uint32_t normal) {
    if (isnan(v)) return DIM;
    return v >= red ? RED : v >= warn ? WARN : normal;
}
float frac(float v, float lo, float hi) {
    if (isnan(v)) return 0;
    const float f = (v - lo) / (hi - lo);
    return f < 0 ? 0 : f > 1 ? 1 : f;
}
float f_or_nan(float c) { return isnan(c) ? NAN : gauge::c_to_f(c); }

// ------------------------------------------------------------------ pages
lv_obj_t* pages[gauge::N_PAGES];
lv_obj_t* dots[gauge::N_PAGES];

struct Drive {
    lv_obj_t *boost_arc, *egt_arc, *boost_val, *boost_sub, *egt_val, *cool, *atf, *volts;
    Tick cut, seen, egt_lim;
    Pill pill;

    void build(lv_obj_t* p) {
        // Boost fills up the left side, 215° -> 335°; EGT up the right, 145° -> 25°.
        boost_arc = ring_arc(p, R, RING_W, 215, 335, TRACK, BOOST, 1000, false);
        egt_arc = ring_arc(p, R, RING_W, 25, 145, TRACK, EGT, 1000, true);
        cut.make(p, RED, 4);
        cut.place(R - 18, R + 18, 215 + 120 * BOOST_CUT_PSI / BOOST_MAX_PSI);
        seen.make(p, FG, 2);
        seen.place(R - 14, R + 14, 215);
        egt_lim.make(p, RED, 4);
        egt_lim.place(R - 18, R + 18, 145 - 120 * gauge::EGT_RED_F / EGT_MAX_F);

        label_c(p, &inter_bold_15, BOOST, "BOOST", C - 122, 128);
        label_c(p, &inter_bold_15, EGT, "EGT", C + 122, 128);
        pill.make(p, C, 92, 140, 30, &inter_bold_15);
        boost_val = label_c(p, &inter_bold_104, FG, "--", C, 190);
        boost_sub = label_c(p, &inter_semi_19, DIM, "psi", C, 254);
        egt_val = label_c(p, &inter_bold_60, EGT, "--", C, 306);
        label_c(p, &inter_semi_19, DIM, "\xC2\xB0" "F pre-turbo", C, 346);
        cool = label_c(p, &inter_semi_19, FG, "COOL --", C - 70, 388);
        atf = label_c(p, &inter_semi_19, FG, "ATF --", C + 70, 388);
        volts = label_c(p, &inter_bold_15, DIM, "-- V", C, 416);
    }

    void update(const gauge::State& s, uint32_t now) {
        const Readings& r = s.readings();
        char b[48], b2[24];

        lv_arc_set_value(boost_arc, (int16_t)(1000 * frac(r.boost_psi, 0, BOOST_MAX_PSI)));
        lv_obj_set_style_arc_color(boost_arc, col(level(r.boost_psi, gauge::BOOST_WARN_PSI,
                                                        gauge::BOOST_RED_PSI, BOOST)), LV_PART_INDICATOR);
        lv_arc_set_value(egt_arc, (int16_t)(1000 * frac(r.egt_f, 0, EGT_MAX_F)));
        lv_obj_set_style_arc_color(egt_arc, col(level(r.egt_f, gauge::EGT_WARN_F, gauge::EGT_RED_F, EGT)),
                                   LV_PART_INDICATOR);

        fmt(b, sizeof b, r.boost_psi < 0 ? 0.0f : r.boost_psi, 1);
        set_text(boost_val, b);
        set_color(boost_val, level(r.boost_psi, gauge::BOOST_WARN_PSI, gauge::BOOST_RED_PSI, FG));

        const bool told = r.pb_armed && !isnan(r.boost_seen_psi);
        if (told) {
            fmt(b2, sizeof b2, r.boost_seen_psi < 0 ? 0.0f : r.boost_seen_psi, 1);
            snprintf(b, sizeof b, "psi \xC2\xB7 ECU sees %s", b2);
            seen.place(R - 14, R + 14, 215 + 120 * frac(r.boost_seen_psi, 0, BOOST_MAX_PSI));
            lv_obj_clear_flag(seen.line, LV_OBJ_FLAG_HIDDEN);
        } else {
            snprintf(b, sizeof b, "psi");
            lv_obj_add_flag(seen.line, LV_OBJ_FLAG_HIDDEN);
        }
        set_text(boost_sub, b);

        fmt(b, sizeof b, r.egt_f, 0);
        set_text(egt_val, b);
        set_color(egt_val, level(r.egt_f, gauge::EGT_WARN_F, gauge::EGT_RED_F, EGT));

        const float cool_f = f_or_nan(r.thw_c), atf_f = f_or_nan(r.tho_c);
        fmt(b2, sizeof b2, cool_f, 0, "\xC2\xB0");
        snprintf(b, sizeof b, "COOL %s", b2);
        set_text(cool, b);
        set_color(cool, level(cool_f, gauge::c_to_f(gauge::THW_WARN_C), gauge::c_to_f(gauge::THW_RED_C), FG));
        fmt(b2, sizeof b2, atf_f, 0, "\xC2\xB0");
        snprintf(b, sizeof b, "ATF %s", b2);
        set_text(atf, b);
        set_color(atf, level(atf_f, gauge::c_to_f(gauge::THO_WARN_C), gauge::c_to_f(gauge::THO_RED_C), FG));
        fmt(b, sizeof b, r.vbat_v, 1, " V");
        set_text(volts, b);

        // Pill: a snoozed alert beats tune state, so it is never forgotten.
        const Alert a = s.alert().code;
        if (gauge::takes_over(a) && !s.overlay(now)) {
            static const char* names[] = {"", "EGT HIGH", "COOLANT HIGH", "OVERBOOST", "ATF HIGH",
                                          "CHECK ENGINE", "LOW VOLTS", ""};
            pill.set(names[(int)a], gauge::is_red(a) ? RED : WARN, false);
        } else if (!r.tap_alive) {
            pill.set("NO TAP", DIM, false);
        } else if (!r.pb_valid || !r.pb_armed) {
            pill.set("STOCK", DIM, false);
        } else if (r.pb_learning) {
            pill.set("LEARNING", LEARN, false);
        } else {
            snprintf(b, sizeof b, "TUNE +%u%%", r.pb_extra_fuel_pct);
            pill.set(b, OK, false);
        }
    }
} drive;

struct Temps {
    struct Row {
        const char* name;
        float lo, hi, warn, red;
        uint32_t color;
        lv_obj_t *val, *bar;
    } rows[5] = {
        {"COOLANT", 120, 250, gauge::c_to_f(gauge::THW_WARN_C), gauge::c_to_f(gauge::THW_RED_C), FG},
        {"ATF", 100, 280, gauge::c_to_f(gauge::THO_WARN_C), gauge::c_to_f(gauge::THO_RED_C), FG},
        {"EGT", 200, 1600, gauge::EGT_WARN_F, gauge::EGT_RED_F, EGT},
        {"INTAKE", 0, 200, 140, 160, FG},
        {"FUEL", 0, 200, 150, 170, FG},
    };
    lv_obj_t* chart;
    lv_chart_series_t* ser;

    void build(lv_obj_t* p) {
        label_c(p, &inter_bold_15, DIM, "TEMPS", C, 62);
        const int x0 = 92, x1 = 388;
        for (int i = 0; i < 5; ++i) {
            Row& row = rows[i];
            const int y = 84 + 54 * i;
            label_at(p, &inter_bold_15, DIM, row.name, x0, y + 12, true);
            row.val = label_at(p, &inter_bold_32, FG, "--", x1, y, false);
            row.bar = lv_bar_create(p);
            lv_obj_set_size(row.bar, x1 - x0, 8);
            lv_obj_set_pos(row.bar, x0, y + 40);
            lv_bar_set_range(row.bar, 0, 1000);
            lv_obj_set_style_bg_color(row.bar, col(TRACK), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(row.bar, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_radius(row.bar, 4, LV_PART_MAIN);
            lv_obj_set_style_radius(row.bar, 4, LV_PART_INDICATOR);
            lv_obj_t* t = lv_obj_create(p);
            lv_obj_remove_style_all(t);
            lv_obj_set_size(t, 2, 16);
            lv_obj_set_pos(t, x0 + (int)((x1 - x0) * frac(row.warn, row.lo, row.hi)), y + 36);
            lv_obj_set_style_bg_color(t, col(WARN), 0);
            lv_obj_set_style_bg_opa(t, LV_OPA_COVER, 0);
        }
        chart = lv_chart_create(p);
        lv_obj_set_size(chart, 240, 52);
        lv_obj_set_pos(chart, 120, 358);
        lv_obj_set_style_bg_opa(chart, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(chart, 0, 0);
        lv_obj_set_style_pad_all(chart, 0, 0);
        lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);   // no point markers
        lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
        lv_chart_set_div_line_count(chart, 0, 0);
        lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(chart, gauge::TREND_N);
        ser = lv_chart_add_series(chart, col(FG), LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_all_value(chart, ser, LV_CHART_POINT_NONE);
        label_c(p, &inter_bold_15, DIM, "coolant \xC2\xB7 last 10 min", C, 424);
    }

    void update(const gauge::State& s) {
        const Readings& r = s.readings();
        const float v[5] = {f_or_nan(r.thw_c), f_or_nan(r.tho_c), r.egt_f, f_or_nan(r.tha_c), f_or_nan(r.thf_c)};
        char b[16];
        for (int i = 0; i < 5; ++i) {
            Row& row = rows[i];
            fmt(b, sizeof b, v[i], 0, "\xC2\xB0" "F");
            set_text(row.val, b);
            const uint32_t c = level(v[i], row.warn, row.red, row.color);
            set_color(row.val, isnan(v[i]) ? DIM : c);
            lv_bar_set_value(row.bar, (int32_t)(1000 * frac(v[i], row.lo, row.hi)), LV_ANIM_OFF);
            lv_obj_set_style_bg_color(row.bar, col(c), LV_PART_INDICATOR);
        }

        // Trend, scaled to its own min..max so a slow climb is visible.
        const int n = s.trend_count();
        float lo = 1e9f, hi = -1e9f;
        for (int i = 0; i < n; ++i) {
            const float f = f_or_nan(s.trend_at(i));
            if (isnan(f)) continue;
            lo = fminf(lo, f);
            hi = fmaxf(hi, f);
        }
        if (lo > hi) { lo = 150; hi = 200; }
        if (hi - lo < 10) { const float m = (hi + lo) / 2; lo = m - 5; hi = m + 5; }
        lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, (lv_coord_t)floorf(lo), (lv_coord_t)ceilf(hi));
        // Newest sample at the right edge.
        for (int i = 0; i < gauge::TREND_N; ++i) {
            const int k = i - (gauge::TREND_N - n);
            const float f = k >= 0 ? f_or_nan(s.trend_at(k)) : NAN;
            lv_chart_set_value_by_id(chart, ser, i, isnan(f) ? LV_CHART_POINT_NONE : (lv_coord_t)lroundf(f));
        }
        lv_chart_refresh(chart);
    }
} temps;

struct Engine {
    lv_obj_t *arc, *rpm, *grid[4], *lockup;
    void build(lv_obj_t* p) {
        arc = ring_arc(p, R, RING_W, 225, 495, TRACK, RPM, 1000, false);
        lv_obj_t* red = ring_arc(p, R + 8, 6, 225 + 270 * RPM_RED / RPM_MAX, 495, 0, RED, 1, false);
        lv_arc_set_value(red, 1);
        for (int k = 1; k <= 4; ++k) {
            const lv_point_t q = at(R - 40, 225 + 270 * k * 1000 / RPM_MAX);
            char b[4];
            snprintf(b, sizeof b, "%d", k);
            label_c(p, &inter_semi_19, DIM, b, q.x, q.y);
        }
        label_c(p, &inter_bold_15, DIM, "RPM", C, 138);
        rpm = label_c(p, &inter_bold_104, FG, "--", C, 202);
        static const char* names[4] = {"TIMING", "FUEL", "PEDAL", "TCV"};
        for (int i = 0; i < 4; ++i) {
            const int x = C - 72 + 144 * (i % 2), y = 294 + 70 * (i / 2);
            label_c(p, &inter_bold_15, DIM, names[i], x, y);
            grid[i] = label_c(p, &inter_bold_32, FG, "--", x, y + 28);
        }
        lockup = label_c(p, &inter_semi_19, DIM, "lock-up --", C, 432);
    }
    void update(const gauge::State& s) {
        const Readings& r = s.readings();
        char b[24];
        lv_arc_set_value(arc, (int16_t)(1000 * frac(r.rpm, 0, RPM_MAX)));
        lv_obj_set_style_arc_color(arc, col(!isnan(r.rpm) && r.rpm >= RPM_RED ? RED : RPM), LV_PART_INDICATOR);
        fmt(b, sizeof b, r.rpm, 0);
        set_text(rpm, b);
        fmt(b, sizeof b, r.timing_btdc, 1, "\xC2\xB0");
        set_text(grid[0], b);
        fmt(b, sizeof b, r.spv_deg, 1, "\xC2\xB0");
        set_text(grid[1], b);
        fmt(b, sizeof b, r.va_pct, 0, "%");
        set_text(grid[2], b);
        fmt(b, sizeof b, r.tcv_pct, 0, "%");
        set_text(grid[3], b);
        if (!r.dig_valid) set_text(lockup, "lock-up --");
        else set_text(lockup, (r.dig & can_proto::DIG_LOCKUP) ? "lock-up ON" : "lock-up off");
    }
} engine;

struct Tune {
    Pill state;
    lv_obj_t *real[3], *seen[3], *authority, *learning;
    void build(lv_obj_t* p) {
        label_c(p, &inter_bold_15, DIM, "PIGGYBACK", C, 66);
        state.make(p, C, 108, 160, 36, &inter_semi_19);
        static const char* names[3] = {"BOOST", "PEDAL", "COOLANT"};
        static const char* units[3] = {"ECU sees \xC2\xB7 psi", "ECU sees \xC2\xB7 %", "ECU sees \xC2\xB7 \xC2\xB0" "F"};
        for (int i = 0; i < 3; ++i) {
            const int y = 150 + 72 * i;
            label_at(p, &inter_bold_15, DIM, names[i], 92, y, true);
            label_at(p, &inter_bold_15, DIM, units[i], 388, y, false);
            real[i] = label_at(p, &inter_bold_32, FG, "--", 92, y + 18, true);
            label_c(p, &inter_semi_19, DIM, "\xE2\x86\x92", C + 6, y + 38);
            seen[i] = label_at(p, &inter_bold_32, FG, "--", 388, y + 18, false);
        }
        authority = label_c(p, &inter_semi_19, FG, "authority --", C, 386);
        learning = label_c(p, &inter_bold_15, DIM, "learning --", C, 414);
    }
    void update(const gauge::State& s) {
        const Readings& r = s.readings();
        if (!r.pb_valid) state.set("NO TAP", DIM, false);
        else if (r.pb_armed) state.set("ARMED", OK, true);
        else state.set("BYPASSED", DIM, false);

        const float rv[3] = {r.boost_psi, r.va_pct, f_or_nan(r.thw_c)};
        const float sv[3] = {r.boost_seen_psi, r.va_seen_pct, f_or_nan(r.thw_seen_c)};
        const int dec[3] = {1, 0, 0};
        const float tol[3] = {0.25f, 1.0f, 1.0f};
        char b[40];
        for (int i = 0; i < 3; ++i) {
            fmt(b, sizeof b, rv[i], dec[i]);
            set_text(real[i], b);
            fmt(b, sizeof b, sv[i], dec[i]);
            set_text(seen[i], b);
            const bool differs = !isnan(rv[i]) && !isnan(sv[i]) && fabsf(rv[i] - sv[i]) >= tol[i];
            set_color(seen[i], differs ? WARN : FG);
        }
        if (r.pb_valid) snprintf(b, sizeof b, "authority %u%% \xC2\xB7 used %u%%", r.pb_authority_pct, r.pb_extra_fuel_pct);
        else snprintf(b, sizeof b, "authority -- \xC2\xB7 used --");
        set_text(authority, b);
        char lam[16];
        fmt(lam, sizeof lam, r.lambda, 2);
        snprintf(b, sizeof b, "learning: %s \xC2\xB7 \xCE\xBB %s", r.pb_valid && r.pb_learning ? "ON" : "OFF", lam);
        set_text(learning, b);
    }
} tune;

struct Overlay {
    lv_obj_t *root, *ring, *title, *value, *sub, *action, *note;
    void build() {
        root = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(root);
        lv_obj_set_size(root, W, W);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
        lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
        ring = ring_arc(root, 226, 14, 0, 360, 0, RED, 1, false);
        lv_arc_set_bg_angles(ring, 0, 360);
        lv_arc_set_value(ring, 1);
        title = label_c(root, &inter_bold_32, RED, "", C, 140);
        value = label_c(root, &inter_bold_104, FG, "", C, 226);
        sub = label_c(root, &inter_semi_19, DIM, "", C, 294);
        action = label_c(root, &inter_bold_32, FG, "", C, 346);
        note = label_c(root, &inter_semi_19, DIM, "", C, 384);
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
    }
    void update(const gauge::State& s, uint32_t now) {
        if (!s.overlay(now)) {
            lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
            return;
        }
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
        const gauge::ActiveAlert& a = s.alert();
        struct Text { const char *title, *unit, *action, *note; int dec; };
        static const Text T[gauge::N_ALERTS] = {
            {"", "", "", "", 0},
            {"EGT HIGH", "\xC2\xB0" "F", "LIFT / DOWNSHIFT", "tune pulled to stock", 0},
            {"COOLANT HIGH", "\xC2\xB0" "F", "EASE OFF", "tune pulled to stock", 0},
            {"OVERBOOST", "psi", "LIFT", "tune pulled to stock", 1},
            {"ATF HIGH", "\xC2\xB0" "F", "EASE OFF", "lock-up helps; let it cool", 0},
            {"CHECK ENGINE", "", "CHECK ENGINE", "logged \xC2\xB7 press to dismiss", 0},
            {"LOW VOLTS", "V", "CHECK CHARGING", "battery low with engine running", 1},
            {"", "", "", "", 0},
        };
        const Text& t = T[(int)a.code];
        const uint32_t c = gauge::is_red(a.code) ? RED : WARN;
        lv_obj_set_style_arc_color(ring, col(c), LV_PART_INDICATOR);
        set_text(title, t.title);
        set_color(title, c);
        char b[40], l[16];
        if (a.code == Alert::CHECK_ENGINE) {
            set_text(value, "");
            set_text(sub, "W lamp on with the engine running");
        } else {
            fmt(b, sizeof b, a.value, t.dec);
            set_text(value, b);
            fmt(l, sizeof l, a.limit, t.dec);
            snprintf(b, sizeof b, "%s \xC2\xB7 limit %s", t.unit, l);
            set_text(sub, b);
        }
        set_text(action, t.action);
        set_text(note, t.note);
    }
} overlay;

}  // namespace

void ui_init() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    for (int i = 0; i < gauge::N_PAGES; ++i) pages[i] = page_obj(scr);
    drive.build(pages[(int)Page::DRIVE]);
    temps.build(pages[(int)Page::TEMPS]);
    engine.build(pages[(int)Page::ENGINE]);
    tune.build(pages[(int)Page::TUNE]);

    for (int k = 0; k < gauge::N_PAGES; ++k) {
        lv_obj_t* d = lv_obj_create(scr);
        lv_obj_remove_style_all(d);
        lv_obj_set_size(d, 8, 8);
        lv_obj_set_style_radius(d, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(d, LV_OPA_COVER, 0);
        lv_obj_align(d, LV_ALIGN_CENTER, -27 + 18 * k, W - 22 - C);
        dots[k] = d;
    }
    overlay.build();
}

void ui_update(const gauge::State& s, uint32_t now) {
    const int cur = (int)s.page();
    for (int i = 0; i < gauge::N_PAGES; ++i) {
        if (i == cur) lv_obj_clear_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(pages[i], LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(dots[i], col(i == cur ? FG : DOT_OFF), 0);
    }
    switch (s.page()) {
        case Page::DRIVE: drive.update(s, now); break;
        case Page::TEMPS: temps.update(s); break;
        case Page::ENGINE: engine.update(s); break;
        case Page::TUNE: tune.update(s); break;
    }
    overlay.update(s, now);
}
