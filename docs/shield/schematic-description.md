# JNKR Gauge Shield - Schematic Description (Legacy)

> ⚠️ **DEPRECATED**: This is for the **legacy Arduino Mega** shield.
> 
> **Current Platform**: Qualia ESP32-S3 with integrated display connector
> 
> This document is kept for reference only.

---

**Circuit Design and Component Specifications (Legacy Arduino Mega Shield)**

This document describes the electrical schematic for the legacy Arduino Mega JNKR Gauge Shield PCB. The actual schematic diagrams would be created using electronic design software (KiCad, Eagle, Altium, etc.) and exported as PDFs.

---

## 🔌 Circuit Blocks

### 1. Power Supply Section

**12V Input Protection:**
```
Vehicle 12V ──┬── Fuse (3A) ──┬── D1 (1N5822) ──┬── LM2596 Input
              │                │                   │
              │                └── C1 (100µF) ────┘
              │
              └── GND ───────────────────────────── GND
```

**Components:**
- **D1 (1N5822):** Schottky diode for reverse polarity protection
  - Forward voltage: 0.5V @ 3A
  - Prevents damage if 12V is connected backward
  - Low voltage drop for efficiency

- **C1 (100µF, 25V):** Input filter capacitor
  - Smooths vehicle electrical noise
  - Protects against voltage spikes
  - Polarized - negative to GND

**5V Regulation:**
```
LM2596 Module
  Input:  12V (9-40V range)
  Output: 5V @ 3A max
  Adj pot: Set to exactly 5.0V
```

- **C2 (100µF, 16V):** Output filter capacitor
  - Stabilizes 5V output
  - Reduces ripple

---

### 2. NTC Thermistor Interfaces (x3)

**Circuit per sensor (IAT, Coolant, Oil):**
```
         5V Rail
            │
            │
          ┌─┴─┐
          │   │  R1 = 2.2kΩ (Reference Resistor)
          └─┬─┘
            ├──────────────→ To Arduino Analog Pin (A0, A1, or A2)
            │
          ┌─┴─┐
          │   │  NTC Thermistor (2252Ω @ 25°C)
          └─┬─┘  Connected via screw terminal
            │
           GND
```

**How it works:**
- Creates voltage divider
- Voltage at junction varies with temperature
- Arduino measures this voltage
- Formula: V_out = V_in × (R_NTC / (R_ref + R_NTC))

**At different temperatures:**
| Temp | NTC Resistance | Junction Voltage |
|------|----------------|------------------|
| 0°C | ~6,500Ω | ~3.8V |
| 25°C | ~2,252Ω | ~2.5V |
| 100°C | ~200Ω | ~0.4V |

**Components per circuit:**
- R1a/b/c: 2.2kΩ, 1/4W, 5% tolerance
- Terminal: 3-pin screw terminal (5mm pitch)
- C3/4/5: 0.1µF decoupling cap at Arduino pin

---

### 3. MAP Sensor Interface

**Circuit:**
```
5V Rail ────→ MAP Sensor Pin 1 (VCC)
             MAP Sensor Pin 2 (Signal) ────→ Arduino A3
GND ────────→ MAP Sensor Pin 3 (GND)
```

**Simple pass-through:**
- No additional components needed
- Sensor outputs 0.5-4.5V
- Maps to 0-3 bar absolute pressure
- 3-pin screw terminal for connection

**Protection:**
- C6 (0.1µF): Decoupling cap near Arduino A3
- Protects against electrical noise

---

### 4. Battery Voltage Divider

**Circuit:**
```
12V Sense Terminal ───┬── R2 (10kΩ) ───┬───→ Arduino A7
                      │                 │
                      │               ┌─┴─┐
                      │               │   │  R3 (2.2kΩ)
                      │               └─┬─┘
                      │                 │
                     GND ──────────────┴───→ GND
```

**Voltage division:**
- Ratio: (R2 + R3) / R3 = 12.2k / 2.2k = 5.55:1
- 12V input → 2.16V at A7
- 14V input → 2.52V at A7
- Arduino can safely read up to 5V

**Calculation in code:**
```cpp
float batteryV = (analogRead(A7) / 1023.0) * 5.0 * 5.55;
```

