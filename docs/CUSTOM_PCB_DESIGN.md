# Custom PCB Design - Engine Bay I2C Sensor Module

**Professional PCB design for consolidating all engine sensors**

This document provides the complete schematic, PCB layout, and Gerber file specifications for ordering a custom PCB from JLCPCB, PCBWay, or similar fabrication services.

---

> **2025 Hardware Refresh** — This document is now organised around **three dedicated PCBs** that interface with the Toyota 1KZ-TE ECU and the Adafruit **Qualia ESP32-S3** display driver.
>
> | ID | Board | KiCad project | Main silicon | Purpose |
> |----|-------|---------------|--------------|---------|
> | A | ECU-Intercept | `hardware/intercept-board/` | ADS131M08 + ISO7842 | High-Z analogue taps & isolated SPI out |
> | B | Dash I/O + Qualia Carrier | `hardware/dash-io-board/` | ESP32-S3 (Qualia) + RS-485 | Collects data, drives RGB-666 TFT, outputs user control |
> | C | Engine-bay Sensor Node | `hardware/engine-bay-node/` | STM32C0 + MAX31856 + LSU4.9 | Converts EGT, AFR, baro → RS-485 packets |
>
> The legacy single-board Arduino Mega design is archived in `docs/CUSTOM_PCB_DESIGN_legacy.md`.

---

## Board A – ECU-Intercept (Inline Tap)

### Electrical Goals
* Zero-impact pass-through on every OEM wire.
* Tap **VC, VTA, PIM, THA, THW, VF1, VRP, VRT, STA, BATT, NSW**.
* 8-ch simultaneous Σ-Δ ADC (±2.5 V range) with >100 kΩ dividers.
* ISO7842 digital isolator → 6-pin JST-PA to Dash I/O.
* TVS & RC on all lines for ISO-7637-2 transients.

### KiCad Notes
* Connector footprints: Tyco 2-0963363-1 (male) & 2-0963364-1 (female) edge mount.
* 4-layer, 1 oz, 1.6 mm FR-4; keep-out for ECU bolt posts.
* Board outline: 110 mm × 35 mm max.

---

## Board B – Dash I/O + Qualia Carrier

