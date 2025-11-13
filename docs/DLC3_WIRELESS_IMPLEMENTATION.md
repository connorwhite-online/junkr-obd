# DLC3 Wireless Implementation Guide

## Overview

This guide shows how to implement DLC3 K-Line reading with **wireless communication** between the DLC3 connector and your main ESP32 (Qualia) mounted in the dash.

---

## Architecture

```
┌─────────────────┐         ┌──────────────────┐         ┌─────────────────┐
│  DLC3 Connector │         │  K-Line Module    │         │  Main ESP32     │
│  (Under Dash)   │────────►│  (Small ESP32)   │────────►│  (Qualia)       │
│                 │  K-Line │  + MC33290       │  WiFi   │  (In Dash)      │
│  21-pin OP3     │         │  Transceiver     │  /BLE   │  Display        │
└─────────────────┘         └──────────────────┘         └─────────────────┘
```

**Benefits:**
- ✅ Main ESP32 stays in dash (no long wires)
- ✅ Small module near DLC3 connector (easy installation)
- ✅ Wireless communication (clean installation)
- ✅ Can still use analog intercept as backup

---

## Hardware Setup

### K-Line Module (Near DLC3 Connector)

**Components:**
- Small ESP32 module (ESP32-C3 Mini, ESP32 Mini, etc.) - $5-10
- MC33290 K-Line transceiver - $3-5
- Basic circuit components (resistors, capacitors) - $2-3
- Small enclosure - $5-10

**Total Cost:** $15-28

**Wiring:**
```
DLC3 Pin 3 (K-Line) ──► MC33290 Pin 1
DLC3 Pin 1 (+12V)   ──► MC33290 Pin 3 (VCC)
DLC3 Pin 2 (GND)    ──► MC33290 Pin 2 (GND)

MC33290 Pin 4 (TX)  ──► ESP32 GPIO17 (RX)
MC33290 Pin 5 (RX)  ──► ESP32 GPIO16 (TX)
MC33290 Pin 7 (GND) ──► ESP32 GND
```

### Main ESP32 (Qualia)

**No additional hardware needed!**
- Uses built-in WiFi
- Receives data wirelessly
- Uses existing display

---

## Software Setup

### Step 1: K-Line Module Code

**File:** `examples/ecu_dlc3_wireless.ino`

**What it does:**
- Connects to DLC3 via K-Line
- Reads engine parameters using OBD9141 library
- Sends data wirelessly via WiFi UDP
- Can also use BLE (Bluetooth Low Energy)

**Configuration:**
```cpp
#define WIFI_SSID "JNKR_Gauge"
#define WIFI_PASSWORD "your_password_here"
#define UDP_PORT 12345
```

### Step 2: Main ESP32 Receiver Code

**File:** `examples/ecu_dlc3_receiver.ino`

**What it does:**
- Creates WiFi Access Point
- Receives UDP packets from K-Line module
- Parses JSON data
- Makes data available to gauge system

**Integration:**
- Can use DLC3 data directly
- Can fallback to analog intercept if DLC3 fails
- Can combine both (use DLC3 for diagnostics, analog for speed)

---

## Communication Options

### Option 1: WiFi UDP (Recommended)

**Pros:**
- ✅ Longer range
- ✅ More reliable
- ✅ Can use Access Point mode (no router needed)
- ✅ Easy to debug

**Cons:**
- ⚠️ Higher power consumption
- ⚠️ Slightly more complex

**Setup:**
- Main ESP32 creates WiFi Access Point
- K-Line module connects as client
- UDP communication

### Option 2: Bluetooth Low Energy (BLE)

**Pros:**
- ✅ Lower power consumption
- ✅ Built into ESP32
- ✅ Simple pairing

**Cons:**
- ⚠️ Shorter range
- ⚠️ More complex code

**Setup:**
- BLE server on K-Line module
- BLE client on main ESP32
- Characteristic-based communication

### Option 3: ESP-NOW (Alternative)

