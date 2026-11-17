// EGT protection and wideband live tuning: pio test -e native -f test_exhaust
#include <math.h>
#include <unity.h>

#include "core/engine.h"
#include "core/sim.h"

using namespace ecu;

void setUp() {}
void tearDown() {}

// Engine on a dyno (speed held) with the exhaust sensors fed from the plant.
struct Bench : Sim::Sink {
    Calibration cal;
    Engine eng{cal};
    Sim sim;
    Sensors s;
    Commands cmd;
    bool egt_plugged = true, wb_plugged = true;
    float egt_max = 0;
    explicit Bench(SimParams p) : sim(cal, p) {
        cal.aux_use[0] = AuxUse::Egt;
        cal.aux_use[1] = AuxUse::Wideband;
    }
    void ne(uint32_t t) override { eng.on_ne(t); }
    void tdc(uint32_t t) override { eng.on_tdc(t); }
    SpvPlan plan() override { return eng.spv_plan(); }
    static float volts(float v, float v0, float y0, float v1, float y1) {
        return clampf(v0 + (v - y0) * (v1 - v0) / (y1 - y0), 0, 5);
    }
    void run(float sec) {
        int ms = (int)(sec * 1000);
        for (int i = 0; i < ms; i++) {
            for (int k = 0; k < 50; k++) sim.step(*this, 20);
            sim.map_kpa = s.map_kpa;
            s.aux_v[0] = egt_plugged ? volts(sim.egt_c(), cal.egt_v0, cal.egt_c0, cal.egt_v1, cal.egt_c1) : 0;
            s.aux_v[1] = wb_plugged ? volts(sim.lambda(), cal.wb_v0, cal.wb_l0, cal.wb_v1, cal.wb_l1) : 0;
            cmd = eng.update(sim.now_us(), s, 0.001f);
            sim.tcv_duty = cmd.tcv_duty;
            if (sim.egt_c() > egt_max) egt_max = sim.egt_c();
        }
    }
    // 2500 rpm, full pedal, 160 kPa: Q sits on the smoke limit.
    void pull(bool takeover) {
        if (takeover) {
            eng.set_mode(Mode::Takeover);
            s.arm_switch = true;
        }
        run(1.0f);
        s.pedal = 100; s.idl = false; s.va_v = 4.2f; s.map_kpa = 160;
    }
};

static SimParams dyno(float air = 1, float egt = 1) {
    SimParams p;
    p.inertia = 1e7f;
    p.air_scale = air;
    p.egt_gain = egt;
    return p;
}

static Bench *make(SimParams p) {
    Bench *b = new Bench(p);
    b->sim.set_rpm(2500);
    return b;
}

void test_unfitted_inputs_do_nothing() {
    Bench *b = make(dyno());
    b->cal.aux_use[0] = b->cal.aux_use[1] = AuxUse::None;
    b->pull(true);
    b->run(40);
    TEST_ASSERT_EQUAL_UINT32(0, b->cmd.dtc & (DTC_EGT | DTC_WIDEBAND));
    TEST_ASSERT_EQUAL_FLOAT(1, b->eng.exhaust().limit_scale());
    TEST_ASSERT_FALSE(b->eng.exhaust().learning());
    delete b;
}

void test_egt_reads_and_flags_unplugged() {
    Bench *b = make(dyno());
    b->pull(false);
    b->run(5);
    TEST_ASSERT_FLOAT_WITHIN(5, b->sim.egt_c(), b->eng.exhaust().egt_c());
    TEST_ASSERT_EQUAL_UINT32(0, b->cmd.dtc & DTC_EGT);
    b->egt_plugged = false;   // 100k pull-down in the pigtail: 0 V
    b->run(1);
    TEST_ASSERT_TRUE(b->cmd.dtc & DTC_EGT);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, b->cal.egt_fault_scale, b->eng.exhaust().egt_scale());
    delete b;
}

void test_egt_pulls_fuel_when_hot() {
    // Towing up a hill on a hot day: unprotected, EGT would run well past 750 C.
    Bench *hot = make(dyno(1, 1.4f));
    hot->cal.egt_derate_start_c = 5000;   // protection off, for comparison
    hot->cal.egt_derate_end_c = 5100;
    hot->pull(true);
    hot->run(20);
    float q_free = hot->cmd.q, egt_free = hot->sim.egt_c();
    delete hot;

    Bench *b = make(dyno(1, 1.4f));
    b->pull(true);
    b->run(20);
    TEST_ASSERT_TRUE(egt_free > 760);
    TEST_ASSERT_TRUE(b->egt_max < b->cal.egt_derate_end_c);
    TEST_ASSERT_TRUE(b->eng.exhaust().egt_limiting());
    TEST_ASSERT_TRUE(b->cmd.q < q_free - 2);
    TEST_ASSERT_TRUE(b->cmd.q > q_free * b->cal.egt_min_scale);   // still drivable
    delete b;
}

