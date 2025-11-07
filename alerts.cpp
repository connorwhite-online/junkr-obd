/*
 * Alert System - Implementation
 * 
 * Monitors sensor thresholds and triggers audio/visual warnings
 */

#include "alerts.h"
#include "config.h"

// ============================================================================
// INTERNAL STATE
// ============================================================================

static uint8_t currentAlertLevel = ALERT_NONE;
static uint8_t previousAlertLevel = ALERT_NONE;
static bool audioEnabled = true;
static bool alertAcknowledged = false;

// Alert message buffer
static char alertMessage[64] = "";

// Buzzer state machine
static unsigned long lastBeepTime = 0;
static unsigned long lastAlertCheck = 0;
static int beepCount = 0;
static int beepsRemaining = 0;
static bool buzzerActive = false;
static unsigned long beepDuration = BEEP_SHORT;
static unsigned long beepPause = BEEP_PAUSE;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Start buzzer (generates tone)
 */
void startBuzzer() {
  if (!audioEnabled) return;
  tone(PIN_BUZZER, BUZZER_FREQUENCY);
  buzzerActive = true;
}

/**
 * Stop buzzer
 */
void stopBuzzer() {
  noTone(PIN_BUZZER);
  buzzerActive = false;
}

/**
 * Check a specific condition and update alert level/message
 */
void checkCondition(
  float value,
  float warningThreshold,
  float criticalThreshold,
  float dangerThreshold,
  const char* paramName,
  const char* unit,
  bool higherIsBad
) {
  uint8_t level = ALERT_NONE;
  
  if (higherIsBad) {
    // Higher values are dangerous (most cases)
    if (value >= dangerThreshold) {
      level = ALERT_DANGER;
      snprintf(alertMessage, sizeof(alertMessage), "%s DANGER: %.1f%s", 
               paramName, value, unit);
    } else if (value >= criticalThreshold) {
      level = ALERT_CRITICAL;
      snprintf(alertMessage, sizeof(alertMessage), "%s CRITICAL: %.1f%s", 
               paramName, value, unit);
    } else if (value >= warningThreshold) {
      level = ALERT_WARNING;
      snprintf(alertMessage, sizeof(alertMessage), "%s Warning: %.1f%s", 
               paramName, value, unit);
    }
  } else {
    // Lower values are dangerous (e.g., battery voltage)
    if (value <= dangerThreshold) {
      level = ALERT_DANGER;
      snprintf(alertMessage, sizeof(alertMessage), "%s DANGER: %.1f%s", 
               paramName, value, unit);
    } else if (value <= criticalThreshold) {
      level = ALERT_CRITICAL;
      snprintf(alertMessage, sizeof(alertMessage), "%s CRITICAL: %.1f%s", 
               paramName, value, unit);
    } else if (value <= warningThreshold) {
      level = ALERT_WARNING;
      snprintf(alertMessage, sizeof(alertMessage), "%s Warning: %.1f%s", 
               paramName, value, unit);
    }
  }
  
  // Update global alert level (keep highest priority)
  if (level > currentAlertLevel) {
    currentAlertLevel = level;
  }
}

// ============================================================================
// PUBLIC FUNCTIONS - INITIALIZATION
// ============================================================================

void Alerts_Init() {
  // Configure buzzer pin as output
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  
  // Reset state
  currentAlertLevel = ALERT_NONE;
  previousAlertLevel = ALERT_NONE;
  alertAcknowledged = false;
  beepsRemaining = 0;
  buzzerActive = false;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alerts: System initialized"));
  #endif
  
  // Play startup beep (single short beep)
  Alerts_PlayBeeps(1, BEEP_SHORT);
}

// ============================================================================
// PUBLIC FUNCTIONS - ALERT CHECKING
// ============================================================================

uint8_t Alerts_CheckAll(
  float boostPSI,
  float iatPre,
  float iatPost,
  float exhaustTemp,
  float coolantTemp
) {
  // Throttle alert checks
  unsigned long now = millis();
  if (now - lastAlertCheck < ALERT_CHECK_INTERVAL) {
    return currentAlertLevel;
  }
  lastAlertCheck = now;
  
  // Reset alert level for this check
  previousAlertLevel = currentAlertLevel;
  currentAlertLevel = ALERT_NONE;
  alertMessage[0] = '\0';  // Clear message
  
  // Check boost pressure
  checkCondition(
    boostPSI,
    BOOST_WARNING_PSI,
    BOOST_CRITICAL_PSI,
    BOOST_DANGER_PSI,
    "BOOST",
    " PSI",
    true  // Higher is bad
  );
  
  // Check intake temperature - Pre-Intercooler
  checkCondition(
    iatPre,
    IAT_PRE_WARNING,
    IAT_PRE_CRITICAL,
    IAT_PRE_CRITICAL + 20.0,  // Danger threshold
    "IAT PRE",
    "C",
    true
  );
  
  // Check intake temperature - Post-Intercooler
  checkCondition(
    iatPost,
    IAT_POST_WARNING,
    IAT_POST_CRITICAL,
    IAT_POST_CRITICAL + 10.0,  // Danger threshold
    "IAT POST",
    "C",
    true
  );
  
  // Check exhaust temperature
  checkCondition(
    exhaustTemp,
    EGT_WARNING,
    EGT_CRITICAL,
    EGT_DANGER,
    "EGT",
    "C",
    true
  );
  
  // Check coolant temperature
  checkCondition(
    coolantTemp,
    COOLANT_WARNING,
    COOLANT_CRITICAL,
    COOLANT_DANGER,
    "COOLANT",
    "C",
    true
  );
  
  
  // If alert level changed, reset acknowledgment
  if (currentAlertLevel != previousAlertLevel) {
    alertAcknowledged = false;
    
    #if ENABLE_SERIAL_DEBUG
    if (currentAlertLevel > ALERT_NONE) {
      Serial.print(F("Alert: Level "));
      Serial.print(currentAlertLevel);
      Serial.print(F(" - "));
      Serial.println(alertMessage);
    }
    #endif
  }
  
  // If all clear, set appropriate message
  if (currentAlertLevel == ALERT_NONE) {
    strcpy(alertMessage, "All Systems Normal");
  }
  
  return currentAlertLevel;
}

