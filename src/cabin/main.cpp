#include <Arduino.h>
#include <lvgl.h>

#include "button.h"
#include "can_rx.h"
#include "display.h"
#include "gauge_state.h"
#include "ui.h"

#ifdef GAUGE_DEMO
#include "demo.h"
#endif

namespace {
// External momentary to ground on header pin A0, for the clock bezel.
// The Qualia's own DOWN button does the same on the bench.
constexpr int PIN_PAGE_BUTTON = A0;

constexpr uint32_t TICK_MS = 50;   // model + alerts, 20 Hz
constexpr uint32_t UI_MS = 100;    // label updates, 10 Hz

gauge::State state;
gauge::Button button;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PAGE_BUTTON, INPUT_PULLUP);

  if (!display_setup()) {
    Serial.println("[cabin] display init failed");
  }
  ui_init();

#ifdef GAUGE_DEMO
  Serial.println("[cabin] demo mode: synthetic bus traffic, CAN not started");
#else
  if (!can_rx_setup()) {
    Serial.println("[cabin] TWAI init failed");
  }
#endif
}

void loop() {
  static uint32_t last_tick = 0, last_ui = 0, last_btn = 0;
  const uint32_t now = millis();

#ifdef GAUGE_DEMO
  static uint32_t last_demo = 0;
  if (now - last_demo >= TICK_MS) {
    last_demo = now;
    can_proto::Frame f[8];
    const int n = demo::frames(now, f);
    for (int i = 0; i < n; ++i) state.ingest(f[i].id, f[i].data, f[i].len, now);
  }
#else
  can_rx_poll(state, now);
#endif

  if (now - last_btn >= 10) {   // expander read is an I2C transaction
    last_btn = now;
    const bool pressed = digitalRead(PIN_PAGE_BUTTON) == LOW || display_button_down();
    switch (button.update(pressed, now)) {
      case gauge::Button::SHORT: state.short_press(now); last_ui = 0; break;
      case gauge::Button::LONG: state.long_press(); last_ui = 0; break;
      default: break;
    }
  }

  if (now - last_tick >= TICK_MS) {
    last_tick = now;
    state.tick(now);
  }
  if (now - last_ui >= UI_MS) {
    last_ui = now;
    ui_update(state, now);
  }
  lv_timer_handler();
  delay(2);
}
