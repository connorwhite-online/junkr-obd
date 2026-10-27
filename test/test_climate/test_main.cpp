// Climate knob: setpoint scale, press timing and homing: pio test -e native
#include <unity.h>

#include "climate_logic.h"

using climate::Knob;
using climate::Scale;

void setUp() {}
void tearDown() {}

namespace {
struct Count {
    int up = 0, down = 0;
};

// Run 1 ms ticks from t0 for ms, counting completed presses (rising edges).
Count run(Knob& k, uint32_t& t, uint32_t ms) {
    Count c;
    Knob::Out last = Knob::IDLE;
    for (uint32_t end = t + ms; t < end; ++t) {
        Knob::Out o = k.tick(t);
        if (o != last && o == Knob::UP) c.up++;
        if (o != last && o == Knob::DOWN) c.down++;
        last = o;
    }
    return c;
}
}  // namespace

void test_scale() {
    TEST_ASSERT_EQUAL(30, Scale::STEPS);
    TEST_ASSERT_TRUE(Scale::is_lo(0));
    TEST_ASSERT_EQUAL(180, Scale::deci_c(1));
    TEST_ASSERT_EQUAL(250, Scale::deci_c(15));
    TEST_ASSERT_EQUAL(320, Scale::deci_c(29));
    TEST_ASSERT_TRUE(Scale::is_hi(30));
    TEST_ASSERT_EQUAL(770, Scale::deci_f(15));
}

void test_one_detent_one_press() {
    Knob k;
    uint32_t t = 1000;
    k.turn(+1);
    Count c = run(k, t, 1000);
    TEST_ASSERT_EQUAL(1, c.up);
    TEST_ASSERT_EQUAL(0, c.down);
    TEST_ASSERT_EQUAL(16, k.assumed());
    TEST_ASSERT_TRUE(k.settled());
}

void test_fast_spin_is_paced() {
    Knob k;
    uint32_t t = 1000;
    k.turn(+10);
    // each press is PULSE + GAP, so 10 presses need about 2 s
    Count c = run(k, t, 500);
    TEST_ASSERT_TRUE(c.up <= 3);
    c = run(k, t, 3000);
    TEST_ASSERT_EQUAL(25, k.assumed());
    TEST_ASSERT_TRUE(k.settled());
}

void test_reverse_mid_spin_cancels() {
    Knob k;
    uint32_t t = 1000;
    k.turn(+5);
    Count a = run(k, t, 450);   // three presses started, the last one still closed
    k.turn(-5);                  // back to where we started
    Count b = run(k, t, 3000);
    // the press in flight finishes (run() sees it as a fresh edge), then
    // exactly that many presses come back down: no new UP is ever started
    TEST_ASSERT_EQUAL(3, a.up);
    TEST_ASSERT_EQUAL(1, b.up);
    TEST_ASSERT_EQUAL(a.up, b.down);
    TEST_ASSERT_EQUAL(15, k.assumed());
}

void test_clamps_at_lo_and_hi() {
    Knob k;
    k.turn(-100);
    TEST_ASSERT_EQUAL(0, k.target());
    k.turn(+100);
    TEST_ASSERT_EQUAL(Scale::STEPS, k.target());
}

void test_home_presses_past_lo_then_climbs() {
    Knob k(20);
    uint32_t t = 1000;
    k.home();
    TEST_ASSERT_TRUE(k.syncing());
    Count c = run(k, t, 20000);
    TEST_ASSERT_EQUAL(Scale::STEPS + 2, c.down);
    TEST_ASSERT_EQUAL(20, c.up);
    TEST_ASSERT_FALSE(k.syncing());
    TEST_ASSERT_TRUE(k.settled());
}

void test_press_length() {
    Knob k;
    uint32_t t = 1000;
    k.turn(+1);
    uint32_t on = 0;
    for (uint32_t end = t + 1000; t < end; ++t)
        if (k.tick(t) == Knob::UP) on++;
    TEST_ASSERT_UINT32_WITHIN(1, Knob::PULSE_MS, on);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_scale);
    RUN_TEST(test_one_detent_one_press);
    RUN_TEST(test_fast_spin_is_paced);
    RUN_TEST(test_reverse_mid_spin_cancels);
    RUN_TEST(test_clamps_at_lo_and_hi);
    RUN_TEST(test_home_presses_past_lo_then_climbs);
    RUN_TEST(test_press_length);
    return UNITY_END();
}
