/*
 * Sensor Library - Implementation
 * 
 * Reads temperature sensors (NTC thermistors), pressure sensors (MAP),
 * and EGT sensor (MAX31855 thermocouple amplifier)
 */

#include "sensors.h"
#include "config.h"
#include <SPI.h>
#include <math.h>

// ============================================================================
// SENSOR STATE VARIABLES
// ============================================================================

// Current sensor readings (filtered)
static float intakeTempPre = 25.0;   // Pre-intercooler
static float intakeTempPost = 25.0;  // Post-intercooler
static float exhaustTemp = 0.0;
static float coolantTemp = 25.0;
static float boostPressureBar = 1.0;  // Absolute pressure

// Previous readings (for filtering)
static float prevIntakeTempPre = 25.0;
static float prevIntakeTempPost = 25.0;
static float prevExhaustTemp = 0.0;
static float prevCoolantTemp = 25.0;
static float prevBoostPressure = 1.0;

// Sensor status
static uint16_t faultFlags = 0;
static bool egtAvailable = false;

// Calibration offsets
static float iatPreOffset = 0.0;
static float iatPostOffset = 0.0;
static float boostOffset = 0.0;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Exponential Moving Average filter
 * Smooths sensor readings to reduce noise
 */
float emaFilter(float newValue, float prevValue, float alpha) {
  return (alpha * newValue) + ((1.0 - alpha) * prevValue);
}

/**
 * Read analog pin with multiple samples and averaging
 * Reduces ADC noise
 */
int readAnalogAveraged(int pin, int samples) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);  // Small delay between samples
  }
  return sum / samples;
}

/**
 * Convert ADC reading to voltage
 */
float adcToVoltage(int adcValue) {
  return (adcValue / (float)ADC_RESOLUTION) * ADC_VREF;
}

/**
 * Convert voltage divider reading to resistance
 * Used for NTC thermistors
 */
float voltageToResistance(float voltage, float refResistor) {
  if (voltage >= ADC_VREF - 0.01) return 999999.0;  // Open circuit
  if (voltage <= 0.01) return 0.0;  // Short circuit
  
  // Voltage divider: Vout = Vin * (R2 / (R1 + R2))
  // Solving for R2 (thermistor): R2 = (Vout * R1) / (Vin - Vout)
  return (voltage * refResistor) / (ADC_VREF - voltage);
}

/**
 * Convert NTC thermistor resistance to temperature using Beta equation
 * More accurate than simple lookup table
 */
float resistanceToTemperature(float resistance) {
  // Steinhart-Hart Beta parameter equation:
  // 1/T = 1/T0 + (1/B) * ln(R/R0)
  // Where: T = temperature (Kelvin), T0 = 298.15K (25°C)
  //        R = measured resistance, R0 = nominal resistance
  //        B = Beta coefficient
  
  if (resistance <= 0.0 || resistance > 100000.0) {
    return -999.0;  // Invalid reading
  }
  
  float steinhart;
  steinhart = resistance / THERMISTOR_NOMINAL_R;     // (R/R0)
  steinhart = log(steinhart);                        // ln(R/R0)
  steinhart /= THERMISTOR_BETA;                      // 1/B * ln(R/R0)
  steinhart += 1.0 / (THERMISTOR_NOMINAL_TEMP + 273.15); // + 1/T0
  steinhart = 1.0 / steinhart;                       // Invert
  steinhart -= 273.15;                               // Convert to Celsius
  
  return steinhart;
}

/**
 * Read NTC thermistor and return temperature
 */
float readThermistor(int pin, float prevValue) {
  // Read ADC with averaging
  int adcValue = readAnalogAveraged(pin, ANALOG_SAMPLES);
  
  // Convert to voltage
  float voltage = adcToVoltage(adcValue);
  
  // Convert to resistance
  float resistance = voltageToResistance(voltage, THERMISTOR_REF_RESISTANCE);
  
  // Convert to temperature
  float temperature = resistanceToTemperature(resistance);
  
  // Check for sensor fault
  if (temperature < TEMP_MIN_VALID || temperature > TEMP_MAX_VALID) {
    return prevValue;  // Keep previous value if reading is invalid
  }
  
  // Apply EMA filter for smoothing
  return emaFilter(temperature, prevValue, FILTER_ALPHA);
}