**Pros:**
- ✅ Very low power
- ✅ Direct peer-to-peer
- ✅ No WiFi setup needed

**Cons:**
- ⚠️ Less common
- ⚠️ May need MAC address configuration

---

## Implementation Steps

### Phase 1: Build K-Line Module (2-3 hours)

1. **Build circuit:**
   - Solder MC33290 to small PCB
   - Connect to ESP32 module
   - Wire to DLC3 connector

2. **Test K-Line communication:**
   - Upload basic K-Line test code
   - Verify communication with ECU
   - Test reading basic parameters

### Phase 2: Add Wireless (1-2 hours)

1. **Upload wireless code:**
   - Use `ecu_dlc3_wireless.ino` on K-Line module
   - Configure WiFi credentials
   - Test UDP transmission

2. **Upload receiver code:**
   - Use `ecu_dlc3_receiver.ino` on main ESP32
   - Test UDP reception
   - Verify data parsing

### Phase 3: Integration (1-2 hours)

1. **Integrate with gauge system:**
   - Add DLC3 data to sensor reading functions
   - Implement fallback to analog intercept
   - Test under driving conditions

2. **Polish:**
   - Add error handling
   - Add reconnection logic
   - Optimize update rates

**Total Time: 4-7 hours**

---

## Code Integration Example

### Keep Code Separate

**Important:** Keep DLC3 and analog intercept code in **separate files**. Don't mix them together.

### Option 1: Use DLC3 Only

```cpp
// In your main gauge code
#include "ecu_dlc3_receiver.h"  // DLC3 receiver

void Sensors_Update() {
  // Get data from DLC3
  if (isDLC3Available()) {
    coolantTemp = getDLC3CoolantTemp();
    intakeTempPost = getDLC3IntakeAirTemp();
    boostPressureBar = getDLC3BoostBar();
    engineRPM = getDLC3RPM();
  }
  
  // EGT always from separate sensor
  exhaustTemp = readEGT(prevExhaustTemp);
}
```

### Option 2: Use Analog Intercept Only

```cpp
// In your main gauge code
// Include analog intercept functions (from ecu_analog_intercept.ino)

void Sensors_Update() {
  // Get data from analog intercept
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  coolantTemp = voltageToTemperature(coolantVoltage);
  
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  intakeTempPost = voltageToTemperature(iatVoltage);
  
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  boostPressureBar = voltageToPressure(mapVoltage);
  
  // EGT always from separate sensor
  exhaustTemp = readEGT(prevExhaustTemp);
}
```

### Option 3: Use Both (Separate, with Fallback)

```cpp
// In your main gauge code
#include "ecu_dlc3_receiver.h"  // DLC3 receiver
// Analog intercept functions in separate file

void Sensors_Update() {
  // Try DLC3 first
  if (isDLC3Available()) {
    coolantTemp = getDLC3CoolantTemp();
    intakeTempPost = getDLC3IntakeAirTemp();
    boostPressureBar = getDLC3BoostBar();
  } else {
    // Fallback to analog intercept (separate function)
    readAnalogSensors();  // Function from ecu_analog_intercept.ino
  }
  
  // EGT always from separate sensor
  exhaustTemp = readEGT(prevExhaustTemp);
}
```

### See Integration Example

**File:** `examples/ecu_dlc3_integration_example.ino`

This shows the pattern for integrating DLC3 while keeping analog intercept separate.

---

## Advantages of This Approach

### 1. Flexibility

**You can:**
- ✅ Use DLC3 for comprehensive diagnostics
- ✅ Use analog intercept for high-speed critical sensors
- ✅ Use both (redundancy/verification)
- ✅ Switch between them easily

### 2. Clean Installation

**Benefits:**
- ✅ No long wires from DLC3 to dash
- ✅ Small module near DLC3 (easy to hide)
- ✅ Main ESP32 stays in dash
- ✅ Professional appearance

### 3. Backup Options

**If DLC3 fails:**
- ✅ Fallback to analog intercept
- ✅ No loss of functionality
- ✅ Best of both worlds

