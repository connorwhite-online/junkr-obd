# I2C Sensor Consolidation Module - Design Document

**Consolidate all engine bay sensors to a single I2C interface module**

This document describes a solution for connecting all sensors to a module in the engine bay, then running a single unified harness with a single connector through the firewall to the ESP32/Display.

---

## 📋 Overview

### Current Setup (Problem)

Right now, you need to run **multiple individual wires** through the firewall:
- 3x NTC thermistor signal wires (IAT Pre, IAT Post, Coolant)
- 1x MAP sensor signal wire
- 1x MAX31855 thermocouple module wires (3-4 wires)
- Power and ground for each sensor
- **Total: ~15-20 wires through firewall**

### Proposed Solution

**Single I2C Sensor Module in Engine Bay:**
- All sensors connect to module in engine bay
- Module converts all analog signals to digital I2C
- Only **4-6 wires** through firewall (5V, GND, SDA, SCL, optional: 12V, GND)
- Single weatherproof connector at firewall
- Professional automotive-style installation

---

## 🎯 Benefits

### Installation
- ✅ **Single connector** through firewall (professional appearance)
- ✅ Cleaner wire routing
- ✅ Easier to remove display/gauge for service
- ✅ Simpler troubleshooting (disconnect one connector)

### Reliability
- ✅ Digital I2C signal less susceptible to noise than analog
- ✅ Longer wire runs possible (up to 10+ feet)
- ✅ Engine bay harness stays in engine bay
- ✅ Cabin harness stays in cabin

### Serviceability
- ✅ Replace sensors without disturbing firewall harness
- ✅ Upgrade/add sensors easily
- ✅ Test module independently
- ✅ Automotive-style connectors (Deutsch, Molex, etc.)

### Cost
- ⚠️ **Additional cost: $35-80** depending on implementation
- Saves time during installation (worth it!)

---

## 🔧 Recommended Implementation

### Option 1: Pre-Built Modules (Easiest)

**Hardware Required:**

| Component | Description | Quantity | Cost | Source |
|-----------|-------------|----------|------|--------|
| ADS1115 Module | 16-bit I2C ADC, 4 channels | 1 | $8-12 | Adafruit #1085, Amazon |
| MCP9600 Module | I2C K-type thermocouple amplifier | 1 | $12-18 | Adafruit #4101 |
| Weatherproof Box | Small IP67 enclosure (100x68x50mm) | 1 | $8-12 | Amazon |
| Deutsch DT Connector | 6-pin connector set | 1 | $12-18 | Amazon |
| Screw Terminals | 2-pin, 5.08mm pitch | 8 | $4-6 | Amazon |
| Protoboard | Perforated PCB for mounting | 1 | $3-5 | Amazon |
| Wire & Connectors | Automotive wire, heat shrink | - | $10-15 | Amazon |

**Total Cost: ~$57-86**

### Option 2: Custom PCB (Most Professional)

**Custom Shield PCB with integrated:**
- ADS1115 ADC (or equivalent)
- MCP9600 thermocouple amplifier (or MAX31855)
- Voltage divider resistors for NTC thermistors
- Screw terminals or automotive connectors
- Mounting holes for enclosure
- Protection circuitry (TVS diodes, filtering)

**Cost:**
- PCB fabrication (5 pcs): $15-25
- Components: $20-30
- Assembly: DIY or $30-50 professional
- **Total: $35-105**

### Option 3: Hybrid (Cost-Effective)

**Use existing MAX31855 in module box:**
- Keep existing MAX31855 module (you already have it)
- Add ADS1115 for analog sensors
- Use I2C expander/multiplexer to share I2C bus
- Mount both modules in weatherproof box

**Total Cost: ~$40-55** (only need to add ADS1115 + box)

---

## 📐 Detailed Design - Option 1 (Recommended)

### Components Breakdown

#### 1. ADS1115 - 16-bit I2C ADC Module

**What it does:**
- Converts analog voltage to digital values
- 4 single-ended channels (or 2 differential)
- 16-bit resolution (0-65535 instead of 0-1023)
- I2C interface (only 2 wires + power)
- Programmable gain amplifier (PGA)

**Connections to Sensors:**
- **Channel 0 (A0):** IAT Pre-Intercooler voltage divider
- **Channel 1 (A1):** IAT Post-Intercooler voltage divider
- **Channel 2 (A2):** Coolant temp voltage divider
- **Channel 3 (A3):** MAP sensor signal

