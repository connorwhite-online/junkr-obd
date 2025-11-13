# Code Separation Guide - DLC3 vs Analog Intercept

## Principle: Keep Code Separate

**Rule:** Keep DLC3 and analog intercept code in **separate files**. Don't mix them together.

---

## File Structure

```
/examples/
  ├── ecu_analog_intercept.ino          ← Analog intercept code (standalone)
  ├── ecu_analog_intercept_v2.ino       ← Analog intercept variant
  ├── ecu_dlc3_wireless.ino             ← DLC3 K-Line module code
  ├── ecu_dlc3_receiver.ino             ← DLC3 receiver code (standalone)
  └── ecu_dlc3_integration_example.ino  ← Shows integration pattern
```

**Your main gauge code:**
```
/jnkr-gauge.ino                          ← Main gauge code
  ├── Includes DLC3 receiver (optional)
  ├── Includes analog intercept (optional)
  └── Chooses which to use (or uses both)
```

---

## Why Keep Them Separate?

### Benefits

1. **Clean Code:**
   - Each method is self-contained
   - Easy to understand
   - Easy to maintain

2. **Flexibility:**
   - Can use DLC3 only
   - Can use analog intercept only
   - Can use both (separate functions)
   - Easy to switch between methods

3. **Testing:**
   - Test each method independently
   - Debug easier (isolated code)
   - Can disable one without affecting the other

4. **Backup:**
   - Keep analog intercept as backup
   - Don't need to modify analog intercept code
   - Just choose which to use in main code

---

## Integration Patterns

### Pattern 1: DLC3 Only

**Main code:**
```cpp
#include "ecu_dlc3_receiver.h"  // Only DLC3

void Sensors_Update() {
  if (isDLC3Available()) {
    coolantTemp = getDLC3CoolantTemp();
    intakeTempPost = getDLC3IntakeAirTemp();
    boostPressureBar = getDLC3BoostBar();
  }
}
```

**Analog intercept:** Not included, not used

---

### Pattern 2: Analog Intercept Only

**Main code:**
```cpp
// Include analog intercept functions
// (from ecu_analog_intercept.ino)

void Sensors_Update() {
  // Use analog intercept functions
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  coolantTemp = voltageToTemperature(coolantVoltage);
  
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  intakeTempPost = voltageToTemperature(iatVoltage);
  
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  boostPressureBar = voltageToPressure(mapVoltage);
}
```

**DLC3:** Not included, not used

---

### Pattern 3: Both (Separate Functions)

**Main code:**
```cpp
#include "ecu_dlc3_receiver.h"  // DLC3 receiver
// Analog intercept functions in separate file

void Sensors_Update() {
  // Try DLC3 first
  if (isDLC3Available()) {
    readFromDLC3();  // Separate function
  } else {
    readFromAnalogIntercept();  // Separate function
  }
}

void readFromDLC3() {
  coolantTemp = getDLC3CoolantTemp();
  intakeAirTemp = getDLC3IntakeAirTemp();
  boostPressureBar = getDLC3BoostBar();
}

void readFromAnalogIntercept() {
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  coolantTemp = voltageToTemperature(coolantVoltage);
  
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  intakeAirTemp = voltageToTemperature(iatVoltage);
  
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  boostPressureBar = voltageToPressure(mapVoltage);
}
```

**Both methods:** Included, but in separate functions

---

## Recommended Structure

### In Your Main Gauge Code

```cpp
// ============================================================================
// CONFIGURATION
// ============================================================================

#define USE_DLC3 true
#define USE_ANALOG_INTERCEPT false

// ============================================================================
// INCLUDES
// ============================================================================

#if USE_DLC3
  #include "ecu_dlc3_receiver.h"
#endif

#if USE_ANALOG_INTERCEPT
  // Include analog intercept functions
  // (from ecu_analog_intercept.ino)
#endif

// ============================================================================
// SENSOR UPDATE
// ============================================================================

void Sensors_Update() {
  #if USE_DLC3
    if (isDLC3Available()) {
      // Use DLC3
      updateFromDLC3();
      return;  // Done, skip analog intercept
    }
  #endif
  
  #if USE_ANALOG_INTERCEPT
    // Use analog intercept (if DLC3 not available or disabled)
    updateFromAnalogIntercept();
  #endif
}

// ============================================================================
// SEPARATE UPDATE FUNCTIONS
// ============================================================================

#if USE_DLC3
void updateFromDLC3() {
  coolantTemp = getDLC3CoolantTemp();
  intakeAirTemp = getDLC3IntakeAirTemp();
  boostPressureBar = getDLC3BoostBar();
  engineRPM = getDLC3RPM();
}
#endif

#if USE_ANALOG_INTERCEPT
void updateFromAnalogIntercept() {
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  coolantTemp = voltageToTemperature(coolantVoltage);
  
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  intakeAirTemp = voltageToTemperature(iatVoltage);
  
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  boostPressureBar = voltageToPressure(mapVoltage);
}
#endif
```

---

## What NOT to Do

### ❌ Don't Mix Code Together

```cpp
// BAD: Mixing DLC3 and analog intercept in same function
void Sensors_Update() {
  if (dlc3Data.valid) {
    coolantTemp = dlc3Data.coolantTemp;
  } else {
    float voltage = readECUSignal(PIN_ECU_COOLANT);  // Mixed together!
    coolantTemp = voltageToTemperature(voltage);
  }
}
```

### ✅ Do: Keep Separate Functions

```cpp
// GOOD: Separate functions
void Sensors_Update() {
  if (isDLC3Available()) {
    updateFromDLC3();  // All DLC3 code here
  } else {
    updateFromAnalogIntercept();  // All analog code here
  }
}
```

---

## File Organization

### Recommended Structure

```
/workspace/
  ├── jnkr-gauge.ino                    ← Main gauge code
  ├── sensors.cpp                       ← Sensor reading functions
  ├── sensors.h                         ← Sensor function declarations
  │
  └── /examples/
      ├── ecu_analog_intercept.ino      ← Standalone analog intercept
      ├── ecu_dlc3_wireless.ino         ← Standalone DLC3 module
      ├── ecu_dlc3_receiver.ino         ← Standalone DLC3 receiver
      └── ecu_dlc3_integration_example.ino  ← Integration pattern
```

**In your main code:**
- Include functions from examples as needed
- Keep each method in separate functions
- Use #ifdef to enable/disable methods

---

## Benefits of Separation

1. **Easy to Switch:**
   - Change one #define to switch methods
   - No code modification needed
   - Test different methods easily

2. **Easy to Debug:**
   - Each method isolated
   - Can disable one to test the other
   - Clear separation of concerns

3. **Easy to Maintain:**
   - Update DLC3 code without touching analog intercept
   - Update analog intercept without touching DLC3
   - Both can evolve independently

4. **Backup Option:**
   - Keep analog intercept ready
   - Can switch if DLC3 fails
   - No need to rewrite code

---

## Summary

**Keep it separate:**
- ✅ DLC3 code in separate file
- ✅ Analog intercept code in separate file
- ✅ Main code chooses which to use
- ✅ Use separate functions for each method

**Don't mix:**
- ❌ Don't put DLC3 and analog code in same function
- ❌ Don't copy-paste code between methods
- ❌ Don't create dependencies between methods

**Result:**
- Clean, maintainable code
- Easy to switch between methods
- Both methods available as backup

---

**See:** `examples/ecu_dlc3_integration_example.ino` for complete example
