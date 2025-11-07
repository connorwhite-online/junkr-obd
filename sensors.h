/*
 * Sensor Library - Header
 * 
 * Handles all sensor reading and calibration for the JNKR Gauge System
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize all sensors and configure pins
 * Must be called once during setup()
 */
void Sensors_Init();

// ============================================================================
// SENSOR UPDATE
// ============================================================================

/**
 * Update all sensor readings
 * Should be called regularly (every 100ms recommended)
 */
void Sensors_Update();

// ============================================================================
// TEMPERATURE READINGS
// ============================================================================

/**
 * Get intake air temperature - Pre-Intercooler (°C)
 * @return Temperature in Celsius
 */
float Sensors_GetIntakeTempPre();

/**
 * Get intake air temperature - Post-Intercooler (°C)
 * @return Temperature in Celsius
 */
float Sensors_GetIntakeTempPost();

/**
 * Get exhaust gas temperature (°C)
 * @return Temperature in Celsius
 */
float Sensors_GetExhaustTemp();

/**
 * Get coolant temperature (°C)
 * @return Temperature in Celsius
 */
float Sensors_GetCoolantTemp();

// ============================================================================
// PRESSURE READINGS
// ============================================================================

/**
 * Get boost pressure in bar (gauge)
 * Note: Returns gauge pressure (0 = atmospheric)
 * @return Pressure in bar gauge
 */
float Sensors_GetBoostBar();

/**
 * Get boost pressure in PSI (gauge)
 * Note: Returns gauge pressure (0 = atmospheric)
 * @return Pressure in PSI gauge
 */
float Sensors_GetBoostPSI();

/**
 * Get absolute manifold pressure in bar
 * @return Pressure in bar absolute
 */
float Sensors_GetBoostAbsolute();


// ============================================================================
// SENSOR STATUS
// ============================================================================

/**
 * Check if a specific sensor has a fault
 * @param sensorFlag Fault flag to check (FAULT_IAT_SENSOR, etc.)
 * @return true if sensor is faulted
 */
bool Sensors_IsFaulted(uint16_t sensorFlag);

/**
 * Get all fault flags as a bitmask
 * @return Fault flags (bitwise OR of FAULT_* constants)
 */
uint16_t Sensors_GetFaultFlags();

/**
 * Check if EGT sensor is connected and working
 * @return true if EGT sensor is available
 */
bool Sensors_IsEGTAvailable();

/**
 * Clear all fault flags (reset after fixing issue)
 */
void Sensors_ClearFaults();

// ============================================================================
// CALIBRATION (Optional - for advanced users)
// ============================================================================

/**
 * Set calibration offset for intake temperature - Pre-IC
 * @param offset Offset to add to reading (°C)
 */
void Sensors_SetIATPreOffset(float offset);

/**
 * Set calibration offset for intake temperature - Post-IC
 * @param offset Offset to add to reading (°C)
 */
void Sensors_SetIATPostOffset(float offset);

/**
 * Set calibration offset for boost pressure
 * @param offset Offset to add to reading (bar)
 */
void Sensors_SetBoostOffset(float offset);

/**
 * Reset all calibration offsets to zero
 */
void Sensors_ResetCalibration();

#endif // SENSORS_H