**I2C Address:** 0x48 (default, can be changed to 0x49, 0x4A, 0x4B)

**Wiring Inside Module:**
```
ADS1115:
  VDD  ← 5V (from harness)
  GND  ← GND (from harness)
  SCL  → SCL (to harness)
  SDA  → SDA (to harness)
  A0   ← IAT Pre-IC voltage divider
  A1   ← IAT Post-IC voltage divider
  A2   ← Coolant temp voltage divider
  A3   ← MAP sensor signal
```

#### 2. MCP9600 - I2C Thermocouple Amplifier

**What it does:**
- K-type thermocouple cold junction compensation
- I2C interface (shares same bus as ADS1115)
- 16-bit resolution (0.0625°C per LSB)
- Temperature range: -200°C to +1372°C
- Built-in filtering

**Why MCP9600 instead of MAX31855?**
- ✅ I2C interface (MAX31855 is SPI)
- ✅ Multiple devices on same bus
- ✅ Higher accuracy
- ✅ Programmable alert outputs
- ⚠️ Slightly more expensive ($12 vs $8)

**Alternative:** Keep MAX31855 and add SPI-to-I2C bridge (see Option 3)

**I2C Address:** 0x60 (default, can be changed to 0x61-0x67)

**Wiring Inside Module:**
```
MCP9600:
  VDD  ← 5V (from harness)
  GND  ← GND (from harness)
  SCL  → SCL (to harness, shared)
  SDA  → SDA (to harness, shared)
  T+   ← K-type thermocouple (+)
  T-   ← K-type thermocouple (-)
```

#### 3. Voltage Dividers for NTC Thermistors

**Same circuit as before, now on module:**

```
For each thermistor (IAT Pre, IAT Post, Coolant):

    5V
     │
     ▼
  ┌──┴──┐
  │     │ 2.2kΩ Resistor (solder to protoboard)
  └──┬──┘
     ├────────► To ADS1115 Channel (A0, A1, or A2)
     │
  ┌──┴──┐
  │     │ NTC Thermistor (external sensor)
  └──┬──┘
     │
    GND
```

**Components needed:**
- 3x 2.2kΩ resistors (1/4W)
- Solder to protoboard
- Screw terminals for NTC sensor connections

---

### Module Schematic

