/*
 * Sensor Library - Implementation
 * 
 * Reads temperature sensors (NTC thermistors), pressure sensors (MAP),
 * and EGT sensor via I2C sensor module in engine bay
 * 
 * I2C Module contains:
 *   - ADS1115: 16-bit ADC for analog sensors (thermistors, MAP)
 *   - MCP9600: K-type thermocouple amplifier for EGT
 * 
 * Note: Legacy analog/SPI code available in git history
 */

#include "sensors.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_ADS1X15.h>  // ADS1115 library
#include <Adafruit_MCP9600.h>   // MCP9600 library
#include <math.h>

// ============================================================================
// I2C SENSOR OBJECTS
// ============================================================================

// Create I2C sensor objects
static Adafruit_ADS1115 ads;  // 16-bit ADC for analog sensors
static Adafruit_MCP9600 mcp;  // Thermocouple amplifier for EGT

// I2C device status
static bool ads1115Available = false;
static bool mcp9600Available = false;

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
 * Read I2C ADC with error handling
 * Returns voltage from ADS1115
 */
float readI2C_ADC_Voltage(uint8_t channel) {
  if (!ads1115Available) {
    return 0.0;  // Return 0 if ADS1115 not available
  }
  
  // Read raw ADC value (16-bit signed)
  int16_t rawADC = ads.readADC_SingleEnded(channel);
  
  // Convert to voltage using ADS1115 library
  float voltage = ads.computeVolts(rawADC);
  
  return voltage;
}

/**
 * Read I2C ADC with multiple samples and averaging
 * Reduces noise on analog signals
 */
float readI2C_ADC_Averaged(uint8_t channel, int samples) {
  if (!ads1115Available) {
    return 0.0;
  }
  
  float sum = 0.0;
  for (int i = 0; i < samples; i++) {
    sum += readI2C_ADC_Voltage(channel);
    delay(2);  // Small delay between samples (ADS1115 conversion time)
  }
  return sum / samples;
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
 * Read NTC thermistor via I2C ADC and return temperature
 */
float readThermistor(uint8_t channel, float prevValue) {
  // Read ADC voltage with averaging
  float voltage = readI2C_ADC_Averaged(channel, ANALOG_SAMPLES);
  
  // If voltage is zero, sensor module may not be responding
  if (voltage < 0.01) {
    return prevValue;  // Keep previous value
  }
  
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
 * Read MAP/Boost pressure sensor via I2C ADC
 */
float readBoostPressure(float prevValue) {
  // Read ADC voltage with averaging (channel 3 for MAP sensor)
  float voltage = readI2C_ADC_Averaged(3, ANALOG_SAMPLES);
  
  // If voltage is zero, sensor may not be connected
  if (voltage < 0.01) {
    faultFlags |= FAULT_BOOST_SENSOR;
    return prevValue;
  }
  
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
 * Read EGT from MCP9600 K-type thermocouple amplifier via I2C
 * Returns temperature in Celsius
 */
float readEGT(float prevValue) {
  if (!mcp9600Available) {
    faultFlags |= FAULT_EGT_SENSOR;
    egtAvailable = false;
    return 0.0;
  }
  
  // Read thermocouple temperature from MCP9600
  float temperature = mcp.readThermocouple();
  
  // Check for errors (NaN indicates fault)
  if (isnan(temperature)) {
    faultFlags |= FAULT_EGT_SENSOR;
    egtAvailable = false;
    return prevValue;
  }
  
  // No fault
  faultFlags &= ~FAULT_EGT_SENSOR;
  egtAvailable = true;
  
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
  // Initialize I2C bus
  Wire.begin();
  Wire.setClock(400000);  // 400kHz fast I2C
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Sensors: Initializing I2C sensor module..."));
  #endif
  
  // Initialize ADS1115 ADC
  if (ads.begin(I2C_ADDR_ADS1115)) {
    ads1115Available = true;
    ads.setGain(ADS1115_GAIN);        // Set voltage range (default ±4.096V)
    ads.setDataRate(ADS1115_DATA_RATE); // Set sample rate
    
    #if ENABLE_SERIAL_DEBUG
    Serial.print(F("  - ADS1115 ADC found at 0x"));
    Serial.println(I2C_ADDR_ADS1115, HEX);
    #endif
  } else {
    ads1115Available = false;
    #if ENABLE_SERIAL_DEBUG
    Serial.println(F("  - WARNING: ADS1115 ADC not found!"));
    Serial.println(F("    Check I2C connections and module power"));
    #endif
  }
  
  // Initialize MCP9600 thermocouple amplifier
  if (mcp.begin(I2C_ADDR_MCP9600)) {
    mcp9600Available = true;
    mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);  // 18-bit resolution
    mcp.setThermocoupleType(MCP9600_TYPE_K);         // K-type thermocouple
    mcp.setFilterCoefficient(3);                     // Light filtering
    mcp.enable(true);
    
    #if ENABLE_SERIAL_DEBUG
    Serial.print(F("  - MCP9600 thermocouple amp found at 0x"));
    Serial.println(I2C_ADDR_MCP9600, HEX);
    #endif
    
    egtAvailable = true;
  } else {
    mcp9600Available = false;
    egtAvailable = false;
    
    #if ENABLE_SERIAL_DEBUG
    Serial.println(F("  - WARNING: MCP9600 thermocouple amp not found!"));
    Serial.println(F("    EGT monitoring will not be available"));
    #endif
  }
  
  // Check if any I2C devices were found
  if (!ads1115Available && !mcp9600Available) {
    #if ENABLE_SERIAL_DEBUG
    Serial.println(F(""));
    Serial.println(F("  *** ERROR: No I2C sensor module detected! ***"));
    Serial.println(F("  *** Check harness connection through firewall ***"));
    Serial.println(F("  *** Verify module power (5V) and I2C lines ***"));
    Serial.println(F(""));
    #endif
  }
  
  // Initial sensor reading
  delay(100);
  Sensors_Update();
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Sensors: Initialization complete"));
  #endif
}

void Sensors_Update() {
  // Only read sensors if I2C devices are available
  if (ads1115Available) {
    // Read all temperature sensors via I2C ADC
    // Channel 0: IAT Pre-Intercooler
    // Channel 1: IAT Post-Intercooler
    // Channel 2: Coolant Temperature
    intakeTempPre = readThermistor(0, prevIntakeTempPre);
    intakeTempPost = readThermistor(1, prevIntakeTempPost);
    coolantTemp = readThermistor(2, prevCoolantTemp);
    
    // Read boost pressure (channel 3)
    boostPressureBar = readBoostPressure(prevBoostPressure);
  } else {
    // If ADS1115 not available, keep previous values
    // Set fault flags for all sensors
    faultFlags |= FAULT_IAT_PRE_SENSOR;
    faultFlags |= FAULT_IAT_POST_SENSOR;
    faultFlags |= FAULT_COOLANT_SENSOR;
    faultFlags |= FAULT_BOOST_SENSOR;
  }
  
  // Read EGT from MCP9600
  if (mcp9600Available) {
    exhaustTemp = readEGT(prevExhaustTemp);
  } else {
    exhaustTemp = 0.0;
    egtAvailable = false;
  }
  
  // Update previous values for next iteration
  prevIntakeTempPre = intakeTempPre;
  prevIntakeTempPost = intakeTempPost;
  prevCoolantTemp = coolantTemp;
  prevExhaustTemp = exhaustTemp;
  prevBoostPressure = boostPressureBar;
  
  // Check for sensor-specific faults (only if ADS1115 is available)
  if (ads1115Available) {
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
