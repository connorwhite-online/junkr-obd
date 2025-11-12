/*
 * ECU Analog Signal Intercept Example
 * 
 * Reads OEM sensor signals from 1KZ-TE ECU by tapping the analog sensor wires
 * between the sensors and ECU. Uses high-impedance voltage dividers to avoid
 * affecting the ECU's readings.
 * 
 * This example demonstrates Method 3 from OEM_ECU_DATA_ACCESS.md
 * 
 * Hardware:
 *   - ESP32-S3 (or any ESP32 board)
 *   - 3x voltage dividers (100kΩ + 100kΩ resistors)
 *   - Connections to ECU sensor signal wires (THW, THA, PIM)
 * 
 * Wiring:
 *   ECU Pin THW (Coolant) ─┬─ To Coolant Sensor
 *                          │
 *                       100kΩ
 *                          │
 *                          ├─ To ESP32 GPIO36
 *                          │
 *                       100kΩ
 *                          │
 *                         GND
 * 
 * (Repeat for THA and PIM signals)
 * 
 * Author: JNKR Project
 * Version: 1.0.0
 * License: MIT
 */

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// ESP32 ADC pins for intercepted ECU signals
#define PIN_ECU_COOLANT     36  // ADC1_CH0 - Coolant temp sensor signal (THW)
#define PIN_ECU_IAT         39  // ADC1_CH3 - Intake air temp sensor signal (THA)
#define PIN_ECU_MAP         34  // ADC1_CH6 - MAP sensor signal (PIM)

// ============================================================================
// ADC CONFIGURATION
// ============================================================================

#define ADC_RESOLUTION     12   // 12-bit ADC (0-4095)
#define ADC_VREF           3.3  // ESP32 ADC reference voltage
#define ADC_SAMPLES        10   // Number of samples to average

// ============================================================================
// SENSOR CALIBRATION - Toyota 1KZ-TE
// ============================================================================

// NTC Thermistor specs (Toyota THW and THA sensors)
#define TOYOTA_NTC_RESISTANCE_25C   2450.0  // Resistance at 25°C (Ohms)
#define TOYOTA_NTC_BETA             3435    // Beta coefficient
#define TOYOTA_NTC_PULLUP           2200.0  // ECU pullup resistor (Ohms)
#define TOYOTA_SENSOR_VREF          5.0     // ECU sensor supply voltage

// MAP sensor specs (Toyota PIM sensor)
#define MAP_VOLTAGE_MIN     0.5     // Voltage at 0 kPa
#define MAP_VOLTAGE_MAX     4.5     // Voltage at 250 kPa
#define MAP_PRESSURE_MIN    0.0     // Minimum pressure (kPa absolute)
#define MAP_PRESSURE_MAX    250.0   // Maximum pressure (kPa absolute)

// Voltage divider attenuation (100kΩ / 100kΩ = 50% = 2x multiplier)
#define VOLTAGE_DIVIDER_RATIO   2.0

// ============================================================================
// CONVERSION CONSTANTS
// ============================================================================

