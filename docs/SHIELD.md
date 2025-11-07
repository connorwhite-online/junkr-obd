# Arduino Mega Shield - JNKR Gauge System

**Custom PCB Shield for Simplified Installation**

This document describes the custom Arduino Mega shield designed for the JNKR Gauge System. The shield eliminates most hand-wiring and provides a professional, plug-and-play solution.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Schematic](#schematic)
- [PCB Layout](#pcb-layout)
- [Bill of Materials](#bill-of-materials-shield)
- [Assembly Instructions](#assembly-instructions)
- [Ordering from PCBWay](#ordering-from-pcbway)
- [Testing](#testing)
- [Modifications](#modifications)

---

## 🎯 Overview

The JNKR Gauge Shield is a custom PCB that stacks on top of an Arduino Mega 2560, providing:

- All necessary voltage dividers built-in
- Screw terminals for easy sensor connections
- Socket for MAX31855 module
- Power input with onboard 5V regulator
- Protection circuitry
- Status LEDs
- Compact, professional design

**Benefits:**

✅ Eliminates hand-wiring of voltage dividers  
✅ Reduces assembly time by 50%  
✅ Professional appearance  
✅ Easier troubleshooting  
✅ Reduced chance of wiring errors  
✅ Reusable for future projects  

---

## ⚡ Features

### Power Management

- **Input:** 12V vehicle power
- **Onboard 5V regulator:** 3A LM2596 buck converter
- **Protection:** Reverse polarity protection diode
- **Filtering:** Input and output capacitors
- **LED indicator:** Power-on LED

### Sensor Interfaces

**NTC Thermistor Inputs (3x):**
- Built-in 2.2kΩ reference resistors
- Screw terminal connections
- Routed to Arduino A0, A1, A2

**MAP Sensor Input:**
- 3-pin screw terminal (5V, Signal, GND)
- Direct connection to Arduino A3
- No external components needed

**Battery Voltage Sense:**
- Built-in voltage divider (10kΩ + 2.2kΩ)
- Screw terminal for 12V input
- Routed to Arduino A7
- Reads 0-16V vehicle voltage

**EGT Sensor (MAX31855):**
- Socket for MAX31855 module
- SPI connections to Arduino (pins 10, 50, 52)
- Screw terminal for K-type thermocouple

### Output Interfaces

**Nextion Display:**
- 4-pin header (5V, TX, RX, GND)
- Connected to Serial2 (pins 16, 17)

**Alert Buzzer:**
- 2-pin screw terminal
- Connected to pin 8
- Current-limited for safety

**Status LED:**
- Heartbeat LED on pin 13
- Visible through enclosure

---

## 📐 Schematic

### Schematic Overview

```
                         JNKR GAUGE SHIELD SCHEMATIC
                         ===========================

12V Input ──┬── D1 (Reverse Protection) ── LM2596 Buck Converter ── 5V Out
            │                                    │
            └── C1 (100µF) ── GND               └── C2 (100µF) ── GND

═══════════════════════════════════════════════════════════════════════

NTC THERMISTOR CIRCUITS (x3 for IAT, Coolant, Oil)

     5V ──┬─── R1 (2.2kΩ) ───┬─── Terminal 2 ─── To Arduino Ax
          │                   │
          └─── Terminal 1 ─── NTC Sensor ─── Terminal 3 ─── GND

═══════════════════════════════════════════════════════════════════════

BATTERY VOLTAGE DIVIDER

    12V Sense Terminal ─── R2 (10kΩ) ───┬─── To Arduino A7
                                         │
                                        R3 (2.2kΩ)
                                         │
                                        GND

═══════════════════════════════════════════════════════════════════════

MAP SENSOR INTERFACE

    MAP Terminal 1 (5V)    ─── 5V Rail
    MAP Terminal 2 (Signal)─── Arduino A3
    MAP Terminal 3 (GND)   ─── GND

═══════════════════════════════════════════════════════════════════════

MAX31855 EGT SENSOR

    Socket Pin 1 (VCC) ─── 5V
    Socket Pin 2 (GND) ─── GND
    Socket Pin 3 (SCK) ─── Arduino Pin 52
    Socket Pin 4 (CS)  ─── Arduino Pin 10
    Socket Pin 5 (DO)  ─── Arduino Pin 50
    Terminals (T+/T-) ─── K-Type Thermocouple

═══════════════════════════════════════════════════════════════════════

NEXTION DISPLAY

    Header Pin 1 (5V)  ─── 5V
    Header Pin 2 (TX)  ─── Arduino Pin 17 (TX2)
    Header Pin 3 (RX)  ─── Arduino Pin 16 (RX2)
    Header Pin 4 (GND) ─── GND

═══════════════════════════════════════════════════════════════════════

BUZZER OUTPUT

    Buzzer Terminal (+) ─── R4 (100Ω) ─── Arduino Pin 8
    Buzzer Terminal (-) ─── GND

═══════════════════════════════════════════════════════════════════════

LEDS

    Power LED (Green)  ─── R5 (1kΩ) ─── 5V ─── GND
    Status LED (Blue)  ─── R6 (1kΩ) ─── Pin 13 ─── GND
```

### Component Values

| Designator | Component | Value | Description |
|------------|-----------|-------|-------------|
| D1 | Diode | 1N5822 | Reverse polarity protection |
| C1 | Capacitor | 100µF 25V | Input filter |
| C2 | Capacitor | 100µF 16V | Output filter |
| C3-C8 | Capacitor | 0.1µF | Decoupling caps |
| R1a-c | Resistor | 2.2kΩ | NTC reference resistors |
| R2 | Resistor | 10kΩ | Voltage divider (upper) |
| R3 | Resistor | 2.2kΩ | Voltage divider (lower) |
| R4 | Resistor | 100Ω | Buzzer current limit |
| R5, R6 | Resistor | 1kΩ | LED current limit |
| LED1 | LED | Green 5mm | Power indicator |
| LED2 | LED | Blue 5mm | Status indicator |
| U1 | Regulator | LM2596 Module | 12V to 5V buck |
| J1-J10 | Terminal | Screw 2-5 pin | Sensor connections |
| H1 | Header | 1x4 Female | Nextion display |
| H2 | Socket | 1x5 Female | MAX31855 module |

---

## 🖼️ PCB Layout

### Board Specifications

- **Dimensions:** 95mm x 65mm (fits Arduino Mega footprint)
- **Layers:** 2 (top and bottom copper)
- **Thickness:** 1.6mm standard
- **Copper weight:** 1 oz
- **Surface finish:** HASL (lead-free)
- **Silkscreen:** White on blue soldermask
- **Mounting:** 4x M3 holes matching Arduino Mega

### Top Layer (Component Side)

```
┌─────────────────────────────────────────────────────────────┐
│                                                               │
│  ╔═══════════════════════════════════════════════════╗      │
│  ║  JNKR GAUGE SHIELD v1.0                          ║      │
│  ╚═══════════════════════════════════════════════════╝      │
│                                                               │
│  [LED1]  [LED2]                      [LM2596 Module]        │
│  Power   Status                            U1                │
│                                                               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                  │
│  │ IAT      │  │ Coolant  │  │ Oil      │                  │
│  │ Sensor   │  │ Sensor   │  │ Sensor   │                  │
│  │ J1       │  │ J2       │  │ J3       │                  │
│  └──────────┘  └──────────┘  └──────────┘                  │
│                                                               │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                  │
│  │ MAP      │  │ Battery  │  │ Buzzer   │                  │
│  │ Sensor   │  │ Voltage  │  │ Output   │                  │
│  │ J4       │  │ J5       │  │ J6       │                  │
│  └──────────┘  └──────────┘  └──────────┘                  │
│                                                               │
│  ┌────────────────┐  ┌────────────────┐                    │
│  │ EGT Sensor     │  │ Power Input    │                    │
│  │ Thermocouple   │  │ 12V            │                    │
│  │ J7             │  │ J8             │                    │
│  └────────────────┘  └────────────────┘                    │
│                                                               │
│  [MAX31855 Socket]   [Nextion Header]                       │
│         H2                  H1                               │
│                                                               │
│  ○   Arduino Mega Headers (pass-through)           ○        │
│  ○                                                  ○        │
│                                                               │
└─────────────────────────────────────────────────────────────┘
```

### Bottom Layer

- Ground plane (fills most of the layer)
- Power traces (5V and 12V distribution)
- Signal traces from voltage dividers to Arduino pins
- Pass-through connections for unused Arduino pins

### Layer Stack-up

```
Top Layer:      Components + signal traces
  Core:         FR4 dielectric (1.6mm)
Bottom Layer:   Ground plane + power traces
```

---

## 🛠️ Bill of Materials (Shield)

### PCB Components

| Qty | Designator | Description | Value | Package | Example P/N |
|-----|------------|-------------|-------|---------|-------------|
| 1 | PCB | Printed Circuit Board | - | 95x65mm | Custom |
| 1 | U1 | LM2596 Buck Module | 12V→5V 3A | Module | LM2596 |
| 1 | D1 | Schottky Diode | 1N5822 | DO-201 | 1N5822 |
| 2 | C1, C2 | Electrolytic Cap | 100µF 25V | Radial | Generic |
| 6 | C3-C8 | Ceramic Cap | 0.1µF 50V | Radial | Generic |
| 3 | R1a-c | Resistor | 2.2kΩ 1/4W | Axial | Generic |
| 1 | R2 | Resistor | 10kΩ 1/4W | Axial | Generic |
| 1 | R3 | Resistor | 2.2kΩ 1/4W | Axial | Generic |
| 1 | R4 | Resistor | 100Ω 1/4W | Axial | Generic |
| 2 | R5, R6 | Resistor | 1kΩ 1/4W | Axial | Generic |
| 1 | LED1 | LED 5mm Green | - | THT 5mm | Generic |
| 1 | LED2 | LED 5mm Blue | - | THT 5mm | Generic |

### Connectors

| Qty | Designator | Description | Pins | Pitch | Example P/N |
|-----|------------|-------------|------|-------|-------------|
| 3 | J1-J3 | Screw Terminal | 3-pin | 5mm | Phoenix |
| 3 | J4-J6 | Screw Terminal | 2-3 pin | 5mm | Phoenix |
| 2 | J7-J8 | Screw Terminal | 2-pin | 5mm | Phoenix |
| 1 | H1 | Pin Header Female | 1x4 | 2.54mm | Generic |
| 1 | H2 | Pin Header Female | 1x5 | 2.54mm | Generic |
| 4 | - | Arduino Headers | Varies | 2.54mm | Stackable |

### Arduino Headers (for shield to stack on Mega)

| Qty | Description | Pins |
|-----|-------------|------|
| 1 | Header Female | 1x8 (for Digital 0-7) |
| 1 | Header Female | 1x8 (for Digital 8-13, GND) |
| 2 | Header Female | 1x8 (for Analog, Power) |
| 1 | Header Female | 1x10 (for Digital 22-53) |

**Total Component Cost:** ~$15-25 (not including PCB)

---

## 🔨 Assembly Instructions

### Step 1: Inspect PCB

1. **Unpack PCB carefully**
2. **Visual inspection:**
   - Check for scratches or damage
   - Verify silkscreen is readable
   - Check mounting holes

### Step 2: Install Resistors

**Order of assembly: smallest components first**

1. **Install resistors (color coded):**
   - R1a-c (2.2kΩ): Red-Red-Red-Gold
   - R2 (10kΩ): Brown-Black-Orange-Gold
   - R3 (2.2kΩ): Red-Red-Red-Gold
   - R4 (100Ω): Brown-Black-Brown-Gold
   - R5, R6 (1kΩ): Brown-Black-Red-Gold

2. **Procedure:**
   - Bend leads to fit holes
   - Insert from top side
   - Solder on bottom side
   - Trim excess leads flush
   - Clean with isopropyl alcohol

### Step 3: Install Diodes and Capacitors

1. **Install D1 (1N5822):**
   - **Mind polarity!** Silver/white band indicates cathode
   - Cathode toward 5V side (check silkscreen)
   - Solder and trim leads

2. **Install ceramic capacitors (C3-C8):**
   - Non-polarized, can go either way
   - Solder and trim leads

3. **Install electrolytic capacitors (C1, C2):**
   - **Mind polarity!** Negative marked with stripe
   - Negative lead is shorter
   - Insert, solder, trim

### Step 4: Install LEDs

1. **LED1 (Green - Power):**
   - **Mind polarity!** Longer lead = positive (anode)
   - Flat side of LED = negative (cathode)
   - Insert according to silkscreen
   - Solder from bottom

2. **LED2 (Blue - Status):**
   - Same procedure as LED1
   - Check orientation

### Step 5: Install Screw Terminals

1. **All screw terminals (J1-J8):**
   - Orient wire entry facing edge of board
   - Insert pins through holes
   - Ensure seated flush
   - Solder all pins
   - Test by wiggling (should be solid)

### Step 6: Install Headers and Sockets

1. **Nextion header (H1 - 1x4 female):**
   - Insert from top
   - Solder from bottom
   - Ensure perpendicular to board

2. **MAX31855 socket (H2 - 1x5 female):**
   - Same procedure as H1

3. **Arduino stacking headers:**
   - These allow shield to plug onto Arduino
   - And still provide access to pins
   - Insert from top
   - Solder from bottom
   - Use flux for easier soldering
   - **Take your time** - many pins!

### Step 7: Install LM2596 Module

**Two options:**

**Option A: Socket Mount (Removable)**
1. Solder female headers to PCB
2. Insert LM2596 module into headers
3. Can remove for adjustment or replacement

**Option B: Direct Solder (Permanent)**
1. Insert LM2596 module pins into PCB
2. Solder all pins
3. More compact but permanent

**Recommended:** Option A for easier adjustment

### Step 8: Clean PCB

1. **Clean flux residue:**
   - Use isopropyl alcohol (90%+)
   - Soft brush or cotton swabs
   - Clean both sides
   - Let dry completely

2. **Visual inspection:**
   - Check all solder joints (shiny, not dull)
   - Look for solder bridges
   - Verify component orientation
   - Check for cold solder joints

---

## 🧪 Testing

### Pre-Power Tests

**Before connecting to Arduino or applying power:**

1. **Continuity tests with multimeter:**
   - [ ] Check for shorts between 5V and GND
   - [ ] Check for shorts between 12V and GND
   - [ ] Verify ground continuity across all GND terminals

2. **Resistance measurements:**
   - Between 5V and GND: Should read >1kΩ
   - Between 12V and GND: Should read >1kΩ
   - If <100Ω, you have a short!

3. **Component orientation:**
   - [ ] D1 diode correct polarity
   - [ ] All electrolytic capacitors correct polarity
   - [ ] LEDs correct polarity

### Power-On Test (Without Arduino)

1. **Set LM2596 output:**
   - Connect 12V power to J8 terminals
   - **Verify polarity with multimeter first!**
   - Measure output voltage at test points or 5V terminals
   - Adjust pot on LM2596 to exactly 5.0V

2. **LED check:**
   - Power LED should illuminate
   - No smoke or hot components

3. **Disconnect power**

### Arduino Installation Test

1. **Mount shield on Arduino Mega:**
   - Align all headers
   - Press down firmly and evenly
   - Should sit flush on Arduino

2. **Visual check:**
   - All pins inserted properly
   - No bent pins

3. **Connect 12V power to shield J8:**
   - Arduino and shield should power on
   - Arduino built-in LED should light
   - Shield power LED should light

4. **Upload test sketch:**
   - Upload JNKR gauge code
   - Open Serial Monitor
   - Check for initialization messages

### Sensor Tests

1. **Test each thermistor input:**
   - Connect thermistor to terminals
   - Should read room temperature
   - Touch thermistor - temp should rise

2. **Test MAP sensor:**
   - Connect MAP sensor
   - Should read atmospheric pressure (~1.0 bar)

3. **Test EGT:**
   - Insert MAX31855 module into socket
   - Connect thermocouple
   - Should read room temperature

4. **Test buzzer:**
   - Connect buzzer
   - Should beep on startup

---

## 📦 Ordering from PCBWay

### Preparing Gerber Files

**Gerber files are provided in:** `docs/shield/gerbers/jnkr-shield-gerbers.zip`

**Files included:**
- Top copper layer
- Bottom copper layer
- Top silkscreen
- Bottom silkscreen
- Top soldermask
- Bottom soldermask
- Board outline
- Drill file
- Pick and place (optional)

### PCBWay Order Process

1. **Go to:** [www.pcbway.com](https://www.pcbway.com)

2. **Click "Quote Now"**

3. **Upload Gerbers:**
   - Click "Add Gerber File"
   - Upload `jnkr-shield-gerbers.zip`
   - Wait for automatic dimension detection

4. **Configure Options:**
   ```
   PCB Quantity: 5 (minimum order)
   PCB Thickness: 1.6mm
   PCB Color: Blue (recommended) or Green
   Surface Finish: HASL (Lead-free)
   Copper Weight: 1 oz
   Remove Order Number: Yes (optional, costs $5)
   ```

5. **Review and Add to Cart**

6. **Shipping:**
   - Standard shipping (14-20 days): ~$5-10
   - Express shipping (7-10 days): ~$20-30

7. **Checkout**

### Pricing (Estimate)

| Item | Cost |
|------|------|
| PCB (5 pieces) | $5-15 |
| Shipping | $5-30 |
| Remove order number (optional) | $5 |
| **Total** | **$10-50** |

**Note:** Price varies by options and shipping method

### Timeline

- Order processing: 1-2 days
- Production: 2-3 days
- Shipping: 7-20 days (depending on method)
- **Total:** 10-25 days

---

## 🔧 Modifications

### Optional Modifications

**Add I2C Barometric Sensor:**
- BMP280 or BME280
- Solder directly to shield
- Connect to SDA (pin 20) and SCL (pin 21)

**Add SD Card Logging:**
- SD card module socket
- Connect to SPI pins (50, 51, 52)
- Use separate CS pin (e.g., pin 53)

**Add Relay Output:**
- For failsafe or warning light
- Add relay driver circuit
- Connect to digital pin (e.g., pin 9)

**Add More Sensors:**
- Additional analog inputs available
- Use unused Arduino pins
- Add screw terminals as needed

---

## 📝 Shield Assembly Checklist

**Before ordering PCB:**
- [ ] Review schematic carefully
- [ ] Check all part footprints
- [ ] Verify pin connections
- [ ] Review with another person

**During assembly:**
- [ ] All resistors installed and correct values
- [ ] All capacitors installed with correct polarity
- [ ] Diode installed with correct polarity
- [ ] LEDs installed with correct polarity
- [ ] All screw terminals oriented correctly
- [ ] Headers and sockets straight and flush
- [ ] LM2596 module installed or socketed
- [ ] All solder joints clean and shiny
- [ ] No solder bridges
- [ ] PCB cleaned of flux

**Testing:**
- [ ] No shorts between power rails
- [ ] LM2596 adjusted to 5.0V output
- [ ] Power LED lights
- [ ] Status LED controllable from pin 13
- [ ] All sensor inputs functional
- [ ] Buzzer works
- [ ] Display communication works

---

## 📞 Shield Support

**For shield-specific questions:**

1. Check the main [README](../README.md) troubleshooting section
2. Review this document carefully
3. Test continuity with multimeter
4. Post photos of your board in GitHub Issues
5. Join community forum for help

**Common Shield Issues:**

**Problem:** Shield won't power on
- Check 12V input polarity
- Verify D1 diode orientation
- Check LM2596 module

**Problem:** Sensor readings incorrect
- Verify resistor values
- Check for solder bridges
- Test voltage dividers with multimeter

**Problem:** Display doesn't work
- Check header orientation
- Verify TX/RX connections
- Test with direct wiring first

---

**The shield design is open-source!**

Modify, improve, and share your changes with the community. See the main README for contribution guidelines.

---

**Next Steps:**

1. Order PCB from PCBWay
2. Order components from [BOM](#bill-of-materials-shield)
3. Follow [Assembly Instructions](#assembly-instructions)
4. Test thoroughly before installing
5. Install in vehicle following [ASSEMBLY.md](ASSEMBLY.md)

---

**Last Updated:** 2025-11-07

*Shield design by JNKR Project - Open Source Hardware*
