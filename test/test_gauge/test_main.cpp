// Gauge model, alerts and the page button: pio test -e native
#include <math.h>
#include <unity.h>

#include "button.h"
#include "demo.h"
#include "gauge_state.h"

using namespace can_proto;
using gauge::Alert;
using gauge::Page;
using gauge::State;

void setUp() {}
void tearDown() {}

namespace {
void feed(State& s, const Frame& f, uint32_t now) { s.ingest(f.id, f.data, f.len, now); }

// A healthy truck at cruise, every frame, at time `now`.
void cruise(State& s, uint32_t now, float egt_f = 900, int thw = 88, uint16_t pim = 130, bool w_lamp = false,
            uint16_t vbat_mv = 13900, uint16_t rpm = 2000) {
    feed(s, pack_i16(ID_EGT, (int16_t)egt_f), now);
    feed(s, pack(MapAbs{(uint16_t)(pim * 10), 1013, 5000, U16_INVALID}), now);
    feed(s, pack(TapEngine{rpm, 80, 400}), now);
    feed(s, pack(TapFuel{150, 300, pim}), now);
    feed(s, pack(TapTemps{(int16_t)thw, 25, 30, 80}), now);
    feed(s, pack(Piggyback{0, pim, 300, (int16_t)thw, 0}, false, 0), now);
    feed(s, pack(TapStatus{vbat_mv, 800, w_lamp ? (uint32_t)DIG_W_LAMP : 0u}), now);
}

// Run 20 Hz ticks from t0 to t1 with the given frame generator.
template <typename F>
void run(State& s, uint32_t t0, uint32_t t1, F gen) {
    for (uint32_t t = t0; t <= t1; t += 50) {
        gen(t);
        s.tick(t);
    }
}
}  // namespace

void test_values_and_units() {
    State s;
    cruise(s, 1000);
    s.tick(1000);
    const gauge::Readings& r = s.readings();
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 900, r.egt_f);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, (130 - 101.3f) / gauge::KPA_PER_PSI, r.boost_psi);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 2000, r.rpm);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 8.0f, r.timing_btdc);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 88, r.thw_c);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 13.9f, r.vbat_v);
    TEST_ASSERT_TRUE(r.tap_alive);
    TEST_ASSERT_TRUE(r.bay_alive);
    TEST_ASSERT_EQUAL(Alert::NONE, s.alert().code);
}

void test_boost_falls_back_to_engine_bay() {
    State s;
    feed(s, pack_i16(ID_MAP_PSI, 123), 1000);
    s.tick(1000);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.3f, s.readings().boost_psi);
    TEST_ASSERT_FALSE(s.readings().tap_alive);
}

void test_stale_values_go_unknown() {
    State s;
    cruise(s, 1000);
    s.tick(1000);
    s.tick(1000 + gauge::FAST_STALE_MS + 1);
    TEST_ASSERT_TRUE(isnan(s.readings().egt_f));
    TEST_ASSERT_TRUE(isnan(s.readings().rpm));
    TEST_ASSERT_FALSE(isnan(s.readings().thw_c));   // slow frame, still fresh
}

void test_sentinel_is_unknown() {
    State s;
    feed(s, pack_i16(ID_EGT, SENTINEL_INVALID), 1000);
    s.tick(1000);
    TEST_ASSERT_TRUE(isnan(s.readings().egt_f));
}

void test_short_frames_are_dropped() {
    State s;
    const uint8_t d[2] = {0x10, 0x00};
    s.ingest(ID_TAP_ENGINE, d, 2, 1000);
    s.tick(1000);
    TEST_ASSERT_TRUE(isnan(s.readings().rpm));
}

void test_egt_needs_two_seconds_then_clears_below_warn() {
    State s;
    run(s, 1000, 2900, [&](uint32_t t) { cruise(s, t, 1300); });
    TEST_ASSERT_EQUAL(Alert::NONE, s.alert().code);   // 1.9 s over: not yet
    run(s, 2950, 3100, [&](uint32_t t) { cruise(s, t, 1300); });
    TEST_ASSERT_EQUAL(Alert::EGT_HIGH, s.alert().code);
    TEST_ASSERT_TRUE(s.overlay(3100));
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 1300, s.alert().value);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 1250, s.alert().limit);
    run(s, 3150, 4000, [&](uint32_t t) { cruise(s, t, 1200); });   // below red, above warn
    TEST_ASSERT_EQUAL(Alert::EGT_HIGH, s.alert().code);
    run(s, 4050, 4100, [&](uint32_t t) { cruise(s, t, 1100); });
    TEST_ASSERT_EQUAL(Alert::NONE, s.alert().code);
}

void test_brief_spike_does_not_trip() {
    State s;
    run(s, 1000, 6000, [&](uint32_t t) { cruise(s, t, (t / 1000) % 2 ? 1300 : 1000); });
    TEST_ASSERT_FALSE(s.alert_active(Alert::EGT_HIGH));
}

