#include "can_rx.h"
#include "can_protocol.h"

#include <Arduino.h>
#include "driver/twai.h"

namespace {
// Qualia header pins SCK / MISO, wired to the cabin shield's TJA1051.
constexpr gpio_num_t TWAI_TX_PIN = GPIO_NUM_5;
constexpr gpio_num_t TWAI_RX_PIN = GPIO_NUM_6;
}

bool can_rx_setup() {
  // Normal mode, not listen-only: the gauge never sends a frame, but on a
  // two-node bench bus its ACK bit is the only thing that keeps the
  // engine-bay node from retrying forever.
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_PIN, TWAI_RX_PIN, TWAI_MODE_NORMAL);
  g.rx_queue_len = 64;
  twai_timing_config_t  t = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t  f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g, &t, &f) != ESP_OK) return false;
  return twai_start() == ESP_OK;
}

void can_rx_poll(gauge::State& s, uint32_t now) {
  twai_message_t msg;
  while (twai_receive(&msg, 0) == ESP_OK) {
    if (msg.extd || msg.rtr) continue;
    s.ingest(msg.identifier, msg.data, msg.data_length_code, now);
  }
}
