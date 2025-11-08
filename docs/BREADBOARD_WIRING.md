# Breadboard Wiring Diagram - JNKR Gauge System

**Complete Prototyping Guide for Qualia ESP32-S3**

This guide shows you how to wire up your JNKR Gauge system on a breadboard for testing before final installation.

---

## 📋 What You Need

### Hardware
- **Qualia ESP32-S3** (Adafruit #5800)
- **2.1" Round RGB Display** (480x480, RGB-666)
- **40-pin FFC Cable** (included with display)
- **ADS1015 I2C ADC Module** (for extra analog inputs)
- **Stemma QT Cable** (for I2C connection)
- MAX31855 Thermocouple Amplifier
- K-Type Thermocouple
- 3-Bar MAP Sensor
- 3x NTC Thermistors (2.2kΩ)
- Piezo Buzzer
- 830-point Breadboard
- DuPont Jumper Wires (Male-Male, Male-Female)

> **Note**: The Qualia board has the display connector built-in. Simply plug the 40-pin FFC cable from the display into the Qualia board.

### Components
- 3x 2.2kΩ Resistors (for NTC voltage dividers)
- 3x 0.1µF Ceramic Capacitors (power filtering)
- 5V Power Supply (USB cable for testing)

---

## 🔌 Complete Wiring Diagram

### System Overview

```
                           ┌─────────────────────────┐
                           │   Arduino Mega 2560     │
                           │                         │
   ┌──────────┐           │  [USB]  [ ][ ][ ][ ]   │
   │  TFT     │◄──SPI────►│                         │
   │ Display  │           │   ┌───────────┐         │
   └──────────┘           │   │ ATmega    │         │
                          │   │  2560     │         │
   ┌──────────┐           │   └───────────┘         │
   │ MAX31855 │◄──SPI────►│                         │
   │  (EGT)   │           │  A0 A1 A2 A3 ... A15   │
   └─────┬────┘           │  │  │  │  │            │
         │                │  │  │  │  │            │
         │ Thermocouple   │  │  │  │  │            │
         ▼                │  │  │  │  │            │
    [ EGT Sensor ]        │  │  │  │  └────┐       │
                          │  │  │  │       │       │
    ┌─────────────────────┼──┘  │  │       │       │
    │  ┌──────────────────┼─────┘  │       │       │
    │  │  ┌───────────────┼────────┘       │       │
    │  │  │  ┌────────────┼────────────────┘       │
    │  │  │  │            │                         │
    ▼  ▼  ▼  ▼            │  10 11 12 13  50 51 52 │
   NTC NTC NTC MAP        │  │  │  │  │   │  │  │  │
   IAT IAT Cool Boost     │  │  │  │  │   │  │  │  │
   Pre Post Temp Press    │  │  │  │  └───┼──┼──┼──┼─► TFT CS
                          │  │  │  │      │  │  │  │
   ┌──────────┐           │  └──┼──┼──────┼──┼──┼──┼─► TFT Touch CS
   │  Buzzer  │◄─────PWM──┼─────┘  │      │  │  │  │
   └──────────┘           │        └──────┼──┼──┼──┼─► Touch IRQ (optional)
                          │               │  │  │  │
                          │         LED►──┼──┼──┼──┼─► Status LED
                          │               │  │  │  │
                          └───────────────┼──┼──┼──┼──
                                         MISO│  │  │
                                         MOSI┘  │  │
                                           SCK──┘  │
                                           GND─────┘
```

---

## 📐 Detailed Connection Tables

### 1. TFT Display (3.5" Breakout #2050)

**Arduino Mega → TFT Display**

| Arduino Pin | TFT Pin | Wire Color | Function |
|-------------|---------|------------|----------|
| **5V** | VIN | Red | Power (5V) |
| **GND** | GND | Black | Ground |
| **10** | TFT_CS | Orange | TFT Chip Select |
| **9** | TFT_DC | Yellow | Data/Command |
| **8** | TFT_RST | White | Reset |
| **51** | MOSI | Blue | SPI Data Out |
| **50** | MISO | Green | SPI Data In |
| **52** | SCK | Purple | SPI Clock |
| **11** | TOUCH_CS | Brown | Touch Chip Select |
| **12** | TOUCH_IRQ | Gray | Touch Interrupt (optional) |

**Visual Pin Mapping:**

```
┌─────────────────────────────────┐
│   Adafruit 3.5" TFT Display     │
│          (Back View)            │
├─────────────────────────────────┤
│                                 │
│  [microSD Card Slot]            │
│                                 │
│   Pin Headers (Right Side):     │
│   ┌─────────────────────────┐   │
│   │ GND  ────────────────── │◄─── Black   (Ground)
│   │ VIN  ────────────────── │◄─── Red     (5V)
│   │ 3.3V (not used)         │
│   │ TFT_CS  ───────────────│◄─── Orange  (Pin 10)
│   │ TFT_RST ───────────────│◄─── White   (Pin 8)
│   │ TFT_DC  ───────────────│◄─── Yellow  (Pin 9)
│   │ MOSI ──────────────────│◄─── Blue    (Pin 51)
│   │ SCK ───────────────────│◄─── Purple  (Pin 52)
│   │ MISO ──────────────────│◄─── Green   (Pin 50)
│   │ LITE (backlight, to 5V)│◄─── Red     (5V)
│   └─────────────────────────┘   │
│                                 │
│   Pin Headers (Left Side):      │
│   ┌─────────────────────────┐   │
│   │ X+     (touch - no conn)│   │
│   │ Y+     (touch - no conn)│   │
│   │ X-     (touch - no conn)│   │
│   │ Y-     (touch - no conn)│   │
│   │ IM0    (no connection)  │   │
│   │ IM1    (no connection)  │   │
│   │ IM2    (no connection)  │   │
│   │ IM3    (no connection)  │   │
│   └─────────────────────────┘   │
│                                 │
│   Touch Controller Pins:        │
│   ┌─────────────────────────┐   │
│   │ TOUCH_CS ──────────────│◄─── Brown   (Pin 11)
│   │ TOUCH_IRQ ─────────────│◄─── Gray    (Pin 12, optional)
│   └─────────────────────────┘   │
│                                 │
└─────────────────────────────────┘
```

---

### 2. MAX31855 Thermocouple Amplifier (EGT Sensor)

**Arduino Mega → MAX31855**

| Arduino Pin | MAX31855 Pin | Wire Color | Function |
|-------------|--------------|------------|----------|
| **5V** | VCC | Red | Power (3.3-5V) |
| **GND** | GND | Black | Ground |
| **7** | CS | Orange | Chip Select |
| **50** | SO (MISO) | Green | SPI Data Out |
| **52** | SCK | Purple | SPI Clock |

**MAX31855 → Thermocouple**

| MAX31855 | Thermocouple | Notes |
|----------|--------------|-------|
| **+** | Red Wire | Positive lead |
| **-** | Yellow/Blue Wire | Negative lead |

**Visual:**

```
┌──────────────────────────┐
│   MAX31855 Breakout      │
│   (Thermocouple Amp)     │
├──────────────────────────┤
│                          │
│   Screw Terminals:       │
│   ┌────┬────┐            │
│   │ +  │ -  │◄─────────── K-Type Thermocouple
│   └────┴────┘            │     (Red = +, Yellow = -)
│                          │
│   Pin Headers:           │
│   ┌──────────────────┐   │
│   │ VCC ───────────  │◄─── Red    (5V)
│   │ GND ───────────  │◄─── Black  (GND)
│   │ DO (SO/MISO) ── │◄─── Green  (Pin 50)
│   │ CS ────────────  │◄─── Orange (Pin 7)
│   │ CLK (SCK) ─────  │◄─── Purple (Pin 52)
│   └──────────────────┘   │
│                          │
└──────────────────────────┘
```

---

### 3. NTC Thermistors (Temperature Sensors)

Each NTC thermistor needs a **voltage divider** circuit with a 2.2kΩ resistor.

**Circuit Schematic for ONE NTC:**

```
    5V (Arduino)
     │
     ├─────────┐
     │         │
    ┌┴┐      [2.2kΩ]
    │ │ NTC    Resistor
    │ │ 2.2kΩ  │
    └┬┘        │
     │         │
     └─────────┼─────► To Arduino Analog Pin
               │
              GND
```

**Breadboard Layout:**

```
    Power Rails              Breadboard              Arduino
    ───────────              ──────────              ───────
    5V  GND                  a b c d e               Analog
    │    │                   │ │ │ │ │               Pins
    │    │                   │ │ │ │ │               ──────
    │    └───────────────────┼─┼─┼─┼─┼───────────► GND
    │                        │ │ │ │ │
    └────────[2.2kΩ]─────────┼─┼─┴─┼─┼──► A0 (IAT Pre)
             Resistor        │ │ │ │ │
                 │           │ │ │ │ │
                 └───[NTC]───┼─┼─┬─┼─┼──► GND
                             │ │ │ │ │
    ─────────[2.2kΩ]─────────┼─┼─┴─┼─┼──► A1 (IAT Post)
                 │           │ │ │ │ │
                 └───[NTC]───┼─┼─┬─┼─┼──► GND
                             │ │ │ │ │
    ─────────[2.2kΩ]─────────┼─┼─┴─┼─┼──► A2 (Coolant)
                 │           │ │   │ │
                 └───[NTC]───┼─┼───┴─┼──► GND
                             │ │     │
```

**Connection Table (All 3 Thermistors):**

| Arduino Pin | Component | Connection |
|-------------|-----------|------------|
| **5V** | 2.2kΩ Resistor (all 3) | One end |
| **A0** | Junction (Resistor + NTC Pre) | Middle point |
| **A1** | Junction (Resistor + NTC Post) | Middle point |
| **A2** | Junction (Resistor + NTC Coolant) | Middle point |
| **GND** | All 3 NTC sensors | Other end |

---

### 4. MAP Sensor (Boost Pressure)

**Arduino Mega → MAP Sensor**

| Arduino Pin | MAP Pin | Wire Color | Function |
|-------------|---------|------------|----------|
| **5V** | VCC (+) | Red | Power (5V) |
| **GND** | GND (-) | Black | Ground |
| **A3** | Signal (OUT) | Yellow | Analog Output |

**Visual:**

```
┌────────────────────────┐
│   3-Bar MAP Sensor     │
│   (Boost Pressure)     │
├────────────────────────┤
│                        │
│   [Vacuum Nipple]      │  ◄──── Connect vacuum hose
│   (to intake manifold) │         to intake manifold
│                        │
│   Connector (3-pin):   │
│   ┌────────────────┐   │
│   │ 1. VCC (Red)   │◄──── Red    (5V)
│   │ 2. GND (Black) │◄──── Black  (GND)
│   │ 3. OUT (Yellow)│◄──── Yellow (Pin A3)
│   └────────────────┘   │
│                        │
└────────────────────────┘
```

---

### 5. Piezo Buzzer

**Arduino Mega → Buzzer**

| Arduino Pin | Buzzer Pin | Wire Color | Function |
|-------------|------------|------------|----------|
| **6** | Positive (+) | Red | PWM Signal |
| **GND** | Negative (-) | Black | Ground |

**Visual:**

```
┌────────────────┐
│ Piezo Buzzer   │
│   (Active)     │
├────────────────┤
│                │
│   ┌────┬────┐  │
│   │ +  │ -  │  │
│   └─┬──┴─┬──┘  │
│     │    │     │
│     │    └─────┼──► Black (GND)
│     │          │
│     └──────────┼──► Red (Pin 6)
│                │
└────────────────┘
```

**Note:** Pin 6 moved from original pin 8 to avoid conflict with TFT display.

---

## 🍞 Complete Breadboard Layout

### Breadboard Setup Diagram

```
Power Rails               Breadboard Rows                      Arduino Mega
──────────────           ───────────────────                  ─────────────
  +5V    GND                                                    
   ║      ║              ┌───────────────────┐                Digital Pins
   ║      ║              │ a b c d e │ f g h i j              ────────────
   ║      ║              ├───────────┼─────────┤               6  7  8  9 10 11 12 13
   ║      ║           1  │ ● ● ● ● ● │ ● ● ● ● ●              │  │  │  │  │  │  │  │
   ║      ║           2  │ ● ● ● ● ● │ ● ● ● ● ●              │  │  │  │  │  │  │  │
   ╠══[2.2kΩ]══════►  3  │ ● ● ● ● ● │ ● ● ● ● ●              │  │  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │              │  │  │  │  │  │  │ LED
   ║   ◄══[NTC1]═════  4  │ ● ● ● ● ● │ ● ● ● ● ●──────► A0   │  │  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │              │  │  │  │  │  │  │  │
   ╠══[2.2kΩ]══════►  5  │ ● ● ● ● ● │ ● ● ● ● ●              │  │  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │              │  │  │  │  │  │  │  │
   ║   ◄══[NTC2]═════  6  │ ● ● ● ● ● │ ● ● ● ● ●──────► A1   │  │  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │              │  │  │  │  │  │  │  │
   ╠══[2.2kΩ]══════►  7  │ ● ● ● ● ● │ ● ● ● ● ●              │  │  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │              │  │  │  │  │  │  │  │
   ║   ◄══[NTC3]═════  8  │ ● ● ● ● ● │ ● ● ● ● ●──────► A2   │  │  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │              │  │  │  │  │  │  │  │
   ║      ║           9  │ ● ● ● ● ● │ ● ● ● ● ●              Buzzer  │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               (+)    │  │  │  │  │  │
   ║  ◄═══[MAP]═════► 10  │ ● ● ● ● ● │ ● ● ● ● ●──────► A3    │      │  │  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │      │  │  │  │  │  │
   ║      ║          11  │ ● ● ● ● ● │ ● ● ● ● ●               │   MAX31855  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │    (CS)    │  │  │  │
   ║  ◄══[MAX]══════► 12  │ ● ● ● ● ● │ ● ● ● ● ●               │      │  TFT  │  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │      │ (RST) │  │  │  │
   ║      ║          13  │ ● ● ● ● ● │ ● ● ● ● ●               │      │   │ TFT  │  │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │      │   │(DC) │  │  │
   ║      ║          14  │ ● ● ● ● ● │ ● ● ● ● ●               │      │   │  │TFT │  │
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │      │   │  │(CS)│  │
   ║      ║          15  │ ● ● ● ● ● │ ● ● ● ● ●               │      │   │  │  │Touch│
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │      │   │  │  │(CS) │
   ║      ║          16  │ ● ● ● ● ● │ ● ● ● ● ●               │      │   │  │  │  │Touch
   ║      ║              │ │ │ │ │ │ │ │ │ │ │ │               │      │   │  │  │  │ IRQ
   ║      ║              └───────────┴─────────┘               
   ║      ║                                                      
   ║      ╠═════════════════════════════════════════════► GND (multiple)
   ╚══════╩═════════════════════════════════════════════► 5V (multiple)

                                    SPI Bus (Shared)
                                    ────────────────
                                    Pin 50 (MISO) ──────► MAX31855, TFT
                                    Pin 51 (MOSI) ──────► TFT
                                    Pin 52 (SCK) ───────► MAX31855, TFT
```

---

## 📝 Step-by-Step Assembly

### Step 1: Set Up Power Rails

1. Connect **5V pin** from Arduino to **red (+) power rail** on breadboard
2. Connect **GND pin** from Arduino to **blue (-) power rail** on breadboard
3. Use jumper wires to bridge power rails on both sides if needed

### Step 2: Install NTC Thermistor Circuits

For **each** of the 3 thermistors:

1. Insert **2.2kΩ resistor** with one end in **5V rail**
2. Insert **NTC thermistor** with one end in **GND rail**
3. Connect the **other ends together** in a breadboard row
4. Connect a jumper from this junction to Arduino analog pin:
   - IAT Pre-IC: **A0**
   - IAT Post-IC: **A1**
   - Coolant: **A2**

### Step 3: Install MAP Sensor

1. Connect **red wire** (VCC) to **5V rail**
2. Connect **black wire** (GND) to **GND rail**
3. Connect **yellow wire** (signal) to Arduino **A3**

### Step 4: Install MAX31855 Module

1. Connect **VCC** to **5V rail**
2. Connect **GND** to **GND rail**
3. Connect **CS** to Arduino pin **7**
4. Connect **SO (MISO)** to Arduino pin **50**
5. Connect **SCK** to Arduino pin **52**
6. Attach **K-type thermocouple** to screw terminals (red = +, yellow = -)

### Step 5: Install TFT Display

**If using 2.8" Shield (#1651):**
- Simply plug it on top of Arduino Mega (it's a shield!)
- **DONE!** Skip to Step 6.

**If using 3.5" Breakout (#2050):**

1. **Power:**
   - VIN → 5V rail
   - GND → GND rail
   - LITE → 5V rail (backlight)

2. **TFT Control:**
   - TFT_CS → Pin 10
   - TFT_DC → Pin 9
   - TFT_RST → Pin 8

3. **SPI (shared with MAX31855):**
   - MOSI → Pin 51
   - MISO → Pin 50
   - SCK → Pin 52

4. **Touch:**
   - TOUCH_CS → Pin 11
   - TOUCH_IRQ → Pin 12 (optional)

### Step 6: Install Buzzer

1. Connect **positive (+)** to Arduino pin **6**
2. Connect **negative (-)** to **GND rail**

### Step 7: Add Filtering Capacitors (Recommended)

Place a **0.1µF capacitor** across power and ground near:
- MAX31855 module
- MAP sensor
- Each NTC circuit

This reduces noise and improves readings.

---

## 🔍 Testing Procedure

### 1. Visual Inspection

Before powering on:

- [ ] Check all connections match the tables above
- [ ] Verify no short circuits (use multimeter continuity mode)
- [ ] Ensure correct polarity on all components
- [ ] Check SPI pins are correct (50, 51, 52)
- [ ] Verify all GND connections go to GND rail
- [ ] Verify all 5V connections go to 5V rail

### 2. Power-On Test

1. **Connect USB cable** (don't upload code yet)
2. Check that:
   - Arduino LED lights up
   - TFT display backlight turns on (if connected)
   - No components get hot
   - No smoke or burning smell!

3. **Measure voltages** with multimeter:
   - 5V rail should read ~4.8-5.2V
   - Each NTC junction should read 1.5-3.5V (room temp)
   - MAP sensor output should read ~0.5V (atmospheric pressure)

### 3. Upload Test Code

Use the examples from:
- **File → Examples → Adafruit GFX → graphicstest** (test display)
- **File → Examples → Adafruit MAX31855 → serialthermocouple** (test EGT)

### 4. Upload Main Code

Once individual components work, upload the full **jnkr-gauge.ino** sketch.

---

## ⚠️ Troubleshooting

### Display doesn't light up
- Check 5V and GND connections
- Verify backlight (LITE) is connected to 5V
- Try adjusting brightness in code

### EGT reads 0°C or error
- Check thermocouple polarity (red = +)
- Verify MAX31855 wiring (especially CS pin)
- Check SPI connections (50, 52)
- Test with a lighter (should read >100°C immediately)

### NTC temps read wrong
- Verify 2.2kΩ resistor value with multimeter
- Check voltage divider connections
- Confirm NTC is 2.2kΩ @ 25°C type
- Measure junction voltage (should be ~2.5V at room temp)

### MAP sensor reads wrong
- Should read ~0.5V at sea level (atmospheric)
- Check 5V power supply
- Verify analog pin connection (A3)

### Display is garbled
- Check all SPI connections (50, 51, 52)
- Verify CS pin assignments match code
- Try swapping MOSI/MISO if still wrong

---

## 📷 Photo Documentation Tips

Take photos during assembly:

1. **Overall layout** - bird's eye view of breadboard
2. **Power connections** - closeup of 5V and GND rails
3. **NTC circuits** - show resistor/thermistor junctions
4. **SPI connections** - highlight shared pins 50, 51, 52
5. **Sensor connections** - each sensor's wiring

This helps troubleshooting and final PCB design!

---

## 🎯 Next Steps

Once breadboard prototype works:

1. ✅ **Document working connections** (take photos!)
2. ✅ **Test all sensors** with real-world conditions
3. ✅ **Verify alert thresholds** trigger correctly
4. ✅ **Check display updates** smoothly
5. ✅ **Consider PCB design** for permanent installation

See: [PCB_SCHEMATIC.md](PCB_SCHEMATIC.md) for shield PCB design.

---

## 📦 Wire Management Tips

Keep your breadboard tidy:

- **Color coding:**
  - Red = 5V power
  - Black = Ground
  - Orange = Chip selects
  - Blue/Green/Purple = SPI bus
  - Yellow = Analog signals

- **Use pre-cut jumpers** for cleaner layout
- **Group related connections** (all sensors together)
- **Keep wires flat** on breadboard surface
- **Label sensors** with masking tape

---

**Ready to build? Order parts from [COMPLETE_BOM.md](COMPLETE_BOM.md) first!**

---

**Last Updated:** 2025-11-08
