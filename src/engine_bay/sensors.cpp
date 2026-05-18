#include "sensors.h"
#include "can_protocol.h"

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_MAX31855.h>
#include <Adafruit_ADS1X15.h>

namespace {

constexpr int PIN_MAX_SCK  = 4;
constexpr int PIN_MAX_MISO = 5;
constexpr int PIN_MAX_CS   = 10;
constexpr int PIN_I2C_SDA  = 6;
constexpr int PIN_I2C_SCL  = 7;

constexpr float SERIES_RESISTOR    = 10000.0f;
constexpr float NOMINAL_RESISTANCE = 10000.0f;
constexpr float NOMINAL_TEMP_C     = 25.0f;
constexpr float B_COEFFICIENT      = 3950.0f;
constexpr float VREF               = 3.3f;
constexpr float ADS_LSB_V          = 4.096f / 32767.0f;  // GAIN_ONE = ±4.096 V FS

constexpr float MAP_V_ZERO   = 0.2f;
constexpr float MAP_V_MAX    = 4.7f;
constexpr float MAP_PSI_MAX  = 29.0f;

Adafruit_MAX31855 max_amp(PIN_MAX_SCK, PIN_MAX_CS, PIN_MAX_MISO);
Adafruit_ADS1115  ads;

float c_to_f(float c) { return c * 9.0f / 5.0f + 32.0f; }

}  // namespace

void sensors_setup() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  ads.setGain(GAIN_ONE);
  ads.begin();
  max_amp.begin();
}

int16_t read_egt_f() {
  double c = max_amp.readCelsius();
  if (isnan(c)) return can_proto::SENTINEL_INVALID;
  return (int16_t)lround(c_to_f((float)c));
}

int16_t read_coolant_f() {
  int16_t raw = ads.readADC_SingleEnded(0);
  float v = raw * ADS_LSB_V;
  if (v <= 0.05f || v >= (VREF - 0.05f)) return can_proto::SENTINEL_INVALID;

  float resistance = SERIES_RESISTOR * (VREF / v - 1.0f);
  float steinhart  = logf(resistance / NOMINAL_RESISTANCE) / B_COEFFICIENT
                   + 1.0f / (NOMINAL_TEMP_C + 273.15f);
  float temp_c = 1.0f / steinhart - 273.15f;
  return (int16_t)lroundf(c_to_f(temp_c));
}

int16_t read_map_psi_x10() {
  int16_t raw = ads.readADC_SingleEnded(1);
  float v = raw * ADS_LSB_V;
  float psi = (v - MAP_V_ZERO) * (MAP_PSI_MAX / (MAP_V_MAX - MAP_V_ZERO));
  if (psi < 0.0f) psi = 0.0f;
  return (int16_t)lroundf(psi * 10.0f);
}
