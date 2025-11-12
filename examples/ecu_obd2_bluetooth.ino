/*
 * ECU OBD-II Bluetooth Reader Example
 * 
 * Reads engine parameters from 1KZ-TE ECU via OBD-II protocol
 * using an ELM327 Bluetooth adapter.
 * 
 * This example demonstrates Method 1 from OEM_ECU_DATA_ACCESS.md
 * 
 * Hardware:
 *   - ESP32-S3 (or any ESP32 board with Bluetooth)
 *   - ELM327 Bluetooth OBD-II adapter
 *   - Vehicle with OBD-II port (1996+ USA/Canada typically)
 * 
 * Setup:
 *   1. Plug ELM327 adapter into OBD-II port
 *   2. Turn ignition ON (engine can be off for testing)
 *   3. Upload this sketch to ESP32
 *   4. ESP32 will connect to ELM327 automatically
 *   5. Open Serial Monitor to view readings
 * 
 * Note: Many 1KZ-TE vehicles (especially pre-1996 or non-US) do NOT
 *       support OBD-II. Use ecu_analog_intercept.ino instead.
 * 
 * Author: JNKR Project
 * Version: 1.0.0
 * License: MIT
 */

#include "BluetoothSerial.h"

// Check if Bluetooth is enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run 'make menuconfig' and enable it.
#endif

// ============================================================================
// CONFIGURATION
// ============================================================================

// ELM327 Bluetooth device name (varies by adapter)
// Common names: "OBDII", "OBD-II", "ELM327", "V1.5", "CHX"
#define ELM327_NAME         "OBDII"  // Change to match your adapter

// OBD-II PID definitions (Mode 01)
#define OBD_MODE_CURRENT    0x01
#define PID_ENGINE_RPM      0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05
#define PID_INTAKE_TEMP     0x0F
#define PID_INTAKE_PRESSURE 0x0B
#define PID_THROTTLE_POS    0x11

// Update intervals (milliseconds)
#define UPDATE_INTERVAL     200  // 5Hz refresh rate

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

BluetoothSerial SerialBT;

// OBD-II connection status
bool elm327Connected = false;
bool ecuConnected = false;

// Sensor readings
float coolantTemp = 0.0;
float intakeAirTemp = 0.0;
float manifoldPressureKPa = 0.0;
float boostPressurePSI = 0.0;
uint16_t engineRPM = 0;
uint8_t vehicleSpeed = 0;
uint8_t throttlePosition = 0;

// Timing
unsigned long lastUpdate = 0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  ECU OBD-II Bluetooth Reader");
  Serial.println("  Toyota 1KZ-TE Engine");
  Serial.println("========================================\n");
  
  // Initialize Bluetooth Serial
  Serial.println("Initializing Bluetooth...");
  SerialBT.begin("JNKR_Gauge", true);  // Device name, master mode
  Serial.println("Bluetooth initialized");
  
  // Connect to ELM327
  Serial.print("Searching for ELM327 '");
  Serial.print(ELM327_NAME);
  Serial.println("'...");
  
  bool connected = SerialBT.connect(ELM327_NAME);
  
  if (connected) {
    Serial.println("✓ Connected to ELM327");
    elm327Connected = true;
    
    // Initialize ELM327
    if (initELM327()) {
      Serial.println("✓ ELM327 initialized");
      Serial.println("✓ ECU connected\n");
      ecuConnected = true;
    } else {
      Serial.println("✗ ECU connection failed");
      Serial.println("  Check OBD-II port and vehicle ignition\n");
    }
  } else {
    Serial.println("✗ Failed to connect to ELM327");
    Serial.println("  Possible issues:");
    Serial.println("  - ELM327 not powered on");
    Serial.println("  - Wrong Bluetooth name (check ELM327_NAME)");
    Serial.println("  - ELM327 already paired to another device\n");
  }
  
  if (ecuConnected) {
    Serial.println("System Ready - Reading OBD-II Data...\n");
  } else {
    Serial.println("System NOT ready - Check connection\n");
  }
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long now = millis();
  
  // Only read if connected
  if (!ecuConnected) {
    Serial.println("Waiting for ECU connection...");
    delay(1000);
    return;
  }
  
  // Update at specified interval
  if (now - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = now;
    
    // Read all OBD-II PIDs
    coolantTemp = readPID_Temperature(PID_COOLANT_TEMP);
    intakeAirTemp = readPID_Temperature(PID_INTAKE_TEMP);
    manifoldPressureKPa = readPID_Pressure(PID_INTAKE_PRESSURE);
    engineRPM = readPID_RPM(PID_ENGINE_RPM);
    vehicleSpeed = readPID_Speed(PID_VEHICLE_SPEED);
    throttlePosition = readPID_Percent(PID_THROTTLE_POS);
    
    // Calculate boost pressure (gauge = absolute - atmospheric)
    boostPressurePSI = (manifoldPressureKPa - 101.325) * 0.145038;
    
    // Display readings
    printReadings();
  }
  
  // Small delay
  delay(10);
}

