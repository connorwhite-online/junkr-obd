#include "sensors.h"
#include <Arduino.h>
#include <Adafruit_MAX31855.h>
#include <Adafruit_ADS1X15.h>

static const int PIN_MAX31855_CS = 10;
static Adafruit_MAX31855 max31855(PIN_MAX31855_CS);
static Adafruit_ADS1115 ads;

static constexpr float SERIES_RESISTOR = 10000.0;
static constexpr float NOMINAL_RESISTANCE = 10000.0;
static constexpr float NOMINAL_TEMPERATURE_C = 25.0;
static constexpr float B_COEFFICIENT = 3950.0;

static float thermistor_temp_f() {
  int16_t raw = ads.readADC_SingleEnded(0);
  float voltage = raw * (4.096 / 32767.0);
  float resistance = SERIES_RESISTOR * (3.3 / voltage - 1.0);
  float steinhart = log(resistance / NOMINAL_RESISTANCE) / B_COEFFICIENT + 1.0 / (NOMINAL_TEMPERATURE_C + 273.15);
  steinhart = 1.0 / steinhart - 273.15;
  return steinhart * 9.0 / 5.0 + 32.0;
}

static float pressure_psi() {
  int16_t raw = ads.readADC_SingleEnded(1);
  float voltage = raw * (4.096 / 32767.0);
  constexpr float V_ZERO = 0.2;
  constexpr float V_MAX = 4.7;
  float psi = (voltage - V_ZERO) * (29.0 / (V_MAX - V_ZERO));
  if (psi < 0) psi = 0;
  return psi;
}

void sensors_setup() {
  ads.begin();
  max31855.begin();
}

void sensors_update() {
  double egt_f = max31855.readCelsius() * 9.0 / 5.0 + 32.0;
  lv_label_set_text_fmt(label_egt, "EGT: %.0f F", egt_f);
  lv_label_set_text_fmt(label_coolant, "Coolant: %.0f F", thermistor_temp_f());
  lv_label_set_text_fmt(label_boost, "Boost: %.1f psi", pressure_psi());
}
