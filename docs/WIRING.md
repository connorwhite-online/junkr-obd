# Wiring Guide - JNKR Gauge System

**Complete Electrical Connection Instructions for ESP32-S3**

This guide provides step-by-step instructions for wiring the JNKR Gauge System with the Qualia ESP32-S3. Even if you're new to electronics, this guide will walk you through every connection with detailed explanations.

> **Important**: Before starting, read [AUTOMOTIVE_POWER.md](AUTOMOTIVE_POWER.md) to understand how to power your gauge from the vehicle!

> **Note**: For breadboard prototyping, see [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md)

---

## 📋 Table of Contents

- [Safety First](#safety-first)
- [Tools and Materials](#tools-and-materials)
- [Understanding the Basics](#understanding-the-basics)
- [Power Supply Wiring](#power-supply-wiring)
- [Sensor Connections](#sensor-connections)
- [Display Connection](#display-connection)
- [Buzzer Connection](#buzzer-connection)
- [Testing and Verification](#testing-and-verification)
- [Wiring Diagrams](#wiring-diagrams)

---

## ⚠️ Safety First

### Before You Start

1. **Disconnect battery negative terminal** - Always disconnect the vehicle battery before working on electrical systems
2. **No smoking** - Flammable fuel vapors may be present
3. **Proper ventilation** - Work in a well-ventilated area
4. **Fire extinguisher** - Have one nearby when working on vehicle systems
5. **Eye protection** - Wear safety glasses when drilling or cutting
6. **Insulated tools** - Use proper tools to prevent shorts

### Electrical Safety

- Never create a short circuit (red to ground)
- Use proper fuse ratings (3A for this system)
- Protect wires from sharp edges and heat
- Use heat shrink or electrical tape on all connections
- Double-check polarity before powering on
- Test connections with a multimeter first

---

## 🧰 Tools and Materials

### Required Tools

- **Multimeter** - For testing voltage and continuity ($15-30)
- **Wire strippers** - 20-22 AWG ($10-20)
- **Crimping tool** - For terminals and connectors ($15-40)
- **Soldering iron** - 25-40W (optional but recommended) ($20-50)
- **Heat gun or lighter** - For heat shrink tubing
- **Screwdrivers** - Phillips and flathead set
- **Wire cutters** - Side-cutting pliers
- **Drill and bits** - For mounting and cable entry

### Required Materials

- **22 AWG automotive wire** - Red, black, and various colors
- **Heat shrink tubing** - Various sizes (3mm, 5mm, 10mm)
- **Crimp connectors** - Spade, ring, and butt connectors
- **Zip ties** - For securing wire bundles
- **Electrical tape** - High-quality vinyl tape
- **Fuse holder** - Inline blade fuse holder
- **3A blade fuse** - For system protection
- **Wire labels** - For identifying connections
- **Solder** - 60/40 rosin core (if soldering)

**Estimated cost**: $50-100 for all materials

---

## 🎓 Understanding the Basics

### Voltage (V)

Think of voltage like water pressure in a pipe. Your vehicle has a 12V electrical system (actually 12.6V-14.4V when running).

- **12V** = Vehicle power (from battery/alternator)
- **5V** = Arduino and sensor power (from buck converter)
- **GND** = Ground / 0V reference (vehicle chassis)

### Current (A)

Current is like water flow. Our system uses about 1-2 amps total.

- **mA (milliamps)** = 1/1000 of an amp
- Sensors use 5-50 mA each
- Display uses 100-300 mA
- Arduino uses 100-200 mA
- **Total: ~1.5A maximum**

### Resistance (Ω - Ohms)

Resistance restricts current flow. We use resistors to create voltage dividers.

- **2.2kΩ** = 2,200 ohms (for thermistor reference)
- **10kΩ** = 10,000 ohms (for voltage divider)
- Color code: Brown-Black-Red = 1-0-00 = 1,000Ω (1kΩ)

### Polarity

**Critical**: Always connect positive to positive, negative to negative!

- **Red wire** = Positive (+12V or +5V)
- **Black wire** = Negative (Ground, GND, 0V)
- **Reversing polarity will damage components**

---

## 🔋 Power Supply Wiring

> **📖 FIRST**: Read [AUTOMOTIVE_POWER.md](AUTOMOTIVE_POWER.md) for complete details on:
> - Where to tap 12V power in your vehicle
> - Switched vs constant power (and why it matters)
> - Proper grounding techniques
> - Fuse selection and placement
> - Toyota 1KZTE-specific guidance

### Step 1: Understanding Power Flow

```
Vehicle 12V Battery
        │
        ├─ Fuse (3A) ─→ Buck Converter Input (12V)
        │                      │
        │                      ├─→ Buck Converter Output (5V)
        │                      │         │
        │                      │         ├─→ Arduino Mega (5V pin)
        │                      │         ├─→ Nextion Display (5V)
        │                      │         ├─→ MAX31855 Module (VCC)
        │                      │         └─→ MAP Sensor (5V)
        │                      │
        └─ Ground ──────────→ Buck GND ─→ Arduino GND ─→ All Components GND
```

### Step 2: Buck Converter Wiring

The buck converter steps down 12V vehicle power to 5V for the Arduino and sensors.

**Connections:**

1. **12V Input (+)**
   - Wire: Red 20 AWG
   - Source: Vehicle switched 12V (fuse box)
   - Through: 3A inline fuse
   - To: Buck converter input positive terminal

2. **12V Input (-)**
   - Wire: Black 20 AWG
   - Source: Vehicle chassis ground
   - To: Buck converter input negative terminal

3. **5V Output (+)**
   - Wire: Red 22 AWG
   - From: Buck converter output positive
   - To: Arduino Mega "5V" pin

4. **5V Output (-)**
   - Wire: Black 22 AWG
   - From: Buck converter output negative
   - To: Arduino Mega "GND" pin

### Step 3: Vehicle 12V Connection

> **⚠️ Critical**: See [AUTOMOTIVE_POWER.md](AUTOMOTIVE_POWER.md) for detailed instructions!

**Quick Summary for 1KZTE:**

1. **Power Source**: Interior fuse box, radio/accessory circuit (15A fuse)
2. **Method**: Add-A-Circuit fuse tap with 3A fuse
3. **Wire**: 20 AWG red (+12V) and black (GND)
4. **Ground**: Chassis ground bolt near fuse box (clean bare metal)

**Connection Method:**

```
Vehicle Fuse Box
    │
    ├── Add-a-circuit Fuse Tap (3A fuse)
    │       │
    │       └── Red wire (20 AWG) ──→ Buck Converter 12V+
    │
    └── Vehicle Chassis Ground Point
            │
            └── Black wire (20 AWG) ──→ Buck Converter 12V-
```

**Why Switched Power?**
- ✅ Turns on/off with ignition automatically
- ✅ No battery drain when vehicle is off
- ✅ No separate power switch needed
- ✅ Professional installation

For complete step-by-step with photos and troubleshooting, see [AUTOMOTIVE_POWER.md](AUTOMOTIVE_POWER.md).

### Step 4: Arduino Power

**Method 1: Via 5V Pin (Recommended)**
- Connect buck converter 5V output to Arduino "5V" pin
- Connect buck converter GND to Arduino "GND" pin
- This bypasses Arduino's voltage regulator for cooler operation

**Method 2: Via DC Jack**
- Use buck converter set to 7-9V
- Connect to Arduino's DC power jack
- Arduino's onboard regulator provides 5V

**We recommend Method 1** - more efficient and cooler running.

---

## 🌡️ Sensor Connections

### NTC Thermistor Wiring (Temperature Sensors)

**What is an NTC Thermistor?**

An NTC (Negative Temperature Coefficient) thermistor is a temperature sensor that changes resistance with temperature. As temperature increases, resistance decreases.

**Wiring Configuration: Voltage Divider**

```
                    +5V
                     │
                     │
                    [2.2kΩ Reference Resistor]
                     │
                     ├──────→ To Arduino Analog Pin
                     │
                    [NTC Thermistor]
                     │
                     │
                    GND
```

**How it works:**
- At 25°C, NTC = 2,252Ω → Voltage = ~2.5V
- At 100°C, NTC = ~200Ω → Voltage = ~0.5V
- Arduino reads voltage and calculates temperature

#### Intake Air Temperature (IAT) - Pin A0

**What it monitors:** Temperature of air entering the engine (after intercooler)

**Installation:**
1. Install sensor in intake pipe between intercooler and engine
2. Use M10 or 1/8" NPT fitting
3. Ensure sensor tip is in airstream (not touching pipe wall)

**Wiring:**
```
Arduino 5V ─┬─ 2.2kΩ Resistor ─┬─ Arduino A0
            │                    │
            │                    │
            └─ NTC Thermistor ───┴─ Arduino GND
```

**Step-by-Step:**

1. **Solder 2.2kΩ resistor to NTC thermistor:**
   - One end of resistor to one thermistor wire
   - Solder joint and heat shrink

2. **Wire connections:**
   - Free resistor end → Arduino 5V pin
   - Resistor-thermistor junction → Arduino A0 pin
   - Free thermistor end → Arduino GND pin

3. **Sensor placement:**
   - Drill hole in intake pipe
   - Install bung/fitting
   - Thread thermistor into pipe
   - Seal with silicone or thread sealant

4. **Wire routing:**
   - Route wire along existing harness
   - Protect from heat and moving parts
   - Secure with zip ties every 12 inches

#### Coolant Temperature - Pin A1

**What it monitors:** Engine coolant temperature

**Installation Options:**

1. **Replace existing sensor** (not recommended - may trigger check engine light)
2. **Add T-adapter** to existing sensor location
3. **Install in heater hose** using compression fitting

**Wiring:**
- Same voltage divider as IAT (2.2kΩ + NTC)
- Connect to Arduino A1 instead of A0

#### Oil Temperature (Optional) - Pin A2

**What it monitors:** Engine oil temperature

**Installation:**
- Oil pan drain plug adapter
- Oil filter sandwich plate
- Dipstick tube adapter

**Wiring:**
- Same voltage divider as IAT (2.2kΩ + NTC)
- Connect to Arduino A2

---

### Boost Pressure Sensor (MAP) - Pin A3

**What is a MAP Sensor?**

A MAP (Manifold Absolute Pressure) sensor measures air pressure in the intake manifold. We use it to monitor turbo boost pressure.

**Sensor Type:** 3-bar absolute pressure sensor
- Output: 0.5V to 4.5V
- Range: 0 to 3 bar absolute
- Connector: Usually 3-pin (5V, Signal, GND)

**Installation:**

1. **Sensor Mounting:**
   - Mount in engine bay, protected from heat
   - Keep sensor higher than manifold (prevents oil contamination)
   - Secure with bracket or zip ties

2. **Vacuum Line Connection:**
   - Find boost reference port on intake manifold
   - Install T-fitting (if needed)
   - Connect 4mm silicone vacuum line
   - Route to sensor nipple
   - Secure with zip ties or clamps

3. **Electrical Connection:**
   ```
   MAP Sensor Pin 1 (5V)     → Arduino 5V
   MAP Sensor Pin 2 (Signal) → Arduino A3
   MAP Sensor Pin 3 (GND)    → Arduino GND
   ```

4. **Wire Routing:**
   - Use 3-conductor 22 AWG cable
   - Route away from ignition wires
   - Secure with zip ties

**Testing:**
- With engine off: Should read ~1.0V (atmospheric pressure)
- With engine running at idle: Should read ~0.5-0.8V (vacuum)
- Under boost: Should read >1.0V (increasing with boost)

---

### Exhaust Gas Temperature (EGT) - MAX31855 Module

**What is EGT?**

Exhaust Gas Temperature is the temperature of exhaust gases leaving the engine. High EGT can indicate excessive load or lean condition.

**Components:**
- K-type thermocouple (probe)
- MAX31855 thermocouple amplifier module
- SPI communication to Arduino

**Thermocouple Installation:**

1. **Location:**
   - Exhaust manifold or
   - Downpipe, 6-12 inches after turbo outlet
   - Use M6 or 1/8" NPT bung

2. **Mounting:**
   - Weld bung into exhaust pipe
   - Angle slightly downstream
   - Probe tip should be in exhaust stream center
   - Avoid contact with pipe wall

3. **Wire Routing:**
   - Use high-temperature wire
   - Route away from other wires (can cause interference)
   - Protect from heat with heat shrink or fiberglass sleeving
   - Do not splice or extend thermocouple wires (causes errors)

**MAX31855 Module Wiring:**

```
MAX31855 Module Pins:
VCC   → Arduino 5V
GND   → Arduino GND
DO    → Arduino Pin 50 (MISO)
CS    → Arduino Pin 10
CLK   → Arduino Pin 52 (SCK)

Thermocouple Connections:
+ (Yellow)  → MAX31855 T+
- (Red)     → MAX31855 T-
```

**Step-by-Step:**

1. **Module to Arduino:**
   - VCC → Arduino 5V (red wire)
   - GND → Arduino GND (black wire)
   - DO → Pin 50 (green wire)
   - CS → Pin 10 (yellow wire)
   - CLK → Pin 52 (blue wire)

2. **Thermocouple to Module:**
   - Yellow wire → T+ terminal
   - Red wire → T- terminal
   - **Note:** Do NOT reverse polarity!

3. **Testing:**
   - Power on Arduino
   - Open Serial Monitor
   - Should read room temperature (~20-25°C)
   - Heat probe tip with lighter
   - Temperature should increase rapidly

---

### Battery Voltage Monitoring - Pin A7

**Why Monitor Battery Voltage?**

Battery voltage indicates electrical system health:
- <12V when running = charging system problem
- >15V = overcharging (regulator failure)

**Voltage Divider Circuit:**

```
           12V Vehicle
             │
             │
            [10kΩ Resistor]
             │
             ├──────→ Arduino A7
             │
            [2.2kΩ Resistor]
             │
             │
            GND
```

**Why We Need This:**

Arduino can only read 0-5V. Vehicle voltage is 11-15V. The voltage divider reduces voltage to safe levels:
- 12V input → 2.18V at A7
- 14V input → 2.54V at A7

**Building the Voltage Divider:**

1. **Solder resistors in series:**
   - One end of 10kΩ resistor = 12V input
   - Other end of 10kΩ → one end of 2.2kΩ
   - Junction point → Arduino A7
   - Free end of 2.2kΩ → GND

2. **Connection:**
   ```
   Vehicle 12V ─── 10kΩ ───┬─── Arduino A7
                           │
                          2.2kΩ
                           │
                          GND ─── Arduino GND
   ```

3. **Heat shrink all joints** to prevent shorts

4. **Testing:**
   - Measure vehicle voltage with multimeter: e.g., 12.6V
   - Measure A7 voltage: should be ~2.3V
   - Calculation: A7 voltage × 5.5 = vehicle voltage

---

## 📺 Display Connection

### Nextion NX4832T035 Display

**Display Specifications:**
- Size: 3.5 inches (480×320 pixels)
- Power: 5V, 100-300mA
- Communication: UART (Serial) at 115200 baud
- Connector: 4-pin (5V, TX, RX, GND)

**Wiring:**

```
Nextion Display          Arduino Mega
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Pin 1: 5V (Red)      →   5V
Pin 2: TX (Blue)     →   Pin 16 (RX2)
Pin 3: RX (Yellow)   →   Pin 17 (TX2)
Pin 4: GND (Black)   →   GND
```

**Important:** TX goes to RX, RX goes to TX (crossover)

**Step-by-Step:**

1. **Identify Display Pins:**
   - Look at back of display
   - Pins should be labeled
   - Pin 1 is usually marked with triangle or "1"

2. **Wire Connection:**
   - Red wire from Pin 1 → Arduino 5V
   - Blue wire from Pin 2 → Arduino Pin 16
   - Yellow wire from Pin 3 → Arduino Pin 17
   - Black wire from Pin 4 → Arduino GND

3. **Physical Mounting:**
   - Measure display dimensions
   - Cut opening in enclosure
   - Secure with mounting screws or hot glue
   - Ensure front glass is protected

4. **Cable Management:**
   - Keep display cable away from high-current wires
   - Avoid sharp bends
   - Secure with zip tie near display

**Testing:**
- Power on system
- Display should show boot screen
- After Arduino starts, should show "JNKR Gauge"
- If blank: Check power and TX/RX connections

---

## 🔊 Buzzer Connection

### Piezo Buzzer

**Buzzer Type:** Active piezo buzzer, 2kHz, 3-5V

**What is Active vs Passive?**
- **Active:** Has built-in oscillator, just apply voltage
- **Passive:** Requires PWM signal to make sound
- **We use active** for simplicity

**Wiring:**

```
Buzzer Pin (+) Red    →  Arduino Pin 8
Buzzer Pin (-) Black  →  Arduino GND
```

**Step-by-Step:**

1. **Identify Buzzer Polarity:**
   - Red wire or "+" marking = Positive
   - Black wire or "-" marking = Negative
   - If unmarked: Look for "+" symbol on buzzer body

2. **Connect:**
   - Solder red wire to positive pin
   - Solder black wire to negative pin
   - Heat shrink connections

3. **Route to Arduino:**
   - Red wire → Arduino Pin 8
   - Black wire → Arduino GND

4. **Mount Buzzer:**
   - Use double-sided tape or hot glue
   - Mount with sound port facing open space
   - Don't mount against solid surface (muffles sound)

**Testing:**
- Power on system
- Should hear single beep on startup
- Simulate alert: Heat EGT sensor or disconnect sensor
- Should hear alert beeps

---

## ✅ Testing and Verification

### Pre-Power Tests

**Before connecting power, check with multimeter:**

1. **Continuity Tests:**
   - All GND connections should have continuity (0Ω)
   - 5V rail should NOT have continuity to GND
   - 12V rail should NOT have continuity to GND

2. **Resistance Tests:**
   - Between 5V and GND: Should read >1kΩ
   - Between 12V and GND: Should read >1kΩ
   - If <100Ω: You have a short circuit!

3. **Visual Inspection:**
   - No exposed wire touching each other
   - All solder joints shiny and solid
   - Heat shrink on all connections
   - No wires under stress or strain

### Power-On Tests

**Step 1: Bench Test (No Vehicle)**

1. Connect 12V power supply (2A minimum) to buck converter
2. Verify buck converter outputs 5V
3. Connect Arduino
4. Verify Arduino powers on (LED lights)
5. Upload code and open Serial Monitor
6. Check for initialization messages

**Step 2: Sensor Tests (No Vehicle)**

1. **Thermistor Test:**
   - Should read room temperature (~20-25°C)
   - Pinch sensor with fingers
   - Temperature should increase

2. **MAP Sensor Test:**
   - Should read ~1.0 bar (atmospheric)
   - Cover sensor port
   - Blow into sensor port
   - Should see pressure change

3. **EGT Test:**
   - Should read room temperature
   - Heat probe tip with lighter
   - Should read increasing temperature

4. **Display Test:**
   - Should show gauge values
   - Values should update
   - Touch screen should respond

5. **Buzzer Test:**
   - Should beep once on startup
   - Heat EGT probe >600°C
   - Should trigger alert beeps

**Step 3: Vehicle Installation Test**

1. Install all sensors
2. Connect power to vehicle (through fuse)
3. Turn ignition ON (engine OFF)
4. Verify all readings:
   - IAT: Ambient temperature
   - Coolant: Engine temperature
   - EGT: Low (engine not running)
   - Boost: ~0 PSI (atmospheric)
   - Battery: 12-13V

5. Start engine and verify:
   - All sensors reading
   - No error messages
   - Display updating

---

## 📊 Wiring Diagrams

### Complete System Wiring Diagram

```
                                    JNKR GAUGE SYSTEM
                                    =================

Vehicle 12V ──[3A Fuse]── Buck Converter 12V Input
   Battery                         │
      │                            ├─ Output: 5V
      │                            │
      └──── GND ──────────────┬────┴────┬─────────────────┬──────────────┐
                              │         │                 │              │
                        Arduino Mega    │                 │              │
                        ┌─────────┐     │                 │              │
                        │         │     │                 │              │
                5V ─────┤ 5V      │     │                 │              │
                GND ────┤ GND     │     │                 │              │
                        │         │     │                 │              │
                        │ A0 ◄────┼─────┴─ IAT Sensor ────┤              │
                        │         │     (2.2kΩ + NTC)     5V            GND
                        │ A1 ◄────┼─────── Coolant Sensor ┤              │
                        │         │     (2.2kΩ + NTC)     5V            GND
                        │ A2 ◄────┼─────── Oil Sensor ────┤              │
                        │         │     (2.2kΩ + NTC)     5V            GND
                        │ A3 ◄────┼─────── MAP Sensor ────┤              │
                        │         │                       5V            GND
                        │ A7 ◄────┴─── Battery ───────────┤              │
                        │         │    (10k + 2.2k)      12V            GND
                        │         │                                     │
                        │ Pin 8 ──┴─── Buzzer ────────────────────── GND
                        │         │        (+)
                        │         │
                        │ Pin 10 ─┼─── MAX31855 CS
                        │ Pin 50 ─┼─── MAX31855 DO (MISO)
                        │ Pin 52 ─┼─── MAX31855 CLK (SCK)
                        │         │
                        │ Pin 16 ─┼─── Nextion TX (Display RX)
                        │ Pin 17 ─┼─── Nextion RX (Display TX)
                        │         │
                        └─────────┘
                              │
                              ├─── MAX31855 Module
                              │    ├─ VCC ← 5V
                              │    ├─ GND ← GND
                              │    ├─ DO → Pin 50
                              │    ├─ CS → Pin 10
                              │    ├─ CLK → Pin 52
                              │    └─ T+/T- ← K-Type Thermocouple
                              │
                              └─── Nextion Display
                                   ├─ 5V ← 5V
                                   ├─ GND ← GND
                                   ├─ TX → Pin 16
                                   └─ RX ← Pin 17
```

### Voltage Divider Diagrams

**Thermistor Circuit (IAT, Coolant, Oil):**
```
     5V
      │
      ▼
   ┌──┴──┐
   │     │ 2.2kΩ Reference Resistor
   └──┬──┘
      ├────────► To Arduino Analog Pin (A0, A1, or A2)
      │
   ┌──┴──┐
   │     │ NTC Thermistor (2252Ω @ 25°C)
   └──┬──┘
      │
     GND
```

**Battery Voltage Divider:**
```
    12V
      │
      ▼
   ┌──┴──┐
   │     │ 10kΩ Resistor
   └──┬──┘
      ├────────► To Arduino A7
      │
   ┌──┴──┐
   │     │ 2.2kΩ Resistor
   └──┬──┘
      │
     GND

Ratio: 12.2kΩ / 2.2kΩ = 5.55:1
12V input → 2.16V at A7
```

---

## 🔍 Wire Color Code Reference

**Standard Color Coding:**

| Color | Common Use | This Project |
|-------|------------|--------------|
| Red | Positive power | 12V, 5V positive |
| Black | Ground/Negative | GND, 0V |
| Yellow | Signal/Data | Thermocouple, signals |
| Blue | Signal/Data | Nextion TX, signals |
| Green | Signal/Data | SPI data lines |
| White | Signal/Data | Analog signals |
| Brown | Low voltage | Sensor outputs |
| Orange | Switched power | Ignition switched 12V |

**Project-Specific:**
- Red = Power (12V or 5V)
- Black = Ground
- Yellow = Analog sensor signals
- Blue/Green = Digital signals
- White = Optional signals

---

## 📝 Wiring Checklist

Before final assembly, verify:

- [ ] All connections heat-shrunk
- [ ] All wires labeled
- [ ] No exposed conductors
- [ ] Proper strain relief
- [ ] Secured with zip ties
- [ ] Routed away from heat sources
- [ ] Routed away from moving parts
- [ ] Fuse installed on 12V line
- [ ] All grounds connected
- [ ] Continuity tested
- [ ] No short circuits (5V to GND)
- [ ] Voltage measured at all points
- [ ] Display connected and working
- [ ] Sensors reading correctly
- [ ] Buzzer tested
- [ ] All connections documented

---

## 🆘 Common Wiring Problems

### Problem: Arduino doesn't power on
- Check 5V supply from buck converter
- Verify GND connection
- Check for short circuit
- Try different power source

### Problem: Sensor reads wrong values
- Check voltage divider resistor values
- Verify connections
- Test sensor resistance with multimeter
- Check for loose connections

### Problem: Display doesn't work
- Check 5V power to display
- Verify TX/RX connections (may be swapped)
- Check baud rate setting
- Test display independently

### Problem: Intermittent readings
- Check for loose connections
- Verify solder joints
- Check for damaged wires
- Test under vibration

---

## 📞 Need Help?

If you're stuck:

1. **Re-read this guide** - Answer is usually here
2. **Check Serial Monitor** - Look for error messages
3. **Test with multimeter** - Verify voltages and continuity
4. **Open an Issue** on GitHub with:
   - Clear description of problem
   - Photos of wiring
   - Serial Monitor output
   - What you've already tried

---

**Next Steps:**

After completing wiring, proceed to:
- [Assembly Guide](ASSEMBLY.md) - Final installation
- [Calibration Guide](CALIBRATION.md) - Sensor calibration
- Main README for software upload

---

*Remember: Take your time, double-check everything, and when in doubt, ask for help!*