```
┌────────────────────────────────────────────────────────────┐
│  ENGINE BAY SENSOR MODULE (IP67 Enclosure)                  │
│                                                              │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐    │
│  │  Screw      │    │  Screw      │    │  Screw      │    │
│  │  Terminals  │    │  Terminals  │    │  Terminals  │    │
│  │             │    │             │    │             │    │
│  │ IAT Pre     │    │ IAT Post    │    │ Coolant     │    │
│  │ NTC  NTC    │    │ NTC  NTC    │    │ NTC  NTC    │    │
│  └──┬─────┬───┘    └──┬─────┬───┘    └──┬─────┬───┘    │
│     │ 5V  │GND         │ 5V  │GND         │ 5V  │GND      │
│     │     │            │     │            │     │         │
│     ▼     │            ▼     │            ▼     │         │
│   [2.2k]  │          [2.2k]  │          [2.2k]  │         │
│     │     │            │     │            │     │         │
│     ├─────┼────────────┼─────┼────────────┼─────┤         │
│     │     │            │     │            │     │         │
│     │     │            │     │            │     │         │
│  ┌──▼─────▼────────────▼─────▼────────────▼─────▼──┐      │
│  │            ADS1115 ADC Module                     │      │
│  │  VDD GND SCL SDA  A0  A1  A2  A3                 │      │
│  └───┬───┬───┬───┬────────────────────┬─────────────┘      │
│      │   │   │   │                    │                    │
│      │   │   │   │                    │                    │
│      │   │   │   │                ┌───▼───────┐            │
│      │   │   │   │                │  Screw    │            │
│      │   │   │   │                │  Terminal │            │
│      │   │   │   │                │           │            │
│      │   │   │   │                │ MAP Sensor│            │
│      │   │   │   │                │ 5V  SIG GND           │
│      │   │   │   │                └───────────┘            │
│      │   │   │   │                                         │
│      │   │   │   │  ┌─────────────────────┐               │
│      │   │   │   │  │  MCP9600 Module     │               │
│      │   │   │   │  │  Thermocouple Amp   │               │
│      │   │   │   │  │                     │               │
│      ├───┼───┼───┼──┤ VDD GND SCL SDA     │               │
│      │   │   │   │  │  T+   T-            │               │
│      │   │   │   │  └───┬────┬────────────┘               │
│      │   │   │   │      │    │                             │
│      │   │   │   │  ┌───▼────▼───┐                        │
│      │   │   │   │  │  Screw     │                        │
│      │   │   │   │  │  Terminal  │                        │
│      │   │   │   │  │            │                        │
│      │   │   │   │  │ EGT K-Type │                        │
│      │   │   │   │  │  +     -   │                        │
│      │   │   │   │  └────────────┘                        │
│      │   │   │   │                                         │
│  ┌───▼───▼───▼───▼──────────────────────────┐             │
│  │  6-Pin Deutsch DT Connector               │             │
│  │  (Harness to Cabin/ESP32)                 │             │
│  │                                            │             │
│  │  Pin 1: 5V Power                           │             │
│  │  Pin 2: GND                                │             │
│  │  Pin 3: I2C SCL                            │             │
│  │  Pin 4: I2C SDA                            │             │
│  │  Pin 5: Shield/Drain (optional)            │             │
│  │  Pin 6: NC (spare)                         │             │
│  └────────────────────────────────────────────┘             │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

### Harness Through Firewall

**Engine Bay Side → Firewall → Cabin Side**

```
Engine Bay Module                    Firewall                ESP32
┌───────────────┐                       │              ┌──────────────┐
│ 6-Pin Female  │                       │              │ 6-Pin Male   │
│ Deutsch DT    │                       │              │ Deutsch DT   │
│               │                       │              │              │
│ 1: 5V  ───────┼───[ 4 wire cable ]───┼──────────────┤ 1: 5V        │
│ 2: GND ───────┼───[ 22 AWG       ]───┼──────────────┤ 2: GND       │
│ 3: SCL ───────┼───[ shielded     ]───┼──────────────┤ 3: SCL (Pin 22)
│ 4: SDA ───────┼───[ twisted pair ]───┼──────────────┤ 4: SDA (Pin 21)
│ 5: Shield ────┼───[              ]───┼──────────────┤ 5: GND       │
│ 6: NC   ──────┼───[   ~6 feet    ]───┼──────────────┤ 6: NC        │
│               │                       │              │              │
└───────────────┘                       │              └──────────────┘
```

**Cable Requirements:**
- **4-conductor shielded cable** (22 AWG minimum)
- Twisted pair for I2C (SCL/SDA)
- Separate wires for power (5V/GND)
- Shield connected to GND at one end only
- Length: 6-10 feet typical
- Recommended: Automotive-grade CAN bus cable

**Example Cables:**
- Belden 3086A (22 AWG, 4-conductor, shielded) - $2/ft
- Alpha Wire 5026C (4-conductor, shielded) - $1.50/ft
- Generic CAN bus cable - $1/ft

---

## 🛠️ Build Instructions - Option 1

### Step 1: Gather Components

**Order from Amazon/Adafruit:**
- [ ] ADS1115 ADC module (Adafruit #1085 or generic)
- [ ] MCP9600 thermocouple amplifier (Adafruit #4101)
- [ ] IP67 weatherproof box (100x68x50mm minimum)
- [ ] Deutsch DT 6-pin connector kit (DTM06-6S housing)
- [ ] Protoboard or perfboard (7x5cm)
- [ ] 3x 2.2kΩ resistors (1/4W)
- [ ] 4x 2-position screw terminals (5mm pitch)
- [ ] 4-conductor shielded cable (10 feet)
- [ ] Cable glands for enclosure (2x PG9)
- [ ] Heat shrink tubing
- [ ] Mounting hardware (M3 standoffs, screws)

**Total cost: ~$60-85**

### Step 2: Prepare Enclosure

1. **Drill holes:**
   - 2x cable gland holes (15mm) - one for sensors, one for harness
   - 4x mounting holes (M4 or self-tapping)

2. **Mount protoboard:**
   - Use M3 standoffs to mount board inside box
   - Leave room for wire routing
   - Ensure components won't touch metal (if metal box)

### Step 3: Assemble Modules on Board

**Layout on protoboard:**
```
┌─────────────────────────────────────────┐
│                                         │
│  [Screw Term] [Screw Term] [Screw Term]│ ← NTC inputs
│     IAT Pre      IAT Post     Coolant   │
│                                         │
│    [2.2k]        [2.2k]       [2.2k]   │ ← Reference resistors
│      │             │             │      │
│  ┌───▼─────────────▼─────────────▼───┐ │
│  │        ADS1115 Module              │ │
│  └────────────────────────────────────┘ │
│                                         │
│  [Screw Terminal] ← MAP Sensor         │
│                                         │
│  ┌──────────────────────────────┐      │
│  │    MCP9600 Module            │      │
│  │                              │      │
│  └──────────────────────────────┘      │
│                                         │
│  [Screw Terminal] ← EGT Thermocouple   │
│                                         │
│  [Deutsch Connector] ← Harness to cabin│
│                                         │
└─────────────────────────────────────────┘
```

### Step 4: Wire Modules Together

**Power distribution:**
```
5V (red) ───┬─── ADS1115 VDD
            ├─── MCP9600 VDD
            ├─── MAP sensor 5V
            └─── Voltage divider 5V rail

