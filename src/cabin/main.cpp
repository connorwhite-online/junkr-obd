#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_LvGL_Glue.h>

#include "ui.h"
#include "can_rx.h"

namespace {
constexpr int PIN_TFT_CS  = 34;
constexpr int PIN_TFT_DC  = 37;
constexpr int PIN_TFT_RST = 38;
constexpr int PIN_SPI_SCK  = 36;
constexpr int PIN_SPI_MISO = 33;
constexpr int PIN_SPI_MOSI = 35;

Adafruit_ST7789 tft(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
Adafruit_LvGL_Glue glue;
}

void setup() {
  Serial.begin(115200);

  lv_init();
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);
  tft.init(240, 240);   // adjust to match the Qualia panel variant
  tft.setRotation(3);
  glue.begin(&tft, false, true);

  ui_init();

  if (!can_rx_setup()) {
    Serial.println("[cabin] TWAI init failed");
  }
}

void loop() {
  lv_timer_handler();
  can_rx_poll();
  delay(5);
}
