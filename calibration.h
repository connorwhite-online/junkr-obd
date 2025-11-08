/*
 * JNKR Gauge System - Sensor Calibration Configuration
 * 
 * This file contains calibration values for your specific sensors.
 * You can easily modify these values without changing the main code.
 * 
 * HOW TO USE:
 * 1. Measure the resistance of each sensor at a known temperature
 * 2. Update the values below to match YOUR sensors
 * 3. Upload the code to your Arduino/ESP32
 * 
 * Common automotive NTC thermistor values at 25°C:
 *   - GM sensors: ~2,450-2,500Ω
 *   - Ford sensors: ~2,070-2,500Ω  
 *   - Cummins sensors: ~2,200-2,300Ω
 *   - CAT sensors: ~2,250Ω
 *   - Generic 2.2kΩ: ~2,200-2,252Ω
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

// ============================================================================
// THERMISTOR CALIBRATION VALUES
// ============================================================================
// Each temperature sensor uses an NTC thermistor with a voltage divider.
// These values define the thermistor characteristics for accurate readings.

// === INTAKE AIR TEMPERATURE - PRE-INTERCOOLER ===
// This sensor measures air temperature coming from the turbo (hot side)
#define IAT_PRE_THERMISTOR_NOMINAL_R    2252.0   // Resistance at 25°C (Ohms)
#define IAT_PRE_THERMISTOR_BETA         3950     // Beta coefficient (K)
#define IAT_PRE_REF_RESISTOR            2200.0   // Reference resistor value (Ohms)
#define IAT_PRE_OFFSET                  0.0      // Temperature offset (°C) - adjust after testing

// === INTAKE AIR TEMPERATURE - POST-INTERCOOLER ===
// This sensor measures air temperature after the intercooler (cool side)
#define IAT_POST_THERMISTOR_NOMINAL_R   2252.0   // Resistance at 25°C (Ohms)
#define IAT_POST_THERMISTOR_BETA        3950     // Beta coefficient (K)
#define IAT_POST_REF_RESISTOR           2200.0   // Reference resistor value (Ohms)
#define IAT_POST_OFFSET                 0.0      // Temperature offset (°C) - adjust after testing

// === COOLANT TEMPERATURE ===
// This sensor measures engine coolant temperature
#define COOLANT_THERMISTOR_NOMINAL_R    2252.0   // Resistance at 25°C (Ohms)
#define COOLANT_THERMISTOR_BETA         3950     // Beta coefficient (K)
#define COOLANT_REF_RESISTOR            2200.0   // Reference resistor value (Ohms)
#define COOLANT_OFFSET                  0.0      // Temperature offset (°C) - adjust after testing

// ============================================================================
// BOOST/MAP SENSOR CALIBRATION
// ============================================================================
// MAP sensors typically output a linear voltage from 0.5V to 4.5V
// representing pressure from 0 to 3 bar absolute (or other ranges)

// === 3-BAR MAP SENSOR (most common) ===
#define MAP_VOLTAGE_MIN_CAL             0.5      // Voltage at 0 bar (V)
#define MAP_VOLTAGE_MAX_CAL             4.5      // Voltage at max pressure (V)
#define MAP_PRESSURE_MIN_CAL            0.0      // Minimum pressure (bar absolute)
#define MAP_PRESSURE_MAX_CAL            3.0      // Maximum pressure (bar absolute)
#define MAP_PRESSURE_OFFSET             0.0      // Pressure offset (bar) - adjust after testing

// ============================================================================
// EGT SENSOR CALIBRATION
// ============================================================================
// K-type thermocouples are typically accurate without calibration,
// but you can add an offset if needed

#define EGT_OFFSET                      0.0      // Temperature offset (°C)

// ============================================================================
// COMMON AUTOMOTIVE SENSOR SPECIFICATIONS
// ============================================================================
// Reference values for popular OEM and aftermarket sensors
// Measure your actual sensors and update the values above

/*
 * ===== GM SENSORS =====
 * 
 * GM Intake Air Temperature (IAT) Sensors:
 *   Part Numbers: 25037334, 25036751, 12146312
 *   - Resistance @ 25°C: ~2,450Ω
 *   - Beta: ~3,500K
 *   - Thread: 3/8" NPT or M10x1.0
 *   - Temperature Range: -40°C to 150°C
 * 
 * GM Coolant Temperature (ECT) Sensors:
 *   Part Numbers: 12146312, 25036751, 213-928
 *   - Resistance @ 25°C: ~2,450Ω  
 *   - Beta: ~3,500K
 *   - Thread: 3/8" NPT
 *   - Temperature Range: -40°C to 150°C
 * 
 * GM 3-Bar MAP Sensors:
 *   Part Numbers: 12223861, 16040749, 12247571
 *   - Output: 0.5V @ 0 bar, 4.5V @ 3 bar
 *   - Connector: 3-pin Delphi Weatherpack
 *   - Operating Voltage: 5V
 *   - Accuracy: ±2% full scale
 * 
 * 
 * ===== FORD SENSORS =====
 * 
 * Ford IAT/ECT Sensors:
 *   Part Numbers: F4TZ-12A648-BA, 3F2Z-12A648-BA
 *   - Resistance @ 25°C: ~2,070-2,500Ω (varies by application)
 *   - Beta: ~3,700K
 *   - Thread: 3/8" NPT or M12x1.5
 *   - Temperature Range: -40°C to 150°C
 * 
 * Ford MAP Sensors:
 *   Part Numbers: F77F-9F479-AA, 3F2Z-9F479-AB
 *   - Output: 0.5V @ 0 bar, 4.5V @ 3 bar
 *   - Connector: 3-pin Ford connector
 *   - Operating Voltage: 5V
 * 
 * 
 * ===== CUMMINS DIESEL SENSORS =====
 * 
 * Cummins Intake Air Temperature:
 *   Part Numbers: 4921475, 3417142
 *   - Resistance @ 25°C: ~2,200-2,300Ω
 *   - Beta: ~3,900K
 *   - Thread: M14x1.5 or 1/2" NPT
 *   - Temperature Range: -40°C to 200°C
 *   - Heavy-duty construction for harsh environments
 * 
 * Cummins Coolant Temperature:
 *   Part Numbers: 4087991, 3417115
 *   - Resistance @ 25°C: ~2,250Ω
 *   - Beta: ~3,950K
 *   - Thread: M14x1.5
 *   - Temperature Range: -40°C to 150°C
 * 
 * Cummins Boost Pressure Sensors:
 *   Part Numbers: 4921517, 4903577
 *   - Output: 0.5-4.5V (3-bar absolute) or 0.5-4.5V (5-bar absolute)
 *   - High vibration and temperature tolerance
 *   - Sealed military-spec connector
 * 
 * 
 * ===== CATERPILLAR (CAT) SENSORS =====
 * 
 * CAT Temperature Sensors:
 *   Part Numbers: 274-6721, 320-3060, 161-9926
 *   - Resistance @ 25°C: ~2,250Ω
 *   - Beta: ~3,950K
 *   - Thread: M14x1.5 or 1/2" NPT
 *   - Temperature Range: -40°C to 200°C
 *   - Industrial-grade construction
 * 
 * CAT Pressure Sensors:
 *   Part Numbers: 224-4536, 268-9384
 *   - Output: 0.5-4.5V ratiometric
 *   - Range: 0-3 bar or 0-5 bar absolute
 *   - IP67 rated sealed connector
 * 
 * 
 * ===== BOSCH AUTOMOTIVE SENSORS =====
 * 
 * Bosch Temperature Sensors:
 *   Part Numbers: 0 280 130 026, 0 280 130 107
 *   - Resistance @ 25°C: ~2,700Ω (varies by model)
 *   - Beta: ~3,450K
 *   - Thread: M10x1.0 or M12x1.5
 *   - Temperature Range: -40°C to 150°C
 * 
 * Bosch MAP Sensors:
 *   Part Numbers: 0 261 230 034, 0 281 002 437
 *   - Output: 0.5-4.5V (3-bar) or analog frequency
 *   - High precision ±1% full scale
 *   - Automotive-grade sealed housing
 * 
 * 
 * ===== AEM PERFORMANCE SENSORS (Aftermarket) =====
 * 
 * AEM 3.5 Bar MAP Sensor:
 *   Part Number: 30-2130-50
 *   - Output: 0.5V @ 0 bar, 4.5V @ 3.5 bar
 *   - Connector: Deutsch DT04-3P
 *   - Accuracy: ±0.5% full scale
 *   - Operating Temp: -40°C to 125°C
 * 
 * AEM Temperature Sensors:
 *   Part Number: 30-2012
 *   - Resistance @ 25°C: 2,486Ω
 *   - Beta: 3,950K
 *   - Thread: 1/8" NPT
 *   - Range: -40°C to 150°C
 * 
 * 
 * ===== AUTOMETER GAUGES (Aftermarket Senders) =====
 * 
 * AutoMeter Temperature Senders:
 *   Part Numbers: 2258, 2262
 *   - Custom resistance curve for AutoMeter gauges
 *   - Not standard NTC - requires custom calibration
 * 
 * 
 * ===== GENERIC INDUSTRIAL NTC THERMISTORS =====
 * 
 * Standard 2.2kΩ NTC Thermistors:
 *   - Resistance @ 25°C: 2,200Ω or 2,252Ω
 *   - Beta: 3,950K (most common)
 *   - Thread: M6, M8, M10, M12, or 1/8" NPT
 *   - Temperature Range: -40°C to 150°C (some to 200°C)
 *   - Tolerance: ±1% to ±5%
 *   - NOT recommended for harsh automotive environments
 * 
 * 
 * ===== EXHAUST GAS TEMPERATURE (EGT) SENSORS =====
 * 
 * K-Type Thermocouples (Standard):
 *   - Temperature Range: 0-1200°C
 *   - Wire Colors: Yellow (+), Red (-)
 *   - Thread: M6, M8, 1/8" NPT
 *   - Probe length: 35-75mm typical
 *   - Response time: 0.5-2 seconds
 * 
 * Heavy-Duty EGT Probes:
 *   - Inconel sheath (corrosion resistant)
 *   - Vibration-resistant construction
 *   - High-temperature ceramic insulation
 *   - Grounded or ungrounded junction
 *   - Recommended brands: Omega, Pyromation, Watlow
 */

