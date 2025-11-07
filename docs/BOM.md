# Bill of Materials (BOM) - JNKR Gauge System

**Complete parts list with part numbers, specifications, and suppliers**

This document lists every component needed to build the JNKR Gauge System, including electronics, hardware, and consumables. All prices are approximate in USD as of 2025.

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

### NTC Thermistors

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| NTC Thermistor | 2252Ω @ 25°C, β=3950K | 3-4 | $2-4 | $6-16 | Generic NTC M10 |
| M10 Threaded Housing | Stainless steel, M10x1.0 | 3-4 | $3-5 | $9-20 | - |

**Notes:**
- Get automotive-grade with threaded housing
- Standard 2.2kΩ nominal resistance
- Temperature range: -40°C to 150°C
- 1m cable minimum

**Typical Applications:**
- IAT (Intake Air Temp)
- Coolant temp
- Oil temp

**Where to buy:**
- [Amazon "NTC thermistor M10"](https://amazon.com/s?k=ntc+thermistor+m10)
- Automotive sensor suppliers
- AliExpress (budget option)

**Alternatives:**
- Pre-wired automotive sensors from GM, Ford, etc.
- Resistance must be verified (2-2.5kΩ @ 25°C)

---

### EGT Sensor (Exhaust Gas Temperature)

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| K-Type Thermocouple | M6 or M8 thread, 0-1000°C | 1 | $10-20 | $10-20 | Generic K-type EGT |
| MAX31855 Module | Thermocouple amplifier, SPI | 1 | $5-10 | $5-10 | MAX31855 breakout |
| EGT Bung/Fitting | M6 or 1/8" NPT weld bung | 1 | $5-10 | $5-10 | Steel weld bung |

**Notes:**
- K-type is standard for automotive EGT
- Probe length: 35-50mm ideal for exhaust
- MAX31855 module handles cold junction compensation
- Weld bung needs professional welding

**Where to buy:**
- [Amazon "K type thermocouple M6"](https://amazon.com/s?k=k+type+thermocouple+exhaust)
- [Adafruit MAX31855 module](https://www.adafruit.com/product/269)
- Automotive performance shops

**Alternatives:**
- MAX6675 (older, slower, 0-1024°C)
- Direct ADC with amplifier (not recommended)

---

### MAP Sensor (Boost Pressure)

| Item | Specifications | Quantity | Unit Price | Total | Part Number / Link |
|------|---------------|----------|------------|-------|-------------------|
| 3-Bar MAP Sensor | 0-3 bar absolute, 0.5-4.5V | 1 | $15-30 | $15-30 | MPX4250 or GM 3-bar |
| Vacuum Hose | 4mm ID silicone, 1 meter | 1 | $3-5 | $3-5 | Generic silicone |
| Vacuum T-fitting | 4mm barbed T | 1 | $2-3 | $2-3 | Brass or nylon |

**Notes:**
- 3-bar absolute = 0-30 PSI gauge
- Linear output: 0.5V = 0 bar, 4.5V = 3 bar
- 3-pin connector: 5V, Signal, GND
- Automotive grade recommended

**Common Part Numbers:**
- GM 12223861 (OEM)
- MPX4250AP (Freescale/NXP)
- Delphi 12247571

**Where to buy:**
- [Amazon "3 bar MAP sensor"](https://amazon.com/s?k=3+bar+map+sensor)
- Auto parts stores
- Performance tuning shops

**Alternatives:**
- 2-bar sensor (0-15 PSI max)
- 4-bar sensor (0-45 PSI, for high boost)

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