#define BAR_TO_PSI          14.5038     // Conversion: bar to PSI
#define KPA_TO_BAR          0.01        // Conversion: kPa to bar
#define ATMOSPHERIC_KPA     101.325     // Atmospheric pressure (kPa)

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Current sensor readings
float coolantTemp = 25.0;
float intakeAirTemp = 25.0;
float boostPressurePSI = 0.0;
float manifoldPressureKPa = 0.0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  ECU Analog Signal Intercept");
  Serial.println("  Toyota 1KZ-TE Engine");
  Serial.println("========================================\n");
  
  // Configure ADC
  analogReadResolution(ADC_RESOLUTION);
  analogSetAttenuation(ADC_11db);  // 0-3.3V input range
  
  Serial.println("ADC Configuration:");
  Serial.print("  Resolution: "); Serial.print(ADC_RESOLUTION); Serial.println(" bits");
  Serial.print("  Reference:  "); Serial.print(ADC_VREF); Serial.println(" V");
  Serial.print("  Range:      0-"); Serial.print(ADC_VREF); Serial.println(" V");
  Serial.println();
  
  Serial.println("Pin Assignments:");
  Serial.print("  Coolant (THW): GPIO"); Serial.println(PIN_ECU_COOLANT);
  Serial.print("  IAT (THA):     GPIO"); Serial.println(PIN_ECU_IAT);
  Serial.print("  MAP (PIM):     GPIO"); Serial.println(PIN_ECU_MAP);
  Serial.println();
  
  Serial.println("Voltage Dividers:");
  Serial.println("  100kΩ + 100kΩ = 50% attenuation");
  Serial.print("  Multiplier: "); Serial.println(VOLTAGE_DIVIDER_RATIO);
  Serial.println();
  
  Serial.println("System Ready - Reading ECU Signals...\n");
  
  delay(1000);
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Read intercepted ECU sensor signals
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  
  // Convert voltages to engineering units
  coolantTemp = voltageToTemperature(coolantVoltage);
  intakeAirTemp = voltageToTemperature(iatVoltage);
  manifoldPressureKPa = voltageToPressure(mapVoltage);
  
  // Calculate boost pressure (gauge pressure = absolute - atmospheric)
  boostPressurePSI = (manifoldPressureKPa - ATMOSPHERIC_KPA) * KPA_TO_BAR * BAR_TO_PSI;
  
  // Display readings
  printReadings();
  
  // Delay for readability (10Hz refresh rate)
  delay(100);
}

// ============================================================================
// SENSOR READING FUNCTIONS
// ============================================================================

/**
 * Read ECU signal voltage with averaging
 * Accounts for voltage divider attenuation (2x multiplier)
 */
float readECUSignal(uint8_t pin) {
  uint32_t sum = 0;
  
  // Take multiple samples and average
  for (int i = 0; i < ADC_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(100);  // Small delay between samples
  }
  
  // Calculate average
  float avgADC = sum / (float)ADC_SAMPLES;
  
  // Convert ADC value to voltage
  float voltage = (avgADC / 4095.0) * ADC_VREF;
  
  // Compensate for voltage divider (50% attenuation = 2x multiplier)
  voltage *= VOLTAGE_DIVIDER_RATIO;
  
  return voltage;
}

/**
 * Convert NTC thermistor voltage to temperature
 * Uses Steinhart-Hart Beta parameter equation
 */
float voltageToTemperature(float voltage) {
  // Handle out-of-range voltages
  if (voltage < 0.1 || voltage > 4.9) {
    return -999.0;  // Error value
  }
  
  // Convert voltage to resistance using voltage divider equation
  // Voltage divider: Vout = Vin * (R2 / (R1 + R2))
  // Solving for R2 (thermistor): R2 = (Vout * R1) / (Vin - Vout)
  float resistance = (voltage * TOYOTA_NTC_PULLUP) / (TOYOTA_SENSOR_VREF - voltage);
  
  // Steinhart-Hart Beta parameter equation:
  // 1/T = 1/T0 + (1/B) * ln(R/R0)
  // Where: T = temperature (Kelvin)
  //        T0 = 298.15K (25°C)
  //        R = measured resistance
  //        R0 = nominal resistance at 25°C
  //        B = Beta coefficient
  
  float tempKelvin = 1.0 / (
    (1.0 / (25.0 + 273.15)) + 
    ((1.0 / TOYOTA_NTC_BETA) * log(resistance / TOYOTA_NTC_RESISTANCE_25C))
  );
  
  // Convert to Celsius
  float tempCelsius = tempKelvin - 273.15;
  
  return tempCelsius;
}

/**
 * Convert MAP sensor voltage to pressure (kPa absolute)
 * Toyota MAP sensor: 0.5V = 0 kPa, 4.5V = 250 kPa (linear)
 */