GND (black) ┬─── ADS1115 GND
            ├─── MCP9600 GND
            ├─── MAP sensor GND
            └─── Voltage divider GND rail
```

**I2C bus:**
```
SCL (yellow) ┬─── ADS1115 SCL
             └─── MCP9600 SCL

SDA (green)  ┬─── ADS1115 SDA
             └─── MCP9600 SDA
```

**Sensor connections:**
```
IAT Pre NTC:
  Wire 1 → 5V (shared with 2.2k resistor)
  Wire 2 → GND (shared with 2.2k resistor)
  Junction → ADS1115 A0

IAT Post NTC:
  Wire 1 → 5V
  Wire 2 → GND
  Junction → ADS1115 A1

Coolant NTC:
  Wire 1 → 5V
  Wire 2 → GND
  Junction → ADS1115 A2

MAP Sensor:
  Pin 1 (5V) → 5V rail
  Pin 2 (SIG) → ADS1115 A3
  Pin 3 (GND) → GND rail

EGT K-Type:
  + (yellow) → MCP9600 T+
  - (red) → MCP9600 T-
```

### Step 5: Wire Deutsch Connector

**Pin assignments:**

| Pin | Wire Color | Function | Connects To |
|-----|------------|----------|-------------|
| 1 | Red | 5V Power | 5V rail (from ESP32) |
| 2 | Black | Ground | GND rail (to ESP32) |
| 3 | Yellow | I2C SCL | SCL bus (to ESP32 Pin 22) |
| 4 | Green | I2C SDA | SDA bus (to ESP32 Pin 21) |
| 5 | Bare/Drain | Shield | GND (one end only) |
| 6 | - | Spare | NC |

**Crimp and assemble:**
1. Strip wires 3-4mm
2. Crimp pins to wires
3. Insert pins into housing (listen for click)
4. Test pull (should not come out)
5. Add strain relief (zip tie or cable clamp)

### Step 6: Test Before Sealing

**Before closing enclosure, test with multimeter:**

1. **Power continuity:**
   - Measure resistance: Harness 5V to module 5V = 0Ω
   - Measure resistance: Harness GND to module GND = 0Ω
   - Measure resistance: 5V to GND = >10kΩ (no short!)

2. **I2C pullups:**
   - Measure voltage: SCL to GND = 3.3-5V (with ESP32 connected)
   - Measure voltage: SDA to GND = 3.3-5V (with ESP32 connected)

3. **Sensor inputs:**
   - Connect test thermistor to IAT Pre terminal
   - Measure voltage at ADS1115 A0 = ~2.5V (for 2.2kΩ @ 25°C)
   - Repeat for other channels

### Step 7: Seal and Mount

1. **Apply conformal coating** (optional but recommended):
   - Spray PCB with conformal coating
   - Let dry 24 hours
   - Protects from moisture and vibration

2. **Cable strain relief:**
   - Use cable glands properly
   - Add silicone sealant around glands
   - Zip tie cables inside box before they exit

3. **Close enclosure:**
   - Check gasket is clean and seated
   - Apply silicone to screw holes (optional)
   - Torque screws evenly

4. **Mount in engine bay:**
   - Choose location: Protected from direct heat/water
   - Good mounting surface
   - Accessible for future service
   - Away from hot exhaust, turbo, radiator
   - Zip tie to existing harness or bracket

---

## 💻 Code Changes Required

### Current Code Structure

Your current code uses:
```cpp
// Analog reads
int adcValue = analogRead(PIN_IAT_PRE_IC);
float voltage = adcToVoltage(adcValue);

