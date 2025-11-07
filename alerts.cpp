/*
 * Alert System - Implementation
 * 
 * Monitors sensor thresholds and triggers audio/visual warnings
 * Uses DFPlayer Mini MP3 module for preloaded sound alerts
 */

#include "alerts.h"
#include "config.h"
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// ============================================================================
// SPEAKER MODULE
// ============================================================================

// DFPlayer Mini communication
SoftwareSerial speakerSerial(PIN_SPEAKER_RX, PIN_SPEAKER_TX);
DFRobotDFPlayerMini dfPlayer;
bool speakerReady = false;

// ============================================================================
// INTERNAL STATE
// ============================================================================

static uint8_t currentAlertLevel = ALERT_NONE;
static uint8_t previousAlertLevel = ALERT_NONE;
static bool audioEnabled = true;
static bool alertAcknowledged = false;

// Alert message buffer
static char alertMessage[64] = "";

// Speaker state machine
static unsigned long lastSoundTime = 0;
static unsigned long lastAlertCheck = 0;
static uint8_t currentSoundFile = 0;
static bool soundPlaying = false;
static uint8_t currentVolume = SPEAKER_VOLUME;

// Alert-specific sound tracking
static uint8_t lastAlertType = ALERT_NONE;
static const char* lastAlertParam = "";

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

/**
 * Play sound through speaker module
 */
void playSpeaker(uint8_t soundNumber) {
  if (!audioEnabled || !speakerReady) return;
  
  dfPlayer.play(soundNumber);
  soundPlaying = true;
  currentSoundFile = soundNumber;
  lastSoundTime = millis();
  
  #if ENABLE_SERIAL_DEBUG
  Serial.print(F("Speaker: Playing sound "));
  Serial.println(soundNumber);
  #endif
}

/**
 * Stop speaker playback
 */
void stopSpeaker() {
  if (!speakerReady) return;
  
  dfPlayer.pause();
  soundPlaying = false;
  currentSoundFile = 0;
}

/**
 * Get sound file for specific alert type and parameter
 */
uint8_t getSoundForAlert(uint8_t alertLevel, const char* paramName) {
  // First, check for parameter-specific sounds
  if (strstr(paramName, "BOOST") != NULL) {
    switch (alertLevel) {
      case ALERT_WARNING:  return SOUND_BOOST_WARNING;
      case ALERT_CRITICAL: return SOUND_BOOST_CRITICAL;
      case ALERT_DANGER:   return SOUND_BOOST_DANGER;
    }
  }
  else if (strstr(paramName, "IAT") != NULL) {
    switch (alertLevel) {
      case ALERT_WARNING:  return SOUND_IAT_WARNING;
      case ALERT_CRITICAL: return SOUND_IAT_CRITICAL;
    }
  }
  else if (strstr(paramName, "EGT") != NULL) {
    switch (alertLevel) {
      case ALERT_WARNING:  return SOUND_EGT_WARNING;
      case ALERT_CRITICAL: return SOUND_EGT_CRITICAL;
      case ALERT_DANGER:   return SOUND_EGT_DANGER;
    }
  }
  else if (strstr(paramName, "COOLANT") != NULL) {
    switch (alertLevel) {
      case ALERT_WARNING:  return SOUND_COOLANT_WARNING;
      case ALERT_CRITICAL: return SOUND_COOLANT_CRITICAL;
      case ALERT_DANGER:   return SOUND_COOLANT_DANGER;
    }
  }
  
  // Fall back to generic alert sounds
  switch (alertLevel) {
    case ALERT_INFO:     return SOUND_INFO;
    case ALERT_WARNING:  return SOUND_WARNING;
    case ALERT_CRITICAL: return SOUND_CRITICAL;
    case ALERT_DANGER:   return SOUND_DANGER;
    default:             return 0;
  }
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
      lastAlertParam = paramName;
    } else if (value >= criticalThreshold) {
      level = ALERT_CRITICAL;
      snprintf(alertMessage, sizeof(alertMessage), "%s CRITICAL: %.1f%s", 
               paramName, value, unit);
      lastAlertParam = paramName;
    } else if (value >= warningThreshold) {
      level = ALERT_WARNING;
      snprintf(alertMessage, sizeof(alertMessage), "%s Warning: %.1f%s", 
               paramName, value, unit);
      lastAlertParam = paramName;
    }
  } else {
    // Lower values are dangerous (e.g., battery voltage)
    if (value <= dangerThreshold) {
      level = ALERT_DANGER;
      snprintf(alertMessage, sizeof(alertMessage), "%s DANGER: %.1f%s", 
               paramName, value, unit);
      lastAlertParam = paramName;
    } else if (value <= criticalThreshold) {
      level = ALERT_CRITICAL;
      snprintf(alertMessage, sizeof(alertMessage), "%s CRITICAL: %.1f%s", 
               paramName, value, unit);
      lastAlertParam = paramName;
    } else if (value <= warningThreshold) {
      level = ALERT_WARNING;
      snprintf(alertMessage, sizeof(alertMessage), "%s Warning: %.1f%s", 
               paramName, value, unit);
      lastAlertParam = paramName;
    }
  }
  
  // Update global alert level (keep highest priority)
  if (level > currentAlertLevel) {
    currentAlertLevel = level;
    lastAlertType = level;
  }
}

// ============================================================================
// PUBLIC FUNCTIONS - INITIALIZATION
// ============================================================================

