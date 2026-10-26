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

constexpr float ADS_LSB_V = 4.096f / 32768.0f;  // GAIN_ONE = ±4.096 V FS
constexpr float DIVIDER   = 2.0f;               // every analog input has a 10k/10k divider

Adafruit_MAX31855 max_amp(PIN_MAX_SCK, PIN_MAX_CS, PIN_MAX_MISO);
Adafruit_ADS1115  ads;

float c_to_f(float c) { return c * 9.0f / 5.0f + 32.0f; }

float read_ch_v(uint8_t ch) {
  return ads.readADC_SingleEnded(ch) * ADS_LSB_V * DIVIDER;
}

}  // namespace

void sensors_setup() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_860SPS);  // ~1.2 ms per read instead of 8 ms
  ads.begin();
  max_amp.begin();
}

int16_t read_egt_f() {
  double c = max_amp.readCelsius();
  if (isnan(c)) return can_proto::SENTINEL_INVALID;
  return (int16_t)lround(c_to_f((float)c));
}

float read_map_v() { return read_ch_v(1); }
float read_aux_v() { return read_ch_v(2); }
float read_v5_v()  { return read_ch_v(3); }