float voltageToPressure(float voltage) {
  // Handle out-of-range voltages
  if (voltage < 0.3 || voltage > 4.8) {
    return -999.0;  // Error value
  }
  
  // Linear conversion: voltage to pressure
  // P = Pmin + ((V - Vmin) / (Vmax - Vmin)) * (Pmax - Pmin)
  float pressure = MAP_PRESSURE_MIN + 
                   ((voltage - MAP_VOLTAGE_MIN) / (MAP_VOLTAGE_MAX - MAP_VOLTAGE_MIN)) * 
                   (MAP_PRESSURE_MAX - MAP_PRESSURE_MIN);
  
  return pressure;
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

/**
 * Print sensor readings to serial monitor
 */
void printReadings() {
  static unsigned long lastPrint = 0;
  static int lineCount = 0;
  
  unsigned long now = millis();
  
  // Print header every 20 lines
  if (lineCount % 20 == 0) {
    Serial.println("\n┌────────────────┬────────────────┬─────────────────┬─────────────────┐");
    Serial.println("│ Coolant Temp   │ Intake Air Tmp │ Manifold Press  │ Boost Pressure  │");
    Serial.println("├────────────────┼────────────────┼─────────────────┼─────────────────┤");
  }
  
  // Print data line
  Serial.print("│ ");
  
  // Coolant temperature
  if (coolantTemp > -100) {
    Serial.print(coolantTemp, 1);
    Serial.print(" C");
    Serial.print("       ");
  } else {
    Serial.print("ERROR     ");
  }
  
  Serial.print(" │ ");
  
  // Intake air temperature
  if (intakeAirTemp > -100) {
    Serial.print(intakeAirTemp, 1);
    Serial.print(" C");
    Serial.print("       ");
  } else {
    Serial.print("ERROR     ");
  }
  
  Serial.print(" │ ");
  
  // Manifold pressure
  if (manifoldPressureKPa > -100) {
    Serial.print(manifoldPressureKPa, 1);
    Serial.print(" kPa");
    Serial.print("       ");
  } else {
    Serial.print("ERROR      ");
  }
  
  Serial.print(" │ ");
  
  // Boost pressure
  if (boostPressurePSI > -100) {
    Serial.print(boostPressurePSI, 1);
    Serial.print(" PSI");
    Serial.print("       ");
  } else {
    Serial.print("ERROR      ");
  }
  
  Serial.println(" │");
  
  lineCount++;
  
  // Print footer every 20 lines
  if (lineCount % 20 == 0) {
    Serial.println("└────────────────┴────────────────┴─────────────────┴─────────────────┘");
  }
}

/**
 * Print voltage diagnostics (call from loop for debugging)
 */
void printVoltageDiagnostics() {
  Serial.println("\n========== Voltage Diagnostics ==========");
  
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  Serial.print("Coolant (THW): ");
  Serial.print(coolantVoltage, 3);
  Serial.print(" V  ->  ");
  Serial.print(voltageToTemperature(coolantVoltage), 1);
  Serial.println(" C");
  
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  Serial.print("IAT (THA):     ");
  Serial.print(iatVoltage, 3);
  Serial.print(" V  ->  ");
  Serial.print(voltageToTemperature(iatVoltage), 1);
  Serial.println(" C");
  
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  Serial.print("MAP (PIM):     ");
  Serial.print(mapVoltage, 3);
  Serial.print(" V  ->  ");
  Serial.print(voltageToPressure(mapVoltage), 1);
  Serial.println(" kPa");
  
  Serial.println("==========================================\n");
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Celsius to Fahrenheit
 */
float celsiusToFahrenheit(float celsius) {
  return (celsius * 9.0 / 5.0) + 32.0;
}

/**
 * kPa to PSI
 */
float kpaToPSI(float kpa) {
  return kpa * 0.145038;
}

/**
 * kPa to bar
 */
float kpaToBar(float kpa) {
  return kpa * 0.01;
}