// ============================================================================
// CALIBRATION INSTRUCTIONS
// ============================================================================
/*
 * HOW TO MEASURE YOUR SENSOR RESISTANCE:
 * 
 * 1. Disconnect the sensor from the circuit
 * 2. Set your multimeter to resistance (Ω) mode
 * 3. Measure resistance across the sensor terminals
 * 4. Note the ambient temperature (use accurate thermometer)
 * 5. Compare to expected value at that temperature
 * 
 * COMMON RESISTANCE VALUES AT DIFFERENT TEMPERATURES:
 * (For standard 2252Ω @ 25°C, Beta 3950K thermistor)
 * 
 *   Temperature   |   Resistance
 *   --------------|-------------
 *      -40°C      |   ~45,300Ω
 *      -20°C      |   ~16,200Ω
 *        0°C      |   ~6,500Ω
 *       25°C      |   ~2,252Ω   <-- Nominal
 *       50°C      |   ~832Ω
 *       75°C      |   ~336Ω
 *      100°C      |   ~149Ω
 *      125°C      |   ~72Ω
 *      150°C      |   ~37Ω
 * 
 * If your sensor doesn't match this table:
 *   1. Check if it's a different nominal resistance (2.0kΩ, 2.5kΩ, 10kΩ, etc.)
 *   2. Look up the manufacturer's datasheet for Beta coefficient
 *   3. Update the calibration values above
 * 
 * 
 * HOW TO CALIBRATE YOUR SENSORS:
 * 
 * === Ice Water Test (Most Accurate) ===
 * 1. Fill container with crushed ice and water
 * 2. Stir and let settle (should be 0°C)
 * 3. Submerge sensor and wait 2-3 minutes
 * 4. Read displayed temperature on gauge
 * 5. Calculate offset: 0.0 - displayed_temp
 * 6. Update offset value above
 * 
 * === Boiling Water Test ===
 * 1. Boil water (100°C at sea level, adjust for altitude)
 * 2. Carefully submerge sensor
 * 3. Read displayed temperature
 * 4. Calculate offset: actual_temp - displayed_temp
 * 5. Update offset value above
 * 
 * === MAP Sensor Calibration ===
 * 1. With engine OFF, ignition ON
 * 2. MAP sensor should read atmospheric pressure
 * 3. At sea level: ~1.0 bar absolute (0.0 bar gauge)
 * 4. Compare to weather station barometric pressure
 * 5. Calculate offset and update above
 * 
 * 
 * TROUBLESHOOTING:
 * 
 * Q: My temperature reads way off (>20°C error)
 * A: Wrong sensor type! Verify Beta coefficient and nominal resistance
 * 
 * Q: My readings jump around a lot
 * A: Check wiring connections. May need better filtering (increase FILTER_ALPHA)
 * 
 * Q: Sensor reads -999°C or other invalid value
 * A: Open circuit - check wiring and connections
 * 
 * Q: Sensor reads exactly 25°C all the time
 * A: Short circuit or wrong pin assignment
 * 
 * Q: Need more help?
 * A: See docs/CALIBRATION.md for detailed instructions
 */

#endif // CALIBRATION_H
