#include <Arduino.h>
#include <lvgl.h>
#include "sensors.h"
#include <Adafruit_ST7789.h>
#include <Adafruit_LvGL_Glue.h>
#include <SPI.h>

// Forward declarations for hardware-specific setup functions (to be implemented later)
void lvgl_setup();
void sensors_setup();
void sensors_update();

static lv_obj_t *label_egt;
static lv_obj_t *label_coolant;
static lv_obj_t *label_boost;

static Adafruit_ST7789 tft(/*CS=*/34, /*DC=*/37, /*RST=*/38);
static Adafruit_LvGL_Glue glue;

void setup() {
  Serial.begin(115200);
  lvgl_setup();
  sensors_setup();

  // Create a very simple UI – three text labels stacked vertically
  lv_obj_t *scr = lv_scr_act();

  label_egt = lv_label_create(scr);
  lv_label_set_text(label_egt, "EGT: --- F");
  lv_obj_align(label_egt, LV_ALIGN_TOP_MID, 0, 20);

  label_coolant = lv_label_create(scr);
  lv_label_set_text(label_coolant, "Coolant: --- F");
  lv_obj_align(label_coolant, LV_ALIGN_TOP_MID, 0, 60);

  label_boost = lv_label_create(scr);
  lv_label_set_text(label_boost, "Boost: --.- psi");
  lv_obj_align(label_boost, LV_ALIGN_TOP_MID, 0, 100);
}

void loop() {
  lv_timer_handler();     // Let LVGL handle refreshing
  sensors_update();       // Acquire sensor data and update labels
  delay(10);
}

void lvgl_setup() {
  lv_init();
  SPI.begin(36 /*SCK*/, -1 /*MISO*/, 35 /*MOSI*/);
  tft.init(320, 240);
  tft.setRotation(3);
  glue.begin(&tft, false, true);
}
