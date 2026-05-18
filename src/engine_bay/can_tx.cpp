#include "can_tx.h"
#include "can_protocol.h"

#include <Arduino.h>
#include "driver/twai.h"

namespace {
constexpr gpio_num_t TWAI_TX_PIN = GPIO_NUM_0;
constexpr gpio_num_t TWAI_RX_PIN = GPIO_NUM_1;
}

bool can_tx_setup() {
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_PIN, TWAI_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t  t = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t  f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g, &t, &f) != ESP_OK) return false;
  return twai_start() == ESP_OK;
}

bool can_tx_send(uint32_t id, int16_t value) {
  twai_message_t msg = {};
  msg.identifier       = id;
  msg.data_length_code = 2;
  msg.data[0] = (uint8_t)((value >> 8) & 0xFF);  // big-endian
  msg.data[1] = (uint8_t)(value & 0xFF);
  return twai_transmit(&msg, pdMS_TO_TICKS(10)) == ESP_OK;
}
