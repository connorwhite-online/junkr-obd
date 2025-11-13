# DLC3 (21-pin OP3) Connector Pinout

## Connector Information

**Connector Type:** DLC3 (Data Link Connector 3)  
**Designation:** OP3 (Toyota part number)  
**Pin Count:** 21 pins  
**Location:** Under dash  
**Vehicle:** 1996 Land Cruiser Prado (1KZ-TE)

---

## Required Pins for K-Line Communication

**You only need 3 pins for K-Line communication:**

| Pin | Function | Wire Color (Typical) | Required? |
|-----|----------|---------------------|-----------|
| **1** | BATT | Red/Brown | ✅ Yes (+12V power) |
| **2** | GND | Black/Brown | ✅ Yes (Ground) |
| **3** | K-Line | Yellow/Green | ✅ Yes (Diagnostic data) |

**All other pins:** Not needed for basic K-Line communication

---

## Full Pinout (Reference)

**Note:** This is a generic DLC3 pinout. The actual pinout for your specific vehicle needs to be verified from the wiring diagrams or service manual.

| Pin | Function | Description | Voltage | Use Case |
|-----|----------|-------------|---------|----------|
| 1 | BATT | Battery +12V | 12V | Power for K-Line module |
| 2 | GND | Ground | 0V | Ground reference |
| 3 | K-Line | Diagnostic data | 0-12V | K-Line communication |
| 4 | L-Line | Diagnostic line 2 | 0-12V | Rarely used |
| 5 | TE1 | Test terminal 1 | 0-12V | Diagnostic mode |
| 6 | TE2 | Test terminal 2 | 0-12V | Diagnostic mode |
| 7 | E1 | Engine ground | 0V | ECU ground |
| 8 | VF1 | Voltage feedback 1 | 0-5V | Mixture voltage |
| 9-21 | Various | Diagnostic signals | Varies | Model-specific |

---

## Verified Pins (From ECU_PINOUT_1KZTE.md)

The ECU pinout document confirms these pins are accessible via CHECK CONNECTOR (OP3):

- **E1** (Ground) - Sensor ground reference
- **TE1** (Diagnostic) - Bi-directional diagnostic terminal
- **TE2** (Diagnostic) - Bi-directional diagnostic terminal
- **VF1** (Mixture voltage) - 0-5V rich/lean indicator
- **BATT** (Battery) - +12V battery sense

**These correspond to:**
- Pin 1: BATT (+12V)
- Pin 2: E1/GND (Ground)
- Pin 3: K-Line (diagnostic data - likely TE1/TE2 related)
- Pin 8: VF1 (mixture voltage)

---

## Physical Connector

### Connector Type

**DLC3 connectors are Toyota proprietary:**
- Not a standard connector (like OBD-II)
- Specific to Toyota vehicles
- May be hard to source

### Connector Appearance

- **Shape:** Rectangular, proprietary Toyota design
- **Size:** Approximately 2-3 inches long
- **Color:** Usually gray or black plastic
- **Pins:** 21 pins in single or double row

### Mating Connector

**To connect to DLC3, you need:**
- **Male connector** that mates with DLC3 (female in vehicle)
- **May be hard to find** - Toyota proprietary part
- **Alternative:** Use backprobe pins or wire taps

---

## Connection Methods

### Method 1: Backprobe Pins (Recommended for Testing)

**What:** Insert test pins into back of DLC3 connector

**Pros:**
- ✅ No connector needed
- ✅ Non-invasive
- ✅ Easy to remove
- ✅ Quick setup

**Cons:**
- ⚠️ Not permanent
- ⚠️ May be loose

**How:**
1. Locate DLC3 connector
2. Insert backprobe pins into pins 1, 2, 3
3. Connect wires to your module
4. Secure with zip ties

---

### Method 2: Adapter Cable (Recommended for Permanent)

**What:** Create adapter cable with DLC3 connector on one end, breakout on other

**Pros:**
- ✅ Professional
- ✅ Permanent
- ✅ Easy to disconnect
- ✅ Clean installation

**Cons:**
- ⚠️ Need to source DLC3 connector (may be difficult)

**How:**
1. Source DLC3 male connector
2. Wire to breakout connector (Deutsch DT, etc.)
3. Plug into vehicle DLC3
4. Connect module to breakout

---

### Method 3: Wire Tap (If Connector Unavailable)

**What:** Tap into wires going to DLC3 connector

**Pros:**
- ✅ Works if connector unavailable
- ✅ Permanent installation

**Cons:**
- ⚠️ More invasive
- ⚠️ Requires wire tapping

**How:**
1. Locate wires to DLC3
2. Identify K-Line, +12V, GND wires
3. Tap with T-tap or scotchlok connectors
4. Run wires to module

---

## Finding the Connector

### Location

**Typical locations:**
- Under dash, driver side
- Under dash, passenger side
- Behind kick panel
- Near ECU location

### Identification

**Look for:**
- 21-pin connector
- May have cover
- Usually gray/black
- May be labeled "CHECK CONNECTOR" or "OP3"

### Verification

**Confirm it's DLC3:**
1. Count pins (should be 21)
2. Check with multimeter:
   - Pin 1: +12V (with ignition on)
   - Pin 2: 0V (ground)
3. Check service manual for location

---

## Wiring to K-Line Module

### Minimum Connection

```
DLC3 Pin 1 (BATT +12V) ──► K-Line Module VCC
DLC3 Pin 2 (GND)       ──► K-Line Module GND
DLC3 Pin 3 (K-Line)    ──► MC33290 Pin 1 (K-Line IN)
```

**That's it!** Only 3 wires needed.

---

## PCB Header Option

### If You Want a PCB Header

**You could create a small PCB with:**
- Header pins for DLC3 connection (if you can source connector)
- MC33290 K-Line transceiver
- ESP32 module footprint
- Power regulation
- Breakout connector for programming

**But for simplicity:**
- Start with backprobe method
- Or use wire taps
- PCB can come later if needed

---

## Recommended Starting Point

### Step 1: Test with Backprobe

1. **Locate DLC3** under dash
2. **Identify pins 1, 2, 3** with multimeter
3. **Insert backprobe pins**
4. **Connect to your K-Line module**
5. **Test communication**

**Time:** ~30-40 minutes  
**Cost:** ~$10-15 (backprobe pins, wire)

### Step 2: If Working, Make Permanent

1. **Source DLC3 connector** (if available)
2. **Create adapter cable**
3. **Install permanently**

**Or:**
- Use wire tap method
- More permanent than backprobe
- Less professional than adapter cable

---

## Summary

**To connect to DLC3:**
- ✅ **Only need 3 pins:** BATT (+12V), GND, K-Line
- ✅ **Start with backprobe method** (easiest, non-invasive)
- ✅ **No PCB header needed** (can use backprobe or wire taps)
- ✅ **Can make adapter cable later** if you want permanent install

**The connector is proprietary Toyota, so:**
- May be hard to source mating connector
- Backprobe method is easiest to start
- Wire tap method works if connector unavailable

---

**Document Created:** 2025-01-27  
**Status:** Connection guide  
**Recommendation:** Start with backprobe pins for testing