**Components:**
- R2: 10kΩ, 1/4W, 1% tolerance (for accuracy)
- R3: 2.2kΩ, 1/4W, 1% tolerance
- Terminal: 2-pin screw terminal
- C7 (0.1µF): Filter cap at A7

---

### 5. MAX31855 EGT Interface

**Circuit:**
```
5V ────────────→ MAX31855 VCC
GND ───────────→ MAX31855 GND

Arduino Pin 52 ─→ MAX31855 SCK (Clock)
Arduino Pin 10 ─→ MAX31855 CS  (Chip Select)
Arduino Pin 50 ←─ MAX31855 DO  (Data Out / MISO)

K-Type Thermocouple:
  Yellow wire ──→ MAX31855 T+
  Red wire ─────→ MAX31855 T-
```

**SPI Communication:**
- Clock: 4 MHz maximum
- Data read on falling clock edge
- 32-bit data packet

**Components:**
- Socket: 1×5 female header for MAX31855 module
- Terminal: 2-pin screw terminal for thermocouple
- C8 (0.1µF): Decoupling cap at MAX31855 VCC

**Important:** Do not reverse thermocouple polarity!

---

### 6. Nextion Display Interface

**Circuit:**
```
5V ─────────────→ Nextion Pin 1 (5V)
Arduino Pin 17 ─→ Nextion Pin 3 (RX)  ← Note: TX→RX crossover
Arduino Pin 16 ←─ Nextion Pin 2 (TX)  ← Note: RX←TX crossover
GND ────────────→ Nextion Pin 4 (GND)
```

**UART Communication:**
- Baud rate: 115200 (configured in code)
- 8 data bits, no parity, 1 stop bit
- Simple text commands

**Components:**
- Header: 1×4 female pin header (2.54mm pitch)
- Wire: 4-conductor cable to display

---

### 7. Buzzer Output

**Circuit:**
```
              ┌─────┐
Arduino Pin 8 ─┤ R4  ├─┬─→ Buzzer (+) Terminal
              └─────┘ │
                     GND ─→ Buzzer (-) Terminal
```

**Current limiting:**
- R4 = 100Ω
- Limits current to ~40mA (safe for Arduino pin)
- Buzzer: Active type, 5V, 2kHz

**Components:**
- R4: 100Ω, 1/4W resistor
- Terminal: 2-pin screw terminal
- Buzzer: External active piezo

---

### 8. LED Indicators

**Power LED (Green):**
```
         ┌─────┐
5V ──────┤ R5  ├─┬─────┐
         └─────┘ │     │
                 │  ┌──┴──┐
                 │  │ LED1│  Green LED
                 │  └──┬──┘
                 │     │
                GND ───┘
```

**Status LED (Blue):**
```
                ┌─────┐
Arduino Pin 13 ─┤ R6  ├─┬─────┐
                └─────┘ │     │
                        │  ┌──┴──┐
                        │  │ LED2│  Blue LED
                        │  └──┬──┘
                        │     │
                       GND ───┘
```

**Components:**
- R5, R6: 1kΩ, 1/4W (current limiting resistors)
- LED1: 5mm green LED (power indicator)
- LED2: 5mm blue LED (heartbeat/status)
- Forward voltage: ~2V
- Forward current: ~3mA

---

## 📊 Power Budget

**Current consumption analysis:**

| Component | Current (mA) | Notes |
|-----------|--------------|-------|
| Arduino Mega | 100-200 | Base consumption |
| Nextion Display | 100-300 | Varies with brightness |
| MAX31855 | 1-2 | Low power |
| MAP Sensor | 5-10 | Typical automotive sensor |
| LEDs (2x) | 6 | @ 3mA each |
| Buzzer | 30-40 | When active |
| **Total** | **250-550** | **Typical: 350mA** |

**LM2596 efficiency:** ~85% @ 3A load

**Vehicle battery draw:**
- 5V @ 350mA = 1.75W
- 12V @ 170mA (accounting for efficiency)
- Negligible compared to typical vehicle systems

---

## 🛡️ Protection Features

### Reverse Polarity Protection

**D1 (1N5822 Schottky Diode):**
- If 12V connected backward, diode blocks current
- Protects all downstream components
- Low forward voltage (0.5V) minimizes loss

### Overvoltage Protection

