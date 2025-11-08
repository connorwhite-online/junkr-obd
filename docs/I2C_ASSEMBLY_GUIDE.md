# I2C Sensor Module Assembly Guide

**Step-by-step instructions for building the engine bay sensor module**

This guide walks you through assembling either the pre-built module version or custom PCB version of the I2C sensor consolidation module.

---

## 📋 Before You Start

### What You'll Build

A weatherproof module that mounts in the engine bay containing:
- ADS1115 ADC for analog sensors (thermistors + MAP sensor)
- MCP9600 thermocouple amplifier for EGT
- Voltage divider circuits for NTC thermistors
- Single Deutsch connector for harness to cabin

### Choose Your Path

**Path A: Pre-Built Modules** (Easier, ~3 hours)
- Use breakout boards on protoboard
- Good for beginners
- Hand-solderable

**Path B: Custom PCB** (More advanced, ~4 hours + PCB wait time)
- Order custom PCB from JLCPCB
- SMD soldering required
- Professional result

This guide covers **both paths**.

---

## 🛠️ Path A: Pre-Built Module Assembly

### Step 1: Gather Materials

**Check that you have everything:**

- [ ] ADS1115 breakout board (Adafruit or compatible)
- [ ] MCP9600 breakout board (Adafruit or compatible)
- [ ] Protoboard (7x5cm or larger)
- [ ] 3x 2.2kΩ resistors (1/4W through-hole)
- [ ] 5x 2-pin screw terminals (5.08mm pitch)
- [ ] 1x 3-pin screw terminal (5.08mm pitch)
- [ ] Deutsch DT 6-pin receptacle + male plug
- [ ] IP67 enclosure (100x68x50mm)
- [ ] 2x cable glands (PG9)
- [ ] Wire (22 AWG, various colors)
- [ ] Heat shrink tubing
- [ ] M3 standoffs and screws

**Tools:**
- [ ] Soldering iron
- [ ] Solder
- [ ] Wire strippers
- [ ] Flush cutters
- [ ] Multimeter
- [ ] Drill with step bit
- [ ] Helping hands (optional)

### Step 2: Plan the Layout

**Sketch your protoboard layout:**

```
┌─────────────────────────────────────────┐
│                                         │
│  [Terminal] [Terminal] [Terminal]      │  ← Thermistor inputs
│   IAT Pre     IAT Post    Coolant      │
│                                         │
│    [R1]        [R2]        [R3]        │  ← 2.2kΩ resistors
│   2.2kΩ       2.2kΩ       2.2kΩ        │
│                                         │
│  ┌─────────────────────┐               │
│  │   ADS1115 Module    │               │  ← ADC breakout
│  │   (with headers)    │               │
│  └─────────────────────┘               │
│                                         │
│  [Terminal]    ← MAP sensor             │
│                                         │
│  ┌──────────────────┐                  │
│  │  MCP9600 Module  │                  │  ← Thermocouple amp
│  └──────────────────┘                  │
│                                         │
│  [Terminal] ← EGT thermocouple         │
│                                         │
│  [Wires to Deutsch connector]          │
│                                         │
└─────────────────────────────────────────┘
```

**Tips:**
- Keep high-current traces wide (5V, GND)
- Keep analog inputs away from I2C lines
- Leave room for wire routing underneath

### Step 3: Solder Modules to Protoboard

**Order of assembly:**

**3.1: Install screw terminals first**
- Screw terminals are tallest, do these first
- Place on protoboard (terminals facing edge)
- Solder from bottom
- J2, J3, J4: IAT Pre, IAT Post, Coolant (2-pin each)
- J5: MAP sensor (3-pin)
- J6: EGT thermocouple (2-pin)

**3.2: Install voltage divider resistors**
- Place 2.2kΩ resistor near each thermistor terminal
- One end connects to 5V rail
- Other end connects to terminal and ADC input
- Solder in place

**3.3: Install pin headers for modules**
- Solder male pin headers to ADS1115 if not already
- Solder male pin headers to MCP9600 if not already
- Or solder female headers to protoboard (module plugs in)
- Or solder modules directly (permanent)

**Recommended:** Use female headers on protoboard so modules are removable

**3.4: Mount modules**
- Insert modules into headers
- Check that they're fully seated
- Solder pins from bottom (if permanent)

### Step 4: Wire Power Rails

**Create power distribution:**

```
5V Rail (Red wire):
  Deutsch Pin 1 ──┬── ADS1115 VDD
                  ├── MCP9600 VDD  
                  ├── MAP sensor 5V
                  └── Voltage divider resistors (3x)

GND Rail (Black wire):
  Deutsch Pin 2 ──┬── ADS1115 GND
                  ├── MCP9600 GND
                  ├── MAP sensor GND
                  └── Thermistor common (3x)
```