/**
 * Read MAP/Boost pressure sensor
 */
float readBoostPressure(float prevValue) {
  // Read ADC with averaging
  int adcValue = readAnalogAveraged(PIN_BOOST_PRESSURE, ANALOG_SAMPLES);
  
  // Convert to voltage
  float voltage = adcToVoltage(adcValue);
  
  // Map voltage to pressure (linear)
  float pressure = MAP_PRESSURE_MIN + 
                   ((voltage - MAP_VOLTAGE_MIN) / (MAP_VOLTAGE_MAX - MAP_VOLTAGE_MIN)) * 
                   (MAP_PRESSURE_MAX - MAP_PRESSURE_MIN);
  
  // Check for sensor fault
  if (pressure < PRESSURE_MIN_VALID || pressure > PRESSURE_MAX_VALID) {
    faultFlags |= FAULT_BOOST_SENSOR;
    return prevValue;  // Keep previous value
  } else {
    faultFlags &= ~FAULT_BOOST_SENSOR;
  }
  
  // Apply calibration offset
  pressure += boostOffset;
  
  // Apply EMA filter
  return emaFilter(pressure, prevValue, FILTER_ALPHA);
}

/**
 * Read EGT from MAX31855 K-type thermocouple amplifier
 * Returns temperature in Celsius
 */
float readEGT(float prevValue) {
  // Select MAX31855 chip
  digitalWrite(PIN_EGT_CS, LOW);
  delayMicroseconds(10);
  
  // Read 32-bit data
  uint32_t data = 0;
  for (int i = 0; i < 4; i++) {
    data <<= 8;
    data |= SPI.transfer(0x00);
  }
  
  // Deselect chip
  digitalWrite(PIN_EGT_CS, HIGH);
  
  // Check for faults (bit 16)
  if (data & 0x04) {
    // Fault detected (no thermocouple connected)
    faultFlags |= FAULT_EGT_SENSOR;
    egtAvailable = false;
    return 0.0;
  }
  
  // No fault
  faultFlags &= ~FAULT_EGT_SENSOR;
  egtAvailable = true;
  
  // Extract temperature data (bits 31-18)
  // Temperature is in 0.25°C increments
  int16_t temp = (data >> 18) & 0x3FFF;
  
  // Check sign bit (bit 13 of the 14-bit value)
  if (temp & 0x2000) {
    // Negative temperature (sign extend)
    temp |= 0xC000;
  }
  
  // Convert to Celsius (0.25°C per bit)
  float temperature = temp * 0.25;
  
  // Validate range
  if (temperature < TEMP_MIN_VALID || temperature > TEMP_MAX_VALID) {
    return prevValue;
  }
  
  // Apply EMA filter (slower for EGT due to thermal mass)
  return emaFilter(temperature, prevValue, FILTER_ALPHA * 0.5);
}


// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

void Sensors_Init() {
  // Configure analog input pins
  pinMode(PIN_IAT_PRE_IC, INPUT);
  pinMode(PIN_IAT_POST_IC, INPUT);
  pinMode(PIN_COOLANT_TEMP, INPUT);
  pinMode(PIN_BOOST_PRESSURE, INPUT);
  
  // Configure SPI for MAX31855
  pinMode(PIN_EGT_CS, OUTPUT);
  digitalWrite(PIN_EGT_CS, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);  // Slow SPI clock for MAX31855
  
  // Set ADC reference to default (5V on Mega)
  analogReference(DEFAULT);
  
  // Initial sensor reading
  delay(100);
  Sensors_Update();
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Sensors: Initialized"));
  if (egtAvailable) {
    Serial.println(F("  - EGT sensor detected"));
  } else {
    Serial.println(F("  - WARNING: EGT sensor not detected"));
  }
  #endif
}