// SPI for MAX31855
digitalWrite(PIN_EGT_CS, LOW);
uint32_t data = SPI.transfer(0x00);
```

### New Code with I2C

**Add library:**
```cpp
#include <Wire.h>
#include <Adafruit_ADS1X15.h>  // For ADS1115
#include <Adafruit_MCP9600.h>   // For MCP9600
```

**Initialize in setup():**
```cpp
Wire.begin();  // I2C on pins 21 (SDA), 22 (SCL)

Adafruit_ADS1115 ads;  // Create ADS1115 object
ads.begin(0x48);       // I2C address 0x48
ads.setGain(GAIN_ONE); // +/- 4.096V range

Adafruit_MCP9600 mcp;  // Create MCP9600 object
mcp.begin(0x60);       // I2C address 0x60
mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);
mcp.setThermocoupleType(MCP9600_TYPE_K);
mcp.setFilterCoefficient(3);
```

**Read sensors:**
```cpp
// Read thermistors from ADS1115
int16_t adc0 = ads.readADC_SingleEnded(0);  // IAT Pre
int16_t adc1 = ads.readADC_SingleEnded(1);  // IAT Post
int16_t adc2 = ads.readADC_SingleEnded(2);  // Coolant
int16_t adc3 = ads.readADC_SingleEnded(3);  // MAP sensor

// Convert to voltage
float voltage = ads.computeVolts(adc0);

