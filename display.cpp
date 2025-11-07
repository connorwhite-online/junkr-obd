/*
 * Display Manager - Implementation
 * 
 * Handles all communication with the Nextion HMI touchscreen display
 */

#include "display.h"
#include "config.h"

// ============================================================================
// INTERNAL STATE
// ============================================================================

static uint8_t currentPage = PAGE_MAIN;
static bool displayInitialized = false;

// Message buffer for receiving data from display
#define RX_BUFFER_SIZE 64
static uint8_t rxBuffer[RX_BUFFER_SIZE];
static int rxIndex = 0;

// Update throttling
static unsigned long lastGaugeUpdate = 0;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Send command terminator (three 0xFF bytes)
 */
void sendTerminator() {
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
  NEXTION_SERIAL.write(0xFF);
}

/**
 * Wait for response from display (optional)
 */
bool waitForResponse(unsigned long timeout = 100) {
  unsigned long start = millis();
  while (millis() - start < timeout) {
    if (NEXTION_SERIAL.available()) {
      return true;
    }
    delay(1);
  }
  return false;
}

// ============================================================================
// PUBLIC FUNCTIONS - INITIALIZATION
// ============================================================================

void Display_Init() {
  // Initialize serial communication with Nextion
  NEXTION_SERIAL.begin(NEXTION_BAUD);
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Display: Initializing Nextion..."));
  #endif
  
  // Wait for display to boot
  delay(500);
  
  // Send reset command (optional, uncomment if needed)
  // Display_SendCommand("rest");
  // delay(1000);
  
  // Set to main page
  Display_SetPage(PAGE_MAIN);
  delay(100);
  
  // Set initial brightness (0-100)
  Display_SendCommand("dim=100");
  delay(50);
  
  // Clear any startup messages
  Display_SetStatus("JNKR Gauge");
  Display_ClearAlert();
  
  displayInitialized = true;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Display: Nextion initialized"));
  #endif
}

// ============================================================================
// PUBLIC FUNCTIONS - GAUGE UPDATES
// ============================================================================

void Display_UpdateGauges(
  float boostPSI,
  float intakeTemp,
  float exhaustTemp,
  float coolantTemp,
  float oilTemp,
  float batteryV
) {
  // Throttle updates to prevent overwhelming the display
  unsigned long now = millis();
  if (now - lastGaugeUpdate < DISPLAY_UPDATE_INTERVAL) {
    return;
  }
  lastGaugeUpdate = now;
  
  // Update each gauge
  Display_UpdateBoost(boostPSI);
  Display_UpdateIntakeTemp(intakeTemp);
  Display_UpdateExhaustTemp(exhaustTemp);
  Display_UpdateCoolantTemp(coolantTemp);
  Display_UpdateOilTemp(oilTemp);
  Display_UpdateBatteryVoltage(batteryV);
}

void Display_UpdateBoost(float boostPSI) {
  // Format to 1 decimal place
  char buffer[16];
  dtostrf(boostPSI, 0, 1, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_BOOST_VALUE, buffer);
  Display_SendCommand(cmd);
  
  // Set color based on value
  uint16_t color = COLOR_GREEN;
  if (boostPSI > BOOST_CRITICAL_PSI) {
    color = COLOR_RED;
  } else if (boostPSI > BOOST_WARNING_PSI) {
    color = COLOR_YELLOW;
  }
  Display_SetColor(NEXTION_BOOST_VALUE, "pco", color);
}

void Display_UpdateIntakeTemp(float temp) {
  // Format to 0 decimal places
  char buffer[16];
  dtostrf(temp, 0, 0, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_IAT_VALUE, buffer);
  Display_SendCommand(cmd);
  
  // Set color based on value
  uint16_t color = COLOR_GREEN;
  if (temp > IAT_CRITICAL) {
    color = COLOR_RED;
  } else if (temp > IAT_WARNING) {
    color = COLOR_YELLOW;
  }
  Display_SetColor(NEXTION_IAT_VALUE, "pco", color);
}

void Display_UpdateExhaustTemp(float temp) {
  // Format to 0 decimal places
  char buffer[16];
  dtostrf(temp, 0, 0, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_EGT_VALUE, buffer);
  Display_SendCommand(cmd);
  
  // Set color based on value
  uint16_t color = COLOR_GREEN;
  if (temp > EGT_DANGER) {
    color = COLOR_RED;
  } else if (temp > EGT_CRITICAL) {
    color = COLOR_ORANGE;
  } else if (temp > EGT_WARNING) {
    color = COLOR_YELLOW;
  }
  Display_SetColor(NEXTION_EGT_VALUE, "pco", color);
}

void Display_UpdateCoolantTemp(float temp) {
  // Format to 0 decimal places
  char buffer[16];
  dtostrf(temp, 0, 0, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_COOLANT_VALUE, buffer);
  Display_SendCommand(cmd);
  
  // Set color based on value
  uint16_t color = COLOR_GREEN;
  if (temp > COOLANT_DANGER) {
    color = COLOR_RED;
  } else if (temp > COOLANT_CRITICAL) {
    color = COLOR_ORANGE;
  } else if (temp > COOLANT_WARNING) {
    color = COLOR_YELLOW;
  }
  Display_SetColor(NEXTION_COOLANT_VALUE, "pco", color);
}

