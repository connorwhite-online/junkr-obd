// jlog format and the ECU telemetry stream: pio test -e native -f test_log
//
// Set JLOG_OUT=path to also write the simulated drive as a .jlog file
// (that is how test/fixtures/sim_drive.jlog, used by tools/test_junkrlog.py,
// is made).
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unity.h>

#include <map>
#include <string>
#include <vector>

#include "core/engine.h"
#include "core/sim.h"
#include "core/telemetry.h"
#include "ecu_log.h"

using namespace ecu;

void setUp() {}
void tearDown() {}

struct VecTx : LinkTx {
    std::vector<uint8_t> bytes;
    size_t cap = 1 << 30;     // room per poll (models the UART buffer)
    size_t used = 0;
    size_t room() override { return cap - used; }
    void write(const uint8_t *p, size_t n) override {
        bytes.insert(bytes.end(), p, p + n);
        used += n;
    }
};

// Parsed view of a stream, the way a decoder sees it.
struct Decoded {
    struct Col { std::string name, unit; char type; float scale; };
    std::vector<Col> cols;
    std::vector<std::vector<double>> samples;   // [i][0] = t_us, then channels
    std::vector<jlog::Injection> inj;
    std::vector<std::pair<uint8_t, uint32_t>> events;
    std::string meta;
    uint32_t bad = 0, frames = 0;

    void feed(const std::vector<uint8_t> &b) {
        jlog::Parser p;
        for (uint8_t c : b) {
            if (!p.push(c)) continue;
            frames++;
            const uint8_t *d = p.payload();
            switch (p.type()) {
                case jlog::T_SCHEMA: {
                    cols.clear();
                    size_t k = 2;
                    for (int i = 0; i < d[1]; i++) {
                        Col col;
                        col.type = (char)d[k++];
                        col.scale = jlog::getf(d + k); k += 4;
                        col.name = (const char *)d + k; k += col.name.size() + 1;
                        col.unit = (const char *)d + k; k += col.unit.size() + 1;
                        cols.push_back(col);
                    }
                    break;
                }
                case jlog::T_SAMPLE: {
                    std::vector<double> row{(double)jlog::get32(d)};
                    size_t k = 8;
                    for (auto &col : cols) {
                        double v;
                        switch (col.type) {
                            case 'h': v = (int16_t)jlog::get16(d + k); break;
                            case 'H': v = jlog::get16(d + k); break;
                            case 'i': v = (int32_t)jlog::get32(d + k); break;
                            default: v = jlog::get32(d + k); break;
                        }
                        k += jlog::type_size(col.type);
                        row.push_back(v / col.scale);
                    }
                    samples.push_back(row);
                    break;
                }
                case jlog::T_INJ: {
                    jlog::Injection r;
                    r.t_us = jlog::get32(d); r.period_us = jlog::get32(d + 4); r.segment = jlog::get32(d + 8);
                    r.q = jlog::get16(d + 12) / 100.0f;
                    r.inject = d[20];
                    inj.push_back(r);
                    break;
                }
                case jlog::T_EVENT: events.push_back({d[4], jlog::get32(d + 5)}); break;
                case jlog::T_META: meta.assign((const char *)d, p.len()); break;
            }
        }
        bad = p.bad();
    }
    int col(const char *name) const {
        for (size_t i = 0; i < cols.size(); i++)
            if (cols[i].name == name) return (int)i + 1;
        return -1;
    }
};

