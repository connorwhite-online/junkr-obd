# Bench Testing Guide - JNKR Gauge System

**Complete guide to powering and connecting ESP32 and I2C modules on the bench**

This guide shows you exactly how to power your ESP32 and I2C sensor module during development and testing, plus how to connect temporary wiring to ESP32 boards that have pin holes instead of Arduino-style headers.

---

## 📋 Table of Contents

- [The ESP32 Pin Hole Problem](#the-esp32-pin-hole-problem)
- [Solutions for Temporary Connections](#solutions-for-temporary-connections)
- [Power Supply Options](#power-supply-options)
- [Bench Testing the I2C Module](#bench-testing-the-i2c-module)
- [Bench Testing the ESP32 System](#bench-testing-the-esp32-system)
- [Complete Bench Testing Procedures](#complete-bench-testing-procedures)
- [Troubleshooting](#troubleshooting)

---

## 🔧 The ESP32 Pin Hole Problem

### Arduino vs ESP32 Pin Layout

**Arduino Mega/Uno** - Beginner Friendly:
```
┌────────────────────────┐
│                        │
│  ╔══╗  ╔══╗  ╔══╗     │  ← Male header pins stick UP
│  ║  ║  ║  ║  ║  ║     │    Easy to connect jumper wires!
│  ╚══╝  ╚══╝  ╚══╝     │
│   A0    A1    A2       │
└────────────────────────┘
```

**ESP32 Development Boards** - Pin Holes:
```
┌────────────────────────┐
│                        │
│   ○     ○     ○        │  ← Just holes in PCB
│   ○     ○     ○        │    Can't plug jumper wires in!
│  GND   GPIO  3V3       │
│                        │
└────────────────────────┘
```

**The Problem:**
- Standard male-to-male jumper wires won't stay in the holes
- Can't plug directly into breadboard without headers
- Alligator clips won't grip the flat PCB surface
- Makes prototyping frustrating!

**But don't worry - there are several easy solutions!** ⬇️

---

## 💡 Solutions for Temporary Connections

### Solution 1: Soldering Pin Headers (Recommended) ⭐

**What you need:**
- Male breakaway pin headers (40-pin strip, $2 on Amazon)
- Soldering iron
- 10-15 minutes

**Advantages:**
- ✅ Permanent solution (but removable if needed)
- ✅ Works exactly like Arduino
- ✅ Compatible with breadboards
- ✅ Most reliable connection

**How to do it:**

1. **Break off correct length** of pin header strip (count pins on your ESP32)
2. **Insert pins from TOP** of ESP32 board (short pins through holes)
3. **Place board upside down** on foam or helping hands
4. **Solder each pin** from bottom side
5. **Test continuity** with multimeter

```
BEFORE:                          AFTER:
┌────────────┐                   ┌────────────┐
│  ESP32     │                   │  ESP32     │
│            │                   │            │
│  ○  ○  ○   │  ← Holes          │  ║  ║  ║   │  ← Pins!
└────────────┘                   └──╨──╨──╨───┘
                                     │  │  │
                                  Now works with
                                  jumper wires!
```

**Tip:** If you're not confident soldering, any electronics repair shop or maker space can do this for $5-10.

---

### Solution 2: IC Test Clips (No Soldering!)

**What you need:**
- IC test clip hooks (also called "SMD test clips")
- Available on Amazon: Search "IC test hook clips" (~$8 for 10 pieces)

**Advantages:**
- ✅ No soldering required
- ✅ Temporary - remove anytime
- ✅ Good for testing individual pins
- ✅ Can clip onto bare PCB pads

**Disadvantages:**
- ⚠️ Not suitable for many connections at once
- ⚠️ Can be fiddly to attach
- ⚠️ Not suitable for vibration/movement

```
┌────────────────────────┐
│  ESP32                 │
│                        │
│         ○  ← Pin hole  │
│         ╱              │
│        🔴 ← IC clip    │
│         ╲              │
│          Jumper wire   │
└────────────────────────┘
```

**How to use:**
1. Identify pin you need on ESP32
2. Hook clip onto exposed copper pad
3. Connect jumper wire to clip's wire
4. Secure with tape if needed

**Amazon Link:** Search "ETEPON IC Test Hook Clips" or "SMD Test Clips"

---

### Solution 3: Breadboard Adapter Method

**What you need:**
- Female-to-male jumper wires (DuPont cables)
- Breadboard
- Tape or helping hands

**Advantages:**
- ✅ No soldering
- ✅ Uses standard jumper wires you already have
- ✅ Can connect multiple pins at once

**Disadvantages:**
- ⚠️ Wires must be pressed firmly into holes
- ⚠️ Connections can be unreliable
- ⚠️ Tape required to hold in place

```
ESP32 Board (pin holes):
┌────────────────────┐
│   ○   ○   ○        │
└───╱───╱───╱────────┘
   ╱   ╱   ╱  ← Female-to-male wires
  ╱   ╱   ╱      (male end pushed into holes)
 ╱   ╱   ╱
│   │   │
└───┴───┴───────────┐  ← Breadboard
│ ● ● ● ● ● ● ● ● ● │
└───────────────────┘
```

**How to use:**
1. Take female-to-male jumper wire
2. **Firmly push male end** into ESP32 pin hole
3. May need to **wiggle and apply pressure**
4. **Tape wire to board** to prevent falling out
5. Connect female end to breadboard or components

**Tip:** Add a tiny dab of Blu-Tack or poster putty around the wire to hold it in place.

---

### Solution 4: Pogo Pins with 3D Printed Jig (Advanced)

**What you need:**
- Spring-loaded pogo pins ($10 for 20 on Amazon)
- 3D printed holder (design available on Thingiverse)
- Access to 3D printer or printing service

**Advantages:**
- ✅ Professional solution
- ✅ No modifications to ESP32
- ✅ Repeatable connections
- ✅ Good for production testing

**Disadvantages:**
- ⚠️ Requires 3D printing
- ⚠️ Complex setup
- ⚠️ Overkill for hobbyist projects

**Best for:** If you're making multiple units or commercial product.

---

### Solution 5: Conductive Wire Insertion Method (Quick & Dirty)

**What you need:**
- Solid core wire (22 AWG, stripped)
- Tape

**Advantages:**
- ✅ Uses materials you already have
- ✅ Free solution
- ✅ Works in a pinch

**Disadvantages:**
- ⚠️ Very temporary
- ⚠️ Unreliable
- ⚠️ Easy to short circuit adjacent pins
- ⚠️ Not recommended for permanent use

```
1. Strip 5mm of solid core wire
2. Insert stripped end into ESP32 hole
3. Tape down to secure
4. Connect other end with alligator clip or jumper

    Solid core wire
         │
    ┌────┴────┐
    │  ESP32  │
    │    ○    │  ← Insert wire into hole
    └─────────┘
```

**⚠️ WARNING:** Be very careful not to bridge adjacent pins! This can damage your ESP32.

---

## 🔌 Power Supply Options

### Option A: USB-C Power (Easiest) ⭐

**For: ESP32-S3 Qualia Board**

The Adafruit Qualia ESP32-S3 has a USB-C connector for both programming AND power.

**Advantages:**
- ✅ Simplest option
- ✅ No extra equipment needed
- ✅ Can program and power simultaneously
- ✅ Built-in voltage regulation
- ✅ Safe - hard to damage board

**How to use:**
1. Plug USB-C cable into Qualia board
2. Plug other end into:
   - Computer USB port (for programming + power)
   - USB wall adapter (5V, 1A minimum)
   - USB power bank (for portable testing)

**Power specs:**
- **Input:** 5V via USB-C
- **Current:** ~500mA typical, 2A max (with display)
- **Internal regulation:** Provides 3.3V to ESP32

```
┌──────────────────┐
│  USB Wall Adapter │  5V / 2A
│  (phone charger)  │
└────────┬─────────┘
         │
    USB-C cable
         │
         ▼
┌────────────────────┐
│ Qualia ESP32-S3    │
│                    │
│  [USB-C] ●●●●●    │
│                    │
│  ○ ○ ○ ○ ○ ○ ○    │ ← GPIO pins available
│ 3V3 GND I21 I22... │
└────────────────────┘
```

**Recommended USB Power Supply:**
- Any USB phone charger (5V, 1A minimum)
- Amazon Basics USB Wall Charger (5V, 2.4A) - $8
- Anker PowerPort (5V, 2A) - $12

---

### Option B: Bench Power Supply (Professional)

**For: Serious development work**

A variable bench power supply gives you precise control and current limiting.

**Advantages:**
- ✅ Adjustable voltage (test different levels)
- ✅ Current limiting (protects against shorts)
- ✅ Precise measurements (built-in meters)
- ✅ Multiple outputs (power ESP32 + sensors separately)

**Disadvantages:**
- ⚠️ Costs $50-150
- ⚠️ Requires understanding of voltage/current
- ⚠️ Can damage board if set incorrectly

**How to use with ESP32:**

```
┌────────────────────────┐
│ Bench Power Supply     │
│ ┌────────┬────────┐    │
│ │   +    │   -    │    │  ← Output terminals
│ └───┬────┴───┬────┘    │
│     │        │         │
│  [5.0V] [0.5A]         │  ← Set voltage and current limit
└─────┼────────┼──────────┘
      │        │
   Red wire  Black wire
      │        │
      ▼        ▼
┌─────────────────────┐
│  ESP32-S3 Qualia    │
│                     │
│   VIN  ●  (Red)     │  ← Connect 5V here (NOT 3.3V!)
│   GND  ●  (Black)   │  ← Connect GND here
│                     │
└─────────────────────┘
```

**Settings for ESP32:**
- **Voltage:** 5.0V (DO NOT use 12V!)
- **Current Limit:** 500mA initially (increase to 2A if using display)
- **Connect to:** VIN and GND pins (NOT USB-C)

**⚠️ CRITICAL WARNINGS:**
1. **NEVER connect to 3.3V pin** - this bypasses voltage regulator!
2. **Set voltage BEFORE connecting** to board
3. **Check polarity** with multimeter before connecting
4. **Start with low current limit** (500mA) to prevent damage from shorts

**Recommended Models:**
- **Budget:** Wanptek DPS3010U (~$50) - Good for hobbyists
- **Mid-range:** Tekpower TP3005T (~$90) - More features
- **Professional:** Siglent SPD3303X (~$300) - Lab quality

---

### Option C: 12V Automotive Battery with Buck Converter

**For: Testing the complete automotive installation**

Simulates real vehicle power for full system testing.

**Advantages:**
- ✅ Realistic testing conditions
- ✅ Tests buck converter functionality
- ✅ Can test voltage fluctuations
- ✅ Powers sensors at correct voltage

**Disadvantages:**
- ⚠️ Requires 12V battery or power supply
- ⚠️ Needs buck converter module
- ⚠️ More complex wiring

```
┌──────────────────┐
│  12V Car Battery │  OR 12V Power Supply
│   (or power bank)│
└───────┬──────────┘
        │
   Automotive fuse (3A)
        │
        ▼
┌────────────────────┐
│  Buck Converter    │  12V → 5V
│  (Step-Down)       │  (e.g. LM2596 module)
│  IN+  IN-          │
│   │    │           │
│  OUT+ OUT-         │
└───┬────┬───────────┘
    │    │
    │    └─────────┐
    │              │
 5V OUT          GND
    │              │
    ▼              ▼
┌────────────────────┐
│  ESP32 + Sensors   │
│  VIN        GND    │
└────────────────────┘
```

**Buck Converter Settings:**
1. **Disconnect from ESP32** before adjusting!
2. Set 12V input
3. Adjust output to **exactly 5.0V** using trim pot
4. Verify with multimeter
5. Connect to ESP32 VIN pin

**Recommended Buck Converters:**
- LM2596 Module (Amazon, $6 for 5 pieces)
- Adjustable output: 1.25-35V
- Max current: 3A
- Built-in overcurrent protection

---

### Option D: USB Power Bank (Portable Testing)

**For: Mobile bench testing**

Perfect for testing in garage or near vehicle without AC power.

**Advantages:**
- ✅ Portable - test anywhere
- ✅ Built-in battery
- ✅ Safe - regulated 5V output
- ✅ Can power ESP32 for hours

**How to use:**
1. Get USB power bank (10,000mAh or larger)
2. Connect USB-C cable to ESP32
3. Turn on power bank
4. Test normally

**Battery life calculation:**
```
Power bank: 10,000 mAh @ 5V = 50 Wh
ESP32 + sensors: ~2W typical
Runtime: 50 Wh / 2W = 25 hours
```

**Recommended Power Banks:**
- Anker PowerCore 10000 (~$25) - Compact
- RAVPower 20000mAh (~$35) - Longer runtime
- Any brand with 2A+ output works

---

## 🧪 Bench Testing the I2C Module

### What You're Testing

The I2C sensor module (see [I2C_ASSEMBLY_GUIDE.md](I2C_ASSEMBLY_GUIDE.md)) consolidates all engine sensors in one weatherproof box. Before installing in vehicle, test it on the bench!

### Equipment Needed

| Item | Purpose | Where to Get |
|------|---------|-------------|
| **5V Power Supply** | Power I2C module | USB adapter, bench supply, or battery |
| **Arduino or ESP32** | I2C master device | Your main project board |
| **I2C Scanner Sketch** | Detect I2C devices | Arduino IDE examples |
| **Multimeter** | Check voltages | Amazon, $20 |
| **Jumper Wires** | Temporary connections | Amazon, $8 |
| **Test Resistors** | Simulate thermistors | 2.2kΩ resistors |

---

### Test Setup: I2C Module on Bench

```
┌─────────────────────────┐
│ USB Power Supply (5V)   │
│ (Phone Charger)         │
└──────┬──────────────────┘
       │
   Cut USB cable or use:
   USB breakout board ($3)
       │
    5V │ GND
       │  │
       ▼  ▼
┌──────────────────────┐
│  I2C Sensor Module   │
│  (Engine Bay Box)    │
│                      │
│  ┌────────────────┐  │
│  │ ADS1115 (0x48) │  │
│  │ MCP9600 (0x60) │  │
│  └────────────────┘  │
│                      │
│  Deutsch Connector:  │
│  Pin 1: 5V    ───────┼──► Red wire
│  Pin 2: GND   ───────┼──► Black wire
│  Pin 3: SCL   ───────┼──► Yellow wire
│  Pin 4: SDA   ───────┼──► Green wire
└──────────────────────┘
       │  │  │  │
       │  │  │  └─────────┐
       │  │  └────────┐   │
       │  └───────┐   │   │
       └──────┐   │   │   │
           5V │GND│SCL│SDA│
              │   │   │   │
              ▼   ▼   ▼   ▼
       ┌──────────────────────┐
       │  Arduino / ESP32     │
       │                      │
       │  5V  GND  D21  D22   │  (ESP32 I2C pins)
       │                      │
       │  [USB] ← To computer │
       └──────────────────────┘
```

---

### Step-by-Step: Testing I2C Module

#### Step 1: Visual Inspection (Power OFF)

**Check before powering:**

- [ ] No solder bridges between pins
- [ ] All components properly soldered
- [ ] Correct component orientation (ICs, capacitors)
- [ ] No loose wires inside enclosure
- [ ] Deutsch connector pins locked in place
- [ ] Cable glands tight (if installed)

---

#### Step 2: Continuity Tests (Power OFF)

**Use multimeter in continuity mode (beep mode):**

1. **Check for shorts:**
   ```
   Deutsch Pin 1 (5V) to Pin 2 (GND): Should NOT beep
   If beeps → SHORT CIRCUIT! Find and fix before powering!
   ```

2. **Check connections:**
   ```
   Deutsch Pin 1 → ADS1115 VDD: Should beep (0Ω)
   Deutsch Pin 2 → ADS1115 GND: Should beep (0Ω)
   Deutsch Pin 3 → ADS1115 SCL: Should beep (0Ω)
   Deutsch Pin 4 → ADS1115 SDA: Should beep (0Ω)
   ```

3. **Repeat for MCP9600:**
   ```
   Pin 1 → MCP9600 VDD: Beep
   Pin 2 → MCP9600 GND: Beep
   Pin 3 → MCP9600 SCL: Beep
   Pin 4 → MCP9600 SDA: Beep
   ```

**If any test fails:** Re-solder that connection!

---

#### Step 3: Resistance Test (Power OFF)

**Measure resistance between power and ground:**

```
Set multimeter to resistance (Ω) mode
Red probe on Deutsch Pin 1 (5V)
Black probe on Deutsch Pin 2 (GND)

Expected: 1kΩ - 10kΩ
Too low (<100Ω): Possible short circuit
Too high (>100kΩ): Possible open circuit
```

**Why?** The I2C modules have internal components that draw some current, so there's a finite resistance between power and ground. This test verifies the circuit is intact but not shorted.

---

#### Step 4: Power-On Test (No Load)

**Connect 5V power supply to module ONLY (not connected to ESP32 yet):**

1. **Set up power:**
   - If using bench supply: Set to 5.0V, current limit 500mA
   - If using USB adapter: Use voltage meter to verify 5V
   - Connect red wire to Deutsch Pin 1
   - Connect black wire to Deutsch Pin 2

2. **Apply power and measure:**
   ```
   Multimeter on Deutsch connector:
   
   Pin 1 to Pin 2: Should read 4.8-5.2V
   Current draw: Should be 30-80mA (idle)
   
   If current >200mA: Possible short or damaged IC!
   If current <10mA: Possible open circuit or loose connection
   ```

3. **Check module temperatures:**
   - After 30 seconds, touch ICs carefully
   - ADS1115: Should be cool or slightly warm
   - MCP9600: Should be cool or slightly warm
   - **If hot to touch:** POWER OFF immediately! Component damaged or wired wrong

4. **Measure voltages at ICs:**
   ```
   ADS1115 VDD pin: 5.0V ±0.2V
   ADS1115 GND pin: 0V
   MCP9600 VDD pin: 5.0V ±0.2V
   MCP9600 GND pin: 0V
   ```

**✅ If all checks pass: Module power circuitry is good!**

---

#### Step 5: I2C Device Detection

**Upload I2C scanner sketch to Arduino/ESP32:**

1. **Open Arduino IDE**

2. **File → Examples → Wire → i2c_scanner**

3. **Upload to your Arduino/ESP32**

4. **Connect I2C module to Arduino:**
   ```
   I2C Module → Arduino/ESP32
   
   Pin 1 (5V)  → 5V pin
   Pin 2 (GND) → GND pin
   Pin 3 (SCL) → GPIO 22 (ESP32) or SCL pin (Arduino)
   Pin 4 (SDA) → GPIO 21 (ESP32) or SDA pin (Arduino)
   ```

5. **Open Serial Monitor (115200 baud)**

6. **Expected output:**
   ```
   Scanning...
   I2C device found at address 0x48  !  ← ADS1115 ADC
   I2C device found at address 0x60  !  ← MCP9600 Thermocouple Amp
   done
   ```

**Troubleshooting:**

| Problem | Cause | Solution |
|---------|-------|----------|
| No devices found | Wrong I2C pins | Check SCL/SDA connections |
| | No power to module | Check 5V and GND wiring |
| | Damaged ICs | Visual inspection, test with new ICs |
| Only one device found | Incomplete soldering | Check solder joints on missing device |
| | Wrong I2C address | Verify address jumpers/resistors |
| Wrong addresses shown | Address configuration wrong | Check I2C address selection |

---

#### Step 6: Functional Tests

**Test each sensor input with known values:**

**A. Test Voltage Dividers (Thermistor Inputs):**

1. **Connect 2.2kΩ resistor** to thermistor input terminal
2. **Read ADC value** (should show ~2.5V at 25°C)
3. **Repeat for all 3 thermistor inputs**

**Expected voltage at 25°C with 2.2kΩ resistor:**
```
Voltage divider: 5V ─── [2.2kΩ] ─── [2.2kΩ] ─── GND
                              │
                          Vmeasured = 2.5V
```

**B. Test MAP Sensor Input:**

1. **Connect MAP sensor** to 3-pin terminal
2. **Should read atmospheric pressure** (~0.5V at sea level)
3. **Apply vacuum** (gently suck on sensor nipple)
4. **Voltage should decrease** (vacuum = lower pressure)

**C. Test Thermocouple Input:**

1. **Connect K-type thermocouple** to MCP9600 terminal
2. **Should read room temperature** (~20-25°C)
3. **Touch thermocouple** with your finger
4. **Temperature should rise** to body temp (~30-35°C)
5. **Heat with lighter** (carefully!)
6. **Should read >100°C quickly**

---

### ✅ I2C Module Bench Test Checklist

**Complete this checklist before vehicle installation:**

- [ ] Visual inspection passed (no shorts, all components installed)
- [ ] Continuity tests passed (all connections verified)
- [ ] Resistance test passed (1kΩ - 10kΩ between 5V and GND)
- [ ] Power-on test passed (5V at ICs, 30-80mA current draw)
- [ ] ICs stay cool (not hot after 1 minute)
- [ ] I2C scanner detects both devices (0x48 and 0x60)
- [ ] Voltage dividers read ~2.5V with 2.2kΩ test resistors
- [ ] MAP sensor input responds to vacuum
- [ ] Thermocouple input reads room temperature correctly
- [ ] Enclosure seals properly (no gaps)
- [ ] Deutsch connector locks and unlocks smoothly

**If all checks pass: ✅ Module ready for vehicle installation!**

---

## 🖥️ Bench Testing the ESP32 System

### Complete System Test Setup

Now test the ESP32 with display and I2C module together.

```
                    ┌──────────────────┐
                    │  5V Power Supply │
                    │  (2A minimum)    │
                    └────────┬─────────┘
                             │
                        USB-C cable
                             │
                             ▼
        ┌────────────────────────────────────┐
        │   Adafruit Qualia ESP32-S3         │
        │                                    │
        │   [USB-C Port] ●●●●               │
        │                                    │
        │   Display Connector:               │
        │   ════════════════════             │
        │   40-pin FFC cable ────────────┐   │
        │                                │   │
        │   I2C Pins (use headers!):     │   │
        │   GPIO 21 (SDA) ●              │   │
        │   GPIO 22 (SCL) ●              │   │
        │   5V          ●                │   │
        │   GND         ●                │   │
        └────┬───┬──┬───┬────────────────┘   │
             │   │  │   │                    │
          SDA SCL 5V GND                     │
             │   │  │   │                    │
             │   │  │   └───────────┐        │
             │   │  │               │        │
             │   │  └─────────┐     │        │
             │   │            │     │        │
             │   └──────┐     │     │        │
             │          │     │     │        │
             ▼          ▼     ▼     ▼        ▼
        ┌────────────────────────────┐  ┌─────────────┐
        │  I2C Sensor Module         │  │ 2.1" Round  │
        │  (from previous test)      │  │ Display     │
        │                            │  │ 480x480     │
        │  Pin 4: SDA (Green)        │  │             │
        │  Pin 3: SCL (Yellow)       │  │ [Graphics]  │
        │  Pin 1: 5V  (Red)          │  │             │
        │  Pin 2: GND (Black)        │  └─────────────┘
        └────────────────────────────┘
```

---

### Step-by-Step: Complete System Test

#### Step 1: Connect Display

1. **Identify 40-pin FFC connector** on Qualia board
2. **Lift locking tab** (pull up gently)
3. **Insert FFC cable** (contacts facing correct direction - check board markings)
4. **Push locking tab down** to secure cable
5. **Verify cable is fully inserted** (no gaps)

**⚠️ WARNING:** Inserting FFC cable backwards can damage display! Check orientation carefully.

---

#### Step 2: Connect I2C Module (Using Headers)

**If you soldered headers on ESP32 (Solution 1):**

1. Connect 4 jumper wires from I2C module to ESP32:
   ```
   I2C Module         ESP32 Pin
   ──────────         ─────────
   Pin 4 (SDA) ───►   GPIO 21 (or SDA marking)
   Pin 3 (SCL) ───►   GPIO 22 (or SCL marking)
   Pin 1 (5V)  ───►   5V pin
   Pin 2 (GND) ───►   GND pin
   ```

**If you're using temporary connections (Solution 2 or 3):**

1. **Use IC test clips or female-to-male wires**
2. **Carefully connect to pin holes:**
   - Red wire → 5V hole
   - Black wire → GND hole
   - Yellow wire → GPIO 22 hole (SCL)
   - Green wire → GPIO 21 hole (SDA)
3. **Secure with tape** to prevent wires pulling out
4. **Double-check connections** with multimeter (continuity mode)

---

#### Step 3: Power On System

1. **Connect USB-C power** (2A minimum for display)
2. **Display should light up** (backlight on)
3. **Watch Serial Monitor** (if connected to computer)

**Expected startup sequence:**
```
JNKR Gauge System v2.0
Initializing...
Display: OK
I2C Bus: Scanning...
Found ADS1115 at 0x48
Found MCP9600 at 0x60
All sensors: OK
System ready!
```

---

#### Step 4: Verify Display Output

**Check that display shows:**

- [ ] Graphics render correctly (no corruption)
- [ ] Text is readable
- [ ] Colors are correct (not inverted or washed out)
- [ ] No flickering
- [ ] Updates smoothly

**If display is blank:**
- Check FFC cable connection
- Verify 5V power (measure with multimeter)
- Check code uploaded correctly

---

#### Step 5: Verify Sensor Readings

**Connect actual sensors or simulators to I2C module:**

1. **Thermistors:** Connect 2.2kΩ resistors (should read ~25°C)
2. **MAP sensor:** Connect sensor (should read atmospheric pressure)
3. **Thermocouple:** Connect K-type probe (should read room temp)

**On display/serial, verify:**

- [ ] IAT Pre-IC: Reads room temperature
- [ ] IAT Post-IC: Reads room temperature
- [ ] Coolant: Reads room temperature
- [ ] Boost: Reads ~0 PSI (atmospheric)
- [ ] EGT: Reads room temperature

**Test dynamic response:**
- Touch thermistor → temperature should rise
- Suck on MAP sensor → boost reading should go negative
- Heat thermocouple with lighter → EGT should spike

---

#### Step 6: Test Alerts and Buzzer

**Trigger alerts manually in code or by overheating sensors:**

1. **INFO alert:**
   - Should show yellow warning on display
   - 1 beep from buzzer
   - LED blinks (if installed)

2. **WARNING alert:**
   - Should show orange warning
   - 2 beeps from buzzer

3. **CRITICAL alert:**
   - Should show red warning
   - 3 beeps from buzzer
   - Flashing display

4. **DANGER alert:**
   - Red flashing
   - Continuous alarm
   - (Test briefly - it's loud!)

---

### ✅ Complete System Bench Test Checklist

- [ ] Display connected and showing graphics correctly
- [ ] I2C module detected (0x48 and 0x60 found)
- [ ] All 5 sensors reading correctly
- [ ] Sensor readings update in real-time
- [ ] Temperature values respond to heat/cooling
- [ ] Boost pressure responds to vacuum
- [ ] Alerts trigger at correct thresholds
- [ ] Buzzer sounds for all alert levels
- [ ] System runs stable for 30+ minutes without crashes
- [ ] No overheating (ESP32 or sensors stay cool)
- [ ] Power consumption acceptable (<2A total)

**If all pass: ✅ System ready for vehicle installation!**

---

## 🔬 Complete Bench Testing Procedures

### Test Procedure 1: Initial Power-On

**Purpose:** Verify nothing is damaged before full testing

**Time:** 5 minutes

1. **Visual inspection** - check for damage, correct assembly
2. **Continuity test** - verify no shorts between 5V and GND
3. **Power on with current limiting** - set to 100mA first
4. **Gradually increase current limit** - watch for excessive draw
5. **Check component temperatures** - nothing should be hot
6. **Measure voltages** - 5V at all ICs

**Pass criteria:** Current draw <100mA, voltages correct, nothing hot

---

### Test Procedure 2: I2C Communication

**Purpose:** Verify digital communication works

**Time:** 10 minutes

1. **Run I2C scanner sketch**
2. **Verify both devices detected** (0x48, 0x60)
3. **Test repeated scans** - should be consistent
4. **Try different I2C speeds** - 100kHz and 400kHz
5. **Check for errors in serial output**

**Pass criteria:** Both devices detected, no communication errors

---

### Test Procedure 3: Sensor Accuracy

**Purpose:** Verify readings are correct

**Time:** 20 minutes

1. **Thermistors:**
   - Measure room temperature with reference thermometer
   - Compare to sensor readings
   - Should be within ±2°C

2. **Thermocouple:**
   - Check ice water (0°C)
   - Check boiling water (100°C)
   - Should be within ±2°C

3. **MAP sensor:**
   - Compare to known pressure gauge
   - Should be within ±0.1 PSI

**Pass criteria:** All sensors within ±5% of reference

---

### Test Procedure 4: Long-Term Stability

**Purpose:** Verify system doesn't crash or drift

**Time:** 2+ hours

1. **Power on system**
2. **Let run continuously** for 2 hours minimum
3. **Monitor serial output** for errors
4. **Check sensor readings periodically** - should be stable
5. **Check power consumption** - should not increase over time
6. **Check temperatures** - should remain cool

**Pass criteria:** No crashes, no drift, stable readings

---

### Test Procedure 5: Environmental Stress

**Purpose:** Verify system handles temperature changes

**Time:** 30 minutes

1. **Heat test:**
   - Place system in warm location (30-40°C)
   - Verify still operates correctly
   - Readings should change appropriately

2. **Cool test:**
   - Place system in cool location (5-10°C)
   - Verify still operates correctly
   - Readings should change appropriately

3. **Rapid change test:**
   - Move between hot and cold quickly
   - Verify no crashes or glitches

**Pass criteria:** Works correctly at all temperatures

---

## 🛠️ Troubleshooting

### Problem: ESP32 won't power on

**Symptoms:**
- No LEDs light up
- No display backlight
- No response on serial monitor

**Checks:**
1. Measure 5V at USB-C connector with multimeter
2. Check USB cable (try different cable - some are charge-only!)
3. Try different power supply
4. Check for shorts (5V to GND should be >1kΩ)

**Solutions:**
- Use data-capable USB cable
- Use 2A+ power supply
- Check for damaged components

---

### Problem: I2C devices not detected

**Symptoms:**
- I2C scanner shows "No devices found"
- Serial monitor shows "I2C communication error"

**Checks:**
1. Verify I2C wiring:
   ```
   Multimeter continuity test:
   ESP32 GPIO 21 → I2C Module Pin 4 (SDA): Should beep
   ESP32 GPIO 22 → I2C Module Pin 3 (SCL): Should beep
   ```

2. Check I2C pullup resistors:
   ```
   Measure resistance:
   SDA to 5V: Should be ~4.7kΩ (pullup resistor)
   SCL to 5V: Should be ~4.7kΩ (pullup resistor)
   ```

3. Verify module has power:
   ```
   Measure voltage at ADS1115 VDD pin: Should be 5V
   Measure voltage at MCP9600 VDD pin: Should be 5V
   ```

**Solutions:**
- Check for swapped SCL/SDA wires
- Add external 4.7kΩ pullup resistors if missing
- Try slower I2C speed (100kHz instead of 400kHz)
- Shorten cable length (<3 feet ideal for testing)

---

### Problem: Sensor readings are wrong

**A. Thermistors read too high/low**

**Checks:**
1. Measure thermistor resistance with multimeter:
   ```
   At room temp (~25°C): Should be ~2.2kΩ
   If much different: Wrong thermistor type!
   ```

2. Measure voltage divider resistor:
   ```
   Should be exactly 2.2kΩ
   If wrong: Replace resistor
   ```

3. Measure voltage at ADC input:
   ```
   At room temp: Should be ~2.5V
   If much different: Check wiring
   ```

**Solutions:**
- Verify correct thermistor (2.2kΩ @ 25°C, B=3950)
- Use correct reference resistor (2.2kΩ 1% tolerance)
- Apply calibration offset in code

---

**B. Boost pressure reads wrong**

**Checks:**
1. Verify MAP sensor voltage:
   ```
   At atmospheric pressure: Should be ~0.5V
   At full vacuum: Should be ~0.1V
   At 3 bar: Should be ~4.5V
   ```

2. Check MAP sensor pinout (varies by manufacturer!)
3. Verify 5V power supply to sensor

**Solutions:**
- Confirm MAP sensor is 0-3 bar type
- Check sensor datasheet for correct pinout
- Apply calibration offset if consistently off

---

**C. Thermocouple reads 0°C or error**

**Checks:**
1. Verify thermocouple polarity:
   ```
   Yellow wire (K-type) → T+ terminal
   Red wire (K-type) → T- terminal
   ```

2. Check for open circuit:
   ```
   Measure resistance across thermocouple: Should be <5Ω
   If infinite: Broken wire!
   ```

3. Test with ice water:
   ```
   Should read 0°C (±2°C)
   ```

**Solutions:**
- Swap thermocouple wires if backwards
- Replace thermocouple if broken
- Verify MCP9600 is configured for K-type in code

---

### Problem: Display is blank or garbled

**Symptoms:**
- Backlight on but no graphics
- Random pixels or lines
- Corrupted text

**Checks:**
1. Verify FFC cable connection:
   - Remove and reseat cable
   - Check for bent pins
   - Verify correct orientation

2. Check power:
   ```
   Measure voltage at display connector: Should be 5V
   Current draw: Should be 200-500mA with backlight
   ```

3. Verify display initialization code:
   - Check display dimensions (480x480)
   - Verify correct RGB mode (RGB-666)
   - Check pin assignments

**Solutions:**
- Reseat FFC cable firmly
- Try different USB power supply (2A minimum)
- Verify LVGL configuration in lv_conf.h
- Update display driver code

---

### Problem: System crashes or freezes

**Symptoms:**
- Serial output stops
- Display freezes
- Buzzer plays partial alert and stops
- System requires power cycle

**Checks:**
1. Monitor serial output for error messages
2. Check memory usage:
   ```cpp
   ESP.getFreeHeap();  // Should be >50kB free
   ```

3. Verify power supply stability:
   ```
   Use oscilloscope to check for voltage dips
   Typical: <50mV ripple on 5V rail
   ```

**Solutions:**
- Add 100µF capacitor across 5V and GND near ESP32
- Reduce LVGL buffer size in lv_conf.h
- Increase power supply current capacity
- Check for infinite loops in code
- Enable watchdog timer

---

### Problem: Connections keep falling out (Pin Hole Issue)

**Symptoms:**
- Jumper wires fall out of ESP32 pin holes
- Intermittent connections
- Sensor readings drop out randomly

**Solutions:**
1. **Solder headers** (best permanent solution)
2. **Use tape** to secure wires temporarily
3. **Apply pressure** with helping hands or clamps
4. **Use IC test clips** for more secure grip
5. **Consider breadboard adapter** (more stable platform)

**Quick fix for urgent testing:**
```
1. Strip solid-core wire
2. Insert into ESP32 hole
3. Wrap tape around wire and board (3-4 wraps)
4. Use alligator clip on exposed wire end
5. Secure board to table so it doesn't move
```

---

## 📸 Documentation Tips

**Take photos during bench testing:**

1. **Power connections** - show exactly how everything is wired
2. **Multimeter readings** - capture voltage measurements
3. **Serial monitor output** - screenshot I2C scanner results
4. **Display showing data** - prove system works
5. **Overall setup** - wide shot of complete bench test rig

**Why?**
- Reference when installing in vehicle
- Troubleshooting if problems occur later
- Share with community to help others
- Proof that it worked before vehicle installation!

---

## ✅ Final Bench Test Checklist

**Complete before vehicle installation:**

### I2C Module
- [ ] All continuity tests pass
- [ ] Power-on test shows correct voltages
- [ ] Current draw is 30-80mA
- [ ] Both I2C devices detected (0x48, 0x60)
- [ ] Voltage dividers read ~2.5V with test resistors
- [ ] No components get hot during operation
- [ ] Module runs stable for 30+ minutes

### ESP32 System
- [ ] Display connects and shows graphics
- [ ] I2C communication works reliably
- [ ] All sensors read correctly
- [ ] Sensor readings respond to changes (heat, vacuum)
- [ ] Alerts trigger at correct thresholds
- [ ] Buzzer sounds correctly
- [ ] System stable for 2+ hours continuous operation
- [ ] Power consumption <2A total
- [ ] No memory leaks or crashes

### Physical Assembly
- [ ] All connections secure (soldered headers or reliable clips)
- [ ] Wiring neat and organized
- [ ] Components mounted securely
- [ ] Enclosure closes properly
- [ ] Connectors lock correctly
- [ ] Labels applied to wires/connectors

### Documentation
- [ ] Photos taken of all connections
- [ ] Voltage measurements recorded
- [ ] Serial output captured
- [ ] Any modifications documented
- [ ] Notes on calibration offsets needed

**When all items checked: ✅ Ready for vehicle installation!**

---

## 🎯 Next Steps

**After successful bench testing:**

1. **Install I2C module in engine bay**
   - See: [I2C_ASSEMBLY_GUIDE.md](I2C_ASSEMBLY_GUIDE.md) - Step 11 onwards
   - Choose mounting location (protected from water/heat)
   - Secure with bolts or brackets
   - Connect sensors to module

2. **Route harness through firewall**
   - Find existing grommet or create new hole
   - Use proper firewall grommet
   - Seal with silicone

3. **Install display in cabin**
   - Choose mounting location (visible from driver seat)
   - Route power from fuse box
   - Connect I2C harness

4. **Final testing in vehicle**
   - Power on and verify operation
   - Check readings with engine running
   - Test under load (drive test)
   - Verify alerts work correctly

5. **Enjoy your professional engine monitoring system!** 🎉

---

## 📚 Related Documents

- **[I2C_ASSEMBLY_GUIDE.md](I2C_ASSEMBLY_GUIDE.md)** - Building the sensor module
- **[I2C_SENSOR_MODULE.md](I2C_SENSOR_MODULE.md)** - Design overview
- **[ESP32_SETUP.md](ESP32_SETUP.md)** - Arduino IDE configuration
- **[BREADBOARD_WIRING.md](BREADBOARD_WIRING.md)** - Prototyping guide
- **[AUTOMOTIVE_POWER.md](AUTOMOTIVE_POWER.md)** - Vehicle power connection
- **[CALIBRATION.md](CALIBRATION.md)** - Sensor calibration procedures

---

**Questions?** Open an issue on GitHub with:
- Photos of your setup
- Serial monitor output
- Multimeter readings
- Description of problem

The community is here to help! 🔧⚡

---

**Last Updated:** 2025-11-08
