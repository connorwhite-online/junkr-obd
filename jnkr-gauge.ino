/*
 * JNKR Gauge System
 * 
 * Read-only engine monitoring gauge for 1KZTE turbodiesel
 * Displays critical engine parameters with audio alerts
 * 
 * Hardware:
 *   - Arduino Mega 2560
 *   - Nextion NX4832T035 (3.5" touch display)
 *   - MAX31855 K-type thermocouple amplifier (EGT)
 *   - 3-bar MAP sensor (boost pressure)
 *   - NTC thermistors (intake, coolant, oil temps)
 *   - Piezo buzzer (alerts)
 * 
 * Features:
 *   - Real-time display of boost, IAT, EGT, coolant, oil temps
 *   - Color-coded warnings (green/yellow/orange/red)
 *   - Multi-level audio alerts with beep patterns
 *   - Sensor fault detection
 *   - Smooth filtered readings
 * 
 * Author: JNKR Project
 * Version: 1.0.0
 * License: MIT
 */

#include "config.h"
#include "sensors.h"
#include "display.h"
#include "alerts.h"

// ============================================================================
// GLOBAL STATE
// ============================================================================

// Sensor readings
struct SensorData {
  float boostPSI;
  float intakeTemp;
  float exhaustTemp;
  float coolantTemp;
  float oilTemp;
  float batteryVoltage;
  uint16_t faultFlags;
} sensorData;

// Timing control
unsigned long lastSensorUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastAlertCheck = 0;
unsigned long lastHeartbeat = 0;
bool heartbeatState = false;

// System status
bool systemInitialized = false;
unsigned long loopCounter = 0;

// ============================================================================
// SETUP - Run once at startup
// ============================================================================

void setup() {
  // Initialize serial for debugging
  #if ENABLE_SERIAL_DEBUG
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(100);
  Serial.println(F("\n========================================"));
  Serial.println(F("   JNKR Gauge System"));
  Serial.println(F("   Version: " VERSION));
  Serial.println(F("========================================\n"));
  #endif
  
  // Configure status LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, HIGH);  // Turn on during init
  
  // Wait for systems to stabilize
  delay(STARTUP_DELAY);
  
  // Initialize modules
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Initializing modules..."));
  #endif
  
  // 1. Initialize sensors first (they take readings)
  Sensors_Init();
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("  [OK] Sensors"));
  #endif
  
  // 2. Initialize display
  Display_Init();
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("  [OK] Display"));
  #endif
  
  // 3. Initialize alert system
  Alerts_Init();
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("  [OK] Alerts"));
  #endif
  
  // Take initial sensor readings
  delay(500);
  Sensors_Update();
  
  // Check for critical sensor faults
  if (!Sensors_IsEGTAvailable()) {
    #if ENABLE_SERIAL_DEBUG
    Serial.println(F("\n*** WARNING: EGT sensor not detected ***"));
    Serial.println(F("    System will operate without exhaust"));
    Serial.println(F("    temperature monitoring.\n"));
    #endif
    Display_SetStatus("No EGT Sensor");
    delay(2000);
  }
  
  // System ready
  systemInitialized = true;
  digitalWrite(PIN_STATUS_LED, LOW);
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("\n========================================"));
  Serial.println(F("   System Ready!"));
  Serial.println(F("   Monitoring engine parameters..."));
  Serial.println(F("========================================\n"));
  #endif
  
  Display_SetStatus("System Ready");
  delay(1000);
}

// ============================================================================
// MAIN LOOP - Runs continuously
// ============================================================================

void loop() {
  unsigned long now = millis();
  loopCounter++;
  
  // -------------------------
  // 1. UPDATE SENSORS (10Hz - every 100ms)
  // -------------------------
  if (now - lastSensorUpdate >= SENSOR_UPDATE_INTERVAL) {
    lastSensorUpdate = now;
    
    // Read all sensor values
    Sensors_Update();
    
    // Store readings in structure
    sensorData.boostPSI = Sensors_GetBoostPSI();
    sensorData.intakeTemp = Sensors_GetIntakeTemp();
    sensorData.exhaustTemp = Sensors_GetExhaustTemp();
    sensorData.coolantTemp = Sensors_GetCoolantTemp();
    sensorData.oilTemp = Sensors_GetOilTemp();
    sensorData.batteryVoltage = Sensors_GetBatteryVoltage();
    sensorData.faultFlags = Sensors_GetFaultFlags();
  }
  
  // -------------------------
  // 2. UPDATE DISPLAY (5Hz - every 200ms)
  // -------------------------
  if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = now;
    
    // Update all gauge values on display
    Display_UpdateGauges(
      sensorData.boostPSI,
      sensorData.intakeTemp,
      sensorData.exhaustTemp,
      sensorData.coolantTemp,
      sensorData.oilTemp,
      sensorData.batteryVoltage
    );
    
    // Process any incoming display messages
    Display_ProcessMessages();
  }
  
  // -------------------------
  // 3. CHECK ALERTS (2Hz - every 500ms)
  // -------------------------
  if (now - lastAlertCheck >= ALERT_CHECK_INTERVAL) {
    lastAlertCheck = now;
    
    // Check all thresholds and determine alert level
    uint8_t alertLevel = Alerts_CheckAll(
      sensorData.boostPSI,
      sensorData.intakeTemp,
      sensorData.exhaustTemp,
      sensorData.coolantTemp,
      sensorData.oilTemp,
      sensorData.batteryVoltage
    );
    
    // Update display alert level
    Display_SetAlertLevel(alertLevel);
    
    // Show alert message if there's a problem
    if (alertLevel > ALERT_NONE) {
      char alertMsg[64];
      Alerts_GetMessage(alertMsg, sizeof(alertMsg));
      
      uint16_t color = COLOR_WHITE;
      switch (alertLevel) {
        case ALERT_WARNING:
          color = COLOR_YELLOW;
          break;
        case ALERT_CRITICAL:
          color = COLOR_ORANGE;
          break;
        case ALERT_DANGER:
          color = COLOR_RED;
          break;
      }
      
      Display_ShowAlert(alertMsg, color);
    } else {
      Display_ClearAlert();
    }
  }
  
  // -------------------------
  // 4. UPDATE ALERT BUZZER (continuous)
  // -------------------------
  Alerts_Update();
  
  // -------------------------
  // 5. HEARTBEAT LED (1Hz - every second)
  // -------------------------
  if (now - lastHeartbeat >= HEARTBEAT_INTERVAL) {
    lastHeartbeat = now;
    heartbeatState = !heartbeatState;
    digitalWrite(PIN_STATUS_LED, heartbeatState ? HIGH : LOW);
    
    // Print debug info to serial
    #if ENABLE_SERIAL_DEBUG
    printDebugInfo();
    #endif
  }
  
  // Small delay to prevent tight loop
  delay(10);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

