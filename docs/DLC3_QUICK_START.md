# DLC3 Wireless Quick Start Guide

## Quick Overview

**What:** Read engine data from DLC3 diagnostic port wirelessly  
**Why:** Comprehensive diagnostics + clean installation (no long wires)  
**How:** Small ESP32 module near DLC3 → WiFi → Main ESP32 in dash

---

## What You Need

### K-Line Module (Near DLC3 Connector)
- Small ESP32 (ESP32-C3 Mini or similar) - $5-10
- MC33290 K-Line transceiver - $3-5
- Basic components - $2-3
- **Total: $15-26**

### Main ESP32 (Qualia)
- ✅ Already have it!
- No additional hardware needed

---

## Quick Setup (30 minutes)

### Step 1: Build K-Line Module (15 min)

1. **Solder MC33290 circuit:**
   ```
   DLC3 Pin 3 → MC33290 Pin 1
   DLC3 Pin 1 → MC33290 Pin 3 (VCC)
   DLC3 Pin 2 → MC33290 Pin 2 (GND)
   MC33290 Pin 4 → ESP32 GPIO17
   MC33290 Pin 5 → ESP32 GPIO16
   ```

2. **Connect to DLC3:**
   - Locate 21-pin OP3 connector under dash
   - Connect wires to pins 1, 2, 3

### Step 2: Upload Code (10 min)

1. **K-Line Module:**
   - Open `examples/ecu_dlc3_wireless.ino`
   - Install OBD9141 library (Arduino Library Manager)
   - Configure WiFi credentials
   - Upload to small ESP32

2. **Main ESP32:**
   - Open `examples/ecu_dlc3_receiver.ino`
   - Upload to Qualia ESP32

### Step 3: Test (5 min)

1. **Power on both ESP32s**
2. **Check Serial Monitor:**
   - K-Line module: Should show "WiFi connected"
   - Main ESP32: Should show "Receiving DLC3 Data"
3. **Verify data:**
   - Should see engine parameters in Serial Monitor

---

## Integration with Your Gauge

### Add to Your Main Code

```cpp
// Include DLC3 receiver
#include "ecu_dlc3_receiver.h"

// In your sensor update function
void Sensors_Update() {
  // Try DLC3 first (if available)
  if (dlc3Data.valid && (millis() - dlc3Data.lastUpdate < 1000)) {
    coolantTemp = dlc3Data.coolantTemp;
    intakeTempPost = dlc3Data.intakeAirTemp;
    boostPressureBar = dlc3Data.boostPressurePSI / 14.5038;
  } else {
    // Fallback to analog intercept
    // ... your existing analog intercept code ...
  }
}
```

---

## Advantages

✅ **Comprehensive data:** Diagnostic codes, ECU calculations, system status  
✅ **Clean installation:** No long wires from DLC3 to dash  
✅ **Backup option:** Can still use analog intercept  
✅ **Flexible:** Use both methods together

---

## Troubleshooting

**No data received?**
- Check WiFi connection (both ESP32s)
- Verify UDP port (12345)
- Check Serial Monitor on both devices

**K-Line not working?**
- Verify MC33290 wiring
- Check DLC3 connector connection
- Test with OBD9141 example first

---

## Full Documentation

- **[DLC3_WIRELESS_IMPLEMENTATION.md](DLC3_WIRELESS_IMPLEMENTATION.md)** - Complete guide
- **[DLC3_LIBRARY_RESEARCH_COMPLETE.md](DLC3_LIBRARY_RESEARCH_COMPLETE.md)** - Library details

---

**Ready to go!** Start with the quick setup above, then refer to full docs for details.
