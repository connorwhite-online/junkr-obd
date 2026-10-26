#include <Arduino.h>

#include "can_protocol.h"
#include "can_bus.h"
#include "map_math.h"
#include "sensors.h"

namespace {

constexpr uint32_t BOOT_BARO_MS = 500;   // key-on window, before anyone cranks
constexpr uint32_t TAP_STALE_MS = 1000;  // rpm older than this is "unknown"

bay::BaroTracker baro;
float v5 = bay::V5_NOMINAL;
float aux_v = NAN;
uint32_t last_rpm_ms = 0;
uint16_t last_rpm = 0xFFFF;

uint16_t mv(float v) { return isnan(v) ? can_proto::U16_INVALID : can_proto::to_u16(v * 1000.0f); }

}  // namespace

void setup() {
  Serial.begin(115200);
  sensors_setup();
  if (!can_setup()) {
    Serial.println("[engine-bay] TWAI init failed");
  }

  v5 = read_v5_v();
  const uint32_t t0 = millis();
  while (millis() - t0 < BOOT_BARO_MS) {
    baro.boot_sample(bay::gm3bar_kpa(read_map_v(), v5));
    delay(10);
  }
  baro.finish_boot();
  Serial.printf("[engine-bay] baro %.1f kPa (%s)\n", baro.baro(), baro.latched() ? "latched" : "default");
}

void loop() {
  static uint32_t last_tx_ms = 0;
  static uint32_t last_slow_ms = 0;
  static uint32_t last_dbg_ms = 0;
  uint32_t now = millis();

  uint16_t rpm;
  if (can_poll_rpm(rpm)) {
    last_rpm = rpm;
    last_rpm_ms = now;
  }

  if (now - last_tx_ms < can_proto::TX_PERIOD_MS) {
    delay(1);
    return;
  }
  const float dt = (now - last_tx_ms) / 1000.0f;
  last_tx_ms = now;

  const float map_kpa = bay::gm3bar_kpa(read_map_v(), v5);
  const bool stopped = (now - last_rpm_ms < TAP_STALE_MS) && last_rpm < 50;
  baro.update(map_kpa, stopped, dt);

  const int16_t egt = read_egt_f();
  const float psi = bay::gauge_psi(map_kpa, baro.baro());
  can_send(can_proto::pack_i16(can_proto::ID_EGT, egt));
  can_send(can_proto::pack_i16(can_proto::ID_MAP_PSI, can_proto::to_i16(psi < 0 ? 0 : psi * 10.0f)));

  if (now - last_slow_ms >= can_proto::SLOW_PERIOD_MS) {
    last_slow_ms = now;
    v5 = read_v5_v();
    aux_v = read_aux_v();
    can_send(can_proto::pack(can_proto::MapAbs{
        can_proto::to_u16(map_kpa * 10.0f),
        can_proto::to_u16(baro.baro() * 10.0f),
        mv(v5),
        mv(aux_v),
    }));
  }

#ifdef DEBUG_PRINTS
  if (now - last_dbg_ms >= 1000) {
    last_dbg_ms = now;
    Serial.printf("EGT=%d F  MAP=%.1f kPa  baro=%.1f  boost=%.1f psi  5V=%.2f  aux=%.2f V\n",
                  egt, map_kpa, baro.baro(), psi, v5, aux_v);
  }
#else
  (void)last_dbg_ms;
#endif
}