// ============================================================================
// ELM327 INITIALIZATION
// ============================================================================

bool initELM327() {
  Serial.println("\nInitializing ELM327...");
  
  // Reset
  sendCommand("ATZ");
  delay(2000);  // Wait for ELM327 to reset
  
  // Echo off (reduces data overhead)
  sendCommand("ATE0");
  delay(100);
  
  // Headers on (useful for debugging)
  sendCommand("ATH1");
  delay(100);
  
  // Linefeed off
  sendCommand("ATL0");
  delay(100);
  
  // Spaces off (more compact responses)
  sendCommand("ATS0");
  delay(100);
  
  // Auto protocol detection
  sendCommand("ATSP0");
  delay(100);
  
  // Test ECU connection
  sendCommand("0100");  // Request supported PIDs
  delay(500);
  
  String response = readResponse();
  
  if (response.indexOf("41") >= 0) {
    // ECU responded with Mode 01 response
    Serial.println("✓ ECU communication established");
    
    // Determine protocol
    sendCommand("ATDPN");
    delay(100);
    String protocol = readResponse();
    Serial.print("  Protocol: ");
    
    if (protocol.indexOf("3") >= 0) {
      Serial.println("ISO 9141-2");
    } else if (protocol.indexOf("4") >= 0) {
      Serial.println("ISO 14230 (KWP2000)");
    } else if (protocol.indexOf("6") >= 0) {
      Serial.println("CAN 11-bit");
    } else if (protocol.indexOf("7") >= 0) {
      Serial.println("CAN 29-bit");
    } else {
      Serial.println("Unknown");
    }
    
    return true;
  } else {
    Serial.println("✗ No response from ECU");
    return false;
  }
}

// ============================================================================
// OBD-II PID READING FUNCTIONS
// ============================================================================

/**
 * Read temperature PID (Mode 01, PIDs 05, 0F, etc.)
 * Returns temperature in Celsius
 */
float readPID_Temperature(uint8_t pid) {
  char cmd[10];
  sprintf(cmd, "01%02X", pid);
  sendCommand(cmd);
  
  String response = readResponse();
  
  // Parse response: "41 05 8F" -> 0x8F = 143
  int dataPos = response.indexOf("41");
  if (dataPos >= 0) {
    // Extract data byte (4 characters after "41 XX ")
    String hexValue = response.substring(dataPos + 6, dataPos + 8);
    int rawValue = strtol(hexValue.c_str(), NULL, 16);
    
    // Temperature formula: A - 40
    return rawValue - 40.0;
  }
  
  return -999.0;  // Error
}

/**
 * Read pressure PID (Mode 01, PID 0B)
 * Returns pressure in kPa (absolute)
 */
float readPID_Pressure(uint8_t pid) {
  char cmd[10];
  sprintf(cmd, "01%02X", pid);
  sendCommand(cmd);
  
  String response = readResponse();
  
  int dataPos = response.indexOf("41");
  if (dataPos >= 0) {
    String hexValue = response.substring(dataPos + 6, dataPos + 8);
    int rawValue = strtol(hexValue.c_str(), NULL, 16);
    
    // Pressure formula: A (in kPa)
    return rawValue;
  }
  
  return -999.0;  // Error
}

/**
 * Read RPM PID (Mode 01, PID 0C)
 * Returns RPM
 */
uint16_t readPID_RPM(uint8_t pid) {
  char cmd[10];
  sprintf(cmd, "01%02X", pid);
  sendCommand(cmd);
  
  String response = readResponse();
  
  int dataPos = response.indexOf("41");
  if (dataPos >= 0) {
    // RPM is 2 bytes: ((A * 256) + B) / 4
    String hexA = response.substring(dataPos + 6, dataPos + 8);
    String hexB = response.substring(dataPos + 8, dataPos + 10);
    
    int A = strtol(hexA.c_str(), NULL, 16);
    int B = strtol(hexB.c_str(), NULL, 16);
    
    return ((A * 256) + B) / 4;
  }
  
  return 0;  // Error
}

