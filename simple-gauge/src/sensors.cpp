#include "sensors.h"
#include <Arduino.h>
#include <Adafruit_MAX31855.h>
#include <Adafruit_ADS1X15.h>

// ---- Configuration ---- //
static const int PIN_MAX31855_CS = 10;      // SPI CS pin for EGT amp (adjust as needed)
static const int PIN_THERMISTOR = 1;        // ADC1_CH1
static const int PIN_PRESSURE = 2;          // ADC1_CH2

static Adafruit_MAX31855 max31855(PIN_MAX31855_CS);
static Adafruit_ADS1115 ads;

// Thermistor constants
static constexpr float SERIES_RESISTOR = 10000.0; // 10k pull-up
static constexpr float NOMINAL_RESISTANCE = 10000.0; // at 25 °C
static constexpr float NOMINAL_TEMPERATURE_C = 25.0;
static constexpr float B_COEFFICIENT = 3950.0;

// ADC settings
static constexpr float ADC_MAX = 4095.0;   // 12-bit
static constexpr float ADC_REF_V = 3.3;

void sensors_setup() {
  ads.begin();
  max31855.begin();
}

static float thermistor_temp_f() {
  int16_t raw = ads.readADC_SingleEnded(0); // Channel 0 thermistor
  float voltage = raw * (4.096 / 32767.0);

  // Calculate resistance of thermistor
  float resistance = SERIES_RESISTOR * (ADC_REF_V / voltage - 1.0);

  // Steinhart–Hart (simplified Beta form)
  float steinhart;
  steinhart = resistance / NOMINAL_RESISTANCE;      // (R/Ro)
  steinhart = log(steinhart);                       // ln(R/Ro)
  steinhart /= B_COEFFICIENT;                       // 1/B * ln(R/Ro)
  steinhart += 1.0 / (NOMINAL_TEMPERATURE_C + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                      // Invert
  float temp_c = steinhart - 273.15;                // K → °C
  return temp_c * 9.0 / 5.0 + 32.0;                 // °F
}

static float pressure_psi() {
  int16_t raw = ads.readADC_SingleEnded(1); // Channel 1 pressure sensor
  float voltage = raw * (4.096 / 32767.0);

  // MPX2200GP outputs 0.2–4.7 V over 0–29 psi (per datasheet)
  constexpr float V_ZERO = 0.2;
  constexpr float V_MAX = 4.7;
  float psi = (voltage - V_ZERO) * (29.0 / (V_MAX - V_ZERO));
  if (psi < 0) psi = 0;
  return psi;
}

void sensors_update() {
  // EGT
  double egt_c = max31855.readCelsius();
  double egt_f = egt_c * 9.0 / 5.0 + 32.0;
  lv_label_set_text_fmt(label_egt, "EGT: %.0f F", egt_f);

  // Coolant
  float coolant_f = thermistor_temp_f();
  lv_label_set_text_fmt(label_coolant, "Coolant: %.0f F", coolant_f);

  // Boost
  float psi = pressure_psi();
  lv_label_set_text_fmt(label_boost, "Boost: %.1f psi", psi);
}
