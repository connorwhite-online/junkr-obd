/*
 * JNKR Gauge System - Configuration
 * 
 * Read-only gauge system for 1KZTE engine monitoring
 * Displays intake temp, exhaust temp, coolant temp, and boost pressure
 * with audio alerts for dangerous thresholds
 * 
 * Hardware: Arduino Mega 2560
 * Display: Nextion NX4832T035 (3.5" touch display)
 * Alert: Piezo buzzer on Pin 8
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// VERSION INFORMATION
// ============================================================================
#define VERSION "1.0.0"
#define PROJECT_NAME "JNKR Gauge System"

// ============================================================================
// PIN ASSIGNMENTS - Arduino Mega 2560
// ============================================================================

// === ANALOG INPUTS (Temperature & Pressure Sensors) ===
#define PIN_INTAKE_TEMP     A0   // Intake Air Temperature (NTC thermistor)
#define PIN_COOLANT_TEMP    A1   // Coolant Temperature (NTC thermistor)
#define PIN_OIL_TEMP        A2   // Oil Temperature (optional, NTC thermistor)
#define PIN_BOOST_PRESSURE  A3   // MAP/Boost Pressure Sensor (0-3 bar)
#define PIN_BATTERY_VOLTAGE A7   // Battery voltage monitor (via voltage divider)

// === SPI PINS (for MAX31855 EGT Sensor) ===
#define PIN_EGT_CS          10   // MAX31855 Chip Select
#define PIN_SPI_MISO        50   // SPI MISO (hardware pin on Mega)
#define PIN_SPI_SCK         52   // SPI SCK (hardware pin on Mega)

// === DIGITAL OUTPUT (Audio Alert) ===
#define PIN_BUZZER          8    // Piezo buzzer for alerts
#define PIN_STATUS_LED      13   // Built-in LED for heartbeat

// === UART (for Nextion Display) ===
#define NEXTION_SERIAL      Serial2  // Hardware Serial2 (pins 16/17)
#define NEXTION_BAUD        115200   // Baud rate for Nextion

// === I2C (optional - for BMP280 barometric sensor) ===
#define PIN_I2C_SDA         20   // I2C Data
#define PIN_I2C_SCL         21   // I2C Clock
#define BMP280_I2C_ADDR     0x76 // Default I2C address

// ============================================================================
// SENSOR CALIBRATION - NTC Thermistors
// ============================================================================

// Standard automotive NTC thermistor specs
// Typical: 2252Ω at 25°C, Beta coefficient 3950K
#define THERMISTOR_REF_RESISTANCE  2200.0   // Reference resistor in voltage divider (Ohms)
#define THERMISTOR_NOMINAL_R       2252.0   // Resistance at 25°C (Ohms)
#define THERMISTOR_NOMINAL_TEMP    25.0     // Temperature for nominal resistance (°C)
#define THERMISTOR_BETA            3950     // Beta coefficient

// ADC reference voltage
#define ADC_VREF               5.0       // Arduino Mega uses 5V reference
#define ADC_RESOLUTION         1024     // 10-bit ADC

// ============================================================================
// BOOST/MAP SENSOR CALIBRATION
// ============================================================================

// Common 3-bar MAP sensor: 0.5V = 0 bar, 4.5V = 3 bar absolute
#define MAP_VOLTAGE_MIN        0.5      // Voltage at minimum pressure (V)
#define MAP_VOLTAGE_MAX        4.5      // Voltage at maximum pressure (V)
#define MAP_PRESSURE_MIN       0.0      // Minimum pressure (bar absolute)
#define MAP_PRESSURE_MAX       3.0      // Maximum pressure (bar absolute)

// Conversion factor: bar to PSI
#define BAR_TO_PSI             14.5038  // 1 bar = 14.5038 psi

// ============================================================================
// BATTERY VOLTAGE MONITOR
// ============================================================================

// Voltage divider: R1=10kΩ, R2=2.2kΩ (gives ~5.5:1 ratio)
// Max input: 16.5V → ADC sees: 3.0V (safe for 5V Arduino)
#define BATTERY_DIVIDER_R1     10000.0  // Upper resistor (Ohms)
#define BATTERY_DIVIDER_R2     2200.0   // Lower resistor (Ohms)
#define BATTERY_DIVIDER_RATIO  ((BATTERY_DIVIDER_R1 + BATTERY_DIVIDER_R2) / BATTERY_DIVIDER_R2)

// Battery voltage limits
#define BATTERY_MIN            11.0     // Below this = warning
#define BATTERY_MAX            15.5     // Above this = warning

// ============================================================================
// TEMPERATURE THRESHOLDS (°C)
// ============================================================================

// === Intake Air Temperature ===
#define IAT_NORMAL_MAX         45.0     // Normal operating max
#define IAT_WARNING            50.0     // Warning threshold
#define IAT_CRITICAL           60.0     // Critical - reduce power

// === Exhaust Gas Temperature (EGT) ===
#define EGT_NORMAL_MAX         550.0    // Normal sustained max
#define EGT_WARNING            600.0    // Warning threshold
#define EGT_CRITICAL           650.0    // Critical - immediate action
#define EGT_DANGER             700.0    // Danger - engine damage imminent

// === Coolant Temperature ===
#define COOLANT_NORMAL_MIN     75.0     // Normal operating minimum
#define COOLANT_NORMAL_MAX     95.0     // Normal operating maximum
#define COOLANT_WARNING        100.0    // Warning threshold
#define COOLANT_CRITICAL       105.0    // Critical - pull over
#define COOLANT_DANGER         110.0    // Danger - engine damage

// === Oil Temperature (if monitored) ===
#define OIL_NORMAL_MAX         110.0    // Normal operating max
#define OIL_WARNING            120.0    // Warning threshold
#define OIL_CRITICAL           130.0    // Critical threshold

// ============================================================================
// PRESSURE THRESHOLDS
// ============================================================================

// === Boost Pressure (bar gauge = absolute - 1.0) ===
#define BOOST_NORMAL_MAX       0.8      // Stock turbo max (bar gauge)
#define BOOST_WARNING          1.0      // Warning for modified setup (bar gauge)
#define BOOST_CRITICAL         1.3      // Critical overboost (bar gauge)
#define BOOST_DANGER           1.5      // Danger - severe overboost (bar gauge)

// === Boost Pressure (PSI gauge) ===
#define BOOST_NORMAL_PSI       11.6     // Stock max (~0.8 bar)
#define BOOST_WARNING_PSI      14.5     // Warning (~1.0 bar)
#define BOOST_CRITICAL_PSI     18.8     // Critical (~1.3 bar)
#define BOOST_DANGER_PSI       21.8     // Danger (~1.5 bar)

// ============================================================================
// ALERT SYSTEM CONFIGURATION
// ============================================================================

// Alert levels
enum AlertLevel {
  ALERT_NONE = 0,      // All normal
  ALERT_INFO = 1,      // Informational (single beep)
  ALERT_WARNING = 2,   // Warning condition (double beep)
  ALERT_CRITICAL = 3,  // Critical - immediate attention (triple beep)
  ALERT_DANGER = 4     // Danger - continuous alarm
};

// Buzzer patterns (in milliseconds)
#define BEEP_SHORT             100      // Short beep duration
#define BEEP_MEDIUM            250      // Medium beep duration
#define BEEP_LONG              500      // Long beep duration
#define BEEP_PAUSE             150      // Pause between beeps
#define BEEP_REPEAT_DELAY      2000     // Delay before repeating alert

// Buzzer frequency
#define BUZZER_FREQUENCY       2000     // Hz (2kHz for piezo buzzer)

// ============================================================================
// FILTERING AND SAMPLING
// ============================================================================

#define ANALOG_SAMPLES         8        // Number of ADC samples to average
#define FILTER_ALPHA           0.15     // Exponential moving average coefficient (0-1)
                                        // Lower = more smoothing, slower response
                                        // Higher = less smoothing, faster response

// Update intervals (milliseconds)
#define SENSOR_UPDATE_INTERVAL 100      // Read sensors every 100ms (10Hz)
#define DISPLAY_UPDATE_INTERVAL 200     // Update display every 200ms (5Hz)
#define ALERT_CHECK_INTERVAL   500      // Check alerts every 500ms (2Hz)
#define HEARTBEAT_INTERVAL     1000     // Status LED heartbeat (1Hz)

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

// Nextion display pages
#define PAGE_MAIN              0        // Main gauge display page
#define PAGE_SETTINGS          1        // Settings page (future)
#define PAGE_ALERTS            2        // Alert history page (future)

// Nextion component names (must match your Nextion HMI project)
#define NEXTION_BOOST_VALUE    "nBoost"      // Boost numeric field
#define NEXTION_IAT_VALUE      "nIAT"        // Intake temp numeric field
#define NEXTION_EGT_VALUE      "nEGT"        // Exhaust temp numeric field
#define NEXTION_COOLANT_VALUE  "nCoolant"    // Coolant temp numeric field
#define NEXTION_OIL_VALUE      "nOil"        // Oil temp numeric field
#define NEXTION_BATTERY_VALUE  "nBattery"    // Battery voltage field
#define NEXTION_STATUS_TEXT    "tStatus"     // Status text field
#define NEXTION_ALERT_TEXT     "tAlert"      // Alert message field

// Nextion colors (RGB565 format)
#define COLOR_GREEN            2016         // Normal (green)
#define COLOR_YELLOW           64512        // Warning (yellow)
#define COLOR_ORANGE           64512        // Elevated warning (orange)
#define COLOR_RED              63488        // Critical (red)
#define COLOR_WHITE            65535        // Text default (white)
#define COLOR_BLACK            0            // Background (black)

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

#define LOOP_TIME_MS           20           // Main loop target time (50Hz)
#define SERIAL_DEBUG_BAUD      115200       // Serial debug baud rate
#define ENABLE_SERIAL_DEBUG    true         // Enable debug output to Serial
#define STARTUP_DELAY          2000         // Startup delay (ms)

// Watchdog timer (if enabled)
#define ENABLE_WATCHDOG        false        // Enable watchdog timer
#define WATCHDOG_TIMEOUT       WDTO_2S      // 2 second timeout

// ============================================================================
// SENSOR VALIDATION RANGES
// ============================================================================

// Valid sensor reading ranges (for fault detection)
#define TEMP_MIN_VALID         -40.0        // Minimum valid temperature (°C)
#define TEMP_MAX_VALID         200.0        // Maximum valid temperature (°C)
#define PRESSURE_MIN_VALID     0.5          // Minimum valid pressure (bar abs)
#define PRESSURE_MAX_VALID     3.0          // Maximum valid pressure (bar abs)
#define VOLTAGE_MIN_VALID      0.0          // Minimum valid voltage (V)
#define VOLTAGE_MAX_VALID      5.0          // Maximum valid ADC voltage (V)

// ============================================================================
// FAULT FLAGS
// ============================================================================

// Bit positions for fault flags
#define FAULT_IAT_SENSOR       (1 << 0)     // Intake temp sensor fault
#define FAULT_EGT_SENSOR       (1 << 1)     // Exhaust temp sensor fault
#define FAULT_COOLANT_SENSOR   (1 << 2)     // Coolant sensor fault
#define FAULT_OIL_SENSOR       (1 << 3)     // Oil sensor fault
#define FAULT_BOOST_SENSOR     (1 << 4)     // Boost sensor fault
#define FAULT_BATTERY          (1 << 5)     // Battery voltage fault
#define FAULT_DISPLAY          (1 << 6)     // Display communication fault
#define FAULT_EGT_HIGH         (1 << 7)     // EGT too high
#define FAULT_COOLANT_HIGH     (1 << 8)     // Coolant temp too high
#define FAULT_OVERBOOST        (1 << 9)     // Overboost condition

// ============================================================================
// ENGINEERING UNITS DISPLAY
// ============================================================================

// Temperature unit preference
#define TEMP_UNIT_CELSIUS      0
#define TEMP_UNIT_FAHRENHEIT   1
#define TEMP_DISPLAY_UNIT      TEMP_UNIT_CELSIUS  // Default: Celsius

// Pressure unit preference
#define PRESSURE_UNIT_BAR      0
#define PRESSURE_UNIT_PSI      1
#define PRESSURE_DISPLAY_UNIT  PRESSURE_UNIT_PSI  // Default: PSI

// ============================================================================
// HELPER MACROS
// ============================================================================

// Temperature conversion
#define CELSIUS_TO_FAHRENHEIT(c)  ((c) * 9.0 / 5.0 + 32.0)
#define FAHRENHEIT_TO_CELSIUS(f)  (((f) - 32.0) * 5.0 / 9.0)

// Pressure conversion
#define BAR_GAUGE_TO_ABSOLUTE(g)  ((g) + 1.0)
#define BAR_ABSOLUTE_TO_GAUGE(a)  ((a) - 1.0)
#define BAR_TO_KPA(b)             ((b) * 100.0)
#define PSI_TO_BAR(p)             ((p) / 14.5038)

// Constrain and map helpers
#define CONSTRAIN(x, min, max)    ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#endif // CONFIG_H