// Simulated truck with the telemetry wired in like the STM32 HAL does it.
struct Bench : Sim::Sink {
    Calibration cal;
    Engine eng{cal};
    Sim sim;
    Sensors s;
    HalStatus hal;
    VecTx tx;
    Telemetry tel{tx, "test"};
    Bench() : sim(cal) {}
    void ne(uint32_t t) override {
        eng.on_ne(t);
        if (eng.ne().synced() && eng.ne().tooth_index() == 0) tel.on_segment(eng);
    }
    void tdc(uint32_t t) override { eng.on_tdc(t); }
    SpvPlan plan() override { return eng.spv_plan(); }
    void run(float sec) {
        int ms = (int)(sec * 1000);
        for (int i = 0; i < ms; i++) {
            for (int k = 0; k < 50; k++) sim.step(*this, 20);
            sim.starter = s.starter;
            const Commands &c = eng.update(sim.now_us(), s, 0.001f);
            sim.tcv_duty = c.tcv_duty;
            tx.used = 0;   // the UART drains between loops
            tel.poll(sim.now_us(), eng, cal, s, hal);
        }
    }
    void drive() {
        s.starter = true; run(1.5f);
        s.starter = false; run(3.0f);
        s.pedal = 40; s.idl = false; s.va_v = 2.0f; run(3.0f);
        s.pedal = 0; s.idl = true; s.va_v = 0.8f; run(2.0f);
    }
};

void test_frame_roundtrip_and_crc() {
    uint8_t pay[40], f[64];
    for (int i = 0; i < 40; i++) pay[i] = (uint8_t)(i * 7);
    size_t n = jlog::frame(f, sizeof f, jlog::T_EVENT, pay, 40);
    TEST_ASSERT_EQUAL(47, n);
    // Garbage, a good frame, a corrupted copy, a good frame.
    std::vector<uint8_t> s = {0x00, 0xA5, 0x13, 0xA5};
    s.insert(s.end(), f, f + n);
    std::vector<uint8_t> bad(f, f + n);
    bad[20] ^= 0x40;
    s.insert(s.end(), bad.begin(), bad.end());
    s.insert(s.end(), f, f + n);
    jlog::Parser p;
    int good = 0;
    for (uint8_t c : s)
        if (p.push(c)) {
            good++;
            TEST_ASSERT_EQUAL(jlog::T_EVENT, p.type());
            TEST_ASSERT_EQUAL(40, p.len());
            TEST_ASSERT_EQUAL_UINT8_ARRAY(pay, p.payload(), 40);
        }
    TEST_ASSERT_EQUAL(2, good);
    TEST_ASSERT_EQUAL(1, p.bad());
    // Known CRC-16/CCITT-FALSE check value.
    TEST_ASSERT_EQUAL_HEX16(0x29B1, jlog::crc16((const uint8_t *)"123456789", 9));
    TEST_ASSERT_EQUAL_HEX32(0xCBF43926, jlog::crc32("123456789", 9));
}

void test_sample_packing() {
    jlog::Sample x;
    x.begin(1234, 5);
    x.set(jlog::C_RPM, 2512.4f);
    x.set(jlog::C_ECT, -12.34f);
    x.set(jlog::C_Q, 23.456f);
    x.set(jlog::C_PEDAL, 150.0f * 1000);   // clamps, never wraps
    x.set_u32(jlog::C_FLAGS, 0x80000001u);
    TEST_ASSERT_EQUAL(jlog::sample_payload_size(), x.size());
    static uint8_t f[jlog::MAX_PAYLOAD + 8];
    uint8_t sch[jlog::MAX_PAYLOAD];
    size_t sn = jlog::schema_payload(sch, sizeof sch);
    TEST_ASSERT_TRUE(sn > 0);
    std::vector<uint8_t> all(f, f + jlog::frame(f, sizeof f, jlog::T_SCHEMA, sch, sn));
    std::vector<uint8_t> sf(300);
    sf.resize(jlog::frame(sf.data(), sf.size(), jlog::T_SAMPLE, x.data(), x.size()));
    all.insert(all.end(), sf.begin(), sf.end());
    Decoded d;
    d.feed(all);
    TEST_ASSERT_EQUAL(1, d.samples.size());
    auto &r = d.samples[0];
    TEST_ASSERT_EQUAL_FLOAT(1234, r[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.6, 2512, r[d.col("rpm")]);
    TEST_ASSERT_FLOAT_WITHIN(0.06, -12.3, r[d.col("ect")]);
    TEST_ASSERT_FLOAT_WITHIN(0.006, 23.46, r[d.col("q")]);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 655.35, r[d.col("pedal")]);
    TEST_ASSERT_EQUAL_FLOAT((double)0x80000001u, r[d.col("flags")]);
}

