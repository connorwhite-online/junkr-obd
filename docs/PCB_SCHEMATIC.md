# PCB Schematic & Shield Design - JNKR Gauge System

**⚠️ LEGACY DOCUMENT - Arduino Mega Version**

**Note:** This project has been updated to use **ESP32-S3 Qualia** platform instead of Arduino Mega. This document is kept for reference but is no longer the recommended approach.

**For current ESP32-S3 build**, see:
- [COMPLETE_BOM.md](COMPLETE_BOM.md) - ESP32-S3 parts list
- [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md) - ESP32-S3 wiring
- [ESP32_SETUP.md](ESP32_SETUP.md) - ESP32-S3 Arduino IDE setup

---

## 📋 Legacy Arduino Mega Shield Documentation

This document provides the complete schematic and PCB layout for a custom Arduino Mega shield that simplifies installation. **This is no longer recommended** as the ESP32-S3 Qualia board provides better performance, easier setup, and lower cost.

---

## 📋 Overview

### What is This Shield?

A custom **Arduino Mega shield** PCB that:
- ✅ Plugs directly onto Arduino Mega 2560
- ✅ Provides screw terminals for all sensor connections
- ✅ Includes voltage divider circuits for NTC thermistors
- ✅ Has socket for MAX31855 module
- ✅ Includes power regulation (12V→5V)
- ✅ Adds protection circuitry and filtering
- ✅ Supports TFT display connection
- ✅ Includes buzzer driver

---

## 🔌 Complete Circuit Schematic

### Full System Schematic

