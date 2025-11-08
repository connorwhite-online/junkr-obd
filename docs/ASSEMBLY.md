# Assembly & Installation Guide - JNKR Gauge System (Legacy)

> ⚠️ **DEPRECATED**: This guide is for the **legacy Arduino Mega** setup.
> 
> **Current Platform**: Qualia ESP32-S3 with 2.1" Round RGB Display
> 
> **See instead**: [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md) and [ESP32_SETUP.md](ESP32_SETUP.md) for current assembly.
>
> This document is kept for reference only.

---

**Step-by-Step Instructions from Parts to Running System (Legacy Arduino Mega)**

This guide walks you through the complete assembly and installation process for the legacy Arduino Mega version, from building the enclosure to mounting sensors in your vehicle.

---

## 📋 Table of Contents

- [Before You Start](#before-you-start)
- [Phase 1: Enclosure Preparation](#phase-1-enclosure-preparation)
- [Phase 2: Component Assembly](#phase-2-component-assembly)
- [Phase 3: Wiring and Connections](#phase-3-wiring-and-connections)
- [Phase 4: Software Upload](#phase-4-software-upload)
- [Phase 5: Bench Testing](#phase-5-bench-testing)
- [Phase 6: Vehicle Sensor Installation](#phase-6-vehicle-sensor-installation)
- [Phase 7: Final Installation](#phase-7-final-installation)
- [Testing and Calibration](#testing-and-calibration)

---

## ⚠️ Before You Start

### Prerequisites

✅ All parts received (see [BOM.md](BOM.md))  
✅ Tools assembled  
✅ Workspace prepared  
✅ Read through entire guide once  
✅ Wiring guide reviewed ([WIRING.md](WIRING.md))

### Safety Reminders

- Disconnect vehicle battery before working
- Work in well-ventilated area
- Wear safety glasses
- Have fire extinguisher nearby
- Never work alone on critical systems

### Time Requirements

| Phase | Time Estimate | Can Pause? |
|-------|---------------|------------|
| Phase 1: Enclosure | 1-2 hours | Yes |
| Phase 2: Component Assembly | 2-3 hours | Yes |
| Phase 3: Wiring | 3-4 hours | Yes |
| Phase 4: Software | 30 minutes | No |
| Phase 5: Bench Testing | 1 hour | Yes |
| Phase 6: Sensor Installation | 4-6 hours | Yes |
| Phase 7: Final Install | 2-3 hours | Yes |
| **Total** | **13-19 hours** | - |

**Recommended**: Spread over 2-3 weekends

---

## 🔧 Phase 1: Enclosure Preparation

### Step 1.1: Plan the Layout

**Goal**: Determine where each component goes inside the enclosure.

1. **Gather all components:**
   - Arduino Mega
   - Buck converter
   - Nextion display
   - Screw terminals
   - Buzzer

2. **Do a dry fit:**
   - Place Arduino in enclosure bottom
   - Position buck converter away from Arduino
   - Check clearances
   - Allow space for wiring

3. **Mark positions with tape or marker**

**Layout Example:**

```
┌─────────────────────────────────────────┐
│  [Display cutout - front panel]         │
│                                          │
│  ┌────────────┐    ┌───────┐            │
│  │  Arduino   │    │ Buck  │  [Buzzer]  │
│  │   Mega     │    │ Conv. │            │
│  │   2560     │    └───────┘            │
│  └────────────┘                         │
│                                          │
│  [Screw Terminals] ──────────────────   │
│                                          │
│  [Cable glands: bottom or sides] ─────  │
└─────────────────────────────────────────┘
```

### Step 1.2: Cut Display Opening

**Tools needed:**
- Drill with step bit or hole saw
- Jigsaw or rotary tool (Dremel)
- File or sandpaper
- Ruler and marker

**Procedure:**

1. **Measure display dimensions:**
   - Nextion 3.5": ~88mm x 59mm visible area
   - Add 2mm for mounting clearance: 90mm x 61mm

2. **Mark cutting area on enclosure front:**
   - Use ruler for straight lines
   - Double-check measurements
   - Mark "X" in waste area

3. **Drill corner holes:**
   - 8mm drill bit
   - One hole in each corner
   - Inside the cut line

4. **Cut opening:**
   - Use jigsaw with fine-tooth blade
   - Cut slowly, support material
   - Stay slightly inside line

5. **Test fit display:**
   - Should fit snugly
   - File or sand as needed

6. **Smooth edges:**
   - File sharp edges
   - Sand smooth
   - Clean dust completely

### Step 1.3: Drill Mounting Holes

**For Arduino Mega (4 holes):**

1. Place Arduino in desired position
2. Mark through mounting holes
3. Drill 3mm pilot holes
4. Install M3 standoffs (10mm height)

**For Buck Converter:**

1. Mark mounting holes
2. Drill 3mm holes
3. Use standoffs or direct mount with screws

**For Screw Terminals:**

1. Position along one edge
2. Can use screws or double-sided tape
3. Ensure accessible with wires connected

### Step 1.4: Install Cable Glands

**Locations:**
- Bottom or side (not top - prevents water entry)
- One for power (12V input)
- One for sensors (multiple wires)
- One for spare/future use

**Installation:**

1. Drill appropriate size hole (check gland spec)
2. Install gland from outside
3. Tighten nut inside
4. Test seal with gentle pull

### Step 1.5: Enclosure Preparation Complete

**Checklist:**
- [ ] Display opening cut and finished
- [ ] Arduino standoffs installed
- [ ] Buck converter mount ready
- [ ] Screw terminals mounted
- [ ] Cable glands installed
- [ ] All holes deburred and smooth
- [ ] Enclosure cleaned (no metal shavings)

---

## 🛠️ Phase 2: Component Assembly

### Step 2.1: Arduino Mega Preparation

1. **Inspect Arduino:**
   - Check for damage
   - Verify all pins present
   - Test with USB cable (LED should light)

2. **Install onto standoffs:**
   - Align mounting holes
   - Use M3 screws
   - Don't overtighten (can crack PCB)

3. **Test installation:**
   - Should be secure
   - No flex when pressing on board
   - All pins accessible

### Step 2.2: Buck Converter Setup

**CRITICAL:** Configure output voltage BEFORE connecting to Arduino!

1. **Wire to power supply:**
   - 12V power supply or car battery
   - Connect input terminals: +12V and GND
   - **VERIFY POLARITY**

2. **Adjust output voltage:**
   - Locate adjustment potentiometer (small screw)
   - Connect multimeter to output terminals
   - Turn pot clockwise to increase voltage
   - Set to exactly 5.0V (±0.1V acceptable)

3. **Disconnect power**

4. **Mount in enclosure:**
   - Use standoffs or screws
   - Keep away from Arduino (electrical noise)
   - Ensure heatsink has airflow

### Step 2.3: Screw Terminals Installation

1. **Arrange terminals in groups:**
   - Power group (12V, GND, 5V)
   - Sensor group (IAT, Coolant, Oil)
   - MAP sensor group (5V, Signal, GND)
   - EGT group (to MAX31855 module)

2. **Label terminals:**
   - Use tape and marker
   - Or print labels
   - Clear, readable text

3. **Secure terminals:**
   - Mount to enclosure or DIN rail
   - Ensure stable, won't move

### Step 2.4: Nextion Display Mounting

1. **Test display first:**
   - Connect 5V and GND temporarily
   - Should show boot screen
   - Disconnect after test

2. **Mount in opening:**
   - From inside or outside (depends on display)
   - Secure with screws or hot glue
   - Ensure screen is centered and level

3. **Cable management:**
   - Route 4-wire cable to Arduino
   - Leave some slack
   - Secure with zip tie

### Step 2.5: MAX31855 Module Preparation

1. **Inspect module:**
   - Check for solder bridges
   - Verify all pins present

2. **Optional: Add headers:**
   - Solder pin headers if not included
   - Makes connection easier

3. **Mounting:**
   - Use double-sided foam tape
   - Or small standoffs
   - Keep accessible for thermocouple connection

4. **Initial wire connections:**
   - Cut wires to appropriate length
   - Strip and tin ends
   - Connect to module: VCC, GND, DO, CS, CLK

### Step 2.6: Buzzer Installation

1. **Identify polarity:**
   - Look for + marking
   - Red wire = positive

2. **Solder wires if needed:**
   - Red wire to +
   - Black wire to -
   - Heat shrink connections

3. **Mount buzzer:**
   - Use double-sided tape
   - Face sound port toward opening (not against wall)
   - In upper corner of enclosure

---

## 🔌 Phase 3: Wiring and Connections

**IMPORTANT:** Follow [WIRING.md](WIRING.md) for detailed instructions on each connection.

### Step 3.1: Power Distribution Wiring

This is the most critical section. Take your time!

**Buck Converter to Arduino:**

1. **5V Power Rail:**
   ```
   Buck Converter (+OUT) → Arduino 5V pin
   Buck Converter (-OUT) → Arduino GND pin
   ```
   - Use 22 AWG wire
   - Red for positive, black for negative
   - Solder connections (or use Dupont connectors)
   - Heat shrink all joints

2. **12V Input Terminals:**
   ```
   Screw Terminal (+12V) → Buck Converter (+IN)
   Screw Terminal (GND)  → Buck Converter (-IN)
   ```
   - Use 20 AWG wire
   - Solid connections, no loose strands
   - Tighten screw terminals firmly

### Step 3.2: Sensor Power Distribution

From Arduino 5V and GND, create distribution points:

**Option A: Screw Terminal Distribution**
```
Arduino 5V → Terminal Block → To all sensors
Arduino GND → Terminal Block → To all sensors
```

**Option B: Direct Wiring**
- Wire each sensor's power directly to Arduino
- More wires but simpler troubleshooting

### Step 3.3: Thermistor Voltage Dividers

**For each NTC thermistor (IAT, Coolant, Oil):**

1. **Build voltage divider:**
   ```
   Arduino 5V ─┬─ [2.2kΩ Resistor] ─┬─ Arduino Analog Pin
               │                      │
               └─ [NTC Thermistor] ───┴─ Arduino GND
   ```

2. **Procedure for IAT (Pin A0):**
   - Solder 2.2kΩ resistor to breadboard or wire
   - One resistor end → Arduino 5V
   - Other resistor end → Arduino A0
   - NTC wire 1 → Resistor-A0 junction
   - NTC wire 2 → Arduino GND
   - Heat shrink all connections

3. **Repeat for:**
   - Coolant (Pin A1)
   - Oil (Pin A2, if used)

4. **Wire to screw terminals:**
   - Thermistor wires from sensors connect to terminals
   - Labeled: "IAT 1", "IAT 2", etc.

### Step 3.4: MAP Sensor Wiring

**MAP sensor has 3 wires:**

1. **Connections:**
   ```
   MAP Pin 1 (5V)     → Arduino 5V (via terminal)
   MAP Pin 2 (Signal) → Arduino A3
   MAP Pin 3 (GND)    → Arduino GND (via terminal)
   ```

2. **Implementation:**
   - Use 3-conductor cable (or 3 separate wires)
   - Solder or use connectors at both ends
   - Label wires
   - Route to screw terminals

### Step 3.5: EGT (MAX31855) Wiring

**MAX31855 to Arduino:**

1. **Module Power:**
   ```
   MAX31855 VCC → Arduino 5V
   MAX31855 GND → Arduino GND
   ```

2. **SPI Communication:**
   ```
   MAX31855 DO  → Arduino Pin 50 (MISO)
   MAX31855 CS  → Arduino Pin 10
   MAX31855 CLK → Arduino Pin 52 (SCK)
   ```

3. **Wire colors (suggested):**
   - VCC: Red
   - GND: Black
   - DO: Green
   - CS: Yellow
   - CLK: Blue

4. **Thermocouple connection:**
   - Route K-type thermocouple wires to enclosure
   - Through cable gland
   - Connect to MAX31855 T+ and T- terminals
   - **DO NOT reverse polarity**
   - Yellow wire usually = T+
   - Red wire usually = T-

### Step 3.6: Battery Voltage Divider

**Build the 10kΩ + 2.2kΩ divider:**

1. **Solder resistors in series:**
   ```
   12V input ─── [10kΩ] ───┬─── Arduino A7
                            │
                          [2.2kΩ]
                            │
                           GND ─── Arduino GND
   ```

2. **Wire to terminals:**
   - 12V input wire to screw terminal (labeled "12V Sense")
   - Junction to Arduino A7
   - GND to Arduino GND

3. **Heat shrink everything**

### Step 3.7: Nextion Display

**4-wire connection:**

```
Nextion Pin 1 (5V)   [Red]    → Arduino 5V
Nextion Pin 2 (TX)   [Blue]   → Arduino Pin 16 (RX2)
Nextion Pin 3 (RX)   [Yellow] → Arduino Pin 17 (TX2)
Nextion Pin 4 (GND)  [Black]  → Arduino GND
```

**Note:** TX goes to RX, RX goes to TX (crossover)

### Step 3.8: Buzzer

**2-wire connection:**

```
Buzzer (+) [Red]   → Arduino Pin 8
Buzzer (-) [Black] → Arduino GND
```

### Step 3.9: External Wiring Preparation

**Prepare wires that will exit enclosure:**

1. **12V Power Input:**
   - Red and black wire, 20 AWG
   - Long enough to reach fuse box
   - Install inline fuse holder on red wire
   - Through cable gland

2. **Sensor Wires:**
   - Group by sensor
   - Label each wire clearly
   - Through cable gland
   - Leave extra length (easier to route)

### Step 3.10: Wiring Verification

**Before proceeding, verify:**

- [ ] All solder joints shiny and solid
- [ ] All connections heat-shrunk
- [ ] No bare wire exposed
- [ ] Polarity correct on all components
- [ ] No shorts between 5V and GND
- [ ] No shorts between 12V and GND
- [ ] All wires labeled or color-coded
- [ ] Strain relief on all cables
- [ ] Cable glands tightened
- [ ] Enclosure can close without pinching wires

---

## 💻 Phase 4: Software Upload

### Step 4.1: Install Arduino IDE

1. **Download:**
   - Go to [arduino.cc/en/software](https://www.arduino.cc/en/software)
   - Download version for your OS
   - Install with default options

2. **Test IDE:**
   - Open Arduino IDE
   - Go to File → Examples → 01.Basics → Blink
   - Verify it compiles (checkmark button)

### Step 4.2: Configure Arduino IDE

1. **Select Board:**
   - Tools → Board → Arduino AVR Boards → Arduino Mega or Mega 2560

2. **Select Processor:**
   - Tools → Processor → ATmega2560 (Mega 2560)

3. **Select Port:**
   - Connect Arduino via USB
   - Tools → Port → COMx (Windows) or /dev/ttyUSBx (Linux)
   - Should say "Arduino Mega" next to port

### Step 4.3: Upload JNKR Gauge Code

1. **Open project:**
   - File → Open → `jnkr-gauge.ino`
   - Should open multiple tabs

2. **Verify all files present:**
   - `jnkr-gauge.ino` (main file)
   - `config.h`
   - `sensors.h` / `sensors.cpp`
   - `display.h` / `display.cpp`
   - `alerts.h` / `alerts.cpp`

3. **Upload:**
   - Click Upload button (→)
   - Wait for "Done uploading"
   - Status LED on Arduino should blink

4. **Open Serial Monitor:**
   - Tools → Serial Monitor
   - Set baud rate to **115200**
   - Should see initialization messages

---

## 🧪 Phase 5: Bench Testing

**Goal:** Verify all functionality before installing in vehicle.

### Step 5.1: Power-On Test

1. **Connect 12V power supply:**
   - Use bench power supply or 12V battery
   - Connect to 12V input terminal
   - Observe current draw (~0.5-1A typical)

2. **Check for issues:**
   - No smoke
   - No hot components
   - No burning smell
   - Arduino LED on
   - Display powered on

3. **Serial Monitor output:**
   - Should show:
   ```
   JNKR Gauge System
   Version: 1.0.0
   Initializing modules...
     [OK] Sensors
     [OK] Display
     [OK] Alerts
   System Ready!
   ```

### Step 5.2: Display Test

1. **Check display:**
   - Should show "JNKR Gauge" or similar
   - Should show gauge values
   - Touch screen should respond (if applicable)

2. **Gauge readings:**
   - Should show current sensor values
   - May not be accurate (sensors not installed)
   - But should not show errors

### Step 5.3: Sensor Continuity Tests

**Thermistor Test:**

1. **Touch IAT thermistor with fingers:**
   - Temperature should increase
   - Should see change on display
   - Should see change in Serial Monitor

2. **Repeat for coolant and oil thermistors**

**MAP Sensor Test:**

1. **Should read ~1.0 bar (atmospheric)**
2. **Cover sensor port:**
   - Lightly blow into port
   - Should see pressure increase
   - Or suck on port
   - Should see pressure decrease

**EGT Test:**

1. **Should read room temperature (~20-25°C)**
2. **Heat thermocouple tip with lighter:**
   - Temperature should increase rapidly
   - Can reach 200-400°C
   - Let cool before proceeding

**Battery Voltage Test:**

1. **Should read 12V input:**
   - Compare to multimeter reading
   - Should be within 0.5V
   - If off, check voltage divider resistors

### Step 5.4: Alert System Test

1. **Test buzzer:**
   - Should beep once on startup
   - Listen for single short beep

2. **Trigger warning alert:**
   - Heat EGT sensor to >600°C with lighter
   - Should hear 2 beeps
   - Display should show yellow warning

3. **Trigger critical alert:**
   - Heat EGT to >650°C
   - Should hear 3 beeps
   - Display should show orange/red alert

4. **Let cool and verify:**
   - Alert should clear
   - Beeping should stop
   - Display should return to green

### Step 5.5: Serial Monitor Verification

**Check output every second:**

```
========== Engine Status ==========
Boost:    0.0 PSI  (1.00 bar)
Intake:   23.5 C
Exhaust:  25.0 C
Coolant:  24.8 C
Oil:      24.2 C
Battery:  12.4 V
Alert:    None (All OK)
===================================
```

**Verify:**
- [ ] All sensors reading
- [ ] No fault messages
- [ ] Reasonable values
- [ ] Display matches serial output

### Step 5.6: Bench Test Complete

**If all tests pass:**
- System is ready for vehicle installation
- Close enclosure
- Prepare for sensor installation

**If tests fail:**
- Review [Troubleshooting](../README.md#troubleshooting) section
- Check wiring with multimeter
- Verify sensor connections
- Re-upload code if needed

---

## 🚗 Phase 6: Vehicle Sensor Installation

**⚠️ DISCONNECT BATTERY BEFORE WORKING ON VEHICLE**

### Step 6.1: EGT Sensor Installation

**Location:** Exhaust manifold or downpipe, 6-12" from turbo outlet

**Requirements:**
- M6 or 1/8" NPT weld bung
- Professional welding recommended
- Or use exhaust clamp with bung

**Procedure:**

1. **Mark location:**
   - 6-12 inches from turbo outlet
   - Accessible for wiring
   - Clear of other components

2. **Install bung:**
   - Weld bung to exhaust pipe
   - Ensure no leaks
   - Angle slightly downstream

3. **Install thermocouple:**
   - Thread into bung
   - Tighten with wrench
   - Use anti-seize compound
   - Probe tip should be in exhaust stream center

4. **Route wiring:**
   - Use high-temp wire sleeving
   - Route away from hot surfaces
   - Secure with zip ties to existing harness
   - Route to enclosure location

### Step 6.2: IAT Sensor Installation

**Location:** Intake pipe between intercooler and throttle body

**Procedure:**

1. **Choose location:**
   - After intercooler
   - Before throttle body
   - In straight section of pipe

2. **Install bung:**
   - Drill appropriate size hole in intake pipe
   - Install threaded bung or compression fitting
   - Use silicone sealant for leak prevention

3. **Install thermistor:**
   - Thread into bung
   - Ensure sensor tip is in airstream
   - Not touching pipe wall

4. **Route wiring:**
   - Along engine harness
   - Keep away from heat
   - Secure with zip ties

### Step 6.3: Coolant Temperature Sensor

**Options:**

**Option A: T-Adapter on Existing Sensor**

1. Remove existing coolant temp sensor
2. Install T-adapter
3. Reinstall original sensor
4. Install NTC thermistor in T-adapter
5. May trigger check engine light (test first)

**Option B: Heater Hose Installation**

1. Find suitable heater hose
2. Install compression fitting
3. Insert thermistor
4. Secure with hose clamps
5. Check for leaks

**Option C: Radiator Hose**

1. Similar to heater hose method
2. Use large hose with adapter
3. Seal properly

### Step 6.4: Oil Temperature Sensor (Optional)

**Options:**

**Option A: Oil Pan Drain Plug Adapter**

1. Remove drain plug
2. Install adapter with NPT port
3. Reinstall drain plug
4. Install thermistor in port
5. Use thread sealant (not Teflon tape)

**Option B: Oil Filter Sandwich Plate**

1. Remove oil filter
2. Install sandwich plate adapter
3. Reinstall oil filter
4. Install thermistor in port
5. Check for leaks after starting engine

### Step 6.5: Boost Pressure Sensor

**Location:** Intake manifold (after throttle body)

**Procedure:**

1. **Find boost reference port:**
   - Look for vacuum ports on intake manifold
   - Or install vacuum tee on existing line

2. **Mount sensor:**
   - Find protected location in engine bay
   - Mount higher than manifold (prevents oil ingress)
   - Use bracket or zip ties

3. **Connect vacuum line:**
   - Use 4mm silicone vacuum hose
   - Connect from manifold port to sensor
   - Ensure no kinks or restrictions
   - Secure with zip ties

4. **Route electrical:**
   - 3-wire cable to enclosure
   - Keep away from ignition wires
   - Secure to existing harness

### Step 6.6: Sensor Installation Verification

**Before proceeding:**

- [ ] All sensors physically installed
- [ ] All wiring routed to gauge location
- [ ] All wires labeled
- [ ] No leaks (coolant, oil, boost)
- [ ] All fittings tightened
- [ ] Wiring secured and protected
- [ ] Clear of moving parts and heat

---

## 🔧 Phase 7: Final Installation

### Step 7.1: Choose Gauge Location

**Considerations:**
- Visible to driver
- Won't obstruct driving view
- Accessible for touch screen
- Near power source
- Can route sensor wires

**Common locations:**
- A-pillar mount
- Dashboard top
- Custom panel in center console
- Replace existing gauge

### Step 7.2: Mount Enclosure

1. **Prepare mounting surface:**
   - Clean with isopropyl alcohol
   - Ensure flat and stable

2. **Mounting method:**
   - **Permanent:** Screws into dashboard
   - **Semi-permanent:** Strong double-sided tape (3M VHB)
   - **Temporary:** Velcro straps

3. **Install enclosure:**
   - Position carefully
   - Ensure display is level
   - Test visibility from driver seat

### Step 7.3: Connect Sensor Wires

1. **Route all sensor wires to enclosure:**
   - Through firewall grommet or existing hole
   - Protect from sharp edges
   - Secure along route

2. **Connect to screw terminals:**
   - Strip 5-7mm of insulation
   - Insert into terminal
   - Tighten screw firmly
   - Tug gently to verify connection

3. **Verify connections with labeling:**
   - IAT sensor → IAT terminals
   - Coolant → Coolant terminals
   - MAP → MAP terminals
   - EGT → MAX31855 terminals
   - Double-check polarity!

### Step 7.4: Connect 12V Power

1. **Route power wire to fuse box:**
   - Through firewall
   - Along existing harness
   - Keep away from hot surfaces

2. **Install Add-a-Circuit:**
   - Choose switched 12V circuit
   - Remove existing fuse
   - Install Add-a-Circuit with both fuses
   - Connect red power wire

3. **Connect ground:**
   - Find good chassis ground point
   - Sand away paint for good contact
   - Install ring terminal with bolt
   - Connect black wire

4. **Install 3A fuse:**
   - In inline fuse holder on red wire
   - Close to Add-a-Circuit

### Step 7.5: Cable Management

1. **Secure all wires:**
   - Use zip ties every 6-12 inches
   - Follow existing harness routes
   - No wires hanging loose

2. **Protect vulnerable areas:**
   - Heat shrink at firewall penetration
   - Loom or conduit for exposed wires
   - Extra protection near moving parts

3. **Final checks:**
   - All connections tight
   - No exposed wire
   - No pinch points
   - Enclosure closes properly

---

## ✅ Testing and Calibration

### Final System Test

1. **Reconnect battery**

2. **Turn ignition to ON (engine off):**
   - Gauge should power on
   - Display should initialize
   - Check all readings

3. **Start engine:**
   - All gauges should update
   - Coolant temp should rise
   - No error messages

4. **Drive test:**
   - Monitor all parameters
   - Boost should register under acceleration
   - EGT should rise under load
   - All alerts should work

### Calibration

If readings are incorrect, see [CALIBRATION.md](CALIBRATION.md) for adjustment procedures.

### Installation Complete! 🎉

**Enjoy your new gauge system!**

Monitor parameters for the first few drives and verify everything is working correctly.

---

## 📞 Post-Installation Support

**If you encounter issues:**

1. Check [Troubleshooting](../README.md#troubleshooting) in main README
2. Review Serial Monitor for error messages
3. Verify all connections with multimeter
4. Open an Issue on GitHub with details

**To share your build:**
- Post photos in GitHub Discussions
- Share any improvements or modifications
- Help others with their installations

---

**Congratulations on completing your JNKR Gauge System!**

*Drive safe and monitor those temps!*
