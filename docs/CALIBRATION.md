# Sensor Calibration Guide

**Ensuring Accurate Readings for the JNKR Gauge System**

This guide explains how to calibrate each sensor for maximum accuracy. While the system is designed to work with standard automotive sensors, calibration may be needed due to component tolerances or specific sensor characteristics.

---

## 📋 Table of Contents

- [Quick Start: Using calibration.h](#quick-start-using-calibrationh)
- [When to Calibrate](#when-to-calibrate)
- [Required Tools](#required-tools)
- [Temperature Sensor Calibration](#temperature-sensor-calibration)
- [Boost Pressure Calibration](#boost-pressure-calibration)
- [Battery Voltage Calibration](#battery-voltage-calibration)
- [EGT Calibration](#egt-calibration)
- [Saving Calibration](#saving-calibration)

---

## 🚀 Quick Start: Using calibration.h

**NEW:** A dedicated calibration file has been added to simplify sensor calibration!

The `calibration.h` file in the project root contains:
- ✅ All sensor calibration values in one place
- ✅ Reference data for popular OEM sensors (GM, Ford, Cummins, CAT, AEM, Bosch)
- ✅ Complete instructions and resistance tables
- ✅ Temperature offset values

**Quick Setup:**

1. **Order sensors** - See [BOM.md](BOM.md) for heavy-duty automotive sensor recommendations
2. **Measure resistance** - Use a multimeter at room temperature (~25°C)
3. **Identify sensor type** - Compare to reference data in `calibration.h`
4. **Update values** - Edit `calibration.h` with your sensor specifications
5. **Upload code** - Flash updated firmware to your device
6. **Test and tune** - Fine-tune offset values using ice water test

**Important:** See [AUTOMOTIVE_SENSORS.md](AUTOMOTIVE_SENSORS.md) for guidance on selecting heavy-duty sensors vs. hobbyist sensors.

---

## ⚙️ When to Calibrate

Calibrate your sensors if:

✅ Readings seem consistently off by a fixed amount  
✅ Sensor resistance doesn't match specification  
✅ Using non-standard sensors  
✅ Voltage divider resistors have >5% tolerance  
✅ Comparison with known-good gauge shows difference  

**Don't calibrate if:**

❌ Readings are completely wrong (likely wiring issue)  
❌ Readings fluctuate wildly (noise or bad connection)  
❌ Sensor is faulty (replace first)

---

## 🧰 Required Tools

### Essential

- **Multimeter** - For measuring voltage and resistance
- **Reference thermometer** - Accurate to ±1°C
- **Known 12V source** - Battery or power supply
- **Ice water** - For temperature reference (0°C)
- **Boiling water** - For temperature reference (100°C)

### Optional

- **Tire pressure gauge** - For boost calibration
- **Infrared thermometer** - Quick temperature checks
- **Known-good gauge** - For comparison

---

## 🌡️ Temperature Sensor Calibration

### Understanding NTC Thermistors

NTC (Negative Temperature Coefficient) thermistors change resistance with temperature:
- At 25°C: ~2,252Ω (nominal)
- At 100°C: ~200Ω
- Resistance decreases as temperature increases

### Method 1: Ice Water Calibration (Most Accurate)

**You'll need:**
- Ice water mixture (crushed ice + water)
- Reference thermometer
- Serial Monitor open

**Procedure:**

1. **Prepare ice bath:**
   - Fill container with crushed ice
   - Add water to just cover ice
   - Stir and let settle for 2 minutes
   - Temperature should be 0°C (32°F)

2. **Immerse sensor:**
   - Place NTC thermistor in ice water
   - Ensure sensor tip is submerged
   - Wait 2-3 minutes for stabilization

3. **Read values:**
   - Open Serial Monitor (115200 baud)
   - Note the displayed temperature
   - Should read close to 0°C
   - Note the difference

4. **Calculate offset:**
   - Example: Sensor reads 3.5°C, actual is 0°C
   - Offset = 0 - 3.5 = -3.5°C

5. **Apply offset in code:**
   ```cpp
   // In setup() function of jnkr-gauge.ino
   Sensors_SetIATOffset(-3.5);  // For IAT sensor
   ```

6. **Verify:**
   - Sensor should now read 0°C in ice water

### Method 2: Boiling Water Calibration

**Warning:** Use caution with boiling water!

1. **Boil water:**
   - Heat water to rolling boil
   - Temperature depends on altitude
   - At sea level: 100°C (212°F)
   - At 1000m: ~97°C
   - At 2000m: ~93°C

2. **Immerse sensor:**
   - Carefully place sensor in water
   - Use tongs or wire
   - Wait 1-2 minutes

3. **Read and calculate offset:**
   - Compare reading to expected temperature
   - Calculate offset
   - Apply in code

### Method 3: Reference Comparison

**If you have a known-good thermometer:**

1. **Place both sensors together:**
   - In same location (engine coolant, air, etc.)
   - Ensure good thermal contact
   - Wait for stabilization (5+ minutes)

2. **Compare readings:**
   - Read both sensors
   - Calculate difference
   - Apply offset

### Method 4: Resistance Verification

**For advanced users with accurate ohmmeter:**

1. **Measure resistance at known temperature:**
   - Use ice water (0°C) or room temp (measure with reference thermometer)
   - Disconnect sensor from circuit
   - Measure resistance across sensor leads

2. **Compare to datasheet:**
   - At 25°C: Should be ~2,252Ω ±5%
   - At 0°C: Should be ~6,500Ω
   - At 100°C: Should be ~200Ω

3. **If significantly different:**
   - Sensor may be wrong type
   - Check reference resistor value (should be 2.2kΩ)
   - May need different Beta coefficient in code

### Applying Calibration

**In code (jnkr-gauge.ino):**

```cpp
void setup() {
  // ... existing setup code ...
  
  Sensors_Init();
  
  // Apply calibration offsets
  Sensors_SetIATOffset(-2.5);      // IAT reads 2.5°C high
  // Note: Only one offset for IAT, one for each sensor
  
  Display_Init();
  Alerts_Init();
  
  // ... rest of setup ...
}
```

**Multiple sensor offsets:**

The current code only supports IAT offset. To add coolant and oil offsets, edit `sensors.cpp`:

```cpp
// Add static variables at top
static float coolantOffset = 0.0;
static float oilOffset = 0.0;

// Add getter functions
float Sensors_GetCoolantTemp() {
  return coolantTemp + coolantOffset;
}

float Sensors_GetOilTemp() {
  return oilTemp + oilOffset;
}

// Add setter functions
void Sensors_SetCoolantOffset(float offset) {
  coolantOffset = offset;
}

void Sensors_SetOilOffset(float offset) {
  oilOffset = offset;
}
```

And declare them in `sensors.h`:
```cpp
void Sensors_SetCoolantOffset(float offset);
void Sensors_SetOilOffset(float offset);
```

---

## 🎚️ Boost Pressure Calibration

### Method 1: Atmospheric Pressure Reference

**Most reliable method:**

1. **Engine off, ignition on:**
   - MAP sensor should read atmospheric pressure
   - At sea level: 1.0 bar absolute (14.7 PSI)
   - At altitude: Lower (calculate expected)

2. **Read displayed value:**
   - Should show 0.0 PSI gauge (1.0 bar absolute)
   - Or slightly negative (vacuum at idle)

3. **Compare to reference:**
   - Use known-accurate barometer or weather report
   - Subtract 1.0 bar to get gauge pressure
   - Calculate offset

4. **Apply offset:**
   ```cpp
   Sensors_SetBoostOffset(0.05);  // Adds 0.05 bar
   ```

### Method 2: Known Gauge Comparison

**If you have calibrated boost gauge:**

1. **Install both gauges** on same port (T-fitting)

2. **Drive and monitor:**
   - Compare readings at several boost levels
   - Note consistent difference

3. **Calculate average offset:**
   - Example: JNKR reads 12 PSI, reference reads 13 PSI
   - Offset = 1 PSI = 0.07 bar

4. **Apply offset**

### Altitude Compensation

Expected atmospheric pressure by altitude:

| Altitude | Pressure (bar) | Pressure (PSI) |
|----------|----------------|----------------|
| Sea level | 1.013 | 14.7 |
| 500m | 0.955 | 13.9 |
| 1000m | 0.899 | 13.0 |
| 1500m | 0.846 | 12.3 |
| 2000m | 0.795 | 11.5 |

If your readings don't match expected for your altitude, apply offset.

---

## 🔋 Battery Voltage Calibration

### Method: Direct Comparison

1. **Measure with multimeter:**
   - Connect multimeter to battery terminals
   - Engine running: Should read 13.5-14.5V
   - Engine off: Should read 12.4-12.8V

2. **Read displayed value:**
   - Compare to multimeter
   - Note difference

3. **Adjust voltage divider (hardware solution):**
   - If reading is consistently off
   - Check resistor values (should be 10kΩ and 2.2kΩ)
   - Replace with exact values if using 5% tolerance

4. **Software correction:**
   
   In `config.h`, adjust the divider ratio:
   ```cpp
   // Current value
   #define BATTERY_DIVIDER_RATIO  ((BATTERY_DIVIDER_R1 + BATTERY_DIVIDER_R2) / BATTERY_DIVIDER_R2)
   
   // If reading is off, manually adjust
   #define BATTERY_DIVIDER_RATIO  5.60  // Tweak this value
   ```

5. **Fine-tune:**
   - Upload code
   - Check reading
   - Adjust ratio up or down slightly
   - Repeat until accurate

---

## 🔥 EGT Calibration

### MAX31855 Module

The MAX31855 thermocouple amplifier is a precision device and typically doesn't need calibration. However:

### Verification Test

1. **Room temperature test:**
   - With engine cold, EGT should read room temp
   - Compare to reference thermometer
   - Should be within ±2°C

2. **Boiling water test:**
   - **Carefully** place thermocouple tip in boiling water
   - Should read ~100°C (adjusted for altitude)
   - If off by >5°C, check:
     - Thermocouple type (must be K-type)
     - Polarity (yellow = +, red = -)
     - Module connections

### Common EGT Issues

**Reading 0°C:**
- Thermocouple not connected
- Bad connection at MAX31855 module
- Faulty module

**Reading jumps around:**
- Loose connection
- Electrical interference (route wire away from ignition)
- Grounding issue

**Reading way off:**
- Wrong thermocouple type (must be K-type)
- Reversed polarity

### Software Adjustment

If EGT is consistently off by a fixed amount, add offset in code:

```cpp
// In sensors.cpp, modify Sensors_GetExhaustTemp():
float Sensors_GetExhaustTemp() {
  return exhaustTemp + 5.0;  // Add 5°C offset if needed
}
```

---

## 💾 Saving Calibration

### ⭐ RECOMMENDED: Use `calibration.h` Configuration File

**The easiest method - all calibration values in one place!**

A dedicated `calibration.h` file has been created in the project root for easy sensor calibration. This file contains:
- Thermistor specifications (nominal resistance, Beta coefficient)
- Reference resistor values
- Temperature offsets
- MAP sensor calibration values
- Reference data for common OEM sensors (GM, Ford, Cummins, CAT, Bosch, AEM)

**To use:**

1. **Measure your sensor resistance** at room temperature (25°C recommended)
2. **Open `calibration.h`** in your editor
3. **Update the values** for your specific sensors:

```cpp
// Example: If your IAT sensor measures 2450Ω at 25°C (GM sensor)
#define IAT_PRE_THERMISTOR_NOMINAL_R    2450.0   // Was 2252.0
#define IAT_PRE_THERMISTOR_BETA         3500     // GM Beta coefficient
#define IAT_PRE_REF_RESISTOR            2200.0   // Your reference resistor
#define IAT_PRE_OFFSET                  -1.5     // Fine-tune offset after testing
```

4. **Upload the code** to your Arduino/ESP32
5. **Test and fine-tune** the offset values if needed

**Benefits:**
- ✅ All calibration values in one file (easy to find)
- ✅ Includes reference data for popular OEM sensors
- ✅ Well-documented with instructions
- ✅ No need to dig through config.h or setup code
- ✅ Separate from hardware configuration

**Note:** The code will be updated in a future version to automatically load values from `calibration.h`.

### Option 1: Hard-Code in Setup (Current Method)

Simplest method - offsets applied every boot:

```cpp
void setup() {
  // ...
  Sensors_Init();
  
  // Your calibration values (apply after measuring sensors)
  Sensors_SetIATPreOffset(-2.5);   // IAT Pre-IC reads 2.5°C high
  Sensors_SetIATPostOffset(-1.8);  // IAT Post-IC reads 1.8°C high
  Sensors_SetBoostOffset(0.05);    // Boost reads 0.05 bar low
  
  // ...
}
```

### Option 2: Store in Config File (Legacy)

For easy modification without uploading:

In `config.h`, add:
```cpp
// Calibration offsets
#define IAT_PRE_CALIBRATION_OFFSET     -2.5
#define IAT_POST_CALIBRATION_OFFSET    -1.8
#define BOOST_CALIBRATION_OFFSET        0.05
```

Then in `setup()`:
```cpp
Sensors_SetIATPreOffset(IAT_PRE_CALIBRATION_OFFSET);
Sensors_SetIATPostOffset(IAT_POST_CALIBRATION_OFFSET);
Sensors_SetBoostOffset(BOOST_CALIBRATION_OFFSET);
```

### Option 3: EEPROM Storage (Advanced)

Store calibration in EEPROM for persistence:

```cpp
#include <EEPROM.h>

// Define EEPROM addresses
#define EEPROM_IAT_OFFSET_ADDR  100
#define EEPROM_BOOST_OFFSET_ADDR 104

void saveCalibration() {
  EEPROM.put(EEPROM_IAT_OFFSET_ADDR, iatOffset);
  EEPROM.put(EEPROM_BOOST_OFFSET_ADDR, boostOffset);
}

void loadCalibration() {
  float iatOffset, boostOffset;
  EEPROM.get(EEPROM_IAT_OFFSET_ADDR, iatOffset);
  EEPROM.get(EEPROM_BOOST_OFFSET_ADDR, boostOffset);
  
  Sensors_SetIATOffset(iatOffset);
  Sensors_SetBoostOffset(boostOffset);
}
```

---

## 📊 Calibration Verification

After calibration, verify accuracy:

1. **Temperature sensors:**
   - Compare to known-good thermometer
   - Check at multiple temperatures
   - Should be within ±2°C

2. **Boost pressure:**
   - Compare to known-good gauge
   - Check at idle (vacuum), light boost, full boost
   - Should be within ±1 PSI

3. **Battery voltage:**
   - Compare to multimeter
   - Check at idle and high RPM
   - Should be within ±0.2V

4. **EGT:**
   - Compare to known-good gauge (if available)
   - Verify matches engine conditions
   - Should be within ±10°C

---

## 🛠️ Calibration Checklist

- [ ] Reference instruments prepared
- [ ] Serial Monitor open at 115200 baud
- [ ] IAT sensor calibrated (if needed)
- [ ] Coolant sensor calibrated (if needed)
- [ ] Oil sensor calibrated (if needed)
- [ ] Boost pressure calibrated (if needed)
- [ ] Battery voltage calibrated (if needed)
- [ ] EGT sensor verified
- [ ] Offsets applied in code
- [ ] Code uploaded to Arduino
- [ ] Verification tests performed
- [ ] All readings accurate

---

## 📝 Calibration Log

Keep a record of your calibration:

```
JNKR Gauge System - Calibration Log
Date: _______________

IAT Sensor:
  Reference: ________°C
  Reading:   ________°C
  Offset:    ________°C

Coolant Sensor:
  Reference: ________°C
  Reading:   ________°C
  Offset:    ________°C

Boost Pressure:
  Reference: ________ PSI
  Reading:   ________ PSI
  Offset:    ________ bar

Battery Voltage:
  Reference: ________ V
  Reading:   ________ V
  Ratio:     ________

EGT:
  Reference: ________°C
  Reading:   ________°C
  Notes:     __________________

Calibrated by: __________________
Vehicle: _______________________
```

---

## 🆘 Troubleshooting Calibration

**Problem:** Offset doesn't fix the issue
- May not be a calibration problem
- Check wiring first
- Verify sensor type is correct
- Check for electrical noise

**Problem:** Reading still jumps around after calibration
- Not a calibration issue
- Check connections
- Add filtering in code (increase `FILTER_ALPHA`)

**Problem:** Offset needed is very large (>10°C or >2 PSI)
- May indicate wrong sensor type
- Verify voltage divider resistors
- Check reference voltage (should be 5.0V)

---

**Calibration complete!**

Your JNKR Gauge System should now provide accurate, reliable readings. Enjoy monitoring your engine with confidence!

---

**Last Updated:** 2025-11-07