```
                                    JNKR GAUGE SHIELD v1.0
                           ┌─────────────────────────────────────────┐
                           │                                         │
    12V Vehicle ────►──┬───┤ DC-DC Buck Converter (LM2596)          │
    Electrical         │   │     12V → 5V @ 3A                       │
                      ┌┴─┐ │  ┌────┐                                 │
                      │F │ │  │Buck│                                 │
                      │u │ │  │Conv│──┬──► 5V Rail                   │
                      │s │ │  └────┘  │                              │
                      │e │ │          ├──► Arduino 5V Pin            │
                      └┬─┘ │          │                              │
                       │   │          │   ┌───────────────────┐      │
    GND ────────────────┴───┤──────────┴──►│ 5V Power Rail     │      │
                           │              └───────────────────┘      │
                           │                                         │
    ┌──────────────────────┼─────────────────────────────────────────┤
    │  TEMPERATURE SENSORS  │                                         │
    ├──────────────────────┼─────────────────────────────────────────┤
    │                      │                                         │
    │  IAT Pre-IC:         │         5V                              │
    │                      │          │                              │
    │  [Screw Terminal]    │          ├──[2.2kΩ]──┐                  │
    │      ┌────┬────┐     │          │           │                  │
    │      │ 1  │ 2  │─────┼──[NTC]───┴───────────┼──► A0 (Arduino)  │
    │      └────┴────┘     │          │           │                  │
    │                      │         GND          [C1]               │
    │                      │                      0.1µF              │
    │                      │                       │                 │
    │  IAT Post-IC:        │         5V           GND                │
    │                      │          │                              │
    │  [Screw Terminal]    │          ├──[2.2kΩ]──┐                  │
    │      ┌────┬────┐     │          │           │                  │
    │      │ 1  │ 2  │─────┼──[NTC]───┴───────────┼──► A1 (Arduino)  │
    │      └────┴────┘     │          │           │                  │
    │                      │         GND          [C2]               │
    │                      │                      0.1µF              │
    │                      │                       │                 │
    │  Coolant Temp:       │         5V           GND                │
    │                      │          │                              │
    │  [Screw Terminal]    │          ├──[2.2kΩ]──┐                  │
    │      ┌────┬────┐     │          │           │                  │
    │      │ 1  │ 2  │─────┼──[NTC]───┴───────────┼──► A2 (Arduino)  │
    │      └────┴────┘     │          │           │                  │
    │                      │         GND          [C3]               │
    │                      │                      0.1µF              │
    │                      │                       │                 │
    └──────────────────────┼─────────────────────────────────────────┤
                           │                                         │
    ┌──────────────────────┼─────────────────────────────────────────┤
    │  PRESSURE SENSOR      │                                         │
    ├──────────────────────┼─────────────────────────────────────────┤
    │                      │                                         │
    │  MAP Sensor (Boost): │         5V                              │
    │                      │          │                              │
    │  [Screw Terminal]    │          ├──────────────► VCC            │
    │      ┌───┬───┬───┐   │          │                              │
    │      │ + │GND│OUT│   │          │         ┌─────► A3 (Arduino) │
    │      └─┬─┴─┬─┴─┬─┘   │          │         │                    │
    │        │   │   │     │          │        [C4]                  │
    │        │   │   └─────┼──────────┼────┐   0.1µF                 │
    │        │   │         │          │    │    │                    │
    │        │   └─────────┼──────────┼────┼────┴─► GND              │
    │        │             │          │    │                         │
    │        └─────────────┼──────────┘    └─[R4]─► GND (Optional)   │
    │                      │                   10kΩ (pull-down)      │
    └──────────────────────┼─────────────────────────────────────────┤
                           │                                         │
    ┌──────────────────────┼─────────────────────────────────────────┤
    │  EGT SENSOR           │                                         │
    ├──────────────────────┼─────────────────────────────────────────┤
    │                      │                                         │
    │  MAX31855 Socket:    │         5V                              │
    │                      │          │                              │
    │  ┌────────────────┐  │          ├──────────► VCC               │
    │  │ MAX31855 Module│  │          │                              │
    │  │  ┌──┬──┬──┬──┐ │  │         ┌┴┐                             │
    │  │  │+ │- │  │  │ │  │  [K-Thermo] Red(+), Yellow(-)           │
    │  │  └──┴──┴──┴──┘ │  │         └┬┘                             │
    │  │  │  │  │  │  │ │  │          │                              │
    │  │  V  G  S  C  S │  │          └──────────► GND               │
    │  │  C  N  O  S  C │  │                                         │
    │  │  C  D     K     │  │          ├──────────► Pin 7 (CS)       │
    │  │  │  │  │  │  │ │  │          ├──────────► Pin 50 (MISO)    │
    │  │  │  │  │  │  │ │  │          └──────────► Pin 52 (SCK)     │
    │  └──┴──┴──┴──┴──┴─┘  │                                         │
    │                      │          [C5] 0.1µF between VCC-GND     │
    │                      │                                         │
    └──────────────────────┼─────────────────────────────────────────┤
                           │                                         │
    ┌──────────────────────┼─────────────────────────────────────────┤
    │  BUZZER               │                                         │
    ├──────────────────────┼─────────────────────────────────────────┤
    │                      │                                         │
    │  [Screw Terminal]    │       Pin 6 (PWM)                       │
    │      ┌────┬────┐     │          │                              │
    │      │ +  │ -  │     │          ├─[R5]──┬──► Buzzer (+)        │
    │      └────┴────┘     │          │ 220Ω  │                      │
    │                      │          │      [Q1]                    │
    │                      │          │      NPN                     │
    │                      │          │    Transistor                │
    │                      │          │       │                      │
    │                      │         GND◄─────┴──── Buzzer (-)       │
    │                      │                                         │
    │  (Optional: Add LED indicator in parallel)                    │
    │                      │                                         │
    └──────────────────────┼─────────────────────────────────────────┤
                           │                                         │
    ┌──────────────────────┼─────────────────────────────────────────┤
    │  TFT DISPLAY          │                                         │
    ├──────────────────────┼─────────────────────────────────────────┤
    │                      │                                         │
    │  [10-Pin Header]     │         5V ──────────► VIN              │
    │   for TFT Breakout   │        GND ──────────► GND              │
    │                      │      Pin 10 ──────────► TFT_CS          │
    │     ┌──┬──┬──┬──┐    │      Pin 9  ──────────► TFT_DC          │
    │     │V │G │T │T │    │      Pin 8  ──────────► TFT_RST         │
    │     │I │N │F │F │    │      Pin 51 ──────────► MOSI            │
    │     │N │D │T │T │    │      Pin 50 ──────────► MISO            │
    │     │  │  │_ │_ │    │      Pin 52 ──────────► SCK             │
    │     │  │  │C │D │    │      Pin 11 ──────────► TOUCH_CS        │
    │     │  │  │S │C │    │      Pin 12 ──────────► TOUCH_IRQ       │
    │     └──┴──┴──┴──┘    │         5V ──────────► LITE (backlight) │
    │                      │                                         │
    │  (TFT connects via ribbon cable or stacking headers)          │
    │                      │                                         │
    └──────────────────────┼─────────────────────────────────────────┤
                           │                                         │
    ┌──────────────────────┼─────────────────────────────────────────┤
    │  STATUS INDICATORS    │                                         │
    ├──────────────────────┼─────────────────────────────────────────┤
    │                      │                                         │
    │  Power LED (Green):  │      5V ──[R6]──┬──[LED1]──► GND        │
    │                      │               1kΩ   (Green)             │
    │                      │                                         │
    │  Status LED (Blue):  │   Pin 13 ──[R7]──┬──[LED2]──► GND       │
    │                      │               1kΩ   (Blue)              │
    │                      │                                         │
    └──────────────────────┴─────────────────────────────────────────┘


    PROTECTION & FILTERING:
    ═══════════════════════
    
    • Reverse polarity protection diode (1N4007) on 12V input
    • 1000µF bulk capacitor after buck converter
    • 0.1µF ceramic capacitors on all sensor power pins
    • 10µF electrolytic capacitor on 5V rail
    • TVS diode (5.1V) on 5V rail for transient protection
```

