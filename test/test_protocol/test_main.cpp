// Wire format of every frame: pio test -e native
#include <math.h>
#include <unity.h>

#include "can_protocol.h"

using namespace can_proto;

void setUp() {}
void tearDown() {}

void test_big_endian_helpers() {
    uint8_t b[4];
    put_u16(b, 0x1234);
    TEST_ASSERT_EQUAL_HEX8(0x12, b[0]);
    TEST_ASSERT_EQUAL_HEX8(0x34, b[1]);
    put_i16(b, -2);
    TEST_ASSERT_EQUAL_INT16(-2, get_i16(b));
    put_u32(b, 0xDEADBEEF);
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEF, get_u32(b));
}

void test_legacy_frames_unchanged() {
    // 0x100 / 0x102 keep the original two-byte big-endian layout.
    Frame f = pack_i16(ID_EGT, 1080);
    TEST_ASSERT_EQUAL_UINT32(0x100, f.id);
    TEST_ASSERT_EQUAL_UINT8(2, f.len);
    TEST_ASSERT_EQUAL_HEX8(0x04, f.data[0]);
    TEST_ASSERT_EQUAL_HEX8(0x38, f.data[1]);
}

void test_rounding_and_sentinels() {
    TEST_ASSERT_EQUAL_INT16(SENTINEL_INVALID, to_i16(NAN));
    TEST_ASSERT_EQUAL_UINT16(U16_INVALID, to_u16(NAN));
    TEST_ASSERT_EQUAL_INT16(-3, to_i16(-2.6f));
    TEST_ASSERT_EQUAL_INT16(3, to_i16(2.5f));
    TEST_ASSERT_EQUAL_UINT16(0, to_u16(-5));
    TEST_ASSERT_EQUAL_UINT16(65534, to_u16(1e9f));   // never collides with the sentinel
    TEST_ASSERT_EQUAL_INT16(32767, to_i16(1e9f));
    TEST_ASSERT_EQUAL_INT16(-32767, to_i16(-1e9f));
}

void test_round_trips() {
    Frame f = pack(MapAbs{1872, 1013, 5004, U16_INVALID});
    TEST_ASSERT_EQUAL_UINT8(8, f.len);
    MapAbs m = unpack_map_abs(f.data);
    TEST_ASSERT_EQUAL_UINT16(1872, m.map_kpa_x10);
    TEST_ASSERT_EQUAL_UINT16(1013, m.baro_kpa_x10);
    TEST_ASSERT_EQUAL_UINT16(5004, m.v5_mv);
    TEST_ASSERT_EQUAL_UINT16(U16_INVALID, m.aux_mv);

    TapEngine e = unpack_tap_engine(pack(TapEngine{2350, -15, 480}).data);
    TEST_ASSERT_EQUAL_UINT16(2350, e.rpm);
    TEST_ASSERT_EQUAL_INT16(-15, e.timing_btdc_x10);
    TEST_ASSERT_EQUAL_UINT16(480, e.tcv_duty_x10);

    TapFuel fu = unpack_tap_fuel(pack(TapFuel{214, 620, 188}).data);
    TEST_ASSERT_EQUAL_UINT16(214, fu.spv_deg_x10);
    TEST_ASSERT_EQUAL_UINT16(620, fu.va_pct_x10);
    TEST_ASSERT_EQUAL_UINT16(188, fu.pim_kpa);

    TapTemps t = unpack_tap_temps(pack(TapTemps{89, -12, 40, SENTINEL_INVALID}).data);
    TEST_ASSERT_EQUAL_INT16(89, t.thw);
    TEST_ASSERT_EQUAL_INT16(-12, t.tha);
    TEST_ASSERT_EQUAL_INT16(40, t.thf);
    TEST_ASSERT_EQUAL_INT16(SENTINEL_INVALID, t.tho);

    TapStatus s = unpack_tap_status(pack(TapStatus{13900, 885, DIG_W_LAMP | DIG_LOCKUP | DIG_OD2}).data);
    TEST_ASSERT_EQUAL_UINT16(13900, s.vbat_mv);
    TEST_ASSERT_EQUAL_UINT16(885, s.speed_kmh_x10);
    TEST_ASSERT_EQUAL_HEX32(DIG_W_LAMP | DIG_LOCKUP | DIG_OD2, s.dig);

    AlertMsg a = unpack_alert(pack(AlertMsg{1, 690, 675}).data);
    TEST_ASSERT_EQUAL_UINT8(1, a.code);
    TEST_ASSERT_EQUAL_INT16(690, a.value);
    TEST_ASSERT_EQUAL_INT16(675, a.limit);
}

void test_piggyback_flags() {
    Frame f = pack(Piggyback{1, 196, 700, 88, 6}, true, 20);
    PiggybackRx p = unpack_piggyback(f.data);
    TEST_ASSERT_TRUE(p.armed);
    TEST_ASSERT_TRUE(p.learning);
    TEST_ASSERT_EQUAL_UINT8(20, p.authority_pct);
    TEST_ASSERT_EQUAL_UINT16(196, p.pim_seen_kpa);
    TEST_ASSERT_EQUAL_UINT16(700, p.va_seen_pct_x10);
    TEST_ASSERT_EQUAL_INT16(88, p.thw_seen);
    TEST_ASSERT_EQUAL_UINT8(6, p.extra_fuel_pct);

    p = unpack_piggyback(pack(Piggyback{0, 0, 0, 0, 0}, false, 8).data);
    TEST_ASSERT_FALSE(p.armed);
    TEST_ASSERT_FALSE(p.learning);
    TEST_ASSERT_EQUAL_UINT8(8, p.authority_pct);
}

void test_every_frame_meets_its_min_len() {
    const Frame fs[] = {pack_i16(ID_EGT, 0), pack_i16(ID_MAP_PSI, 0), pack(MapAbs{}), pack(TapEngine{}),
                        pack(TapFuel{}), pack(TapTemps{}), pack(Piggyback{}, false, 0), pack(TapStatus{}),
                        pack(AlertMsg{})};
    for (const Frame& f : fs) {
        TEST_ASSERT_TRUE(f.len <= 8);
        TEST_ASSERT_EQUAL_UINT8(min_len(f.id), f.len);
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_big_endian_helpers);
    RUN_TEST(test_legacy_frames_unchanged);
    RUN_TEST(test_rounding_and_sentinels);
    RUN_TEST(test_round_trips);
    RUN_TEST(test_piggyback_flags);
    RUN_TEST(test_every_frame_meets_its_min_len);
    return UNITY_END();
}