void test_priority_egt_over_atf() {
    State s;
    run(s, 1000, 4000, [&](uint32_t t) {
        cruise(s, t, 1300);
        feed(s, pack(TapTemps{88, 25, 30, 130}), t);
    });
    TEST_ASSERT_TRUE(s.alert_active(Alert::ATF_HIGH));
    TEST_ASSERT_EQUAL(Alert::EGT_HIGH, s.alert().code);
}

void test_overboost() {
    State s;
    const uint16_t pim = (uint16_t)(101.3f + 18 * gauge::KPA_PER_PSI);
    run(s, 1000, 1600, [&](uint32_t t) { cruise(s, t, 900, 88, pim); });
    TEST_ASSERT_EQUAL(Alert::OVERBOOST, s.alert().code);
    TEST_ASSERT_FLOAT_WITHIN(0.2f, 18, s.alert().value);
}

void test_snooze_hides_then_returns() {
    State s;
    run(s, 1000, 3500, [&](uint32_t t) { cruise(s, t, 1300); });
    TEST_ASSERT_TRUE(s.overlay(3500));
    s.short_press(3500);
    TEST_ASSERT_EQUAL(Page::DRIVE, s.page());   // the press was the acknowledgement
    TEST_ASSERT_FALSE(s.overlay(3600));
    TEST_ASSERT_EQUAL(Alert::EGT_HIGH, s.alert().code);   // still known, just not covering
    run(s, 3550, 3500 + gauge::SNOOZE_MS + 100, [&](uint32_t t) { cruise(s, t, 1300); });
    TEST_ASSERT_TRUE(s.overlay(3500 + gauge::SNOOZE_MS + 100));
}

void test_check_engine_only_when_running_and_ack_sticks() {
    State s;
    // Key on, engine off: the lamp's bulb check must not alarm.
    run(s, 1000, 3000, [&](uint32_t t) { cruise(s, t, 300, 40, 101, true, 12600, 0); });
    TEST_ASSERT_FALSE(s.alert_active(Alert::CHECK_ENGINE));
    run(s, 3050, 4500, [&](uint32_t t) { cruise(s, t, 900, 88, 130, true); });
    TEST_ASSERT_EQUAL(Alert::CHECK_ENGINE, s.alert().code);
    s.short_press(4500);
    run(s, 4550, 4500 + gauge::SNOOZE_MS + 1000, [&](uint32_t t) { cruise(s, t, 900, 88, 130, true); });
    TEST_ASSERT_FALSE(s.overlay(4500 + gauge::SNOOZE_MS + 1000));   // acknowledged for good
    // Lamp goes out, then comes back: shows again.
    uint32_t t0 = 4500 + gauge::SNOOZE_MS + 1050;
    run(s, t0, t0 + 500, [&](uint32_t t) { cruise(s, t); });
    run(s, t0 + 550, t0 + 2000, [&](uint32_t t) { cruise(s, t, 900, 88, 130, true); });
    TEST_ASSERT_TRUE(s.overlay(t0 + 2000));
}

void test_low_volts_needs_ten_seconds_running() {
    State s;
    run(s, 1000, 10000, [&](uint32_t t) { cruise(s, t, 900, 88, 130, false, 12100); });
    TEST_ASSERT_FALSE(s.alert_active(Alert::LOW_VOLTS));
    run(s, 10050, 11200, [&](uint32_t t) { cruise(s, t, 900, 88, 130, false, 12100); });
    TEST_ASSERT_TRUE(s.alert_active(Alert::LOW_VOLTS));
    run(s, 11250, 11400, [&](uint32_t t) { cruise(s, t, 900, 88, 130, false, 12600); });
    TEST_ASSERT_TRUE(s.alert_active(Alert::LOW_VOLTS));   // hysteresis: needs 12.8
    run(s, 11450, 11500, [&](uint32_t t) { cruise(s, t, 900, 88, 130, false, 13500); });
    TEST_ASSERT_FALSE(s.alert_active(Alert::LOW_VOLTS));
}

void test_bus_lost_never_covers_the_page() {
    State s;
    run(s, 1000, 2000, [&](uint32_t t) { cruise(s, t); });
    run(s, 2050, 3500, [&](uint32_t t) { feed(s, pack_i16(ID_EGT, 900), t); });   // tap gone
    TEST_ASSERT_TRUE(s.alert_active(Alert::BUS_LOST));
    TEST_ASSERT_FALSE(s.overlay(3500));
    TEST_ASSERT_TRUE(isnan(s.readings().rpm));

    State quiet;
    quiet.tick(gauge::NOTHING_MS + 10);
    TEST_ASSERT_TRUE(quiet.alert_active(Alert::BUS_LOST));
}