---

## 🎨 PCB Layout (Top View)

```
┌───────────────────────────────────────────────────────────────────┐
│                    JNKR GAUGE SHIELD v1.0                         │
│                   Arduino Mega 2560 Shield                        │
├───────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌─────────────────────┐         ┌──────────────────────┐        │
│  │   12V INPUT         │         │  SENSOR TERMINALS     │        │
│  │   ┌────────────┐    │         │                       │        │
│  │   │ 12V+  GND  │    │         │  ┌──────────────┐    │        │
│  │   └─┬────┬─────┘    │         │  │ IAT Pre      │    │        │
│  │     │    │          │         │  │ 1:[●]  2:[●] │    │        │
│  │   [Fuse]│          │         │  └──────────────┘    │        │
│  │     │    │          │         │                       │        │
│  │   ┌─▼────▼───────┐  │         │  ┌──────────────┐    │        │
│  │   │ LM2596 Buck  │  │         │  │ IAT Post     │    │        │
│  │   │ Converter    │  │         │  │ 1:[●]  2:[●] │    │        │
│  │   │ 12V → 5V/3A  │  │         │  └──────────────┘    │        │
│  │   └──────────────┘  │         │                       │        │
│  │        │             │         │  ┌──────────────┐    │        │
│  │       5V            │         │  │ Coolant Temp │    │        │
│  └────────┼─────────────┘         │  │ 1:[●]  2:[●] │    │        │
│           │                       │  └──────────────┘    │        │
│           │                       │                       │        │
│     ┌─────▼──────────────┐        │  ┌──────────────────┐│        │
│     │  5V POWER RAIL     │        │  │ MAP Sensor       ││        │
│     │  (Copper Trace)    │        │  │ +:[●] G:[●] O:[●]││        │
│     └────────────────────┘        │  └──────────────────┘│        │
│                                   │                       │        │
│                                   │  ┌──────────────┐    │        │
│  ┌──────────────────┐             │  │ Buzzer       │    │        │
│  │  VOLTAGE         │             │  │ +:[●]  -:[●] │    │        │
│  │  DIVIDERS        │             │  └──────────────┘    │        │
│  │                  │             └───────────────────────┘        │
│  │  [R1] [R2] [R3]  │                                              │
│  │  2.2k 2.2k 2.2k  │             ┌──────────────────────┐         │
│  │   │    │    │    │             │  MAX31855 SOCKET     │         │
│  │  [N1] [N2] [N3]  │             │                       │         │
│  │  NTC  NTC  NTC   │             │   ┌────────────────┐ │         │
│  │   │    │    │    │             │   │ [●]  [●]       │ │         │
│  │  [C1] [C2] [C3]  │             │   │  +    -        │ │         │
│  │  0.1µ 0.1µ 0.1µ  │             │   │ (Thermocouple) │ │         │
│  │   │    │    │    │             │   └────────────────┘ │         │
│  │   ▼    ▼    ▼    │             │                       │         │
│  │   A0   A1   A2   │             │   Socket for MAX31855 │         │
│  └───┬────┬────┬────┘             │   ┌─┬─┬─┬─┬─┐         │         │
│      │    │    │                  │   │V│G│S│C│S│         │         │
│      │    │    │                  │   │C│N│O│S│C│         │         │
│      │    │    │                  │   │C│D│ │K│ │         │         │
│      │    │    │                  │   └─┴─┴─┴─┴─┘         │         │
│      │    │    │                  └───────┬───┬───────────┘         │
│      │    │    │                          │   │                     │
│      │    │    │                          ▼   ▼                     │
│  ┌───┴────┴────┴────┐                    P7  P50/52                │
│  │                   │                                              │
│  │  ┌──────────────────────────────────────────────┐               │
│  │  │         TFT DISPLAY HEADER (10-pin)          │               │
│  │  │  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐  │               │
│  │  │  │VIN│GND│CS │DC │RST│MOS│MIS│SCK│TCS│TIR│  │               │
│  │  │  │   │   │ 10│ 9 │ 8 │ 51│ 50│ 52│ 11│ 12│  │               │
│  │  │  └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘  │               │
│  │  └──────────────────────────────────────────────┘               │
│  │                                                                  │
│  │  ┌──────────────────────────────────────────────┐               │
│  │  │      ARDUINO MEGA HEADERS (Female)           │               │
│  │  │  ┌───────────────────────────────────────┐   │               │
│  │  │  │ Digital 0-13, Analog A0-A15, Power    │   │               │
│  │  │  │ 5V, 3.3V, GND, VIN, etc.              │   │               │
│  │  │  └───────────────────────────────────────┘   │               │
│  │  └──────────────────────────────────────────────┘               │
│  │          (Shield plugs onto Arduino Mega)                       │
│  └──────────────────────────────────────────────────────────────┬──┘
│                                                                  │
│  ┌──────────────────────┐         ┌───────────────────────┐    │
│  │   STATUS LEDs         │         │  BUZZER DRIVER        │    │
│  │                       │         │                       │    │
│  │   ● Power (Green)     │         │      Pin 6            │    │
│  │   ● Status (Blue)     │         │       │               │    │
│  │                       │         │      [R5]             │    │
│  └───────────────────────┘         │      220Ω             │    │
│                                    │       │               │    │
│         JNKR v1.0                  │      [Q1]  NPN        │    │
│         2025-11-08                 │       │   2N2222      │    │
│                                    │       └───► Buzzer    │    │
│                                    └───────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📏 PCB Specifications

### Board Dimensions

| Parameter | Specification |
|-----------|--------------|
| **Board Size** | 101.6 x 53.34 mm (Arduino Mega shield footprint) |
| **Thickness** | 1.6 mm |
| **Layers** | 2-layer (Top + Bottom) |
| **Copper Weight** | 1 oz (35 µm) |
| **Surface Finish** | HASL (lead-free) or ENIG |
| **Solder Mask** | Green (or your choice) |
| **Silkscreen** | White text on both sides |

### Component Specifications

| Component | Footprint | Quantity | Notes |
|-----------|-----------|----------|-------|
| **Female Headers** | 2.54mm pitch | 4 sets | For Arduino Mega |
| **Screw Terminals** | 5mm pitch, 2-pos | 5 | Sensors + buzzer |
| **Buck Converter** | LM2596 module | 1 | Through-hole mount |
| **Resistors** | 1/4W axial | 7 | 2.2kΩ (3x), 220Ω, 1kΩ (3x) |
| **Capacitors (ceramic)** | 5mm lead spacing | 5 | 0.1µF (5x) |
| **Capacitors (electrolytic)** | 5mm lead spacing | 2 | 1000µF, 10µF |
| **Transistor** | TO-92 | 1 | 2N2222 NPN |
| **Diode** | DO-41 | 2 | 1N4007, TVS 5.1V |
| **LEDs** | 5mm | 2 | Green, Blue |
| **MAX31855 Socket** | 2.54mm header | 1 | 5-pin female |

---

## 🔧 Component Placement Guide

### Top Side Components

```
Priority Placement Order:
1. Female headers (must align with Arduino Mega)
2. Screw terminals (front edge for easy access)
3. Buck converter module (heat management area)
4. Resistor networks (near analog pins)
5. Capacitors (close to power pins)
6. LEDs (visible on front)
7. TFT display header (side or top)
```

### Bottom Side

```
• Ground plane (copper pour)
• Power traces (thick, 50+ mil width for 3A)
• Signal traces (keep away from power lines)
• Label silkscreen (component names, pin functions)
```

---

## 📊 Bill of Materials (Shield PCB)

### PCB Manufacturing

| Item | Specification | Qty | Cost | Source |
|------|--------------|-----|------|--------|
| **Custom PCB** | 2-layer, 101.6x53.34mm | 5 pcs | $15-25 | PCBWay, JLCPCB |
| **Shipping** | DHL/FedEx | - | $10-20 | - |

### Components for Shield

| Component | Value/Type | Qty | Cost | Link |
|-----------|-----------|-----|------|------|
| **Female Headers** | 2.54mm, stackable | 4 sets | $8.99 | [Amazon](https://www.amazon.com/dp/B07C89T1X7) |
| **Screw Terminals** | 5mm, 2-pos | 5 | $6.99 (10pcs) | [Amazon](https://www.amazon.com/dp/B07C7VTLFH) |
| **LM2596 Buck Module** | 3A adjustable | 1 | $8.99 | [Amazon](https://www.amazon.com/dp/B08CDMXGFX) |
| **Resistors (2.2kΩ)** | 1/4W | 3 | $0.10 ea | [Amazon Kit](https://www.amazon.com/dp/B08FD1XVL6) |
| **Resistors (220Ω)** | 1/4W | 1 | $0.10 | [Amazon Kit](https://www.amazon.com/dp/B08FD1XVL6) |
| **Resistors (1kΩ)** | 1/4W | 3 | $0.10 ea | [Amazon Kit](https://www.amazon.com/dp/B08FD1XVL6) |
| **Capacitors (0.1µF)** | Ceramic, 50V | 5 | $0.15 ea | [Amazon](https://www.amazon.com/dp/B07PBQXQNQ) |
| **Capacitor (1000µF)** | Electrolytic, 25V | 1 | $0.50 | [Amazon](https://www.amazon.com/dp/B07PGY3FS1) |
| **Capacitor (10µF)** | Electrolytic, 25V | 1 | $0.30 | [Amazon](https://www.amazon.com/dp/B07PGY3FS1) |
| **Transistor (2N2222)** | NPN, TO-92 | 1 | $0.15 | [Amazon](https://www.amazon.com/dp/B06XHKSVL9) |
| **Diode (1N4007)** | 1A rectifier | 1 | $0.10 | [Amazon](https://www.amazon.com/dp/B079KDYBQ8) |
| **TVS Diode** | 5.1V, 600W | 1 | $0.50 | [Digi-Key](https://www.digikey.com/en/products/detail/bourns-inc/SMBJ5-0A/1532716) |
| **LED (Green)** | 5mm, clear | 1 | $0.25 | [Amazon](https://www.amazon.com/dp/B077XGF3YR) |
| **LED (Blue)** | 5mm, clear | 1 | $0.25 | [Amazon](https://www.amazon.com/dp/B077XGF3YR) |
| **Fuse Holder** | 5x20mm, PCB mount | 1 | $1.50 | [Amazon](https://www.amazon.com/dp/B07L3QNM8R) |
| **Fuse** | 3A, fast-blow | 2 | $0.50 ea | [Amazon](https://www.amazon.com/dp/B07L9QXJJ4) |

**Shield Total Cost:** ~$45-60 (including PCB manufacturing)

---

## 🛠️ Assembly Instructions

### Step 1: PCB Order

1. **Download Gerber files** (provided separately or design in KiCad)
2. **Upload to PCBWay/JLCPCB:**
   - Select 2-layer
   - 1.6mm thickness
   - HASL finish
   - Quantity: 5 (minimum order)
3. **Wait 5-10 days** for manufacturing + shipping

### Step 2: Gather Components

Order all components from BOM above.

### Step 3: Solder Bottom Side First

1. **Female headers** for Arduino Mega (critical alignment!)
2. **Through-hole components** on bottom
3. **Inspect joints** with magnifier

### Step 4: Solder Top Side

1. **Screw terminals** (front edge)
2. **Resistors** (flat against board)
3. **Capacitors** (observe polarity!)
4. **Buck converter** (use headers for removable mount)
5. **LEDs** (long leg = anode/+)
6. **Transistor** (match pinout to silkscreen)
7. **Diodes** (stripe = cathode)

### Step 5: Testing

1. **Visual inspection** (no solder bridges)
2. **Continuity test** (multimeter)
3. **Power test:**
   - Apply 12V to input
   - Measure 5V at output
   - Check no shorts
4. **Arduino test:**
   - Plug onto Mega (no power yet!)
   - Apply USB power
   - Check 5V rail
5. **Sensor test:**
   - Connect one sensor at a time
   - Upload test sketch
   - Verify readings

---

## 📐 Gerber File Generation

### Using KiCad (Free PCB Software)

1. **Install KiCad:** [kicad.org](https://www.kicad.org/)
2. **Create New Project:** jnkr-gauge-shield
3. **Schematic Entry:**
   - Draw schematic from diagram above
   - Assign footprints to all components
4. **PCB Layout:**
   - Import netlist from schematic
   - Place components
   - Route traces (use 50 mil for power, 15 mil for signals)
   - Add ground plane
5. **Generate Gerbers:**
   - File → Plot
   - Select all layers
   - Generate drill files
   - ZIP all gerber files
6. **Order PCB:**
   - Upload ZIP to PCBWay/JLCPCB
   - Review and order

**Alternative:** Contact me for pre-designed Gerber files (if available).

---

## 🎯 Design Considerations

### Power Delivery

- **12V Input:** Automotive supply (9-16V range)
- **Buck Converter:** LM2596 module (efficient, 3A capacity)
- **5V Rail:** Feeds Arduino, sensors, display
- **Current Budget:**
  - Arduino Mega: 100-200 mA
  - TFT Display: 150-300 mA
  - Sensors: 50 mA total
  - MAX31855: 5 mA
  - **Total: ~600 mA typical, 2A max**

### Signal Integrity

- **Analog Inputs:** Keep away from digital switching lines
- **SPI Bus:** Use ground plane underneath, short traces
- **Power Filtering:** 0.1µF cap at each IC/sensor
- **Grounding:** Star ground topology, single-point ground to Arduino

### Thermal Management

- **Buck Converter:** Heatsink area on PCB
- **Power Resistors:** Use 1/2W if high current expected
- **Ventilation:** Ensure enclosure has air flow

### Protection

- **Reverse Polarity:** Diode on 12V input
- **Overvoltage:** TVS diode on 5V rail
- **ESD:** Keep traces away from exposed metal
- **Fusing:** 3A fast-blow fuse on input

---

## ⚠️ Important Notes

### Critical Measurements

Before ordering PCB, **verify these dimensions match Arduino Mega:**

- **Mounting holes:** 4x M3, 50.8mm x 15.24mm spacing
- **Header positions:** Use official Arduino Mega dimensions
- **Shield clearance:** Allow 8mm height under shield

### Component Orientation

- **Electrolytic caps:** Watch polarity (stripe = negative)
- **LEDs:** Long leg = anode (+)
- **Transistor:** Flat side matches silkscreen
- **Diodes:** Stripe = cathode

### Testing Checklist

Before first power-on:

- [ ] Visual inspection (no solder bridges)
- [ ] Continuity: 5V rail to GND (should be open)
- [ ] Continuity: All GND connections together
- [ ] Polarity: Buck converter input correct
- [ ] Component values: Resistors match schematic
- [ ] Alignment: Shield plugs onto Mega correctly

---

## 📦 Files Provided

(These would be separate downloads/attachments)

- **Schematic PDF:** `jnkr-shield-schematic.pdf`
- **PCB Layout PDF:** `jnkr-shield-pcb.pdf`
- **Gerber Files (ZIP):** `jnkr-shield-gerbers.zip`
- **KiCad Project:** `jnkr-shield.kicad_pro`
- **Bill of Materials (CSV):** `jnkr-shield-bom.csv`
- **Assembly Drawing:** `jnkr-shield-assembly.pdf`

---

## 🔗 Design Resources

### PCB Manufacturers

- **PCBWay:** [pcbway.com](https://www.pcbway.com) - Good quality, fast
- **JLCPCB:** [jlcpcb.com](https://jlcpcb.com) - Cheapest, basic quality
- **OSH Park:** [oshpark.com](https://oshpark.com) - Purple PCBs, USA-based

### Design Software

- **KiCad:** [kicad.org](https://www.kicad.org/) - Free, open-source
- **EasyEDA:** [easyeda.com](https://easyeda.com/) - Free, web-based
- **Fritzing:** [fritzing.org](https://fritzing.org/) - Beginner-friendly

### Component Sources

- **Digi-Key:** [digikey.com](https://www.digikey.com/) - Fast, reliable
- **Mouser:** [mouser.com](https://www.mouser.com/) - Large selection
- **Amazon:** Quick shipping, slightly more expensive

---

## 💡 Alternative: Breadboard-to-PCB Services

Don't want to design a PCB yourself?

### Option 1: Prototype Shield

Use a **prototyping shield** for Arduino Mega:
- [Amazon - Prototype Shield](https://www.amazon.com/dp/B07WNKN1V2) - $9.99
- Hand-solder components on prototype grid
- Same functionality, less professional appearance

### Option 2: Custom PCB Services

Some companies will design the PCB for you:
- **PCBWay PCB Design:** $30-100 for simple designs
- **Freelance PCB Designers:** Fiverr, Upwork ($50-200)

---

## 🎯 Next Steps

1. ✅ Decide: Breadboard, Prototype Shield, or Custom PCB
2. ✅ If custom PCB: Learn KiCad basics or hire designer
3. ✅ Order PCB (5-10 days manufacturing)
4. ✅ Order components (see BOM)
5. ✅ Assemble shield
6. ✅ Test thoroughly before vehicle installation
7. ✅ Install in vehicle (see WIRING.md for automotive connections)

---

**For breadboard prototyping, see: [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md)**

**For complete parts list, see: [COMPLETE_BOM.md](COMPLETE_BOM.md)**

---

**Last Updated:** 2025-11-08