### 4. Future Expansion

**Can add:**
- ✅ Multiple DLC3 modules (different ECUs)
- ✅ Additional diagnostic features
- ✅ Data logging
- ✅ Remote monitoring

---

## Power Considerations

### K-Line Module Power

**Options:**
1. **12V from DLC3 Pin 1:**
   - Use buck converter to 5V/3.3V
   - Always powered when ignition on
   - Simple wiring

2. **Separate 12V source:**
   - Tap into nearby 12V wire
   - More flexible location
   - May need additional wiring

3. **Battery pack:**
   - For testing/development
   - Not recommended for permanent install

**Power Consumption:**
- ESP32 WiFi: ~80-170mA active
- MC33290: ~10-20mA
- **Total: ~100-200mA** (manageable)

---

## Range Considerations

### WiFi Range

**Typical range:**
- Inside vehicle: Excellent (10-20 feet)
- Through dash: Good (should work fine)
- Obstacles: Minimal impact in vehicle

**If range is issue:**
- Use WiFi extender/repeater
- Use BLE (shorter range but may work)
- Use ESP-NOW (direct peer-to-peer)

---

## Troubleshooting

### Problem: No Data Received

**Check:**
- ✅ WiFi connection status
- ✅ UDP port configuration
- ✅ IP addresses correct
- ✅ K-Line module transmitting
- ✅ Main ESP32 receiving

**Solutions:**
- Check serial output on both modules
- Verify WiFi credentials
- Test UDP with simple test code
- Check firewall/network settings

### Problem: Intermittent Data

**Possible causes:**
- WiFi interference
- Range issues
- Power supply problems

**Solutions:**
- Improve antenna placement
- Check power supply stability
- Reduce update rate
- Add reconnection logic

### Problem: K-Line Communication Fails

**Check:**
- MC33290 wiring
- DLC3 connector connection
- ECU communication
- Protocol initialization

**Solutions:**
- Verify circuit with multimeter
- Test with OBD9141 example
- Check ECU compatibility
- Review K-Line initialization

---

## Cost Breakdown

### K-Line Module

| Item | Cost |
|------|------|
| ESP32-C3 Mini | $5-8 |
| MC33290 | $3-5 |
| Components | $2-3 |
| Enclosure | $5-10 |
| **Total** | **$15-26** |

### Main ESP32

| Item | Cost |
|------|------|
| Qualia ESP32-S3 | Already have ✅ |
| No additional hardware | $0 |
| **Total** | **$0** |

**Grand Total: $15-26** (just for K-Line module)

---

## Comparison: Wireless vs Wired

| Aspect | Wireless | Wired |
|--------|----------|-------|
| **Installation** | ✅ Easier (no long wires) | ⚠️ More complex |
| **Reliability** | ⚠️ Depends on WiFi | ✅ More reliable |
| **Cost** | ⚠️ +$15-26 (extra ESP32) | ✅ Lower |
| **Flexibility** | ✅✅ High | ⚠️ Lower |
| **Range** | ⚠️ Limited | ✅ Unlimited |
| **Power** | ⚠️ Higher (WiFi) | ✅ Lower |

**Recommendation:** Wireless is better for clean installation, wired is better for reliability.

---

## Next Steps

1. **Review code examples:**
   - `examples/ecu_dlc3_wireless.ino` (K-Line module)
   - `examples/ecu_dlc3_receiver.ino` (Main ESP32)

2. **Build K-Line module:**
   - Order components
   - Build circuit
   - Test K-Line communication

3. **Test wireless communication:**
   - Upload code to both ESP32s
   - Test WiFi connection
   - Verify data transmission

4. **Integrate with gauge system:**
   - Add receiver code to main project
   - Implement fallback logic
   - Test under driving conditions

---

**Document Created:** 2025-01-27  
**Status:** Ready for implementation  
**Estimated Time:** 4-7 hours  
**Cost:** $15-26 (K-Line module only)
