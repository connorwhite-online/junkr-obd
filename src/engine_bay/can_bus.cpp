#include "can_bus.h"

#include <Arduino.h>
#include "driver/twai.h"

namespace {
constexpr gpio_num_t TWAI_TX_PIN = GPIO_NUM_0;
constexpr gpio_num_t TWAI_RX_PIN = GPIO_NUM_1;
}

bool can_setup() {
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_PIN, TWAI_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t  t = TWAI_TIMING_CONFIG_500KBITS();
  // Only the tap's engine frame is of interest (rpm, for the baro latch).
  twai_filter_config_t  f = {
      .acceptance_code = can_proto::ID_TAP_ENGINE << 21,
      .acceptance_mask = ~(0x7FFu << 21),
      .single_filter = true,
  };

  if (twai_driver_install(&g, &t, &f) != ESP_OK) return false;
  return twai_start() == ESP_OK;
}

bool can_send(const can_proto::Frame& f) {
  twai_message_t msg = {};
  msg.identifier       = f.id;
  msg.data_length_code = f.len;
  memcpy(msg.data, f.data, f.len);
  return twai_transmit(&msg, pdMS_TO_TICKS(10)) == ESP_OK;
}

bool can_poll_rpm(uint16_t& rpm) {
  bool got = false;
  twai_message_t msg;
  while (twai_receive(&msg, 0) == ESP_OK) {
    if (msg.identifier != can_proto::ID_TAP_ENGINE) continue;
    if (msg.data_length_code < can_proto::min_len(msg.identifier)) continue;
    rpm = can_proto::unpack_tap_engine(msg.data).rpm;
    got = true;
  }
  return got;
}
