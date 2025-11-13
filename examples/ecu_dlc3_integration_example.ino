/*
 * DLC3 Integration Example
 * 
 * Shows how to integrate DLC3 data into your main gauge code
 * while keeping analog intercept code completely separate.
 * 
 * This is an EXAMPLE showing the pattern - integrate this into
 * your main jnkr-gauge.ino code.
 * 
 * Architecture:
 *   - DLC3 data comes from ecu_dlc3_receiver.ino
 *   - Analog intercept code stays in ecu_analog_intercept.ino
 *   - Main gauge code chooses which to use (or uses both)
 * 
 * Author: JNKR Project
 * Version: 1.0.0
 * License: MIT
 */

// ============================================================================
// INCLUDES
// ============================================================================

// Include DLC3 receiver (if using DLC3)
// #include "ecu_dlc3_receiver.h"  // Uncomment if using DLC3

// Include analog intercept (if using analog intercept)
// #include "ecu_analog_intercept.h"  // Uncomment if using analog intercept

// ============================================================================
// CONFIGURATION
// ============================================================================

// Choose your data source (or use both)
#define USE_DLC3 true          // Set to true to use DLC3
#define USE_ANALOG_INTERCEPT false  // Set to true to use analog intercept

// Note: You can use BOTH by setting both to true
// The code will prefer DLC3 if available, fallback to analog

// ============================================================================
// SENSOR DATA STRUCTURE
// ============================================================================

struct SensorReadings {
  float coolantTemp = 0.0;
  float intakeAirTemp = 0.0;
  float boostPressureBar = 0.0;
  uint16_t engineRPM = 0;
  uint8_t vehicleSpeed = 0;
  uint8_t throttlePosition = 0;
  
  // Source tracking
  bool fromDLC3 = false;
  bool fromAnalog = false;
};

SensorReadings sensorData;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  DLC3 Integration Example");
  Serial.println("========================================\n");
  
  #if USE_DLC3
    Serial.println("✓ DLC3 enabled");
    // Initialize DLC3 receiver here
    // setupDLC3Receiver();
  #endif
  
  #if USE_ANALOG_INTERCEPT
    Serial.println("✓ Analog intercept enabled");
    // Initialize analog intercept here
    // setupAnalogIntercept();
  #endif
  
  Serial.println("\nSystem Ready\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  // Update sensor readings
  updateSensorReadings();
  
  // Use sensor data in your gauge display
  displaySensorData();
  
  delay(100);
}

// ============================================================================
// SENSOR UPDATE FUNCTION
// ============================================================================

void updateSensorReadings() {
  // Reset source flags
  sensorData.fromDLC3 = false;
  sensorData.fromAnalog = false;
  
  #if USE_DLC3
    // Try DLC3 first (if enabled)
    if (isDLC3Available()) {
      // Get data from DLC3
      sensorData.coolantTemp = getDLC3CoolantTemp();
      sensorData.intakeAirTemp = getDLC3IntakeAirTemp();
      sensorData.boostPressureBar = getDLC3BoostBar();
      sensorData.engineRPM = getDLC3RPM();
      sensorData.vehicleSpeed = getDLC3Speed();
      sensorData.throttlePosition = getDLC3Throttle();
      
      sensorData.fromDLC3 = true;
      
      // DLC3 data obtained, skip analog intercept
      return;
    }
  #endif
  
  #if USE_ANALOG_INTERCEPT
    // Fallback to analog intercept (if enabled and DLC3 not available)
    if (!sensorData.fromDLC3) {
      // Get data from analog intercept
      // sensorData.coolantTemp = readAnalogCoolantTemp();
      // sensorData.intakeAirTemp = readAnalogIntakeAirTemp();
      // sensorData.boostPressureBar = readAnalogBoostBar();
      
      sensorData.fromAnalog = true;
    }
  #endif
}

// ============================================================================
// DISPLAY FUNCTION (EXAMPLE)
// ============================================================================

void displaySensorData() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  
  if (now - lastPrint > 1000) {  // Print every second
    lastPrint = now;
    
    Serial.println("\n=== Sensor Data ===");
    Serial.print("Source: ");
    if (sensorData.fromDLC3) {
      Serial.println("DLC3");
    } else if (sensorData.fromAnalog) {
      Serial.println("Analog Intercept");
    } else {
      Serial.println("None");
    }
    
    Serial.print("Coolant Temp: ");
    Serial.print(sensorData.coolantTemp, 1);
    Serial.println(" °C");
    
    Serial.print("Intake Air Temp: ");
    Serial.print(sensorData.intakeAirTemp, 1);
    Serial.println(" °C");
    
    Serial.print("Boost Pressure: ");
    Serial.print(sensorData.boostPressureBar, 2);
    Serial.println(" bar");
    
    Serial.print("Engine RPM: ");
    Serial.println(sensorData.engineRPM);
    
    Serial.print("Vehicle Speed: ");
    Serial.print(sensorData.vehicleSpeed);
    Serial.println(" km/h");
    
    Serial.print("Throttle Position: ");
    Serial.print(sensorData.throttlePosition);
    Serial.println(" %");
    Serial.println();
  }
}

// ============================================================================
// INTEGRATION NOTES
// ============================================================================

/*
 * TO INTEGRATE INTO YOUR MAIN GAUGE CODE:
 * 
 * 1. Copy the updateSensorReadings() function to your code
 * 
 * 2. In your Sensors_Update() function, call updateSensorReadings()
 * 
 * 3. Use sensorData values in your display code:
 *    - sensorData.coolantTemp
 *    - sensorData.intakeAirTemp
 *    - sensorData.boostPressureBar
 *    - etc.
 * 
 * 4. Keep analog intercept code in separate file (ecu_analog_intercept.ino)
 *    - Don't mix the code together
 *    - Use #ifdef or separate functions to choose which to use
 * 
 * 5. You can use both simultaneously:
 *    - DLC3 for comprehensive diagnostics
 *    - Analog intercept for high-speed critical sensors
 *    - Or use one as backup for the other
 */