void Display_UpdateOilTemp(float temp) {
  // Format to 0 decimal places
  char buffer[16];
  dtostrf(temp, 0, 0, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_OIL_VALUE, buffer);
  Display_SendCommand(cmd);
  
  // Set color based on value
  uint16_t color = COLOR_GREEN;
  if (temp > OIL_CRITICAL) {
    color = COLOR_RED;
  } else if (temp > OIL_WARNING) {
    color = COLOR_YELLOW;
  }
  Display_SetColor(NEXTION_OIL_VALUE, "pco", color);
}

void Display_UpdateBatteryVoltage(float voltage) {
  // Format to 1 decimal place
  char buffer[16];
  dtostrf(voltage, 0, 1, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_BATTERY_VALUE, buffer);
  Display_SendCommand(cmd);
  
  // Set color based on value
  uint16_t color = COLOR_GREEN;
  if (voltage < BATTERY_MIN || voltage > BATTERY_MAX) {
    color = COLOR_RED;
  }
  Display_SetColor(NEXTION_BATTERY_VALUE, "pco", color);
}

// ============================================================================
// PUBLIC FUNCTIONS - STATUS AND ALERTS
// ============================================================================

void Display_SetStatus(const char* status) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_STATUS_TEXT, status);
  Display_SendCommand(cmd);
}

void Display_ShowAlert(const char* message, uint16_t color) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", NEXTION_ALERT_TEXT, message);
  Display_SendCommand(cmd);
  
  // Set alert text color
  Display_SetColor(NEXTION_ALERT_TEXT, "pco", color);
  
  // Make alert visible
  snprintf(cmd, sizeof(cmd), "vis %s,1", NEXTION_ALERT_TEXT);
  Display_SendCommand(cmd);
}

void Display_ClearAlert() {
  // Clear alert text
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"\"", NEXTION_ALERT_TEXT);
  Display_SendCommand(cmd);
  
  // Hide alert component
  snprintf(cmd, sizeof(cmd), "vis %s,0", NEXTION_ALERT_TEXT);
  Display_SendCommand(cmd);
}

void Display_SetAlertLevel(uint8_t level) {
  uint16_t color;
  const char* statusText;
  
  switch (level) {
    case ALERT_NONE:
      color = COLOR_GREEN;
      statusText = "OK";
      break;
    case ALERT_INFO:
      color = COLOR_WHITE;
      statusText = "INFO";
      break;
    case ALERT_WARNING:
      color = COLOR_YELLOW;
      statusText = "WARNING";
      break;
    case ALERT_CRITICAL:
      color = COLOR_ORANGE;
      statusText = "CRITICAL";
      break;
    case ALERT_DANGER:
      color = COLOR_RED;
      statusText = "DANGER";
      break;
    default:
      color = COLOR_WHITE;
      statusText = "UNKNOWN";
      break;
  }
  
  Display_SetStatus(statusText);
  Display_SetColor(NEXTION_STATUS_TEXT, "pco", color);
}

// ============================================================================
// PUBLIC FUNCTIONS - PAGE CONTROL
// ============================================================================

void Display_SetPage(uint8_t pageNum) {
  char cmd[16];
  snprintf(cmd, sizeof(cmd), "page %d", pageNum);
  Display_SendCommand(cmd);
  currentPage = pageNum;
}

uint8_t Display_GetCurrentPage() {
  return currentPage;
}

// ============================================================================
// PUBLIC FUNCTIONS - LOW-LEVEL COMMANDS
// ============================================================================

void Display_SendCommand(const char* cmd) {
  if (!displayInitialized) {
    return;  // Don't send commands before initialization
  }
  
  NEXTION_SERIAL.print(cmd);
  sendTerminator();
  NEXTION_SERIAL.flush();
  
  // Small delay for display to process
  delayMicroseconds(100);
}

void Display_SetNumber(const char* component, int value) {
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.val=%d", component, value);
  Display_SendCommand(cmd);
}

void Display_SetText(const char* component, const char* text) {
  char cmd[64];
  snprintf(cmd, sizeof(cmd), "%s.txt=\"%s\"", component, text);
  Display_SendCommand(cmd);
}

void Display_SetColor(const char* component, const char* colorAttribute, uint16_t color) {
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "%s.%s=%u", component, colorAttribute, color);
  Display_SendCommand(cmd);
}

void Display_ProcessMessages() {
  // Read any incoming messages from display
  while (NEXTION_SERIAL.available()) {
    uint8_t byte = NEXTION_SERIAL.read();
    
    // Check for message terminator (0xFF)
    if (byte == 0xFF) {
      static int terminatorCount = 0;
      terminatorCount++;
      
      if (terminatorCount >= 3) {
        // End of message - process buffer if needed
        if (rxIndex > 0) {
          // Process message here (touch events, etc.)
          // For now, just clear buffer
          rxIndex = 0;
        }
        terminatorCount = 0;
      }
    } else {
      // Add byte to buffer
      if (rxIndex < RX_BUFFER_SIZE - 1) {
        rxBuffer[rxIndex++] = byte;
      }
    }
  }
}