/**
 * Read speed PID (Mode 01, PID 0D)
 * Returns speed in km/h
 */
uint8_t readPID_Speed(uint8_t pid) {
  char cmd[10];
  sprintf(cmd, "01%02X", pid);
  sendCommand(cmd);
  
  String response = readResponse();
  
  int dataPos = response.indexOf("41");
  if (dataPos >= 0) {
    String hexValue = response.substring(dataPos + 6, dataPos + 8);
    return strtol(hexValue.c_str(), NULL, 16);
  }
  
  return 0;  // Error
}

/**
 * Read percentage PID (Mode 01, PID 11 for throttle, etc.)
 * Returns percentage (0-100)
 */
uint8_t readPID_Percent(uint8_t pid) {
  char cmd[10];
  sprintf(cmd, "01%02X", pid);
  sendCommand(cmd);
  
  String response = readResponse();
  
  int dataPos = response.indexOf("41");
  if (dataPos >= 0) {
    String hexValue = response.substring(dataPos + 6, dataPos + 8);
    int rawValue = strtol(hexValue.c_str(), NULL, 16);
    
    // Percentage formula: (A * 100) / 255
    return (rawValue * 100) / 255;
  }
  
  return 0;  // Error
}

// ============================================================================
// ELM327 COMMUNICATION
// ============================================================================

/**
 * Send AT or OBD command to ELM327
 */
void sendCommand(const char* cmd) {
  SerialBT.print(cmd);
  SerialBT.print("\r");  // Carriage return
}

/**
 * Read response from ELM327
 * Waits for '>' prompt
 */
String readResponse() {
  String response = "";
  unsigned long timeout = millis() + 1000;  // 1 second timeout
  
  while (millis() < timeout) {
    if (SerialBT.available()) {
      char c = SerialBT.read();
      
      if (c == '>') {
        // ELM327 prompt - response complete
        break;
      }
      
      response += c;
    }
  }
  
  return response;
}

// ============================================================================
// DISPLAY FUNCTIONS
// ============================================================================

void printReadings() {
  static int lineCount = 0;
  
  // Print header every 20 lines
  if (lineCount % 20 == 0) {
    Serial.println("\n┌───────┬───────┬──────────┬──────────┬──────────┬────────┬──────────┐");
    Serial.println("│  RPM  │ Speed │ Coolant  │   IAT    │   MAP    │  Boost │ Throttle │");
    Serial.println("├───────┼───────┼──────────┼──────────┼──────────┼────────┼──────────┤");
  }
  
  Serial.print("│ ");
  
  // Engine RPM
  Serial.print(engineRPM);
  Serial.print("  ");
  
  Serial.print("│ ");
  
  // Vehicle speed
  Serial.print(vehicleSpeed);
  Serial.print(" km/h ");
  
  Serial.print("│ ");
  
  // Coolant temperature
  if (coolantTemp > -100) {
    Serial.print(coolantTemp, 1);
    Serial.print(" C ");
  } else {
    Serial.print("ERROR  ");
  }
  
  Serial.print("│ ");
  
  // Intake air temperature
  if (intakeAirTemp > -100) {
    Serial.print(intakeAirTemp, 1);
    Serial.print(" C ");
  } else {
    Serial.print("ERROR  ");
  }
  
  Serial.print("│ ");
  
  // Manifold pressure
  if (manifoldPressureKPa > -100) {
    Serial.print(manifoldPressureKPa, 1);
    Serial.print(" kPa ");
  } else {
    Serial.print("ERROR   ");
  }
  
  Serial.print("│ ");
  
  // Boost pressure
  if (boostPressurePSI > -100) {
    Serial.print(boostPressurePSI, 1);
    Serial.print(" PSI");
  } else {
    Serial.print("ERROR ");
  }
  
  Serial.print("│ ");
  
  // Throttle position
  Serial.print(throttlePosition);
  Serial.print(" %    ");
  
  Serial.println("│");
  
  lineCount++;
  
  // Print footer every 20 lines
  if (lineCount % 20 == 0) {
    Serial.println("└───────┴───────┴──────────┴──────────┴──────────┴────────┴──────────┘");
  }
}