#if ENABLE_SERIAL_DEBUG
/**
 * Print debug information to serial monitor
 */
void printDebugInfo() {
  static unsigned long lastPrint = 0;
  unsigned long now = millis();
  
  // Only print every second
  if (now - lastPrint < 1000) {
    return;
  }
  lastPrint = now;
  
  Serial.println(F("\n========== Engine Status =========="));
  
  // Boost pressure
  Serial.print(F("Boost:    "));
  Serial.print(sensorData.boostPSI, 1);
  Serial.print(F(" PSI  ("));
  Serial.print(Sensors_GetBoostBar(), 2);
  Serial.println(F(" bar)"));
  
  // Intake temperature
  Serial.print(F("Intake:   "));
  Serial.print(sensorData.intakeTemp, 1);
  Serial.println(F(" C"));
  
  // Exhaust temperature
  Serial.print(F("Exhaust:  "));
  if (Sensors_IsEGTAvailable()) {
    Serial.print(sensorData.exhaustTemp, 0);
    Serial.println(F(" C"));
  } else {
    Serial.println(F("-- C (Not Available)"));
  }
  
  // Coolant temperature
  Serial.print(F("Coolant:  "));
  Serial.print(sensorData.coolantTemp, 1);
  Serial.println(F(" C"));
  
  // Oil temperature
  Serial.print(F("Oil:      "));
  Serial.print(sensorData.oilTemp, 1);
  Serial.println(F(" C"));
  
  // Battery voltage
  Serial.print(F("Battery:  "));
  Serial.print(sensorData.batteryVoltage, 1);
  Serial.println(F(" V"));
  
  // Alert status
  uint8_t alertLevel = Alerts_GetLevel();
  Serial.print(F("Alert:    "));
  switch (alertLevel) {
    case ALERT_NONE:
      Serial.println(F("None (All OK)"));
      break;
    case ALERT_INFO:
      Serial.println(F("Info"));
      break;
    case ALERT_WARNING:
      Serial.println(F("WARNING"));
      break;
    case ALERT_CRITICAL:
      Serial.println(F("CRITICAL"));
      break;
    case ALERT_DANGER:
      Serial.println(F("DANGER!!!"));
      break;
  }
  
  if (alertLevel > ALERT_NONE) {
    char alertMsg[64];
    Alerts_GetMessage(alertMsg, sizeof(alertMsg));
    Serial.print(F("  -> "));
    Serial.println(alertMsg);
  }
  
  // Fault flags
  if (sensorData.faultFlags > 0) {
    Serial.print(F("Faults:   0x"));
    Serial.println(sensorData.faultFlags, HEX);
    
    if (Sensors_IsFaulted(FAULT_IAT_SENSOR))
      Serial.println(F("  - Intake temp sensor"));
    if (Sensors_IsFaulted(FAULT_EGT_SENSOR))
      Serial.println(F("  - Exhaust temp sensor"));
    if (Sensors_IsFaulted(FAULT_COOLANT_SENSOR))
      Serial.println(F("  - Coolant temp sensor"));
    if (Sensors_IsFaulted(FAULT_OIL_SENSOR))
      Serial.println(F("  - Oil temp sensor"));
    if (Sensors_IsFaulted(FAULT_BOOST_SENSOR))
      Serial.println(F("  - Boost pressure sensor"));
    if (Sensors_IsFaulted(FAULT_BATTERY))
      Serial.println(F("  - Battery voltage"));
  }
  
  Serial.println(F("===================================\n"));
}
#endif

// ============================================================================
// END OF FILE
// ============================================================================
