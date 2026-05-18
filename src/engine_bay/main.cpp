#include <Arduino.h>

#include "can_protocol.h"
#include "sensors.h"
#include "can_tx.h"

void setup() {
  Serial.begin(115200);
  sensors_setup();
  if (!can_tx_setup()) {
    Serial.println("[engine-bay] TWAI init failed");
  }
}

void loop() {
  static uint32_t last_tx_ms = 0;
  uint32_t now = millis();

  if (now - last_tx_ms < can_proto::TX_PERIOD_MS) {
    delay(1);
    return;
  }
  last_tx_ms = now;

  int16_t egt        = read_egt_f();
  int16_t coolant    = read_coolant_f();
  int16_t map_psi10  = read_map_psi_x10();

  can_tx_send(can_proto::ID_EGT,     egt);
  can_tx_send(can_proto::ID_COOLANT, coolant);
  can_tx_send(can_proto::ID_MAP_PSI, map_psi10);
}
