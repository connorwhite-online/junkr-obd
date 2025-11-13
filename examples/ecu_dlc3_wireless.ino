/*
 * ECU DLC3 K-Line Wireless Reader
 * 
 * Reads engine parameters from 1KZ-TE ECU via DLC3 diagnostic port
 * using K-Line (ISO 9141-2) protocol with wireless communication.
 * 
 * This allows the main ESP32 (Qualia) to be mounted in the dash
 * while a small module near the DLC3 connector handles K-Line communication.
 * 
 * Architecture:
 *   DLC3 Connector → K-Line Module (ESP32) → WiFi/BLE → Main ESP32 (Qualia)
 * 
 * Hardware:
 *   - Small ESP32 module (ESP32-C3 Mini or similar) near DLC3 connector
 *   - MC33290 or L9637D K-Line transceiver
 *   - Main Qualia ESP32-S3 in dash (receives data wirelessly)
 * 
 * This example is for the K-LINE MODULE (near DLC3 connector)
 * A separate receiver example will be created for the main Qualia ESP32
 * 
 * Based on: OBD9141 library (https://github.com/iwanders/OBD9141)
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

// WiFi Configuration (for wireless communication)
#define WIFI_SSID "JNKR_Gauge"
#define WIFI_PASSWORD "your_password_here"
#define UDP_PORT 12345
#define TARGET_IP "192.168.4.1"  // Main ESP32 IP (or use broadcast)

// K-Line Configuration
#define KLINE_TX_PIN 17  // ESP32 TX → MC33290 RX
#define KLINE_RX_PIN 16  // ESP32 RX ← MC33290 TX
#define KLINE_ENABLE_PIN 4  // Optional: MC33290 enable

// Update interval (milliseconds)
#define UPDATE_INTERVAL 200  // 5Hz refresh rate

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

WiFiUDP udp;
HardwareSerial KLineSerial(1);  // Use UART1 for K-Line

// OBD9141 library would be included here
// #include <OBD9141.h>
// OBD9141 obd(KLineSerial);

// Sensor readings
struct EngineData {
  float coolantTemp = 0.0;
  float intakeAirTemp = 0.0;
  float manifoldPressureKPa = 0.0;
  float boostPressurePSI = 0.0;
  uint16_t engineRPM = 0;
  uint8_t vehicleSpeed = 0;
  uint8_t throttlePosition = 0;
  bool valid = false;
  unsigned long timestamp = 0;
} engineData;

// Timing
unsigned long lastUpdate = 0;
unsigned long lastWiFiCheck = 0;

// ============================================================================
// SETUP
// ============================================================================

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("  ECU DLC3 K-Line Wireless Module");
  Serial.println("  Toyota 1KZ-TE Engine");
  Serial.println("========================================\n");
  
  // Initialize K-Line serial
  Serial.println("Initializing K-Line...");
  KLineSerial.begin(10400, SERIAL_8N1, KLINE_RX_PIN, KLINE_TX_PIN);
  
  // Initialize WiFi
  Serial.println("Initializing WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected!");
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  Target IP: ");
    Serial.println(TARGET_IP);
  } else {
    Serial.println("\n✗ WiFi connection failed");
    Serial.println("  Continuing in offline mode (serial only)");
  }
  
  // Initialize UDP
  udp.begin(UDP_PORT);
  Serial.println("✓ UDP initialized");
  
  // Initialize K-Line (would use OBD9141 library)
  Serial.println("\nInitializing K-Line communication...");
  // if (obd.init()) {
  //   Serial.println("✓ K-Line initialized");
  // } else {
  //   Serial.println("✗ K-Line initialization failed");
  // }
  
  Serial.println("\nSystem Ready - Reading DLC3 Data...\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================

void loop() {
  unsigned long now = millis();
  
  // Check WiFi connection periodically
  if (now - lastWiFiCheck > 10000) {  // Every 10 seconds
    lastWiFiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, attempting reconnect...");
      WiFi.reconnect();
    }
  }
  
  // Update at specified interval
  if (now - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = now;
    
    // Read engine data from DLC3
    readEngineData();
    
    // Send data wirelessly
    sendDataWirelessly();
    
    // Also print to serial for debugging
    printData();
  }
  
  delay(10);
}

// ============================================================================
// K-LINE DATA READING
// ============================================================================

void readEngineData() {
  // This would use OBD9141 library
  // For now, placeholder structure
  
  // Example using OBD9141 library:
  /*
  engineData.coolantTemp = obd.readPID(0x05) - 40.0;
  engineData.intakeAirTemp = obd.readPID(0x0F) - 40.0;
  engineData.manifoldPressureKPa = obd.readPID(0x0B);
  engineData.engineRPM = ((obd.readPID(0x0C, 0) * 256) + obd.readPID(0x0C, 1)) / 4;
  engineData.vehicleSpeed = obd.readPID(0x0D);
  engineData.throttlePosition = (obd.readPID(0x11) * 100) / 255;
  */
  
  // Calculate boost pressure (gauge = absolute - atmospheric)
  engineData.boostPressurePSI = (engineData.manifoldPressureKPa - 101.325) * 0.145038;
  
  engineData.timestamp = millis();
  engineData.valid = true;
}