void test_wideband_trims_rich_smoke_limit() {
    // 15 % less air than the map was written for: lambda ~1.15 at the limit.
    Bench *b = make(dyno(0.85f));
    b->pull(true);
    b->run(30);
    float tgt = b->cal.lambda_target.at(2500);
    float trim = b->eng.exhaust().smoke_scale(160, 2500) - 1;
    TEST_ASSERT_FLOAT_WITHIN(0.03f, tgt, b->sim.lambda());
    TEST_ASSERT_TRUE(trim < -0.05f);
    TEST_ASSERT_TRUE(trim >= b->cal.trim_min - 1e-4f);
    TEST_ASSERT_EQUAL_UINT32(0, b->cmd.dtc & (DTC_EGT | DTC_WIDEBAND));
    // Only the cells around 160 kPa / 2500 rpm moved.
    TEST_ASSERT_EQUAL_FLOAT(0, b->eng.exhaust().trims().z[0][0]);
    delete b;
}

void test_wideband_adds_little_and_never_past_q_max() {
    // Plenty of air: it may lean in, but only up to trim_max.
    Bench *b = make(dyno(1.3f));
    b->pull(true);
    b->run(60);
    float trim = b->eng.exhaust().smoke_scale(160, 2500) - 1;
    TEST_ASSERT_TRUE(trim > 0.01f);
    TEST_ASSERT_TRUE(trim <= b->cal.trim_max + 1e-4f);
    TEST_ASSERT_TRUE(b->cmd.q <= b->cal.q_max.at(2500) + 0.01f);
    delete b;
}

void test_no_lean_trim_without_egt() {
    Bench *b = make(dyno(1.3f));
    b->egt_plugged = false;
    b->pull(true);
    b->run(40);
    TEST_ASSERT_TRUE(b->eng.exhaust().smoke_scale(160, 2500) <= 1.0f + 1e-5f);
    delete b;
}

void test_rich_floor_pulls_fuel_fast() {
    Bench *b = make(dyno(1));
    b->pull(true);
    b->run(30);   // past warm-up, settled
    float q0 = b->cmd.q;
    // Sudden loss of air (a boost leak): lambda ~1.0 at the old fuel.
    b->sim.params().air_scale = 0.72f;
    b->run(1.5f);
    TEST_ASSERT_TRUE(b->eng.exhaust().rich_cut() > 0 || b->eng.exhaust().smoke_scale(160, 2500) < 0.95f);
    TEST_ASSERT_TRUE(b->cmd.q < q0 * 0.92f);
    b->run(20);
    TEST_ASSERT_TRUE(b->sim.lambda() > b->cal.lambda_floor);
    delete b;
}

void test_shadow_mode_only_watches() {
    Bench *b = make(dyno(0.85f));
    b->pull(false);   // shadow: the stock ECU is fueling
    b->run(30);
    TEST_ASSERT_TRUE(b->eng.exhaust().wb_valid());
    TEST_ASSERT_FALSE(b->eng.exhaust().learning());
    TEST_ASSERT_EQUAL_FLOAT(1, b->eng.exhaust().smoke_scale(160, 2500));
    delete b;
}

void test_wideband_fault_after_warmup() {
    Bench *b = make(dyno());
    b->pull(true);
    b->run(30);
    TEST_ASSERT_EQUAL_UINT32(0, b->cmd.dtc & DTC_WIDEBAND);
    b->wb_plugged = false;
    b->run(3);
    TEST_ASSERT_TRUE(b->cmd.dtc & DTC_WIDEBAND);
    TEST_ASSERT_FALSE(b->eng.exhaust().wb_valid());
    delete b;
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_unfitted_inputs_do_nothing);
    RUN_TEST(test_egt_reads_and_flags_unplugged);
    RUN_TEST(test_egt_pulls_fuel_when_hot);
    RUN_TEST(test_wideband_trims_rich_smoke_limit);
    RUN_TEST(test_wideband_adds_little_and_never_past_q_max);
    RUN_TEST(test_no_lean_trim_without_egt);
    RUN_TEST(test_rich_floor_pulls_fuel_fast);
    RUN_TEST(test_shadow_mode_only_watches);
    RUN_TEST(test_wideband_fault_after_warmup);
    return UNITY_END();
}
