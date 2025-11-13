/*
 * ECU DLC3 K-Line Wireless Receiver
 * 
 * Receives engine data wirelessly from DLC3 K-Line module
 * and integrates with main gauge system.
 * 
 * This runs on the MAIN ESP32 (Qualia) mounted in the dash.
 * It receives data from the small K-Line module near the DLC3 connector.
 * 
 * Architecture:
 *   DLC3 Connector → K-Line Module (ESP32) → WiFi/BLE → Main ESP32 (Qualia) ← This code
 * 
 * Hardware:
 *   - Qualia ESP32-S3 (main gauge display)
 *   - Receives data via WiFi UDP or BLE
 * 
 * This example shows how to receive and use DLC3 data
 * alongside your existing sensor readings.
 * 
 * Author: JNKR Project
 * Version: 1.0.0
 * License: MIT
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi Configuration
#define WIFI_SSID "JNKR_Gauge"
#define WIFI_PASSWORD "your_password_here"
#define UDP_PORT 12345

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

WiFiUDP udp;

// Engine data from DLC3
struct DLC3Data {
  float coolantTemp = 0.0;
  float intakeAirTemp = 0.0;
  float manifoldPressureKPa = 0.0;
  float boostPressurePSI = 0.0;
  uint16_t engineRPM = 0;
  uint8_t vehicleSpeed = 0;
  uint8_t throttlePosition = 0;
  bool valid = false;
  unsigned long timestamp = 0;
  unsigned long lastUpdate = 0;
} dlc3Data;

// Timing
unsigned long lastWiFiCheck = 0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  ECU DLC3 Wireless Receiver");
  Serial.println("  Main ESP32 (Qualia)");
  Serial.println("========================================\n");
  
  // Initialize WiFi as Access Point (so K-Line module can connect)
  Serial.println("Initializing WiFi Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("✓ AP started - IP: ");
  Serial.println(IP);
  
  // Initialize UDP
  udp.begin(UDP_PORT);
  Serial.println("✓ UDP receiver initialized");
  Serial.print("  Listening on port: ");
  Serial.println(UDP_PORT);
  
  Serial.println("\nSystem Ready - Receiving DLC3 Data...\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long now = millis();
  
  // Check for incoming UDP packets
  receiveDLC3Data();
  
  // Use DLC3 data in your gauge system
  // You can use dlc3Data.coolantTemp, etc. here
  // This data can supplement or replace analog intercept readings
  
  // Example: Print data periodically
  static unsigned long lastPrint = 0;
  if (now - lastPrint > 1000) {  // Every second
    lastPrint = now;
    if (dlc3Data.valid) {
      printDLC3Data();
    } else {
      Serial.println("Waiting for DLC3 data...");
    }
  }
  
  delay(10);
}

// ============================================================================
// UDP DATA RECEPTION
// ============================================================================

void receiveDLC3Data() {
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    // Read packet
    char packetBuffer[512];
    int len = udp.read(packetBuffer, 511);
    if (len > 0) {
      packetBuffer[len] = 0;  // Null terminate
    }
    
    // Parse JSON
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, packetBuffer);
    
    if (!error) {
      // Extract data
      dlc3Data.coolantTemp = doc["coolantTemp"] | 0.0;
      dlc3Data.intakeAirTemp = doc["intakeAirTemp"] | 0.0;
      dlc3Data.manifoldPressureKPa = doc["manifoldPressureKPa"] | 0.0;
      dlc3Data.boostPressurePSI = doc["boostPressurePSI"] | 0.0;
      dlc3Data.engineRPM = doc["engineRPM"] | 0;
      dlc3Data.vehicleSpeed = doc["vehicleSpeed"] | 0;
      dlc3Data.throttlePosition = doc["throttlePosition"] | 0;
      dlc3Data.timestamp = doc["timestamp"] | 0;
      dlc3Data.valid = doc["valid"] | false;
      dlc3Data.lastUpdate = millis();
      
      // Data received successfully
    } else {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
    }
  }
}

// ============================================================================
// DATA USAGE EXAMPLES
// ============================================================================

// Example: Get coolant temperature (prefer DLC3, fallback to analog)
float getCoolantTemperature() {
  // Check if DLC3 data is recent (within 1 second)
  if (dlc3Data.valid && (millis() - dlc3Data.lastUpdate < 1000)) {
    return dlc3Data.coolantTemp;  // Use DLC3 data
  } else {
    // Fallback to analog intercept reading
    // return readAnalogCoolantTemp();
    return 0.0;  // Placeholder
  }
}

// Example: Get boost pressure
float getBoostPressure() {
  if (dlc3Data.valid && (millis() - dlc3Data.lastUpdate < 1000)) {
    return dlc3Data.boostPressurePSI;
  } else {
    // Fallback to analog intercept
    // return readAnalogBoost();
    return 0.0;
  }
}

// ============================================================================
// DEBUG OUTPUT
// ============================================================================

void printDLC3Data() {
  Serial.println("\n=== DLC3 Data ===");
  Serial.print("Coolant Temp: ");
  Serial.print(dlc3Data.coolantTemp, 1);
  Serial.println(" °C");
  
  Serial.print("Intake Air Temp: ");
  Serial.print(dlc3Data.intakeAirTemp, 1);
  Serial.println(" °C");
  
  Serial.print("Boost Pressure: ");
  Serial.print(dlc3Data.boostPressurePSI, 1);
  Serial.println(" PSI");
  
  Serial.print("Engine RPM: ");
  Serial.println(dlc3Data.engineRPM);
  
  Serial.print("Vehicle Speed: ");
  Serial.print(dlc3Data.vehicleSpeed);
  Serial.println(" km/h");
  
  Serial.print("Throttle Position: ");
  Serial.print(dlc3Data.throttlePosition);
  Serial.println(" %");
  
  Serial.print("Last Update: ");
  Serial.print(millis() - dlc3Data.lastUpdate);
  Serial.println(" ms ago");
  Serial.println();
}