void test_tap_alert_frame_counts() {
    State s;
    run(s, 1000, 1500, [&](uint32_t t) { cruise(s, t); });
    feed(s, pack(AlertMsg{(uint8_t)Alert::COOLANT_HIGH, 111, 110}), 1550);
    s.tick(1550);
    TEST_ASSERT_EQUAL(Alert::COOLANT_HIGH, s.alert().code);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, gauge::c_to_f(111), s.alert().value);
    run(s, 1600, 1550 + gauge::TAP_ALERT_MS + 100, [&](uint32_t t) { cruise(s, t); });
    TEST_ASSERT_EQUAL(Alert::NONE, s.alert().code);
}

void test_pages_cycle_and_long_press_returns() {
    State s;
    TEST_ASSERT_EQUAL(Page::DRIVE, s.page());
    s.short_press(0);
    TEST_ASSERT_EQUAL(Page::TEMPS, s.page());
    s.short_press(0);
    s.short_press(0);
    TEST_ASSERT_EQUAL(Page::TUNE, s.page());
    s.short_press(0);
    TEST_ASSERT_EQUAL(Page::DRIVE, s.page());
    s.short_press(0);
    s.long_press();
    TEST_ASSERT_EQUAL(Page::DRIVE, s.page());
}

void test_coolant_trend() {
    State s;
    for (uint32_t t = 1000, k = 0; k < 70; t += gauge::TREND_MS, ++k) {
        cruise(s, t, 900, 80 + (int)(k / 10));
        s.tick(t);
    }
    TEST_ASSERT_EQUAL(gauge::TREND_N, s.trend_count());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 86, s.trend_at(gauge::TREND_N - 1));   // newest
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 81, s.trend_at(0));                    // oldest kept
}

void test_button_short_long_debounce() {
    gauge::Button b;
    using E = gauge::Button;
    TEST_ASSERT_EQUAL(E::NONE, b.update(true, 100));
    TEST_ASSERT_EQUAL(E::NONE, b.update(false, 110));   // 10 ms bounce
    TEST_ASSERT_EQUAL(E::NONE, b.update(false, 200));
    TEST_ASSERT_EQUAL(E::NONE, b.update(true, 300));
    TEST_ASSERT_EQUAL(E::NONE, b.update(true, 340));   // debounced: down
    TEST_ASSERT_EQUAL(E::NONE, b.update(false, 500));
    TEST_ASSERT_EQUAL(E::SHORT, b.update(false, 540));
    TEST_ASSERT_EQUAL(E::NONE, b.update(true, 1000));
    TEST_ASSERT_EQUAL(E::NONE, b.update(true, 1040));
    TEST_ASSERT_EQUAL(E::LONG, b.update(true, 1900));
    TEST_ASSERT_EQUAL(E::NONE, b.update(true, 2500));   // only once
    TEST_ASSERT_EQUAL(E::NONE, b.update(false, 2600));
    TEST_ASSERT_EQUAL(E::NONE, b.update(false, 2640));  // no SHORT after a LONG
}

void test_demo_loop_hits_egt_alert_and_recovers() {
    State s;
    bool saw_alert = false, saw_armed = false;
    Frame f[8];
    for (uint32_t t = 50; t < demo::LOOP_MS; t += 50) {
        const int n = demo::frames(t, f);
        for (int i = 0; i < n; ++i) feed(s, f[i], t);
        s.tick(t);
        saw_alert |= s.alert().code == Alert::EGT_HIGH;
        saw_armed |= s.readings().pb_armed;
        TEST_ASSERT_FALSE(s.alert_active(Alert::BUS_LOST));
    }
    TEST_ASSERT_TRUE(saw_alert);
    TEST_ASSERT_TRUE(saw_armed);
    TEST_ASSERT_EQUAL(Alert::NONE, s.alert().code);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_values_and_units);
    RUN_TEST(test_boost_falls_back_to_engine_bay);
    RUN_TEST(test_stale_values_go_unknown);
    RUN_TEST(test_sentinel_is_unknown);
    RUN_TEST(test_short_frames_are_dropped);
    RUN_TEST(test_egt_needs_two_seconds_then_clears_below_warn);
    RUN_TEST(test_brief_spike_does_not_trip);
    RUN_TEST(test_priority_egt_over_atf);
    RUN_TEST(test_overboost);
    RUN_TEST(test_snooze_hides_then_returns);
    RUN_TEST(test_check_engine_only_when_running_and_ack_sticks);
    RUN_TEST(test_low_volts_needs_ten_seconds_running);
    RUN_TEST(test_bus_lost_never_covers_the_page);
    RUN_TEST(test_tap_alert_frame_counts);
    RUN_TEST(test_pages_cycle_and_long_press_returns);
    RUN_TEST(test_coolant_trend);
    RUN_TEST(test_button_short_long_debounce);
    RUN_TEST(test_demo_loop_hits_egt_alert_and_recovers);
    return UNITY_END();
}
