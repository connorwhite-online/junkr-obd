// 3-bar MAP curve and the baro latch: pio test -e native
#include <math.h>
#include <unity.h>

#include "map_math.h"

using namespace bay;

void setUp() {}
void tearDown() {}

void test_gm3bar_curve() {
    // Mid-scale at 5 V supply: 1.1 + 314.4 * 0.5
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 158.3f, gm3bar_kpa(2.5f, 5.0f));
    // Ratiometric: same ratio on a 4.8 V rail reads the same pressure.
    TEST_ASSERT_FLOAT_WITHIN(0.01f, gm3bar_kpa(2.5f, 5.0f), gm3bar_kpa(2.4f, 4.8f));
    // Sea level is about 1.6 V.
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 101.3f, gm3bar_kpa(1.594f, 5.0f));
}

void test_supply_monitor_missing_uses_nominal() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, gm3bar_kpa(2.5f, 5.0f), gm3bar_kpa(2.5f, 0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, gm3bar_kpa(2.5f, 5.0f), gm3bar_kpa(2.5f, 9.0f));
}

void test_open_or_shorted_sensor_is_nan() {
    TEST_ASSERT_TRUE(isnan(gm3bar_kpa(0.0f, 5.0f)));
    TEST_ASSERT_TRUE(isnan(gm3bar_kpa(5.0f, 5.0f)));
}

void test_gauge_psi() {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 15.0f, gauge_psi(101.3f + 15 * KPA_PER_PSI, 101.3f));
}

void test_baro_boot_latch() {
    BaroTracker b;
    TEST_ASSERT_FALSE(b.latched());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, STD_BARO_KPA, b.baro());
    for (int i = 0; i < 50; ++i) b.boot_sample(82.0f + (i % 2 ? 0.2f : -0.2f));
    b.boot_sample(NAN);   // ignored
    b.finish_boot();
    TEST_ASSERT_TRUE(b.latched());
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 82.0f, b.baro());
}

void test_baro_rejects_boost_at_boot() {
    // A reboot under boost must not latch boost as "baro".
    BaroTracker b;
    for (int i = 0; i < 50; ++i) b.boot_sample(160.0f);
    b.finish_boot();
    TEST_ASSERT_FALSE(b.latched());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, STD_BARO_KPA, b.baro());
}

void test_baro_tracks_only_with_engine_stopped() {
    BaroTracker b;
    b.update(95.0f, true, 0.05f);   // first good stopped sample latches
    TEST_ASSERT_TRUE(b.latched());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 95.0f, b.baro());
    for (int i = 0; i < 200; ++i) b.update(150.0f, false, 0.05f);   // running: ignored
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 95.0f, b.baro());
    for (int i = 0; i < 2000; ++i) b.update(90.0f, true, 0.05f);    // drove downhill, stopped
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 90.0f, b.baro());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_gm3bar_curve);
    RUN_TEST(test_supply_monitor_missing_uses_nominal);
    RUN_TEST(test_open_or_shorted_sensor_is_nan);
    RUN_TEST(test_gauge_psi);
    RUN_TEST(test_baro_boot_latch);
    RUN_TEST(test_baro_rejects_boost_at_boot);
    RUN_TEST(test_baro_tracks_only_with_engine_stopped);
    return UNITY_END();
}