void Alerts_Init() {
  // Initialize serial communication with DFPlayer Mini
  speakerSerial.begin(9600);
  delay(500);  // Give module time to initialize
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alerts: Initializing DFPlayer Mini speaker module..."));
  #endif
  
  // Initialize DFPlayer
  if (dfPlayer.begin(speakerSerial)) {
    speakerReady = true;
    
    // Configure speaker module
    dfPlayer.volume(currentVolume);  // Set volume (0-30)
    dfPlayer.EQ(SPEAKER_EQ_MODE);    // Set EQ mode
    
    #if ENABLE_SERIAL_DEBUG
    Serial.println(F("Alerts: Speaker module initialized"));
    Serial.print(F("  - Volume: "));
    Serial.println(currentVolume);
    Serial.print(F("  - EQ Mode: "));
    Serial.println(SPEAKER_EQ_MODE);
    Serial.print(F("  - Files available: "));
    Serial.println(dfPlayer.readFileCounts());
    #endif
    
    // Play startup sound
    delay(100);
    playSpeaker(SOUND_STARTUP);
  } else {
    speakerReady = false;
    
    #if ENABLE_SERIAL_DEBUG
    Serial.println(F("Alerts: ERROR - Failed to initialize speaker module"));
    Serial.println(F("  - Check wiring (RX/TX connections)"));
    Serial.println(F("  - Verify SD card is inserted with audio files"));
    Serial.println(F("  - Files should be named: 0001.mp3, 0002.mp3, etc."));
    #endif
  }
  
  // Reset state
  currentAlertLevel = ALERT_NONE;
  previousAlertLevel = ALERT_NONE;
  alertAcknowledged = false;
  soundPlaying = false;
  lastAlertType = ALERT_NONE;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alerts: System initialized"));
  #endif
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
  lastAlertParam = "";
  
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
  
  
  // If alert level changed, reset acknowledgment and play appropriate sound
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
    
    // Play alert sound immediately when level changes
    if (currentAlertLevel > ALERT_NONE && !alertAcknowledged) {
      uint8_t soundFile = getSoundForAlert(currentAlertLevel, lastAlertParam);
      if (soundFile > 0) {
        playSpeaker(soundFile);
      }
    }
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
  stopSpeaker();
  
  // Play acknowledgment sound
  if (speakerReady && audioEnabled) {
    delay(100);
    playSpeaker(SOUND_ACKNOWLEDGED);
  }
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alert: Acknowledged"));
  #endif
}

void Alerts_Reset() {
  currentAlertLevel = ALERT_NONE;
  previousAlertLevel = ALERT_NONE;
  alertAcknowledged = false;
  alertMessage[0] = '\0';
  stopSpeaker();
  lastAlertType = ALERT_NONE;
  
  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Alert: Reset"));
  #endif
}

void Alerts_SetAudioEnabled(bool enabled) {
  audioEnabled = enabled;
  if (!enabled) {
    stopSpeaker();
  }
  
  #if ENABLE_SERIAL_DEBUG
  Serial.print(F("Alert: Audio "));
  Serial.println(enabled ? F("enabled") : F("disabled"));
  #endif
}

bool Alerts_IsAudioEnabled() {
  return audioEnabled;
}

void Alerts_SetVolume(uint8_t volume) {
  // Constrain volume to valid range
  if (volume > 30) volume = 30;
  
  currentVolume = volume;
  
  if (speakerReady) {
    dfPlayer.volume(volume);
    
    #if ENABLE_SERIAL_DEBUG
    Serial.print(F("Alert: Volume set to "));
    Serial.println(volume);
    #endif
  }
}

// ============================================================================
// PUBLIC FUNCTIONS - SPEAKER CONTROL
// ============================================================================

void Alerts_Update() {
  if (!audioEnabled || !speakerReady) {
    return;
  }
  
  // Handle alert sound playback based on alert level
  if (currentAlertLevel > ALERT_NONE && !alertAcknowledged) {
    unsigned long now = millis();
    
    // Determine repeat interval for current alert level
    unsigned long repeatInterval;
    switch (currentAlertLevel) {
      case ALERT_INFO:
        repeatInterval = ALERT_REPEAT_INFO;
        break;
      case ALERT_WARNING:
        repeatInterval = ALERT_REPEAT_WARNING;
        break;
      case ALERT_CRITICAL:
        repeatInterval = ALERT_REPEAT_CRITICAL;
        break;
      case ALERT_DANGER:
        repeatInterval = ALERT_REPEAT_DANGER;
        break;
      default:
        repeatInterval = 5000;
    }
    
    // Check if it's time to repeat the alert sound
    if (now - lastSoundTime >= repeatInterval) {
      uint8_t soundFile = getSoundForAlert(currentAlertLevel, lastAlertParam);
      if (soundFile > 0) {
        playSpeaker(soundFile);
      }
    }
  }
  
  // Check for DFPlayer events/status
  if (dfPlayer.available()) {
    uint8_t type = dfPlayer.readType();
    int value = dfPlayer.read();
    
    switch (type) {
      case DFPlayerPlayFinished:
        soundPlaying = false;
        break;
        
      case DFPlayerError:
        #if ENABLE_SERIAL_DEBUG
        Serial.print(F("Speaker Error: "));
        Serial.println(value);
        #endif
        break;
    }
  }
}

void Alerts_PlaySound(uint8_t soundNumber) {
  if (soundNumber > 0 && soundNumber <= 17) {
    playSpeaker(soundNumber);
  }
}

void Alerts_StopSpeaker() {
  stopSpeaker();
}

bool Alerts_IsSpeakerReady() {
  return speakerReady;
}
