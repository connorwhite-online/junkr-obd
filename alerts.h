/*
 * Alert System - Header
 * 
 * Monitors sensor values and triggers audio/visual alerts
 */

#ifndef ALERTS_H
#define ALERTS_H

#include <Arduino.h>

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize alert system
 * Configures speaker module and internal state
 */
void Alerts_Init();

// ============================================================================
// ALERT CHECKING
// ============================================================================

/**
 * Check all sensor values and determine alert level
 * Should be called regularly (every 500ms recommended)
 * 
 * @param boostPSI Boost pressure in PSI (gauge)
 * @param iatPre Intake air temperature pre-IC in °C
 * @param iatPost Intake air temperature post-IC in °C
 * @param exhaustTemp Exhaust gas temperature in °C
 * @param coolantTemp Coolant temperature in °C
 * @return Current alert level (ALERT_NONE, ALERT_WARNING, etc.)
 */
uint8_t Alerts_CheckAll(
  float boostPSI,
  float iatPre,
  float iatPost,
  float exhaustTemp,
  float coolantTemp
);

/**
 * Get the current active alert level
 * @return Alert level (ALERT_NONE, ALERT_WARNING, etc.)
 */
uint8_t Alerts_GetLevel();

/**
 * Get descriptive message for current alert
 * @param buffer Buffer to write message into
 * @param bufferSize Size of buffer
 */
void Alerts_GetMessage(char* buffer, size_t bufferSize);

// ============================================================================
// ALERT CONTROL
// ============================================================================

/**
 * Manually set alert level (for testing)
 * @param level Alert level to set
 */
void Alerts_SetLevel(uint8_t level);

/**
 * Clear/acknowledge current alert
 * Stops buzzer but doesn't reset alert condition
 */
void Alerts_Acknowledge();

/**
 * Reset all alert conditions
 */
void Alerts_Reset();

/**
 * Enable or disable audio alerts
 * @param enabled true to enable speaker, false to disable
 */
void Alerts_SetAudioEnabled(bool enabled);

/**
 * Check if audio alerts are enabled
 * @return true if speaker is enabled
 */
bool Alerts_IsAudioEnabled();

/**
 * Set speaker volume
 * @param volume Volume level (0-30)
 */
void Alerts_SetVolume(uint8_t volume);

// ============================================================================
// SPEAKER CONTROL
// ============================================================================

/**
 * Update speaker state (call regularly in main loop)
 * Handles sound playback for different alert levels
 */
void Alerts_Update();

/**
 * Play a specific sound file
 * @param soundNumber Sound file number (1-17)
 */
void Alerts_PlaySound(uint8_t soundNumber);

/**
 * Stop speaker immediately
 */
void Alerts_StopSpeaker();

/**
 * Check if speaker module is ready
 * @return true if module is initialized and responding
 */
bool Alerts_IsSpeakerReady();

#endif // ALERTS_H