**Wiring technique:**
- Use bus strips on protoboard if available
- Or run solid core wire along edges as power rails
- Solder jumpers from rail to each component
- Keep wires flat against board

### Step 5: Wire I2C Bus

**Connect I2C lines:**

```
SCL (Yellow wire):
  Deutsch Pin 3 ──┬── ADS1115 SCL
                  └── MCP9600 SCL

SDA (Green wire):
  Deutsch Pin 4 ──┬── ADS1115 SDA
                  └── MCP9600 SDA
```

**Notes:**
- I2C lines share between both modules
- Keep wires as short as possible
- Twisted pair is ideal but not required for short distances

### Step 6: Wire Voltage Dividers

**For each thermistor input (repeat 3x):**

```
Thermistor Input Example (IAT Pre):

5V rail ─── R1 (2.2kΩ) ───┬─── ADS1115 A0
                          │
                      Terminal J2
                          │
GND rail ─────────────────┴─── (through thermistor)
```

**Connections:**
- Top of resistor → 5V rail
- Bottom of resistor → Terminal + ADC channel
- Terminal other pin → GND rail
- When thermistor connects, it completes the divider

**Channel assignments:**
- J2 (IAT Pre) → ADS1115 A0
- J3 (IAT Post) → ADS1115 A1
- J4 (Coolant) → ADS1115 A2

### Step 7: Wire MAP Sensor Input

```
MAP Sensor (J5):
  Pin 1: 5V  ─── 5V rail
  Pin 2: SIG ─── ADS1115 A3
  Pin 3: GND ─── GND rail
```

**Note:** Verify your MAP sensor pinout - may vary by manufacturer!

### Step 8: Wire Thermocouple Input

```
EGT Thermocouple (J6):
  Pin 1: TC+ ─── MCP9600 T+
  Pin 2: TC- ─── MCP9600 T-
```

**CRITICAL:** Polarity matters!
- Yellow wire (K-type) → T+
- Red wire (K-type) → T-

### Step 9: Prepare Deutsch Connector

**Crimping pins:**

1. Strip wire 3-4mm
2. Insert wire into pin
3. Crimp with Deutsch crimping tool
4. Tug test - should not pull out
5. Repeat for 6 wires

**Pin assignments:**

| Pin | Wire Color | Function |
|-----|------------|----------|
| 1 | Red | 5V Power |
| 2 | Black | Ground |
| 3 | Yellow | I2C SCL |
| 4 | Green | I2C SDA |
| 5 | Bare | Shield (optional) |
| 6 | - | Spare |

**Assembly:**
1. Insert crimped pins into receptacle housing
2. Listen for click (pin locks in place)
3. Verify each pin with pull test
4. Add wedge lock or secondary lock if provided

**Soldering wires to protoboard:**
- Solder 6 wires from Deutsch connector to protoboard
- Use strain relief (zip tie or hot glue)
- Leave 4-6 inches of wire inside enclosure

### Step 10: Testing (Bench Test)

**DO NOT SKIP THIS STEP!**

**Visual inspection:**
- [ ] No solder bridges
- [ ] All components soldered
- [ ] Correct polarity on all connections
- [ ] Neat, secure wiring

**Continuity tests (power OFF):**

1. **Check power rails:**
   - Multimeter in continuity mode
   - 5V to GND: Should be >1kΩ (not a short!)
   - If <100Ω, find and fix short circuit

2. **Check connections:**
   - Deutsch Pin 1 to ADS1115 VDD: 0Ω (beep)
   - Deutsch Pin 2 to ADS1115 GND: 0Ω (beep)
   - Deutsch Pin 3 to ADS1115 SCL: 0Ω (beep)
   - Deutsch Pin 4 to ADS1115 SDA: 0Ω (beep)

**Power-on test (5V bench supply):**

1. Connect 5V power supply (set current limit to 500mA)
2. Apply 5V to Deutsch pins 1 (+) and 2 (-)
3. Check current: Should be 30-80mA
4. Measure voltages:
   - ADS1115 VDD: 5.0V ±0.2V
   - MCP9600 VDD: 5.0V ±0.2V
   - Voltage dividers: ~2.5V (no thermistors connected)

**I2C device detection:**

Connect to Arduino/ESP32 and run I2C scanner:
- Expected: Devices at 0x48 (ADS1115) and 0x60 (MCP9600)
- If not found: Check I2C wiring, check solder joints

### Step 11: Enclosure Preparation

