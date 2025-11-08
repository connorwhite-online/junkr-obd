# Bill of Materials (BOM) - JNKR Gauge System

**Complete parts list with part numbers, specifications, and suppliers**

This document lists every component needed to build the JNKR Gauge System with Qualia ESP32-S3, including electronics, hardware, and consumables. All prices are approximate in USD as of 2025.

> **Note**: For complete parts list with exact links, see [COMPLETE_BOM.md](COMPLETE_BOM.md)

---

## 📋 Quick Summary

| Category | Estimated Cost |
|----------|---------------|
| Core Electronics | $80-120 |
| Sensors | $60-100 |
| Power & Wiring | $30-50 |
| Enclosure & Hardware | $20-40 |
| Tools & Consumables | $50-100 |
| **Total** | **$240-410** |

**Note:** Assumes you have basic tools. First-time builders need to budget for tools.

---

## 🔌 Core Electronics

### Microcontroller

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| Arduino Mega 2560 R3 | ATmega2560, 54 I/O, USB | 1 | $15-25 | $15-25 | [Amazon](https://amazon.com/s?k=arduino+mega+2560) |

**Notes:**
- Genuine Arduino or compatible clone both work
- Ensure it includes USB cable
- Verify 5V and 3.3V pin headers

**Alternatives:**
- Arduino Mega 2560 PRO Mini (smaller, no USB)

---

### Display

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| Nextion NX4832T035 | 3.5", 480x320, Touch | 1 | $25-35 | $25-35 | NX4832T035_011 |

**Notes:**
- Enhanced model recommended (more memory)
- Basic model works for simple displays
- Includes 4-pin cable

**Where to buy:**
- [Amazon](https://amazon.com/s?k=nextion+3.5)
- [AliExpress](https://aliexpress.com/wholesale?SearchText=NX4832T035)
- [Itead.cc](https://itead.cc) (official)

**Alternatives:**
- 2.4" model (NX3224T024) - cheaper but smaller
- 4.3" model (NX4832T043) - larger but more expensive

---

### Power Supply

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| DC-DC Buck Converter | 12V→5V, 3A, LM2596 | 1 | $5-8 | $5-8 | LM2596 module |
| Step-down module | Adjustable, 4-40V in | 1 | $3-5 | $3-5 | XL4015 / LM2596 |

**Notes:**
- Must handle 3A continuous
- Adjustable voltage preferred
- Heatsink included
- Screw terminals for easy wiring

**Where to buy:**
- [Amazon](https://amazon.com/s?k=lm2596+buck+converter)
- [eBay](https://ebay.com/sch/i.html?_nkw=dc+buck+converter+3a)
- Electronics retailers (Adafruit, SparkFun)

**Alternatives:**
- Fixed 5V USB car charger (2.4A minimum)
- Integrated 7805 regulator (NOT recommended - gets too hot)

---

## 🌡️ Temperature Sensors

### Heavy-Duty Automotive NTC Thermistors (RECOMMENDED)

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| **OEM Automotive Sensors (Best)** |
| GM IAT Sensor | 2450Ω @ 25°C, 3/8" NPT | 2-3 | $15-30 | $30-90 | GM 25037334, 12146312 |
| GM Coolant Sensor | 2450Ω @ 25°C, 3/8" NPT | 1 | $15-25 | $15-25 | GM 12146312, 213-928 |
| Ford IAT/ECT Sensor | 2070-2500Ω @ 25°C | 2-3 | $12-25 | $24-75 | Ford F4TZ-12A648-BA |
| **Heavy-Duty Diesel Sensors (Best for Rigorous Use)** |
| Cummins IAT Sensor | 2200-2300Ω @ 25°C, M14x1.5 | 2-3 | $25-45 | $50-135 | Cummins 4921475, 3417142 |
| Cummins Coolant Sensor | 2250Ω @ 25°C, M14x1.5 | 1 | $20-35 | $20-35 | Cummins 4087991 |
| CAT Temperature Sensor | 2250Ω @ 25°C, M14x1.5 | 2-3 | $30-50 | $60-150 | CAT 274-6721, 320-3060 |
| **Aftermarket Performance (Good Quality)** |
| AEM Temperature Sensor | 2486Ω @ 25°C, 1/8" NPT | 2-3 | $25-35 | $50-105 | AEM 30-2012 |
| Bosch Temp Sensor | 2700Ω @ 25°C, M12x1.5 | 2-3 | $20-35 | $40-105 | Bosch 0 280 130 026 |
| **Generic Industrial (Budget - NOT Recommended for Engine Bay)** |
| Generic NTC M10 | 2252Ω @ 25°C, β=3950K | 3-4 | $2-4 | $6-16 | Generic NTC M10 |

**Comparison Guide:**

| Sensor Type | Durability | Temperature Range | Vibration Resistance | Recommended Use |
|-------------|-----------|-------------------|---------------------|-----------------|
| Cummins/CAT | Excellent | -40°C to 200°C | Excellent | Heavy-duty diesel, severe conditions |
| GM/Ford OEM | Very Good | -40°C to 150°C | Very Good | Standard automotive, reliable |
| AEM/Bosch | Good | -40°C to 150°C | Good | Performance applications |
| Generic Industrial | Fair | -40°C to 150°C | Poor | Breadboard testing only |

**Why Heavy-Duty Sensors Matter:**
- ✅ **Vibration Resistance:** OEM sensors have hardened terminals and strain relief
- ✅ **Temperature Cycling:** Military-spec construction handles thermal shock
- ✅ **Moisture/Contamination:** Sealed connectors prevent corrosion
- ✅ **EMI Immunity:** Shielded construction reduces electrical interference
- ✅ **Long-Term Stability:** OEM sensors maintain calibration for years
- ❌ **Generic Sensors:** May fail within months in engine bay environment

**Typical Applications:**
- IAT Pre-Intercooler (high temperature, vibration)
- IAT Post-Intercooler (moderate conditions)
- Coolant temp (high heat, coolant contamination risk)

**Where to buy:**
- **OEM Sensors:**
  - [RockAuto.com](https://www.rockauto.com) - Best prices on OEM parts
  - Local auto parts stores (AutoZone, O'Reilly, NAPA)
  - Dealer parts departments (GM, Ford, Cummins)
- **Heavy-Duty Diesel:**
  - [Diesel Parts Direct](https://www.dieselpartsdirect.com)
  - Cummins dealers
  - CAT dealers
- **Performance Aftermarket:**
  - [Summit Racing](https://www.summitracing.com)
  - [Jegs](https://www.jegs.com)
  - AEM website
- **Generic (testing only):**
  - [Amazon "NTC thermistor M10"](https://amazon.com/s?k=ntc+thermistor+m10)
  - AliExpress (budget option)

**Installation Notes:**
- Use appropriate thread sealant (Teflon tape or pipe dope)
- Verify thread size before ordering (3/8" NPT, M10x1.0, M12x1.5, M14x1.5)
- Keep wiring away from ignition components (EMI)
- Use shielded cable in high-noise environments
- **IMPORTANT:** Measure resistance of your specific sensors and update `calibration.h`

---

### EGT Sensor (Exhaust Gas Temperature)

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| **Heavy-Duty EGT Probes (RECOMMENDED)** |
| Omega K-Type Probe | Inconel sheath, M8 or 1/8" NPT | 1 | $60-120 | $60-120 | Omega KQXL/KMQXL series |
| Pyromation EGT Probe | Stainless/Inconel, compression | 1 | $70-150 | $70-150 | Pyromation 6450/6460 |
| Watlow K-Type | Industrial grade, 1/8" NPT | 1 | $50-100 | $50-100 | Watlow 942A series |
| **Performance Automotive (Good)** |
| AEM EGT Sensor | K-type, M8x1.0, 0-1800°F | 1 | $40-60 | $40-60 | AEM 30-2058 |
| Innovate EGT Probe | K-type, M8, fast response | 1 | $35-55 | $35-55 | Innovate 3853 |
| **Budget/Generic (Testing Only)** |
| Generic K-Type | M6 or M8 thread, 0-1000°C | 1 | $10-20 | $10-20 | Generic K-type EGT |
| **I2C Thermocouple Amplifier** |
| MCP9600 Module | I2C, K-type, 18-bit | 1 | $8-15 | $8-15 | Adafruit 4101 |
| **EGT Installation Hardware** |
| EGT Bung/Fitting | M6, M8, or 1/8" NPT weld bung | 1 | $5-10 | $5-10 | Steel weld bung |

**Heavy-Duty EGT Probe Features:**
- ✅ **Inconel Sheath:** Corrosion-resistant superalloy, handles extreme heat
- ✅ **Vibration Resistant:** Spring-loaded or compression mounting
- ✅ **Fast Response:** <1 second thermal response time
- ✅ **High Temperature:** Rated to 1200°C (2200°F) continuous
- ✅ **Long Service Life:** 5-10 years in harsh exhaust environments
- ❌ **Generic Probes:** May fail in <1 year, slow response, oxidation issues

**Comparison:**

| Probe Type | Durability | Response Time | Max Temp | Recommended Use |
|------------|-----------|---------------|----------|-----------------|
| Omega/Pyromation Industrial | Excellent | 0.5-1s | 1200°C | Heavy-duty diesel, industrial |
| AEM/Innovate Performance | Very Good | 1-2s | 1000°C | Performance gas/diesel engines |
| Generic K-Type | Fair | 2-5s | 800°C | Breadboard testing only |

**Installation Notes:**
- **Probe Depth:** 35-50mm insertion into exhaust stream (not touching wall)
- **Location:** Pre-turbo for max EGT, post-turbo for turbine health
- **Bung Installation:** Requires TIG welding into exhaust pipe (professional recommended)
- **Thread Sealant:** Use high-temp anti-seize (never Teflon tape)
- **Wire Routing:** Use ceramic beads near probe, keep away from hot surfaces
- **Grounding:** Ungrounded junction preferred for automotive (reduces electrical noise)

**Thermocouple Amplifier:**
- **MCP9600:** Modern I2C interface, 18-bit resolution, integrated cold junction
- Recommended for this project (simpler wiring than legacy MAX31855)
- Operating range: -200°C to 1372°C
- Accuracy: ±1°C

**Where to buy:**
- **Industrial Grade:**
  - [Omega.com](https://www.omega.com) - Industry standard, technical support
  - [Pyromation.com](https://www.pyromation.com) - Heavy-duty industrial
  - [Watlow.com](https://www.watlow.com) - High-quality industrial
- **Performance Automotive:**
  - [Summit Racing](https://www.summitracing.com)
  - [Jegs](https://www.jegs.com)
  - AEM Performance Electronics
- **Amplifier Module:**
  - [Adafruit MCP9600](https://www.adafruit.com/product/4101) - Recommended
  - SparkFun, Amazon
- **Generic (testing only):**
  - [Amazon "K type thermocouple M8"](https://amazon.com/s?k=k+type+thermocouple+exhaust)

---

### MAP Sensor (Boost Pressure)

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| **OEM Automotive MAP Sensors (RECOMMENDED)** |
| GM 3-Bar MAP | 0-3 bar, 0.5-4.5V analog | 1 | $25-45 | $25-45 | GM 12223861, 16040749 |
| Delphi 3-Bar MAP | 0-3 bar, Delphi connector | 1 | $30-50 | $30-50 | Delphi 12247571 |
| Ford 2-Bar MAP | 0-2 bar, Ford connector | 1 | $20-35 | $20-35 | Ford F77F-9F479-AA |
| **Heavy-Duty Diesel Sensors (Best)** |
| Cummins Boost Sensor | 0-3 bar or 0-5 bar, sealed | 1 | $50-90 | $50-90 | Cummins 4921517, 4903577 |
| CAT Pressure Sensor | 0-3 bar, IP67 rated | 1 | $60-100 | $60-100 | CAT 224-4536, 268-9384 |
| **Performance Aftermarket** |
| AEM 3.5-Bar MAP | 0-3.5 bar, 0.5-4.5V, Deutsch | 1 | $60-90 | $60-90 | AEM 30-2130-50 |
| Bosch 3-Bar MAP | 0-3 bar, high precision | 1 | $45-75 | $45-75 | Bosch 0 261 230 034 |
| **Generic/Budget (NOT Recommended)** |
| MPX4250AP Bare IC | 0-3 bar IC (no housing) | 1 | $8-15 | $8-15 | Freescale/NXP MPX4250AP |
| Generic 3-Bar | Chinese clone, unknown specs | 1 | $10-20 | $10-20 | Generic |
| **Installation Hardware** |
| Vacuum Hose | 4mm ID silicone, 1 meter | 1 | $3-5 | $3-5 | High-temp silicone |
| Vacuum T-fitting | 4mm or 1/8" NPT barbed T | 1 | $2-3 | $2-3 | Brass or stainless |

**Why Heavy-Duty MAP Sensors Matter:**
- ✅ **Vibration Resistance:** Sealed diaphragm, hardened mounting
- ✅ **Temperature Stability:** Compensated for -40°C to 125°C
- ✅ **Moisture Resistant:** IP67 sealed connector (Cummins/CAT)
- ✅ **EMI Shielded:** Integrated filtering reduces electrical noise
- ✅ **Accuracy:** ±0.5% to ±2% full scale (vs ±5% for generic)
- ✅ **Long-Term Stability:** Maintains calibration for years
- ❌ **Generic Sensors:** Poor accuracy, drift over time, moisture ingress

**Comparison Guide:**

| Sensor Type | Accuracy | Durability | Temperature Range | Recommended Use |
|-------------|----------|-----------|-------------------|-----------------|
| Cummins/CAT Diesel | ±0.5% | Excellent | -40°C to 125°C | Heavy-duty diesel, severe environments |
| GM/Delphi OEM | ±1.5% | Very Good | -40°C to 125°C | Standard automotive, reliable |
| AEM/Bosch Performance | ±0.5-1% | Good | -40°C to 125°C | Performance tuning, data logging |
| Generic/MPX4250 IC | ±5% | Poor | 0°C to 85°C | Breadboard testing only |

**Specifications:**

**3-Bar MAP Sensors (Most Common):**
- Pressure Range: 0-3 bar absolute (0-29 PSI gauge)
- Output: 0.5V @ 0 bar, 4.5V @ 3 bar (linear)
- Supply Voltage: 5.0V ±0.25V
- Connector: 3-pin (5V, Signal, GND)
- Update Rate: 10-100Hz typical
- Max Boost: ~29 PSI gauge (good for stock turbo + mild tuning)

**Alternative Ranges:**
- **2-Bar Sensors:** 0-2 bar (0-14.5 PSI gauge) - stock naturally aspirated/low boost
- **4-Bar Sensors:** 0-4 bar (0-43.5 PSI gauge) - high boost applications
- **5-Bar Sensors:** 0-5 bar (0-58 PSI gauge) - extreme boost / compound turbos

**Installation Notes:**
- **Location:** Before throttle body, after intercooler (represents actual manifold pressure)
- **Vacuum Line:** Use 4mm silicone or rubber vacuum hose (not plastic - cracks)
- **T-Fitting:** Tap into existing vacuum port or install T-fitting
- **Sensor Mounting:** Mount with connector facing DOWN (prevents moisture ingress)
- **Wiring:** Use shielded cable, keep away from ignition components
- **Calibration:** Verify sensor reads atmospheric pressure (~1.0 bar) with engine off

**Connector Pinouts (Common):**
- Pin 1: 5V Supply (red wire typical)
- Pin 2: Signal Output (yellow/white wire)
- Pin 3: Ground (black wire)

**Where to buy:**
- **OEM Sensors:**
  - [RockAuto.com](https://www.rockauto.com) - Best prices on OEM
  - Local auto parts stores (AutoZone, O'Reilly, NAPA)
  - GM/Ford/Mopar dealer parts departments
- **Heavy-Duty Diesel:**
  - [Diesel Parts Direct](https://www.dieselpartsdirect.com)
  - Cummins dealers
  - CAT dealers
- **Performance:**
  - [Summit Racing](https://www.summitracing.com)
  - [Jegs](https://www.jegs.com)
  - AEM Performance Electronics
  - HP Tuners, Holley EFI
- **Generic (testing only):**
  - [Amazon "3 bar MAP sensor"](https://amazon.com/s?k=3+bar+map+sensor)
  - eBay, AliExpress

**IMPORTANT:** Different sensors have different resistance values and Beta coefficients. 
After purchasing, measure your sensor resistance at room temperature and update the values in `calibration.h`

---

## 🔊 Audio Alert

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| Piezo Buzzer | Active, 5V, 85dB @ 10cm | 1 | $1-3 | $1-3 | Generic active buzzer |

**Notes:**
- **Active** type (has built-in oscillator)
- 2-wire: Red (+), Black (-)
- 5V operating voltage
- 2kHz typical frequency

**Where to buy:**
- [Amazon "active buzzer 5v"](https://amazon.com/s?k=active+buzzer+5v)
- Electronics retailers
- AliExpress (bulk pricing)

**Alternatives:**
- Small speaker with amplifier
- Passive buzzer (requires tone() function)

---

## 🔌 Electrical Components

### Resistors

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| 2.2kΩ Resistor | 1/4W, 5% tolerance | 5 | $0.10 | $0.50 | Generic |
| 10kΩ Resistor | 1/4W, 5% tolerance | 2 | $0.10 | $0.20 | Generic |

**Notes:**
- Through-hole type
- 1/4W power rating sufficient
- Buy resistor assortment kit (cheapest option)

**Color Codes:**
- 2.2kΩ = Red-Red-Red-Gold
- 10kΩ = Brown-Black-Orange-Gold

**Where to buy:**
- Any electronics retailer
- Assortment kits on Amazon ($10 for 500pcs)

---

### Capacitors

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| 0.1µF Ceramic Capacitor | 50V, X7R/Z5U | 5 | $0.10 | $0.50 | Generic |
| 100µF Electrolytic | 25V, radial | 2 | $0.20 | $0.40 | Generic |

**Notes:**
- For power filtering
- 0.1µF near each IC power pin
- 100µF on 5V power rail

**Markings:**
- 0.1µF = "104"
- 100µF = marked on body

**Where to buy:**
- Electronics retailers
- Capacitor assortment kits

---

### Connectors & Terminals

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| Screw Terminal Blocks | 2-pin, 5mm pitch | 10 | $0.30 | $3.00 | Phoenix-style |
| JST-XH Connectors | 2-5 pin, with housing | 1 set | $5-8 | $5-8 | JST-XH kit |
| Dupont Connectors | 40pcs, male/female | 1 kit | $5-8 | $5-8 | Dupont kit |
| Heat Shrink Tubing | Assorted sizes, 2:1 | 1 kit | $8-12 | $8-12 | Heat shrink kit |

**Where to buy:**
- Amazon connector kits
- Electronics retailers
- AliExpress (cheapest)

---

## 🔧 Wiring & Cable

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| 22 AWG Wire | Stranded, automotive | 10m | $0.30/m | $3.00 | Various colors |
| 20 AWG Wire | Stranded, power wire | 5m | $0.40/m | $2.00 | Red & Black |
| 4-conductor Cable | 22 AWG, shielded | 2m | $1.00/m | $2.00 | For Nextion |

**Colors needed:**
- Red (power)
- Black (ground)
- Various (yellow, blue, green, white)

**Where to buy:**
- Auto parts stores (automotive wire)
- Electronics retailers
- Amazon wire assortment

---

### Power Wiring Components

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| Inline Fuse Holder | Blade fuse, 10-16 AWG | 1 | $3-5 | $3-5 | Standard blade |
| 3A Blade Fuse | ATO/ATC style | 2 | $0.50 | $1.00 | Standard |
| Add-a-Circuit | Fuse tap, mini/standard | 1 | $5-8 | $5-8 | Various brands |
| Ring Terminals | 20 AWG, #8 stud | 2 | $0.50 | $1.00 | Generic |

**Where to buy:**
- Auto parts stores
- Amazon
- Hardware stores

---

## 📦 Enclosure & Hardware

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| Project Enclosure | IP65, 200x150x75mm | 1 | $15-25 | $15-25 | Hammond, Polycase |
| M3 Standoffs | 10mm + screws, brass/nylon | 8 | $0.30 | $2.40 | M3x10mm kit |
| Cable Glands | PG9, IP68 rated | 4 | $1.50 | $6.00 | Nylon cable glands |
| Self-tapping Screws | M3, various lengths | 1 set | $3-5 | $3-5 | Assortment |
| Double-sided Tape | 3M VHB or similar | 1 roll | $5-8 | $5-8 | Heavy duty |

**Enclosure Requirements:**
- Waterproof (IP65 minimum)
- Large enough for Arduino + display
- Mounting holes or flat surface
- Cable entry provisions

**Where to buy:**
- Electronics retailers
- Amazon project box
- Hardware stores

---

## 🛠️ Tools (if you don't have them)

| Item | Specifications | Quantity | Unit Price | Total | Notes |
|------|---------------|----------|------------|-------|-------|
| Multimeter | Digital, auto-ranging | 1 | $15-30 | $15-30 | Essential |
| Wire Strippers | 20-30 AWG | 1 | $10-20 | $10-20 | Essential |
| Soldering Iron | 25-40W, temperature control | 1 | $20-40 | $20-40 | Highly recommended |
| Solder | 60/40 rosin core, 0.8mm | 1 | $5-10 | $5-10 | Necessary if soldering |
| Crimping Tool | For Dupont/JST connectors | 1 | $15-30 | $15-30 | Optional |
| Heat Gun | For heat shrink tubing | 1 | $15-25 | $15-25 | Lighter works too |
| Drill & Bits | For enclosure holes | 1 | $30-60 | $30-60 | May already have |

**Total for tools:** $110-215 (one-time purchase)

---

## 📝 Consumables

| Item | Specifications | Quantity | Unit Price | Total | Notes |
|------|---------------|----------|------------|-------|-------|
| Electrical Tape | Vinyl, black | 1 roll | $2-3 | $2-3 | Basic |
| Zip Ties | Assorted, 100pcs | 1 bag | $5-8 | $5-8 | Cable management |
| Wire Labels | Write-on or printable | 1 pack | $3-5 | $3-5 | Organization |
| Solder Flux | Rosin, pen or paste | 1 | $5-8 | $5-8 | If soldering |
| Isopropyl Alcohol | 90%+, for cleaning | 1 bottle | $3-5 | $3-5 | PCB cleaning |

---

## 🏗️ Optional: Custom Shield PCB

If ordering the custom shield PCB from PCBWay:

| Item | Specifications | Quantity | Unit Price | Total | Notes |
|------|---------------|----------|------------|-------|-------|
| PCB Fabrication | 2-layer, 100x100mm | 5 pcs | $2-3 | $10-15 | Min order 5pcs |
| Shipping | Express (7-14 days) | 1 | $15-30 | $15-30 | Varies by location |
| Components | Resistors, caps, terminals | - | - | $10-15 | Shield components |

**Total for shield option:** $35-60

**Benefits:**
- Professional appearance
- Simplified wiring
- Reduced errors
- Faster assembly

See [SHIELD.md](SHIELD.md) for details.

---

## 💰 Cost Breakdown by Category

### Minimum Build (No Tools)

| Category | Cost |
|----------|------|
| Core Electronics | $80 |
| Sensors | $60 |
| Power & Wiring | $30 |
| Enclosure | $20 |
| **Subtotal** | **$190** |
| Shipping (~20%) | $40 |
| **Total** | **$230** |

### Recommended Build (With Quality Components)

| Category | Cost |
|----------|------|
| Core Electronics | $120 |
| Sensors | $100 |
| Power & Wiring | $50 |
| Enclosure & Hardware | $40 |
| **Subtotal** | **$310** |
| Shipping (~15%) | $45 |
| **Total** | **$355** |

### First-Time Builder (Including Tools)

| Category | Cost |
|----------|------|
| Components | $310 |
| Tools | $150 |
| Consumables | $30 |
| **Subtotal** | **$490** |
| Shipping | $45 |
| **Total** | **$535** |

**Note:** Tools are a one-time investment

---

## 🛒 Recommended Suppliers

### United States

1. **Amazon** - Fast shipping, easy returns
   - Best for: Complete kits, Arduino, display
   - Price: Moderate to high
   - Shipping: 1-2 days (Prime)

2. **Adafruit** - Quality electronics, great support
   - Best for: Sensors, modules, quality parts
   - Price: Moderate to high
   - Shipping: 3-5 days

3. **SparkFun** - Similar to Adafruit
   - Best for: Development boards, sensors
   - Price: Moderate to high
   - Shipping: 3-5 days

4. **Digi-Key / Mouser** - Professional components
   - Best for: Exact part numbers, bulk orders
   - Price: Varies
   - Shipping: 1-3 days

5. **AliExpress** - Budget option
   - Best for: Generic parts, bulk items
   - Price: Very low
   - Shipping: 2-6 weeks

### International

- **Banggood** - Electronics, similar to AliExpress
- **eBay** - Mixed sellers, verify ratings
- **Local electronics retailers** - Immediate availability

---

## 📦 Suggested Purchasing Strategy

### Option 1: All-from-Amazon (Fastest)

**Pros:**
- Fastest delivery (1-2 days)
- Easy returns
- All in one order

**Cons:**
- Higher cost
- Variable quality

**Estimated cost:** $320-400
**Time to delivery:** 1-2 days

### Option 2: Mixed Sources (Balanced)

**Strategy:**
1. Arduino, display from Amazon
2. Sensors from Adafruit/SparkFun
3. Wire, connectors from AliExpress
4. Enclosure from Amazon

**Estimated cost:** $250-350
**Time to delivery:** 1-2 weeks

### Option 3: Budget Build (Cheapest)

**Strategy:**
1. Everything from AliExpress
2. Wait for deals
3. Buy in bulk

**Estimated cost:** $180-250
**Time to delivery:** 3-6 weeks

---

## ✅ Shopping List Checklist

Print this checklist when ordering:

### Electronics
- [ ] Arduino Mega 2560 (1x)
- [ ] Nextion display 3.5" (1x)
- [ ] DC-DC buck converter (1x)
- [ ] MAX31855 module (1x)
- [ ] K-type thermocouple (1x)
- [ ] MAP sensor 3-bar (1x)
- [ ] NTC thermistors (3-4x)
- [ ] Piezo buzzer (1x)

### Components
- [ ] 2.2kΩ resistors (5x)
- [ ] 10kΩ resistors (2x)
- [ ] 0.1µF capacitors (5x)
- [ ] 100µF capacitors (2x)
- [ ] Screw terminals (10x)
- [ ] JST connectors kit (1x)
- [ ] Dupont connectors kit (1x)

### Wiring
- [ ] 22 AWG wire assortment (10m)
- [ ] 20 AWG power wire (5m red, 5m black)
- [ ] Heat shrink tubing assortment
- [ ] Fuse holder (1x)
- [ ] 3A fuses (2x)
- [ ] Add-a-circuit fuse tap (1x)
- [ ] Ring terminals (2x)

### Enclosure
- [ ] Project box IP65 (1x)
- [ ] M3 standoffs & screws kit
- [ ] Cable glands (4x)
- [ ] Mounting hardware
- [ ] Double-sided tape

### Consumables
- [ ] Electrical tape
- [ ] Zip ties (100pc bag)
- [ ] Wire labels
- [ ] Solder (if soldering)
- [ ] Isopropyl alcohol

### Tools (if needed)
- [ ] Multimeter
- [ ] Wire strippers
- [ ] Soldering iron
- [ ] Crimping tool
- [ ] Heat gun
- [ ] Drill & bits

---

## 📊 Alternative Component Options

### Budget Alternatives

- **Arduino clone** instead of genuine ($10 vs $25)
- **2.4" Nextion** instead of 3.5" ($18 vs $30)
- **Aluminum enclosure** instead of IP65 plastic ($8 vs $20)
- **Single NTC** for coolant only ($3 vs $12 for 3)

**Savings:** ~$50-70

### Premium Upgrades

- **Genuine Arduino Mega** for reliability
- **Enhanced Nextion** for more memory
- **Military-grade sensors** for accuracy
- **Waterproof connectors** for all connections
- **Custom shield PCB** for professional build

**Additional cost:** ~$80-120

---

## 🔄 Component Substitutions

### Can I use different components?

**Arduino:**
- ✅ Arduino Mega 2560 clone
- ✅ Arduino Mega PRO
- ❌ Arduino Uno (not enough pins)
- ❌ Arduino Nano (not enough pins)

**Display:**
- ✅ Nextion 2.4", 3.2", 4.3", 7"
- ✅ Nextion Basic or Enhanced
- ❌ LCD with I2C (requires different code)
- ❌ OLED display (too small, different interface)

**MAP Sensor:**
- ✅ Any 3-bar absolute MAP sensor
- ✅ 2-bar sensor (limited to ~15 PSI)
- ✅ 4-bar sensor (extended range)
- ❌ Differential pressure sensor

**Thermocouples:**
- ✅ Any K-type thermocouple
- ⚠️ J-type (requires code change)
- ❌ RTD (incompatible with MAX31855)

---

## 📞 Need Help Finding Parts?

If you're having trouble sourcing components:

1. Check the project GitHub Issues - others may have posted local sources
2. Post a question with your country/region
3. Join the community forum (link in main README)
4. Contact local electronics retailers with the part specifications

---

**Next Steps:**

After ordering parts:
1. Read [WIRING.md](WIRING.md) while waiting for parts
2. Plan your installation locations
3. Prepare your vehicle (weld EGT bung, etc.)
4. When parts arrive, follow [ASSEMBLY.md](ASSEMBLY.md)

---

**Last Updated:** 2025-11-07

*This BOM is maintained by the community. If you find better sources or pricing, please contribute!*