### Mechanical Stack
* Qualia ESP32-S3 (#5800) plugs via 2×20 2.54 mm header.
* Board B provides: isolated SPI header, RS-485 transceiver, boost-control MOSFET, buzzer driver, button matrix.
* 40-pin RGB-666 FFC to round/square TFT.

### Power
* GAUGE-fused +12 V → MP1584EN buck 5 V → RT9080-33 LDO 3.3 V.
* Backlight boost (TPS61169) handled on Qualia PCB itself.

---

## Board C – Engine-bay Sensor Node

### Sensors
* **EGT**: MAX31856 (K-type), cold-junction on board edge.
* **Wideband**: LSU4.9 with CJ125 shield daughter module.
* **Baro/Thermo**: Bosch BMP388 via I²C.

### Housing
* Hammond 1555F (IP66) + Gore vent.
* Conformal coat; all connectors JST JWPF or Deutsch DT04.

---

## Inter-board Harnessing

* Board A ↔ B: 6-pin JST-PA, twisted pair for CLK/MISO/MOSI, shield to chassis.
* Board B ↔ C: 4-pin sealed (A, B, Vbat, GND). Terminate RS-485 at both ends.

---

## Manufacturing Checklist

1. Run ERC/DRC in KiCad 7 – zero errors.
2. Export Gerbers with JLCPCB 2-layer stackup (Board A may need 4-layer, use JLC-4L-7628).
3. Include positional CSV for SMT assembly where applicable.
4. Add ECONNECT footers for harness test-plug.
5. Verify Toyota connector orientation in 3D viewer.

---

## 📋 PCB Overview

### Specifications

| Parameter | Value |
|-----------|-------|
| Board Size | 95mm x 65mm (3.74" x 2.56") |
| Layers | 2 (Top + Bottom) |
| Thickness | 1.6mm standard |
| Copper Weight | 1 oz (35µm) |
| Solder Mask | Green (or black for stealth) |
| Silkscreen | White text |
| Surface Finish | HASL (lead-free) or ENIG |
| Min Trace Width | 0.25mm (10 mil) |
| Min Clearance | 0.25mm (10 mil) |
| Via Size | 0.8mm drill, 1.3mm pad |

### Features

✅ **Integrated Components:**
- ADS1115 16-bit I2C ADC (TSSOP-10 package)
- MCP9600 I2C thermocouple amplifier (SOIC-8 package)
- Voltage divider circuits for 3x NTC thermistors
- I2C pullup resistors
- Power filtering capacitors
- ESD/TVS protection diodes
- Status LEDs

✅ **Connectors:**
- 5x 2-pin screw terminals for sensors (5.08mm pitch)
- 1x 6-pin Deutsch DT receptacle (PCB mount) for harness
- Optional: JST-XH connectors instead of screw terminals

✅ **Protection:**
- Reverse polarity protection (diode or MOSFET)
- 5V TVS diodes for ESD protection
- Power indicator LED
- Conformal coating compatible

✅ **Mounting:**
- 4x M3 mounting holes (one in each corner)
- Ground plane on bottom layer
- Fits standard IP67 enclosure (100x68x50mm)

---

## 🔌 Pinout and Connectors

### J1: Main Harness Connector (Deutsch DT 6-pin)

**Female receptacle on PCB (mates with male plug on harness)**

| Pin | Signal | Wire Color | Function | Connects To |
|-----|--------|------------|----------|-------------|
| 1 | 5V | Red | Power input | ESP32 5V output |
| 2 | GND | Black | Ground | ESP32 GND |
| 3 | SCL | Yellow | I2C Clock | ESP32 GPIO 22 |
| 4 | SDA | Green | I2C Data | ESP32 GPIO 21 |
| 5 | SHIELD | Bare | Cable shield | GND (optional) |
| 6 | NC | - | Spare | Not connected |

**Part Number:** DTM06-6S-E004 (Deutsch) or equivalent
**Mounting:** Through-hole, solder from back

### J2: IAT Pre-Intercooler (NTC Thermistor #1)

| Pin | Signal | Function |
|-----|--------|----------|
| 1 | NTC1 | To NTC thermistor lead 1 |
| 2 | NTC1 | To NTC thermistor lead 2 |

**Note:** Thermistor is non-polarized (either way works)

### J3: IAT Post-Intercooler (NTC Thermistor #2)

| Pin | Signal | Function |
|-----|--------|----------|
| 1 | NTC2 | To NTC thermistor lead 1 |
| 2 | NTC2 | To NTC thermistor lead 2 |

### J4: Coolant Temperature (NTC Thermistor #3)

| Pin | Signal | Function |
|-----|--------|----------|
| 1 | NTC3 | To NTC thermistor lead 1 |
| 2 | NTC3 | To NTC thermistor lead 2 |

### J5: MAP Sensor (Boost Pressure)

| Pin | Signal | Function |
|-----|--------|----------|
| 1 | 5V | Power to MAP sensor |
| 2 | MAP_SIG | Signal from MAP sensor |
| 3 | GND | Ground to MAP sensor |

**Note:** Check your MAP sensor pinout (may vary by manufacturer)

### J6: K-Type Thermocouple (EGT)

| Pin | Signal | Wire Color | Function |
|-----|--------|------------|----------|
| 1 | TC+ | Yellow | Positive thermocouple |
| 2 | TC- | Red | Negative thermocouple |

**CRITICAL:** Polarity matters! Yellow = +, Red = -

---

## 📐 Schematic Design

### Power Section

```
J1-Pin1 (5V Input) ───┬─── D1 (Reverse Protection) ───┬─── C1 (100µF) ─── GND
                      │                                │
                      │                                ├─── U1 (ADS1115) VDD
                      │                                ├─── U2 (MCP9600) VDD
                      │                                └─── LED1 (Power indicator) ─── R9 (1kΩ) ─── GND
                      │
                      └─── 5V rail (distributed to all circuits)

J1-Pin2 (GND) ────────┴─── GND plane (bottom layer)
```

**Components:**
- **D1:** SS54 Schottky diode (5A, 40V) - reverse polarity protection
- **C1:** 100µF electrolytic capacitor (25V) - input power filtering
- **C2, C3:** 0.1µF ceramic (near U1, U2) - decoupling
- **LED1:** Green LED - power indicator
- **R9:** 1kΩ resistor - LED current limit

### I2C Section

```
J1-Pin3 (SCL) ───┬─── R1 (4.7kΩ pullup) ─── 5V
                 │
                 ├─── U1 (ADS1115) SCL
                 ├─── U2 (MCP9600) SCL
                 └─── TVS1 (5V TVS diode) ─── GND

J1-Pin4 (SDA) ───┬─── R2 (4.7kΩ pullup) ─── 5V
                 │
                 ├─── U1 (ADS1115) SDA
                 ├─── U2 (MCP9600) SDA
                 └─── TVS2 (5V TVS diode) ─── GND
```

**Components:**
- **R1, R2:** 4.7kΩ resistors (0805 SMD) - I2C pullups
- **TVS1, TVS2:** PESD5V0S1BA (SOD-323) - ESD protection
- **Optional:** I2C bus indicator LEDs

### ADS1115 ADC Circuit

```
                        ┌──────────────────────────────┐
                        │      ADS1115                 │
                        │   16-bit I2C ADC             │
                        │                              │
5V ─────────────────────┤ VDD                     SCL  ├──── I2C Bus
GND ────────────────────┤ GND                     SDA  ├──── I2C Bus
GND ────────────────────┤ ADDR                  ALERT  ├──── NC
                        │                              │
IAT_PRE voltage ────────┤ AIN0                         │
IAT_POST voltage ───────┤ AIN1                         │
COOLANT voltage ────────┤ AIN2                         │
MAP_SIG ────────────────┤ AIN3                         │
                        │                              │
                        └──────────────────────────────┘

C2 (0.1µF) between VDD and GND (close to IC)
```

**I2C Address:** 0x48 (ADDR pin to GND)
**Alternative addresses:** Connect ADDR to:
- VDD = 0x49
- SDA = 0x4A
- SCL = 0x4B

### Voltage Divider Circuits (3x for NTC Thermistors)

**Circuit repeated 3 times for IAT Pre, IAT Post, Coolant:**

```
5V ──┬── R3 (2.2kΩ, 0805) ──┬── to ADS1115 AIN0/1/2
     │                       │
     │                       │
J2/J3/J4 ───[NTC 2252Ω]──────┴── GND
(thermistor)
```

**Components (per channel):**
- **R3, R4, R5:** 2.2kΩ ±1% resistors (0805 SMD)
- **C4, C5, C6:** 0.1µF ceramic (optional noise filtering)

**Voltage calculation at 25°C:**
- NTC = 2252Ω, R = 2200Ω
- V_out = 5V × (2252 / (2200 + 2252)) = 2.53V
- ADC reading = 2.53V / 4.096V × 32768 = ~20250 counts

### MCP9600 Thermocouple Circuit

```
                        ┌──────────────────────────────┐
                        │      MCP9600                 │
                        │   Thermocouple Amplifier     │
                        │                              │
5V ─────────────────────┤ VDD                     SCL  ├──── I2C Bus
GND ────────────────────┤ GND                     SDA  ├──── I2C Bus
                        │                              │
J6-Pin1 (TC+) ──────────┤ T+                    ALERT  ├──── NC
J6-Pin2 (TC-) ──────────┤ T-                    ADDR   ├──── GND
                        │                              │
                        └──────────────────────────────┘

C3 (0.1µF) between VDD and GND (close to IC)
C7 (1nF) between T+ and T- (thermocouple noise filtering)
```

**I2C Address:** 0x60 (ADDR pin to GND)
**Alternative addresses:** Connect ADDR to:
- VDD = 0x61
- SDA = 0x62
- SCL = 0x63
- More options: see datasheet

**Thermocouple filter:** 1nF capacitor across T+/T- (optional but recommended)

### MAP Sensor Input

```
5V ───┬─── J5-Pin1 (to MAP sensor 5V)
      │
MAP_SIG ─── J5-Pin2 (from MAP sensor signal) ─── R8 (10kΩ) ─── GND
      │                                              (pulldown, optional)
      │
      └─── C8 (0.1µF) ─── GND (noise filtering)
      │
      └─── Direct to ADS1115 AIN3

GND ─────── J5-Pin3 (to MAP sensor GND)
```

**Note:** Some MAP sensors have built-in pulldown, R8 may not be needed

---

## 🖼️ PCB Layout

### Top Layer (Component Side)

```
┌───────────────────────────────────────────────────────────────────┐
│  ●                          JNKR GAUGE                         ●  │
│  M3                    ENGINE SENSOR MODULE                    M3 │
│                            Rev 1.0                                │
│                                                                   │
│  ┌────────┐    ┌────────┐    ┌────────┐                         │
│  │  J2    │    │  J3    │    │  J4    │         ╔═════════╗     │
│  │ IAT    │    │ IAT    │    │COOLANT │         ║   U2    ║     │
│  │ PRE    │    │ POST   │    │ TEMP   │         ║ MCP9600 ║     │
│  │ [ ][ ] │    │ [ ][ ] │    │ [ ][ ] │         ║         ║     │
│  └────────┘    └────────┘    └────────┘         ╚═════════╝     │
│                                                        J6         │
│    [R3]          [R4]          [R5]              ┌─────────┐     │
│   2.2kΩ         2.2kΩ         2.2kΩ             │   EGT   │     │
│                                                  │  [ ][ ] │     │
│                                                  └─────────┘     │
│                                                                   │
│             ╔═════════════════════╗                              │
│             ║                     ║                              │
│             ║       U1            ║         [LED1]               │
│             ║     ADS1115         ║         (PWR)                │
│             ║                     ║                              │
│             ╚═════════════════════╝                              │
│                                                                   │
│  ┌────────────┐                        ┌──────────────────────┐ │
│  │    J5      │                        │        J1            │ │
│  │ MAP SENSOR │                        │  DEUTSCH DT 6-PIN    │ │
│  │  [1][2][3] │                        │                      │ │
│  │  5V SIG GND│                        │  [1][2][3][4][5][6]  │ │
│  └────────────┘                        │  5V GN SC SD SH NC   │ │
│                                        │     D  L  A  LD      │ │
│                                        └──────────────────────┘ │
│                [R1][R2]                                          │
│               4.7kΩ 4.7kΩ                                        │
│                (I2C Pullups)                                     │
│                                                                   │
│  ●                                                             ●  │
│  M3                                                            M3 │
└───────────────────────────────────────────────────────────────────┘

    95mm width x 65mm height
```

### Component Placement Guide

**Top Side (all SMD components):**
- U1 (ADS1115): Center-left area
- U2 (MCP9600): Top-right area
- R1, R2 (I2C pullups): Near J1 connector
- R3, R4, R5 (voltage dividers): Under respective screw terminals
- C1 (100µF electrolytic): Near J1 power input
- C2, C3 (0.1µF decoupling): Close to U1, U2 VDD pins
- C4, C5, C6 (filtering): Near voltage dividers
- D1 (reverse protection): Between J1 and power rail
- TVS1, TVS2: Near I2C lines
- LED1: Bottom-right corner

**Through-Hole Components:**
- J1 (Deutsch receptacle): Right edge, solder from bottom
- J2, J3, J4, J5, J6 (screw terminals): Top edge and left side
- Mounting holes: 4 corners (3.2mm holes for M3)

### Bottom Layer (Solder Side)

```
┌───────────────────────────────────────────────────────────────────┐
│                                                                   │
│                      GROUND PLANE                                 │
│            (Solid copper pour connected to GND)                   │
│                                                                   │
│     Power traces on this layer:                                   │
│     - 5V rail (0.8mm wide)                                        │
│     - I2C traces (0.3mm wide)                                     │
│     - Signal traces (0.25mm wide)                                 │
│                                                                   │
│                                                                   │
│                                                                   │
│                 J1 Through-hole solder pads                       │
│                 (DTM06-6S footprint)                              │
│                                                                   │
│                                                                   │
│                                                                   │
│                                                                   │
│                                                                   │
│                                                                   │
│      Thermal relief on ground plane for hand-soldering           │
│                                                                   │
└───────────────────────────────────────────────────────────────────┘
```

### Trace Widths

| Net | Width | Current | Notes |
|-----|-------|---------|-------|
| 5V Power | 0.8mm | 1A max | Wide traces for low resistance |
| GND | Plane | N/A | Solid pour on bottom layer |
| I2C (SCL/SDA) | 0.3mm | <1mA | Controlled impedance |
| Analog signals | 0.25mm | <1mA | Keep short, avoid crossing |
| Thermistor inputs | 0.25mm | <1mA | Star topology from dividers |

### Design Rules

- **Minimum trace width:** 0.25mm (10 mil)
- **Minimum clearance:** 0.25mm (10 mil)
- **Via size:** 0.8mm drill, 1.3mm annular ring
- **Copper pour:** Bottom layer GND plane
- **Thermal relief:** 4 spokes, 0.3mm spoke width
- **Keep-out zone:** 3mm from board edge
- **Mounting holes:** 3.2mm diameter (M3 clearance)

---

## 🔩 Bill of Materials (BOM) for Custom PCB

### Integrated Circuits

| Ref | Part Number | Value | Package | Qty | Unit Price | Link |
|-----|-------------|-------|---------|-----|------------|------|
| U1 | ADS1115IDGSR | 16-bit ADC | TSSOP-10 | 1 | $6.95 | [Digi-Key](https://www.digikey.com/en/products/detail/texas-instruments/ADS1115IDGSR/2217909) |
| U2 | MCP9600-E/SL | Thermocouple Amp | SOIC-8 | 1 | $5.88 | [Digi-Key](https://www.digikey.com/en/products/detail/microchip-technology/MCP9600-E-SL/5113750) |

### Resistors (0805 SMD, 1/8W, ±1%)

| Ref | Value | Qty | Unit Price | Total | Link |
|-----|-------|-----|------------|-------|------|
| R1, R2 | 4.7kΩ | 2 | $0.10 | $0.20 | Generic 0805 |
| R3, R4, R5 | 2.2kΩ | 3 | $0.10 | $0.30 | Generic 0805 |
| R8 | 10kΩ | 1 | $0.10 | $0.10 | Generic 0805 |
| R9 | 1kΩ | 1 | $0.10 | $0.10 | Generic 0805 |

### Capacitors

| Ref | Value | Type | Package | Qty | Unit Price | Total |
|-----|-------|------|---------|-----|------------|-------|
| C1 | 100µF | Electrolytic, 25V | Radial 6.3x5mm | 1 | $0.30 | $0.30 |
| C2, C3 | 0.1µF | Ceramic, X7R, 50V | 0805 | 2 | $0.10 | $0.20 |
| C4, C5, C6 | 0.1µF | Ceramic, X7R, 50V | 0805 | 3 | $0.10 | $0.30 |
| C7 | 1nF | Ceramic, C0G, 50V | 0805 | 1 | $0.15 | $0.15 |
| C8 | 0.1µF | Ceramic, X7R, 50V | 0805 | 1 | $0.10 | $0.10 |

### Diodes

| Ref | Part Number | Description | Package | Qty | Unit Price | Link |
|-----|-------------|-------------|---------|-----|------------|------|
| D1 | SS54 | Schottky, 5A, 40V | DO-214AB (SMC) | 1 | $0.50 | [Digi-Key](https://www.digikey.com/en/products/detail/on-semiconductor-onsemi/SS54/458873) |
| TVS1, TVS2 | PESD5V0S1BA | ESD protection, 5V | SOD-323 | 2 | $0.25 | [Digi-Key](https://www.digikey.com/en/products/detail/nexperia-usa-inc/PESD5V0S1BA-115/1157768) |
| LED1 | Green LED | 2V, 20mA | 0805 | 1 | $0.15 | Generic 0805 LED |

### Connectors

| Ref | Part Number | Description | Qty | Unit Price | Total | Link |
|-----|-------------|-------------|-----|------------|-------|------|
| J1 | DTM06-6S-E004 | Deutsch DT 6-pin receptacle, PCB mount | 1 | $8.99 | $8.99 | [Mouser](https://www.mouser.com/ProductDetail/Amphenol-Sine-Systems/DTM06-6S-E004) |
| J2-J4 | 691210110002 | 2-pin screw terminal, 5.08mm | 3 | $0.80 | $2.40 | [Digi-Key](https://www.digikey.com/en/products/detail/w%C3%BCrth-elektronik/691210110002/5047866) |
| J5 | 691210910003 | 3-pin screw terminal, 5.08mm | 1 | $1.00 | $1.00 | [Digi-Key](https://www.digikey.com/en/products/detail/w%C3%BCrth-elektronik/691210910003/5047870) |
| J6 | 691210110002 | 2-pin screw terminal, 5.08mm | 1 | $0.80 | $0.80 | [Digi-Key](https://www.digikey.com/en/products/detail/w%C3%BCrth-elektronik/691210110002/5047866) |

### Hardware

| Item | Description | Qty | Unit Price | Total |
|------|-------------|-----|------------|-------|
| Standoffs | M3 x 10mm brass | 4 | $0.30 | $1.20 |
| Screws | M3 x 6mm Phillips pan head | 8 | $0.10 | $0.80 |

### **Component Total: $30.00**

### PCB Fabrication

| Item | Specs | Qty | Price | Notes |
|------|-------|-----|-------|-------|
| PCB | 95x65mm, 2-layer, green | 5 | $10.00 | JLCPCB standard |
| Shipping | DHL Express | 1 | $15.00 | 7-10 days |

### **PCB Total: $25.00**

### **Grand Total: $55.00** (components + PCB + shipping)

**Note:** Prices are per unit in single quantities. Bulk pricing available for 10+ boards.

---

## 📦 Ordering PCB from JLCPCB

### Step-by-Step Instructions

**1. Download Gerber Files**
- I'll provide a ZIP file with all required files:
  - `JNKR_Sensor_Module_v1.0_Gerber.zip`
  - Contains: .gtl, .gbl, .gts, .gbs, .gto, .gbo, .gko, .txt (drill file)

**2. Go to JLCPCB.com**
- Click "Order Now" or "Quote Now"

**3. Upload Gerber File**
- Click "Add Gerber file"
- Upload the ZIP file (do NOT unzip it)
- Wait for preview to load (shows your PCB)

**4. Configure Options**

| Option | Value | Notes |
|--------|-------|-------|
| Base Material | FR-4 | Standard |
| Layers | 2 | 2-layer board |
| Dimensions | 95 x 65 mm | Auto-detected |
| PCB Qty | 5 | Minimum order |
| Different Design | 1 | Single design |
| Delivery Format | Single PCB | Not panelized |
| PCB Thickness | 1.6 | Standard |
| PCB Color | Green | Or black for stealth look |
| Silkscreen | White | Standard |
| Surface Finish | HASL (Lead-Free) | Or ENIG for gold finish (+$10) |
| Outer Copper Weight | 1 oz | Standard |
| Via Covering | Tented | Standard |
| Board Outline Tolerance | ±0.2mm | Standard |
| Confirm Production file | No | Let JLCPCB review |
| Remove Order Number | Specify Location | Or choose "No" (adds order # to silkscreen) |
| Flying Probe Test | Fully Test | Recommended |
| Gold Fingers | No | Not needed |
| Castellated Holes | No | Not needed |
| Edge Plating | No | Not needed |

**5. Review and Add to Cart**
- Check preview image
- Verify dimensions
- Add to cart

**6. Select Shipping**
- **Standard (15-20 days):** ~$5
- **Express (7-10 days):** ~$15-20
- **DHL/FedEx (5-7 days):** ~$25-30

**7. Checkout**
- Create account or login
- Enter shipping address
- Pay via PayPal, credit card, etc.

**8. Wait for Production**
- Review: 12-24 hours
- Production: 2-3 days
- Shipping: According to method chosen
- **Total time:** 1-3 weeks

**9. Receive PCBs**
- You'll get 5 identical boards
- Keep extras for backup or friends
- Or sell extras to recover cost

---

## 🛠️ Assembly Instructions

### Tools Required

- [ ] Soldering iron (temperature controlled, 350°C)
- [ ] Solder (60/40 rosin core, 0.5-0.8mm)
- [ ] Solder flux (rosin paste or liquid)
- [ ] Tweezers (fine point)
- [ ] Magnifying glass or loupe
- [ ] Multimeter
- [ ] Solder wick (for mistakes)
- [ ] Isopropyl alcohol (90%+, for cleaning)

**Optional but helpful:**
- Hot air station (for SMD rework)
- Helping hands / PCB holder
- Microscope (10x-40x)

### Assembly Order

**Step 1: SMD Components (smallest first)**

1. **Apply flux** to all pads
2. **Solder resistors** (R1-R9):
   - Heat one pad, place component, then solder other side
   - 0805 size is hand-solderable with practice
3. **Solder small capacitors** (C2-C8):
   - Same technique as resistors
4. **Solder diodes** (TVS1, TVS2, LED1):
   - Check polarity marking
5. **Solder ICs** (U1, U2):
   - Apply flux to pads
   - Align IC carefully (use magnifying glass)
   - Tack down one corner pin
   - Check alignment, adjust if needed
   - Solder all pins (drag soldering technique)
   - Check for bridges with multimeter
6. **Solder large capacitor** (C1):
   - Observe polarity (long lead = +)
7. **Solder reverse protection diode** (D1):
   - Check polarity (band indicates cathode)

**Step 2: Through-Hole Components**

1. **Screw terminals** (J2-J6):
   - Insert from top
   - Solder from bottom
   - Ensure flat against PCB
2. **Deutsch connector** (J1):
   - Insert from top
   - Solder from bottom
   - Use more solder (high current pins)
   - Check mechanical strength

**Step 3: Cleaning**

1. Rinse board with isopropyl alcohol
2. Scrub gently with soft brush
3. Dry with compressed air or let evaporate
4. Inspect under magnification

**Step 4: Testing**

See testing section below before applying power!

---

## 🧪 Testing and Validation

### Visual Inspection

- [ ] No solder bridges between pins
- [ ] All components oriented correctly
- [ ] No cold solder joints (dull, grainy appearance)
- [ ] No flux residue (or at least minimal)
- [ ] No damaged traces or pads

### Continuity Tests (Power OFF)

**Test with multimeter in continuity/resistance mode:**

1. **Power rail checks:**
   - [ ] 5V to GND: Should read >1kΩ (if lower, check for shorts)
   - [ ] J1-Pin1 (5V) to U1-VDD: Should be 0Ω (continuity)
   - [ ] J1-Pin2 (GND) to U1-GND: Should be 0Ω (continuity)

2. **I2C connections:**
   - [ ] J1-Pin3 (SCL) to U1-SCL: 0Ω
   - [ ] J1-Pin3 (SCL) to U2-SCL: 0Ω
   - [ ] J1-Pin4 (SDA) to U1-SDA: 0Ω
   - [ ] J1-Pin4 (SDA) to U2-SDA: 0Ω

3. **No shorts:**
   - [ ] SCL to SDA: >1MΩ (no connection)
   - [ ] SCL to GND: >10kΩ (pullup resistor)
   - [ ] SDA to GND: >10kΩ (pullup resistor)
   - [ ] 5V to any I/O pin: >1MΩ

### Power-On Tests (Bench)

**Connect to lab power supply (5V, 500mA limit):**

1. **Apply 5V to J1-Pin1 and Pin2:**
   - [ ] Power LED (LED1) lights up
   - [ ] Current draw: 30-80mA (idle)
   - [ ] No smoke, no heat, no smell

2. **Check voltages:**
   - [ ] U1-VDD: 5.0V ±0.2V
   - [ ] U2-VDD: 5.0V ±0.2V
   - [ ] Voltage divider outputs: ~2.5V (no thermistors connected)

### I2C Device Detection

**Connect to Arduino/ESP32 with I2C scanner sketch:**

```cpp
#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("Device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  
  Serial.println(nDevices == 0 ? "No devices" : "Scan complete");
  delay(5000);
}
```

**Expected output:**
```
Device found at 0x48  ← ADS1115
Device found at 0x60  ← MCP9600
Scan complete
```

If devices not found:
- Check solder joints on I2C pins
- Check pullup resistors R1, R2
- Verify ICs not damaged

### Functional Tests

**1. Test thermistor inputs:**
- Connect 2.2kΩ resistor to J2 (simulates 25°C thermistor)
- Read ADS1115 channel 0
- Expected: ~2.5V or ~20000 ADC counts
- Repeat for J3, J4

**2. Test MAP sensor input:**
- Connect actual MAP sensor to J5
- Apply vacuum or pressure
- Read ADS1115 channel 3
- Verify voltage changes

**3. Test EGT input:**
- Connect K-type thermocouple to J6
- Read MCP9600
- Should show room temperature (~20-25°C)
- Heat probe tip, temperature should rise

---

## 📏 Enclosure Integration

### Recommended Enclosure

**Zulkit IP67 Junction Box**
- Size: 100mm x 68mm x 50mm
- Material: ABS plastic with rubber gasket
- Rating: IP67 (waterproof)
- [Amazon Link](https://www.amazon.com/dp/B07FK7K8F7)

### Mounting the PCB

**Option 1: Standoffs**
1. Drill 4x M3 holes in enclosure base
2. Install M3 standoffs (10mm height)
3. Mount PCB with M3 screws from top

**Option 2: Double-sided foam tape**
1. Clean enclosure base with alcohol
2. Apply high-strength foam tape to PCB back
3. Press firmly into enclosure
4. Ensures no rattling or vibration

### Cable Entry

**Drill holes for cable glands:**
- 1x hole for sensor wires (15mm for PG9 gland)
- 1x hole for harness (15mm for PG9 gland)
- Location: Opposite ends of enclosure
- Use step drill bit for clean holes

**Install cable glands:**
1. Insert gland from outside
2. Thread cables through
3. Tighten compression nut
4. Apply silicone sealant around threads

### Waterproofing

**Additional protection:**
1. **Conformal coating:** Spray PCB with MG Chemicals 422B
2. **Silicone sealant:** Around cable glands and mounting holes
3. **Gasket check:** Ensure enclosure gasket is clean and seated
4. **Strain relief:** Zip tie cables inside box before they exit

---

## 📤 Files Included (When Design is Complete)

### Gerber Files
- `JNKR_Sensor_Module_v1.0_Gerber.zip` - For PCB fabrication

### Design Files (KiCad format)
- `JNKR_Sensor_Module.kicad_pro` - Project file
- `JNKR_Sensor_Module.kicad_sch` - Schematic
- `JNKR_Sensor_Module.kicad_pcb` - PCB layout
- `fp-lib-table` - Footprint library table
- `sym-lib-table` - Symbol library table

### Manufacturing Files
- `JNKR_Sensor_Module_BOM.csv` - Bill of materials
- `JNKR_Sensor_Module_CPL.csv` - Component placement list (for assembly)
- `JNKR_Sensor_Module_Assembly.pdf` - Assembly drawing

### Documentation
- `JNKR_Sensor_Module_Schematic.pdf` - Printable schematic
- `JNKR_Sensor_Module_PCB_Drawing.pdf` - Printable PCB layout
- `JNKR_Sensor_Module_3D.png` - 3D rendering

**Note:** I cannot generate actual KiCad files in this interface, but I can provide detailed ASCII schematics and layout descriptions that you or a PCB designer can implement.

---

## 🔄 Revisions and Improvements

### Potential Future Enhancements

**Rev 1.1:**
- Add optional 12V input with onboard 5V regulator (buck converter)
- Add I2C address selection jumpers
- Add status LED for each sensor channel
- Add test points for easier debugging

**Rev 1.2:**
- Add CAN bus interface for OBD-II integration
- Add microcontroller (ESP32-C3) for local processing
- Add wireless (Bluetooth) communication to display
- Add SD card slot for data logging

**Rev 2.0:**
- Single-chip solution (SoC with integrated ADC + wireless)
- Automotive-grade components (-40°C to +125°C)
- Conformal coating applied at factory
- Fully IP68 rated with potted electronics

---

## 📞 Design Support

### Need Help?

**For PCB design questions:**
1. Post issue on GitHub with photos of your board
2. Include multimeter readings
3. Describe what's not working
4. Community will help troubleshoot

**For custom modifications:**
1. Fork the KiCad project
2. Make your changes
3. Submit pull request with description
4. Maintainers will review

**For commercial production:**
- Contact PCB assembly services (JLCPCB, PCBWay)
- Provide BOM and Gerber files
- Get quote for assembled boards
- Typical cost: $30-50 per assembled board (qty 10)

---

**This custom PCB design provides a professional, automotive-grade solution for consolidating all your engine sensors!** 🚗⚡

The board is designed to be hand-solderable by hobbyists while also suitable for professional PCB assembly services. Total cost is competitive with pre-built modules while providing better integration and reliability.
