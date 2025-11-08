/*
 * Display Manager - Header
 * 
 * Handles display interface for JNKR Gauge System
 * Using LVGL on 2.1" Round RGB Display (Qualia ESP32-S3)
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize Nextion display
 * Must be called once during setup()
 */
void Display_Init();

// ============================================================================
// GAUGE UPDATES
// ============================================================================

/**
 * Update all gauge values on the display
 * @param boostPSI Boost pressure in PSI (gauge)
 * @param iatPre Intake air temperature pre-intercooler in °C
 * @param iatPost Intake air temperature post-intercooler in °C
 * @param exhaustTemp Exhaust gas temperature in °C
 * @param coolantTemp Coolant temperature in °C
 */
void Display_UpdateGauges(
  float boostPSI,
  float iatPre,
  float iatPost,
  float exhaustTemp,
  float coolantTemp
);

/**
 * Update boost pressure display
 * @param boostPSI Boost pressure in PSI (gauge)
 */
void Display_UpdateBoost(float boostPSI);

/**
 * Update intake temperature display - Pre-Intercooler
 * @param temp Temperature in °C
 */
void Display_UpdateIntakeTempPre(float temp);

/**
 * Update intake temperature display - Post-Intercooler
 * @param temp Temperature in °C
 */
void Display_UpdateIntakeTempPost(float temp);

/**
 * Update exhaust temperature display
 * @param temp Temperature in °C
 */
void Display_UpdateExhaustTemp(float temp);

/**
 * Update coolant temperature display
 * @param temp Temperature in °C
 */
void Display_UpdateCoolantTemp(float temp);

// ============================================================================
// STATUS AND ALERTS
// ============================================================================

/**
 * Update status text on display
 * @param status Status message
 */
void Display_SetStatus(const char* status);

/**
 * Show alert message on display
 * @param message Alert message
 * @param color Text color (COLOR_* constant from config.h)
 */
void Display_ShowAlert(const char* message, uint16_t color);

/**
 * Clear alert message
 */
void Display_ClearAlert();

/**
 * Set background color based on alert level
 * @param level Alert level (ALERT_NONE, ALERT_WARNING, etc.)
 */
void Display_SetAlertLevel(uint8_t level);

// ============================================================================
// PAGE CONTROL
// ============================================================================

/**
 * Change to a specific page
 * @param pageNum Page number (PAGE_MAIN, etc.)
 */
void Display_SetPage(uint8_t pageNum);

/**
 * Get current page number
 * @return Current page number
 */
uint8_t Display_GetCurrentPage();

// ============================================================================
// LOW-LEVEL COMMANDS
// ============================================================================

/**
 * Send a raw command to Nextion display
 * @param cmd Command string (without terminator bytes)
 */
void Display_SendCommand(const char* cmd);

/**
 * Update a numeric value component
 * @param component Component name (e.g., "nBoost")
 * @param value Value to set
 */
void Display_SetNumber(const char* component, int value);

/**
 * Update a text component
 * @param component Component name (e.g., "tStatus")
 * @param text Text to display
 */
void Display_SetText(const char* component, const char* text);

/**
 * Update a component color
 * @param component Component name
 * @param colorAttribute Color attribute ("pco", "bco", etc.)
 * @param color Color value (RGB565)
 */
void Display_SetColor(const char* component, const char* colorAttribute, uint16_t color);

/**
 * Process incoming messages from display
 * Call regularly in main loop
 */
void Display_ProcessMessages();

#endif // DISPLAY_H