void test_drive_stream_is_complete() {
    Bench b;
    b.drive();
    Decoded d;
    d.feed(b.tx.bytes);
    TEST_ASSERT_EQUAL(0, d.bad);
    TEST_ASSERT_EQUAL(0, b.tel.dropped());
    TEST_ASSERT_EQUAL(jlog::C_COUNT, d.cols.size());
    TEST_ASSERT_TRUE(d.meta.find("cal_crc=") != std::string::npos);
    // 9.5 s at 100 Hz.
    TEST_ASSERT_INT_WITHIN(2, 950, d.samples.size());
    // Sample times step by 10 ms.
    for (size_t i = 1; i < d.samples.size(); i++)
        TEST_ASSERT_UINT32_WITHIN(1000, 10000, (uint32_t)(d.samples[i][0] - d.samples[i - 1][0]));
    // One injection record per segment, none missing.
    TEST_ASSERT_TRUE(d.inj.size() > 200);
    for (size_t i = 1; i < d.inj.size(); i++) TEST_ASSERT_EQUAL_UINT32(d.inj[i - 1].segment + 1, d.inj[i].segment);
    // The logged rpm follows the engine: idle, then pedal up.
    int rpm = d.col("rpm"), ped = d.col("pedal");
    double idle = d.samples[440][rpm], revs = d.samples[740][rpm];
    TEST_ASSERT_TRUE(idle > 500 && idle < 1000);
    TEST_ASSERT_TRUE(revs > idle + 300);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 40, d.samples[740][ped]);
    // The injection record's crank period matches rpm.
    const jlog::Injection &last = d.inj.back();
    TEST_ASSERT_FLOAT_WITHIN(60, b.eng.ne().rpm(), 30.0e6 / last.period_us);
    // Boot event first.
    TEST_ASSERT_TRUE(!d.events.empty());
    TEST_ASSERT_EQUAL(jlog::E_BOOT, d.events[0].first);

    if (const char *out = getenv("JLOG_OUT")) {
        FILE *f = fopen(out, "wb");
        TEST_ASSERT_NOT_NULL(f);
        fwrite(b.tx.bytes.data(), 1, b.tx.bytes.size(), f);
        fclose(f);
    }
}

void test_slow_link_drops_but_never_blocks() {
    Bench b;
    b.tx.cap = 60;   // less than one sample frame per loop
    b.s.starter = true;
    b.run(1.5f);
    TEST_ASSERT_TRUE(b.tel.dropped() > 0);
    // Whatever made it through is still clean.
    Decoded d;
    d.feed(b.tx.bytes);
    TEST_ASSERT_EQUAL(0, d.bad);
}

void test_events_on_change() {
    Bench b;
    b.run(0.05f);
    b.eng.set_mode(Mode::Takeover);
    b.hal.arm = true;
    b.run(0.05f);
    Decoded d;
    d.feed(b.tx.bytes);
    bool mode = false, arm = false;
    for (auto &e : d.events) {
        if (e.first == jlog::E_MODE && e.second == 1) mode = true;
        if (e.first == jlog::E_ARM && e.second == 1) arm = true;
    }
    TEST_ASSERT_TRUE(mode);
    TEST_ASSERT_TRUE(arm);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_frame_roundtrip_and_crc);
    RUN_TEST(test_sample_packing);
    RUN_TEST(test_drive_stream_is_complete);
    RUN_TEST(test_slow_link_drops_but_never_blocks);
    RUN_TEST(test_events_on_change);
    return UNITY_END();
}