void Sensors_Update() {
  // Read all temperature sensors
  intakeTempPre = readThermistor(PIN_IAT_PRE_IC, prevIntakeTempPre);
  intakeTempPost = readThermistor(PIN_IAT_POST_IC, prevIntakeTempPost);
  coolantTemp = readThermistor(PIN_COOLANT_TEMP, prevCoolantTemp);
  
  // Read EGT
  exhaustTemp = readEGT(prevExhaustTemp);
  
  // Read boost pressure
  boostPressureBar = readBoostPressure(prevBoostPressure);
  
  // Update previous values for next iteration
  prevIntakeTempPre = intakeTempPre;
  prevIntakeTempPost = intakeTempPost;
  prevCoolantTemp = coolantTemp;
  prevExhaustTemp = exhaustTemp;
  prevBoostPressure = boostPressureBar;
  
  // Check for sensor-specific faults
  if (intakeTempPre < TEMP_MIN_VALID || intakeTempPre > TEMP_MAX_VALID) {
    faultFlags |= FAULT_IAT_PRE_SENSOR;
  } else {
    faultFlags &= ~FAULT_IAT_PRE_SENSOR;
  }
  
  if (intakeTempPost < TEMP_MIN_VALID || intakeTempPost > TEMP_MAX_VALID) {
    faultFlags |= FAULT_IAT_POST_SENSOR;
  } else {
    faultFlags &= ~FAULT_IAT_POST_SENSOR;
  }
  
  if (coolantTemp < TEMP_MIN_VALID || coolantTemp > TEMP_MAX_VALID) {
    faultFlags |= FAULT_COOLANT_SENSOR;
  } else {
    faultFlags &= ~FAULT_COOLANT_SENSOR;
  }
}

// ============================================================================
// GETTER FUNCTIONS
// ============================================================================

float Sensors_GetIntakeTempPre() {
  return intakeTempPre + iatPreOffset;
}

float Sensors_GetIntakeTempPost() {
  return intakeTempPost + iatPostOffset;
}

float Sensors_GetExhaustTemp() {
  return exhaustTemp;
}

float Sensors_GetCoolantTemp() {
  return coolantTemp;
}

float Sensors_GetBoostBar() {
  // Return gauge pressure (subtract atmospheric)
  return boostPressureBar - 1.0;
}

float Sensors_GetBoostPSI() {
  // Convert gauge pressure to PSI
  return (boostPressureBar - 1.0) * BAR_TO_PSI;
}

float Sensors_GetBoostAbsolute() {
  return boostPressureBar;
}


// ============================================================================
// STATUS FUNCTIONS
// ============================================================================

bool Sensors_IsFaulted(uint16_t sensorFlag) {
  return (faultFlags & sensorFlag) != 0;
}

uint16_t Sensors_GetFaultFlags() {
  return faultFlags;
}

bool Sensors_IsEGTAvailable() {
  return egtAvailable;
}

void Sensors_ClearFaults() {
  faultFlags = 0;
}

// ============================================================================
// CALIBRATION FUNCTIONS
// ============================================================================

void Sensors_SetIATPreOffset(float offset) {
  iatPreOffset = offset;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.print(F("Sensors: IAT Pre-IC offset set to "));
  Serial.println(offset);
  #endif
}

void Sensors_SetIATPostOffset(float offset) {
  iatPostOffset = offset;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.print(F("Sensors: IAT Post-IC offset set to "));
  Serial.println(offset);
  #endif
}

void Sensors_SetBoostOffset(float offset) {
  boostOffset = offset;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.print(F("Sensors: Boost offset set to "));
  Serial.println(offset);
  #endif
}

void Sensors_ResetCalibration() {
  iatPreOffset = 0.0;
  iatPostOffset = 0.0;
  boostOffset = 0.0;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Sensors: Calibration reset"));
  #endif
}
