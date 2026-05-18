#include "can_rx.h"
#include "can_protocol.h"
#include "ui.h"

#include <Arduino.h>
#include "driver/twai.h"

namespace {
constexpr gpio_num_t TWAI_TX_PIN = GPIO_NUM_5;
constexpr gpio_num_t TWAI_RX_PIN = GPIO_NUM_6;
}

bool can_rx_setup() {
  twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_PIN, TWAI_RX_PIN, TWAI_MODE_NORMAL);
  twai_timing_config_t  t = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t  f = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g, &t, &f) != ESP_OK) return false;
  return twai_start() == ESP_OK;
}

void can_rx_poll() {
  twai_message_t msg;
  while (twai_receive(&msg, 0) == ESP_OK) {
    if (msg.data_length_code != 2) continue;
    int16_t value = (int16_t)((uint16_t)(msg.data[0] << 8) | msg.data[1]);

    switch (msg.identifier) {
      case can_proto::ID_EGT:     ui_set_egt(value);        break;
      case can_proto::ID_COOLANT: ui_set_coolant(value);    break;
      case can_proto::ID_MAP_PSI: ui_set_boost_x10(value);  break;
      default: break;
    }
  }
}