**LM2596 Module:**
- Input range: 9-40V
- Protects against voltage spikes
- Internal overcurrent protection

### Arduino Pin Protection

**Current Limiting:**
- All outputs have series resistors
- Prevents excessive current draw
- Protects Arduino GPIO pins

### ESD Protection

**Design considerations:**
- Screw terminals instead of exposed pins
- Enclosure provides shielding
- Ground plane on bottom layer

---

## 🔧 Design Specifications

### PCB Specifications

- **Layers:** 2 (top copper + bottom copper)
- **Dimensions:** 95mm × 65mm
- **Thickness:** 1.6mm (standard)
- **Copper weight:** 1 oz (35 µm)
- **Track width:**
  - Power (12V, 5V): 1.0mm (2A capability)
  - Signal: 0.4mm
  - Ground: Wide fills
- **Via size:** 0.8mm drill, 1.4mm pad
- **Clearance:** 0.25mm minimum

### Component Footprints

- **Resistors:** Through-hole, axial, 7mm spacing
- **Capacitors:** Radial, 2.5-5mm spacing
- **Diode:** DO-201 package
- **Terminals:** Phoenix-style, 5mm pitch
- **Headers:** 2.54mm pitch (0.1")
- **Module:** 2.54mm headers for LM2596

---

## 📐 PCB Layout Considerations

### Top Layer (Component Side)

- All components on top
- Screw terminals along edges
- Clear silkscreen labels
- Component values marked

### Bottom Layer (Solder Side)

- Continuous ground plane
- Power distribution traces
- Signal routing
- Thermal relief on ground pins

### Grounding

- Star ground configuration
- All grounds connect at single point
- Minimize ground loops
- Wide ground traces

### Noise Reduction

- Decoupling caps near each IC
- Short signal traces
- Separate analog and digital grounds
- Filter caps on all power rails

---

## 🔬 Testing Points

**Recommended test points on PCB:**

| Test Point | Location | Purpose |
|------------|----------|---------|
| TP1 | 12V input | Verify input voltage |
| TP2 | 5V output | Check regulator output |
| TP3 | GND | Ground reference |
| TP4 | IAT signal | Monitor thermistor voltage |
| TP5 | MAP signal | Monitor MAP sensor output |
| TP6 | A7 | Check battery divider |

---

## 📝 Manufacturing Notes

### PCB Fabrication

**Gerber files must include:**
- Top copper layer (.gtl)
- Bottom copper layer (.gbl)
- Top silkscreen (.gto)
- Bottom silkscreen (.gbo)
- Top soldermask (.gts)
- Bottom soldermask (.gbs)
- Board outline (.gko)
- Drill file (.txt or .drl)

### Assembly Notes

- **Lead-free solder recommended**
- Solder smallest components first
- Check polarity of all polarized parts
- Use flux for cleaner joints
- Inspect for cold solder joints
- Clean flux residue

### Quality Control

**Pre-delivery checks:**
- [ ] Visual inspection (no shorts, good joints)
- [ ] Continuity test (power rails)
- [ ] Resistance check (no shorts to ground)
- [ ] Voltage test (12V input, 5V output)
- [ ] Functional test with Arduino

---

## 🔄 Design Files

**For full schematic and PCB layout files:**

The actual design files would be created in EDA software:
- **KiCad:** Free, open-source, cross-platform
- **Eagle:** Popular, free for hobbyists
- **Altium:** Professional, expensive
- **EasyEDA:** Online, free

**File locations** (when created):
- `docs/shield/jnkr-shield.kicad_pro` - KiCad project
- `docs/shield/jnkr-shield.kicad_sch` - Schematic
- `docs/shield/jnkr-shield.kicad_pcb` - PCB layout
- `docs/shield/schematic.pdf` - Printable schematic
- `docs/shield/pcb-layout.pdf` - PCB visualization
- `docs/shield/gerbers/` - Fabrication files

---

## 🤝 Contributing

Want to improve the shield design?

1. Fork the repository
2. Make modifications in EDA software
3. Test thoroughly
4. Submit pull request with:
   - Updated schematic PDF
   - New gerber files
   - Description of changes
   - Test results

---

**For assistance with the shield design, open an issue on GitHub!**

---

**Last Updated:** 2025-11-07