**Drill cable entry holes:**

1. Mark locations for 2x cable glands
   - One for sensor wires (left side)
   - One for harness (right side)
2. Use step drill bit for clean holes (15mm for PG9)
3. Deburr holes with file or sandpaper
4. Clean out plastic chips

**Mount protoboard:**

**Option A: Standoffs**
- Drill 4x M3 mounting holes in enclosure base
- Install M3 x 10mm brass standoffs
- Secure board with M3 screws from top

**Option B: Double-sided tape**
- Clean enclosure base with isopropyl alcohol
- Apply 3M VHB tape to back of protoboard
- Press firmly into enclosure
- Prevents vibration damage

### Step 12: Install Cable Glands

**For each gland:**

1. Insert gland from outside of enclosure
2. Thread lock nut from inside
3. Thread sensor wires (or harness) through gland
4. Tighten compression nut finger-tight
5. Add thread locker or silicone sealant
6. Tighten with wrench (don't over-tighten!)

**Test seal:**
- Spray water on outside
- Check for leaks inside
- Re-seal if necessary

### Step 13: Final Assembly

1. **Route wires** through cable glands
2. **Add strain relief** (zip tie inside before gland)
3. **Close enclosure** (check gasket is clean and seated)
4. **Install screws** (tighten in X pattern)
5. **Label** connector and enclosure
6. **Add zip tie** to harness connector

### Step 14: Waterproofing (Optional but Recommended)

**Conformal coating:**
- Spray PCB with MG Chemicals 422B or similar
- Avoid spraying connectors
- Let dry 24 hours
- Adds extra moisture protection

**Additional sealing:**
- Silicone sealant around cable glands
- Silicone inside screw holes
- Check gasket periodically

---

## 🔬 Path B: Custom PCB Assembly

### Step 1: Order PCB

**Upload to JLCPCB:**
- Go to jlcpcb.com
- Upload `JNKR_Sensor_Module_v1.0_Gerber.zip`
- Configure options (see CUSTOM_PCB_DESIGN.md)
- Order 5 boards minimum
- Wait 1-2 weeks for delivery

### Step 2: Order Components

**From Digi-Key / Mouser:**
- See BOM in CUSTOM_PCB_DESIGN.md
- Order all SMD components
- Order screw terminals and Deutsch connector
- Total: ~$30 per board

**Tip:** Order extras (5-10% overage) for mistakes

### Step 3: SMD Soldering (Advanced)

**Soldering order:**

**3.1: Smallest components first**
- Resistors (0805)
- Capacitors (0805)
- Diodes (SOD-323, SMC)
- LED

**Technique:**
1. Apply flux to pads
2. Tin one pad with small amount of solder
3. Place component with tweezers
4. Reheat tinned pad while holding component
5. Solder other side
6. Reheat first side with more solder if needed

**3.2: ICs**
- Apply flux to all pads
- Align IC carefully (use magnifying glass)
- Tack one corner pin
- Check alignment, adjust if needed
- Solder opposite corner
- Solder all pins (drag soldering)
- Check for bridges with multimeter
- Use solder wick to remove bridges

**3.3: Through-hole components**
- Screw terminals
- Deutsch connector
- Electrolytic capacitor (watch polarity!)

**Tips:**
- Work under good lighting
- Use magnification (10x loupe or microscope)
- Take breaks to rest eyes
- Use flux liberally
- Clean with isopropyl alcohol after soldering

### Step 4: Inspection

**Under magnification, check:**
- [ ] All pads have solder
- [ ] No cold joints (dull, grainy = bad)
- [ ] No bridges between adjacent pins
- [ ] Correct component orientation
- [ ] ICs aligned properly

**Test with multimeter:**
- [ ] Power to GND: >1kΩ (no shorts)
- [ ] All ground pins connected: 0Ω
- [ ] All VDD pins connected: 0Ω

### Step 5: Programming Test Points

**If you added test points on PCB:**
- Connect 5V power
- Probe each test point
- Verify correct voltages
- Check I2C signals with logic analyzer (optional)

### Step 6: Enclosure and Final Assembly

**Follow same steps as Path A:**
- Drill cable gland holes
- Mount PCB with standoffs
- Install cable glands
- Route wires
- Seal enclosure

**Differences:**
- PCB has mounting holes (easier!)
- Deutsch connector already on board
- Cleaner, more professional appearance

---

## 🔌 Harness Fabrication

### Cable Selection

**Recommended: 4-conductor shielded cable**
- Belden 8723 or equivalent
- 22 AWG stranded copper
- Foil + braid shield
- Jacket rated for automotive use

**Budget option:**
- 4-conductor unshielded
- Or use individual wires with spiral wrap

**Length:**
- Measure from engine bay module location to display
- Add 2-3 feet extra for routing and service loops
- Typical: 6-10 feet

### Harness Assembly

**1. Cut cable to length**

**2. Prepare engine bay end:**
- Strip outer jacket 2 inches
- Separate conductors
- Strip each wire 3-4mm
- Crimp Deutsch pins
- Assemble into Deutsch male plug
- Add heat shrink over back of connector

**3. Prepare cabin end:**
- Strip outer jacket 2 inches
- Separate conductors
- Strip and tin wires
- Connect to ESP32/Arduino:
  - Red → 5V output (from buck converter)
  - Black → GND
  - Yellow → GPIO 22 (SCL)
  - Green → GPIO 21 (SDA)
- Optional: Add JST or Molex connector for easy removal

**4. Shield connection:**
- Connect shield to GND at cabin end only
- Leave floating at engine bay end
- This prevents ground loops

**5. Label harness:**
- Use label maker or write on heat shrink
- Mark both ends with function
- Example: "ENGINE SENSORS - I2C"

**6. Protection:**
- Add spiral wrap or split loom tubing
- Protect from abrasion and heat
- Zip tie to existing harnesses every 12 inches

---

## 🚗 Vehicle Installation

### Module Mounting Location

**Ideal locations:**
- Inner fender well (behind headlight)
- Firewall (upper corner away from heat)
- Side of battery tray

**Requirements:**
- Protected from direct water spray
- Away from exhaust/turbo (>150°C)
- Accessible for service
- Solid mounting surface

**Mounting methods:**
- M4 or M5 bolts through enclosure mounting tabs
- L-brackets if no flat surface
- Self-tapping screws into plastic
- Avoid: Zip ties only (vibration will loosen)

### Sensor Connections

**Connect sensors to module terminals:**

| Terminal | Sensor | Wire Length |
|----------|--------|-------------|
| J2 | IAT Pre-IC NTC | 2-4 feet |
| J3 | IAT Post-IC NTC | 2-4 feet |
| J4 | Coolant NTC | 3-6 feet |
| J5 | MAP sensor | 1-2 feet |
| J6 | EGT K-type | 6-10 feet |

**Tips:**
- Use automotive-grade wire (high-temp rated)
- Twist pairs to reduce noise
- Route away from ignition wires
- Secure with zip ties every 12 inches
- Add grommets where wires pass through sheet metal
- Use heat-resistant sleeving near exhaust

### Firewall Penetration

**Clean installation:**

1. Find existing grommet or create new hole
2. Use rubber grommet for wire protection
3. Route harness through firewall
4. Add extra grommet inside (double seal)
5. Seal with silicone sealant
6. Route to display/ESP32 location

**Alternative:**
- Use existing harness passage
- Tag along with other wires
- Secure separately once inside

### Power Connection

**Engine bay module power:**
- 5V from display/ESP32
- Runs back through harness
- No separate 12V needed in engine bay!

**If module is far from display:**
- Consider 12V feed in engine bay
- Add buck converter inside module enclosure
- Requires larger enclosure and more complexity

---

## ✅ Testing and Validation

### Bench Test (Before Vehicle)

**1. Power test:**
- Connect 5V bench supply
- Check current draw: 30-80mA idle
- Module should not get hot

**2. I2C detection:**
- Connect to Arduino/ESP32
- Run I2C scanner
- Verify devices at 0x48 and 0x60

**3. Sensor simulation:**
- Connect 2.2kΩ resistor to thermistor input
- Should read ~25°C
- Connect MAP sensor
- Should read atmospheric pressure
- Touch thermocouple
- Should read body temperature

### Vehicle Installation Test

**1. Install module in engine bay**
- Mount securely
- Connect all sensors
- Route harness through firewall

**2. Connect to display/ESP32**
- Match wire colors correctly
- Double-check polarity (5V/GND)
- Secure connector

**3. Power on system**
- Turn ignition to ON (engine off)
- Check serial monitor for I2C detection
- Verify all sensors reading
- Check for fault codes

**4. Static readings:**
- IAT: Should read ambient temperature
- Coolant: Should read engine temperature (cold)
- EGT: Should read low (engine off)
- Boost: Should read ~0 PSI

**5. Start engine:**
- All readings should update
- No fault messages
- Display shows live data

**6. Drive test:**
- Monitor under load
- Check boost reading under acceleration
- Verify EGT rises appropriately
- No intermittent faults

---

## 🐛 Troubleshooting

### Problem: No I2C devices detected

**Check:**
- [ ] Power: 5V at module
- [ ] Ground: Continuity to ESP32
- [ ] I2C wires: Not swapped (SCL/SDA)
- [ ] Pullup resistors: Present on ADS1115/MCP9600 boards
- [ ] Cable length: <10 feet ideal
- [ ] Solder joints: Reflow if questionable

**Solution:**
- Use I2C scanner to identify address
- Try shorter cable temporarily
- Add external 4.7kΩ pullup resistors if needed

### Problem: Incorrect temperature readings

**Check:**
- [ ] Resistor values: 2.2kΩ confirmed
- [ ] Thermistor: Measure resistance (~2.2kΩ at room temp)
- [ ] Voltage divider: Should read ~2.5V at 25°C
- [ ] ADC gain setting: Correct in code

**Solution:**
- Measure voltage at ADC input with multimeter
- Calculate expected temperature
- Compare to displayed value
- Adjust calibration offset if needed

### Problem: EGT reading wrong

**Check:**
- [ ] Thermocouple polarity: Yellow = +, Red = -
- [ ] K-type wire used: Not regular wire!
- [ ] MCP9600 type: Set to K-type in code
- [ ] Thermocouple intact: Check for opens

**Solution:**
- Swap thermocouple leads if backward
- Replace extension wire with proper K-type
- Verify with handheld thermometer

### Problem: Intermittent faults

**Check:**
- [ ] Loose connections: Wiggle test
- [ ] Vibration: Add foam padding
- [ ] Water ingress: Check seals
- [ ] Wire chafing: Inspect harness
- [ ] Connector: Clean and reseat

**Solution:**
- Secure all connections with lock washers
- Add conformal coating
- Improve waterproofing
- Reroute harness away from vibration sources

---

## 📊 Quality Checklist

### Assembly Quality

- [ ] All solder joints shiny and smooth
- [ ] No cold joints (dull, grainy appearance)
- [ ] No solder bridges between pads
- [ ] All components correct orientation
- [ ] All wires strain-relieved
- [ ] Proper wire gauge used (22 AWG minimum)

### Electrical

- [ ] No short circuits (5V to GND >1kΩ)
- [ ] All grounds continuous (0Ω)
- [ ] I2C pullups present (4.7kΩ to 5V)
- [ ] Proper polarity on all connections
- [ ] Deutsch connector crimped correctly

### Mechanical

- [ ] Enclosure gasket clean and seated
- [ ] Screws tight (no over-tightening)
- [ ] Cable glands sealed properly
- [ ] Module mounted securely
- [ ] No loose components rattling inside

### Waterproofing

- [ ] All cable entries sealed
- [ ] Gasket in good condition
- [ ] Conformal coating applied (if used)
- [ ] Tested with water spray (before final install)

### Functional

- [ ] I2C devices detected
- [ ] All sensors reading correctly
- [ ] No fault codes on startup
- [ ] Values update in real-time
- [ ] Readings stable (not noisy)

---

## 📸 Documentation

**Take photos of:**
- Completed protoboard (top and bottom)
- Wiring inside enclosure
- Installed module in engine bay
- Sensor connections
- Harness routing

**Why?**
- Reference for future service
- Troubleshooting
- Community contributions
- Show off your work! 😊

---

## 🎉 Completion

**Congratulations!** You've built a professional-grade automotive sensor module!

### What You've Accomplished

✅ Consolidated 15+ wires into 4-wire harness
✅ Single connector through firewall
✅ Digital I2C communication (noise immune)
✅ Weatherproof engine bay installation
✅ Automotive-grade construction

### Next Steps

1. **Calibration:** See CALIBRATION.md for sensor tuning
2. **Data logging:** Add SD card or wireless logging
3. **Contribute:** Share photos on GitHub!
4. **Enjoy:** Monitor your engine with confidence

---

## 🔗 Related Documents

- [I2C_SENSOR_MODULE.md](I2C_SENSOR_MODULE.md) - Design overview
- [I2C_SHOPPING_LIST.md](I2C_SHOPPING_LIST.md) - Parts and links
- [CUSTOM_PCB_DESIGN.md](CUSTOM_PCB_DESIGN.md) - PCB specifications
- [CALIBRATION.md](CALIBRATION.md) - Sensor calibration
- [WIRING.md](WIRING.md) - Original wiring guide

---

**Need help?** Post photos and description to GitHub Issues - the community is here to help!

**Found an improvement?** Submit a pull request - make this guide even better!

---

*This build should take 3-4 hours for experienced builders, or a fun weekend project for beginners. Take your time, double-check everything, and enjoy the process!* 🔧⚡🚗