// ============================================================================
// WIRELESS COMMUNICATION
// ============================================================================

void sendDataWirelessly() {
  if (WiFi.status() != WL_CONNECTED) {
    return;  // Skip if WiFi not connected
  }
  
  // Create JSON payload
  StaticJsonDocument<256> doc;
  doc["coolantTemp"] = engineData.coolantTemp;
  doc["intakeAirTemp"] = engineData.intakeAirTemp;
  doc["manifoldPressureKPa"] = engineData.manifoldPressureKPa;
  doc["boostPressurePSI"] = engineData.boostPressurePSI;
  doc["engineRPM"] = engineData.engineRPM;
  doc["vehicleSpeed"] = engineData.vehicleSpeed;
  doc["throttlePosition"] = engineData.throttlePosition;
  doc["timestamp"] = engineData.timestamp;
  doc["valid"] = engineData.valid;
  
  // Serialize to string
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Send via UDP
  udp.beginPacket(TARGET_IP, UDP_PORT);
  udp.write((const uint8_t*)jsonString.c_str(), jsonString.length());
  udp.endPacket();
  
  // Optional: Also send via broadcast for discovery
  // udp.beginPacket("255.255.255.255", UDP_PORT);
  // udp.write((const uint8_t*)jsonString.c_str(), jsonString.length());
  // udp.endPacket();
}

// ============================================================================
// SERIAL OUTPUT (DEBUGGING)
// ============================================================================

void printData() {
  static int lineCount = 0;
  
  // Print header every 20 lines
  if (lineCount % 20 == 0) {
    Serial.println("\n┌───────┬───────┬──────────┬──────────┬──────────┬────────┬──────────┐");
    Serial.println("│  RPM  │ Speed │ Coolant  │   IAT    │   MAP    │  Boost │ Throttle │");
    Serial.println("├───────┼───────┼──────────┼──────────┼──────────┼────────┼──────────┤");
  }
  
  Serial.print("│ ");
  Serial.print(engineData.engineRPM);
  Serial.print("  ");
  
  Serial.print("│ ");
  Serial.print(engineData.vehicleSpeed);
  Serial.print(" km/h ");
  
  Serial.print("│ ");
  if (engineData.valid) {
    Serial.print(engineData.coolantTemp, 1);
    Serial.print(" C ");
  } else {
    Serial.print("ERROR  ");
  }
  
  Serial.print("│ ");
  if (engineData.valid) {
    Serial.print(engineData.intakeAirTemp, 1);
    Serial.print(" C ");
  } else {
    Serial.print("ERROR  ");
  }
  
  Serial.print("│ ");
  if (engineData.valid) {
    Serial.print(engineData.manifoldPressureKPa, 1);
    Serial.print(" kPa ");
  } else {
    Serial.print("ERROR   ");
  }
  
  Serial.print("│ ");
  if (engineData.valid) {
    Serial.print(engineData.boostPressurePSI, 1);
    Serial.print(" PSI");
  } else {
    Serial.print("ERROR ");
  }
  
  Serial.print("│ ");
  Serial.print(engineData.throttlePosition);
  Serial.print(" %    ");
  
  Serial.println("│");
  
  lineCount++;
  
  // Print footer every 20 lines
  if (lineCount % 20 == 0) {
    Serial.println("└───────┴───────┴──────────┴──────────┴──────────┴────────┴──────────┘");
  }
}