// Read EGT from MCP9600
float egtTemp = mcp.readThermocouple();
float ambientTemp = mcp.readAmbient();
```

### Modified sensors.cpp Functions

I can update your code files if you'd like me to. The main changes would be:

1. **sensors.cpp:**
   - Replace `analogRead()` with `ads.readADC_SingleEnded()`
   - Replace MAX31855 SPI code with `mcp.readThermocouple()`
   - Update ADC resolution calculations (16-bit vs 10-bit)

2. **config.h:**
   - Remove analog pin definitions (no longer needed)
   - Add I2C addresses
   - Add library configurations

3. **jnkr-gauge.ino:**
   - Add Wire.begin()
   - Initialize I2C sensor modules

Would you like me to modify the code files now?

---

## 🔌 Connector Recommendations

### Option A: Deutsch DT Series (Best)

**Pros:**
- ✅ Industry standard (automotive grade)
- ✅ Waterproof (IP67/IP68)
- ✅ Vibration resistant
- ✅ Easy to assemble with proper tools
- ✅ Available in many pin counts

**Cons:**
- ⚠️ More expensive ($12-20 per connector set)
- ⚠️ Requires special crimping tool ($30-80)

**Part Numbers:**
- DTM06-6S (6-pin receptacle)
- DT06-6S (6-pin plug)
- 0462-201-16141 (16-18 AWG pins)

**Where to buy:**
- Amazon: "Deutsch DT connector 6 pin"
- Waytek: waytek.com
- Genuine Deutsch: te.com

### Option B: Molex Mini-Fit Jr. (Good)

**Pros:**
- ✅ Reliable
- ✅ Easy to crimp
- ✅ Widely available
- ✅ Lower cost ($5-10 per set)

**Cons:**
- ⚠️ Not waterproof (need to seal with heat shrink)
- ⚠️ Less vibration resistant

**Part Numbers:**
- 39-01-2060 (6-circuit housing)
- Terminals: 39-00-0039 (18-24 AWG)

### Option C: Weatherpack (Budget)

**Pros:**
- ✅ Budget-friendly ($6-12)
- ✅ Automotive-style
- ✅ Sealed connectors

**Cons:**
- ⚠️ Mixed quality from Amazon sellers
- ⚠️ Can be difficult to crimp

**Where to buy:**
- Amazon: "weatherpack 6 pin connector"
- Auto parts stores

### Option D: Amphenol C091 (Premium)

**Pros:**
- ✅ Military/aerospace grade
- ✅ Excellent sealing (IP68)
- ✅ Thread-locked connection

**Cons:**
- ⚠️ Most expensive ($25-40 per set)
- ⚠️ Overkill for this application

---

## 📊 Comparison: Before vs After

### Wire Count Through Firewall

| Connection | Before (Analog) | After (I2C) |
|------------|-----------------|-------------|
| IAT Pre | 3 wires (5V, SIG, GND) | - |
| IAT Post | 3 wires (5V, SIG, GND) | - |
| Coolant | 3 wires (5V, SIG, GND) | - |
| MAP Sensor | 3 wires (5V, SIG, GND) | - |
| MAX31855 | 5 wires (5V, GND, CS, SCK, MISO) | - |
| **I2C Harness** | - | **4 wires (5V, GND, SCL, SDA)** |
| **Total** | **17 wires** | **4 wires** |
| **Connectors** | None or multiple | **1 connector** |

### Pros and Cons

| Aspect | Before | After |
|--------|--------|-------|
| Installation time | 2-3 hours | 1 hour |
| Firewall penetrations | 2-3 holes | 1 hole |
| Wire management | Complex | Simple |
| Serviceability | Difficult | Easy |
| Noise immunity | Moderate | Excellent |
| Wire length limit | ~3 feet | ~10+ feet |
| Cost | $190-230 | $225-310 |
| Complexity | Low (direct wiring) | Moderate (module assembly) |

---

## 🚗 Installation Location (Engine Bay)

### Ideal Mounting Location

**Requirements:**
- ✅ Protected from direct water spray
- ✅ Away from hot components (>200°F)
- ✅ Accessible for service
- ✅ Secure mounting surface
- ✅ Short wire runs to sensors

**Good Locations:**
1. **Inner fender well** (behind headlight)
   - Protected from engine heat
   - Usually has mounting points
   - Easy access to remove headlight

2. **Firewall mounting** (upper corner)
   - Short wire run to cabin
   - Protected from engine
   - May need custom bracket

3. **Side of battery tray**
   - Good protection
   - Easy access
   - May need relocation for some vehicles

4. **Behind grille** (if space permits)
   - Cooler location
   - Protected from engine bay heat
   - Good for intercooler temp sensors nearby

**Avoid:**
- ❌ Near exhaust manifold
- ❌ Near turbo
- ❌ Near radiator hoses
- ❌ Low points (water accumulation)
- ❌ Direct splash zone

---

## 🔧 Sensor Wiring to Module

### Thermistor Extension

**If sensors are far from module:**
- Use 22 AWG wire (stranded, automotive grade)
- Twist pair for noise reduction
- Heat shrink all connections
- Label each wire clearly
- Max extension: 10 feet

**Termination:**
- Screw terminals at module
- Weatherpack connectors for removable sensors
- Heat shrink + zip tie for permanent

### MAP Sensor to Module

**Direct connection recommended:**
- Keep vacuum line short (~2 feet max)
- Mount MAP sensor near module
- 3-wire connection: 5V, Signal, GND

### EGT Thermocouple to Module

**CRITICAL: Do not extend thermocouple wire!**
- Use proper K-type extension wire only
- Or mount module near thermocouple
- Standard wire will cause reading errors
- Route away from ignition wires (interference)

**Thermocouple wire:**
- Type K extension wire (yellow/red jacket)
- Amazon: "K-type thermocouple extension wire"
- Max extension: 20 feet (longer is fine)

---

## ✅ Testing & Validation

### Bench Test (Before Vehicle Install)

1. **Power supply test:**
   - Connect 5V/GND from power supply
   - Verify module powers up
   - Check current draw (<100mA idle)

2. **I2C device detection:**
   - Upload I2C scanner sketch to ESP32
   - Verify devices found at 0x48 (ADS1115) and 0x60 (MCP9600)
   - If not found: Check wiring, pullup resistors

3. **Thermistor test:**
   - Connect known-good 2.2kΩ resistor to IAT Pre input
   - Read value: Should be ~2.5V (or ~25°C)
   - Heat resistor with finger: Should change
   - Repeat for all 3 thermistor inputs

4. **MAP sensor test:**
   - Connect MAP sensor
   - Should read ~1.0V (atmospheric pressure)
   - Cover sensor port: Voltage should change
   - Blow gently: Pressure should increase

5. **EGT test:**
   - Connect K-type thermocouple
   - Should read room temperature (~20-25°C)
   - Heat probe tip with lighter
   - Temperature should increase rapidly

### Vehicle Installation Test

1. **Install module in engine bay**
2. **Connect all sensors**
3. **Route harness through firewall**
4. **Connect to ESP32**
5. **Power on system**
6. **Verify readings:**
   - All sensors showing valid values
   - No fault codes
   - Display updating correctly
7. **Start engine and verify dynamic readings**

---

## 🆘 Troubleshooting

### Problem: I2C devices not detected

**Check:**
- [ ] 5V power connected
- [ ] GND connected
- [ ] SCL/SDA wires not swapped
- [ ] Pullup resistors present (ADS1115 has onboard, but check ESP32)
- [ ] Wire length (<10 feet recommended)
- [ ] No shorts between I2C lines

**Solution:**
- Use I2C scanner sketch
- Check I2C addresses (may be wrong)
- Try shorter wire temporarily
- Add 4.7kΩ pullup resistors on ESP32 side

### Problem: Incorrect temperature readings

**Check:**
- [ ] Correct resistor values (2.2kΩ)
- [ ] NTC thermistor connected correctly
- [ ] ADS1115 gain setting correct
- [ ] Voltage divider wired correctly

**Solution:**
- Measure voltage at ADC input with multimeter
- Compare to expected voltage
- Check resistor color code
- Verify thermistor resistance with multimeter

### Problem: EGT reading wrong or zero

**Check:**
- [ ] Thermocouple polarity correct
- [ ] K-type extension wire used (not regular wire)
- [ ] MCP9600 thermocouple type set to K
- [ ] No shorts in thermocouple circuit

**Solution:**
- Swap +/- if needed
- Replace extension wire with proper K-type
- Check MCP9600 configuration in code

### Problem: MAP sensor reading wrong

**Check:**
- [ ] Sensor getting 5V power
- [ ] Signal wire connected to correct ADC channel
- [ ] Vacuum line connected and not leaking
- [ ] Sensor not damaged

**Solution:**
- Verify voltage at sensor output
- Should be ~1.0V at idle (vacuum)
- Should be >1.0V under boost
- Replace sensor if out of range

---

## 📦 Parts List Summary

### Core Module Components

| Item | Quantity | Estimated Cost |
|------|----------|----------------|
| ADS1115 16-bit ADC module | 1 | $8-12 |
| MCP9600 thermocouple amplifier | 1 | $12-18 |
| IP67 enclosure (100x68x50mm) | 1 | $8-12 |
| Protoboard (7x5cm) | 1 | $3-5 |
| 2.2kΩ resistors (1/4W) | 3 | $0.30 |
| 2-position screw terminals (5mm) | 4 | $4-6 |
| M3 standoffs and screws | 1 set | $2-3 |
| **Subtotal** | | **$37-56** |

### Harness Components

| Item | Quantity | Estimated Cost |
|------|----------|----------------|
| Deutsch DT 6-pin connector set | 1 | $12-18 |
| 4-conductor shielded cable (22 AWG) | 10 feet | $10-20 |
| Cable glands (PG9) | 2 | $2-4 |
| Heat shrink tubing assortment | 1 | $8-12 |
| Wire labels | 1 pack | $3-5 |
| Zip ties | 1 bag | $3-5 |
| **Subtotal** | | **$38-64** |

### **Grand Total: $75-120**

### Alternative: Budget Build (~$50)

- Use generic clone modules ($5 each)
- Weatherpack connectors instead of Deutsch ($6)
- Unshielded 4-conductor cable ($5)
- Basic project box ($5)
- **Total: ~$50**

---

## 📝 Next Steps

### Ready to proceed?

**I can help you with:**

1. ✅ **Modify the code** - Update sensors.cpp, config.h, and .ino files for I2C
2. ✅ **Create shopping list** - Specific Amazon/Adafruit links for all parts
3. ✅ **Design custom PCB** - KiCad schematic and board layout (if you want Option 2)
4. ✅ **Wire harness pinout** - Detailed connector wiring diagram
5. ✅ **Assembly photos/guide** - Step-by-step photo guide (if community contributes)

### Decision Tree

```
Do you want to proceed with I2C sensor module?
├─ YES → Which option?
│  ├─ Option 1: Pre-built modules (~$75, easiest)
│  ├─ Option 2: Custom PCB (~$80, most professional)
│  └─ Option 3: Hybrid (~$55, cost-effective)
│
└─ NO → Keep current analog wiring
   └─ Alternative: Use shielded cables + weatherproof connectors
```

**Let me know:**
1. Which option you prefer (1, 2, or 3)
2. If you want me to update the code files now
3. If you need a shopping list with specific links

---

**This is a significant improvement for installation quality!** The unified harness with single connector is how professional automotive systems are built. Worth the extra cost and effort.