uint8_t Alerts_GetLevel() {
  return currentAlertLevel;
}

void Alerts_GetMessage(char* buffer, size_t bufferSize) {
  strncpy(buffer, alertMessage, bufferSize - 1);
  buffer[bufferSize - 1] = '\0';  // Ensure null termination
}

// ============================================================================
// PUBLIC FUNCTIONS - ALERT CONTROL
// ============================================================================

void Alerts_SetLevel(uint8_t level) {
  previousAlertLevel = currentAlertLevel;
  currentAlertLevel = level;
  alertAcknowledged = false;
}

void Alerts_Acknowledge() {
  alertAcknowledged = true;
  stopBuzzer();
  beepsRemaining = 0;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alert: Acknowledged"));
  #endif
}

void Alerts_Reset() {
  currentAlertLevel = ALERT_NONE;
  previousAlertLevel = ALERT_NONE;
  alertAcknowledged = false;
  alertMessage[0] = '\0';
  stopBuzzer();
  beepsRemaining = 0;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alert: Reset"));
  #endif
}

void Alerts_SetAudioEnabled(bool enabled) {
  audioEnabled = enabled;
  if (!enabled) {
    stopBuzzer();
    beepsRemaining = 0;
  }
  
  #if ENABLE_SERIAL_DEBUG
  Serial.print(F("Alert: Audio "));
  Serial.println(enabled ? F("enabled") : F("disabled"));
  #endif
}

bool Alerts_IsAudioEnabled() {
  return audioEnabled;
}

// ============================================================================
// PUBLIC FUNCTIONS - BUZZER CONTROL
// ============================================================================

void Alerts_Update() {
  if (!audioEnabled) {
    return;
  }
  
  // Handle beep patterns based on alert level
  if (currentAlertLevel > ALERT_NONE && !alertAcknowledged) {
    unsigned long now = millis();
    
    // Determine beep pattern for current alert level
    int targetBeeps = 0;
    unsigned long repeatInterval = BEEP_REPEAT_DELAY;
    
    switch (currentAlertLevel) {
      case ALERT_INFO:
        targetBeeps = 1;
        beepDuration = BEEP_SHORT;
        repeatInterval = 5000;  // 5 seconds
        break;
      case ALERT_WARNING:
        targetBeeps = 2;
        beepDuration = BEEP_MEDIUM;
        repeatInterval = 3000;  // 3 seconds
        break;
      case ALERT_CRITICAL:
        targetBeeps = 3;
        beepDuration = BEEP_MEDIUM;
        repeatInterval = 2000;  // 2 seconds
        break;
      case ALERT_DANGER:
        targetBeeps = 5;
        beepDuration = BEEP_LONG;
        repeatInterval = 1000;  // 1 second (continuous)
        break;
    }
    
    // State machine for beep pattern
    if (beepsRemaining == 0) {
      // Start new beep sequence
      if (now - lastBeepTime >= repeatInterval) {
        beepsRemaining = targetBeeps;
        beepCount = 0;
      }
    } else {
      // Continue beep sequence
      if (buzzerActive) {
        // Buzzer is on - check if beep duration elapsed
        if (now - lastBeepTime >= beepDuration) {
          stopBuzzer();
          lastBeepTime = now;
          beepCount++;
        }
      } else {
        // Buzzer is off - check if pause duration elapsed
        if (now - lastBeepTime >= beepPause) {
          if (beepCount < beepsRemaining) {
            // Start next beep
            startBuzzer();
            lastBeepTime = now;
          } else {
            // Sequence complete
            beepsRemaining = 0;
            lastBeepTime = now;
          }
        }
      }
    }
  } else {
    // No active alert or acknowledged - ensure buzzer is off
    if (buzzerActive) {
      stopBuzzer();
      beepsRemaining = 0;
    }
  }
}

void Alerts_PlayBeeps(int count, unsigned long duration) {
  beepsRemaining = count;
  beepCount = 0;
  beepDuration = duration;
  beepPause = BEEP_PAUSE;
  lastBeepTime = millis();
  
  // Start first beep immediately
  if (count > 0) {
    startBuzzer();
  }
}

void Alerts_StopBuzzer() {
  stopBuzzer();
  beepsRemaining = 0;
}
