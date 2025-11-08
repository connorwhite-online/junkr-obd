# Complete Bill of Materials - JNKR Gauge System

**Everything You Need to Build Your Engine Monitoring Gauge**

This is the complete parts list with exact product links, part numbers, and prices. All links are current as of November 2024.

---

## 🛒 Complete Shopping List

### Summary

| Category | Estimated Cost |
|----------|---------------|
| Core Electronics | $150-180 |
| Sensors | $60-80 |
| Wiring & Connectors | $30-40 |
| Enclosure & Hardware | $40-60 |
| **TOTAL** | **$280-360** |

---

## 📱 Core Electronics

### Arduino Controller

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **Arduino Mega 2560 R3** | A000067 | 1 | $48.90 | [Arduino Store](https://store.arduino.cc/products/arduino-mega-2560-rev3) |
| *Alternative: Compatible Mega* | ELEGOO Mega | 1 | $21.99 | [Amazon](https://www.amazon.com/ELEGOO-ATmega2560-ATMEGA16U2-Projects-Compliant/dp/B01H4ZDYCE) |

**Recommendation:** Genuine Arduino for best quality, ELEGOO for budget build.

---

### Display (Choose ONE Option)

**Option 1: 3.5" TFT (Recommended - Same size as Nextion)**

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **Adafruit 3.5" TFT LCD Touchscreen Breakout** | 2050 | 1 | $54.95 | [Adafruit](https://www.adafruit.com/product/2050) |
| microSD Card (for images/fonts) | Any 8-32GB | 1 | $5-10 | [Amazon](https://www.amazon.com/s?k=microsd+card+8gb) |

- **Resolution:** 320x480 pixels
- **Touch:** Resistive (works with gloves)
- **Interface:** SPI
- **Size:** 3.5" diagonal (similar to Nextion 3.5")

**Option 2: 2.8" TFT Shield (Easiest - Plugs Right In)**

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **Adafruit 2.8" TFT Touch Shield for Arduino** | 1651 | 1 | $44.95 | [Adafruit](https://www.adafruit.com/product/1651) |

- **Resolution:** 240x320 pixels
- **Touch:** Resistive
- **Interface:** SPI
- **Size:** 2.8" diagonal
- **Advantage:** Plugs directly onto Arduino Mega (no wiring needed!)

**Option 3: 2.8" TFT Breakout (Compact)**

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **Adafruit 2.8" TFT LCD Touchscreen Breakout** | 1770 | 1 | $39.95 | [Adafruit](https://www.adafruit.com/product/1770) |

- **Resolution:** 240x320 pixels
- **Touch:** Resistive
- **Interface:** SPI
- **Size:** 2.8" diagonal

**My Recommendation:** Get the **3.5" (#2050)** for maximum visibility, or the **2.8" Shield (#1651)** for easiest assembly.

---

### Power Supply

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **12V to 5V DC-DC Buck Converter (3A)** | LM2596 Module | 1 | $8.99 | [Amazon](https://www.amazon.com/dp/B08CDMXGFX) |
| **Barrel Jack Adapter for Arduino** | 2.1mm Female | 1 | $7.99 | [Amazon](https://www.amazon.com/dp/B07C7KKQWZ) |
| *Alternative: USB Cable Power* | USB A to B | 1 | $6.99 | [Amazon](https://www.amazon.com/dp/B00P0E3954) |

**Note:** For automotive use, use the buck converter. For bench testing, USB cable works.

---

## 🌡️ Sensors

### Temperature Sensors

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **K-Type Thermocouple (EGT)** | M6 x 1.0 Thread, 0-1000°C | 1 | $24.99 | [Amazon](https://www.amazon.com/dp/B07QGQND5V) |
| **MAX31855 Thermocouple Amplifier** | SPI Interface, K-Type | 1 | $14.95 | [Adafruit #269](https://www.adafruit.com/product/269) |
| **NTC Thermistor (Intake Air Temp)** | 2.2kΩ @ 25°C, M8 Sensor | 2 | $12.99/ea | [Amazon](https://www.amazon.com/dp/B07PFYXZWG) |
| **NTC Thermistor (Coolant Temp)** | 2.2kΩ @ 25°C, 1/8" NPT | 1 | $14.99 | [Amazon](https://www.amazon.com/dp/B07L3QNWRJ) |

**EGT Sensor Details:**
- **Probe Length:** 50-100mm (choose based on exhaust pipe size)
- **Thread:** M6 x 1.0 or 1/8" NPT (ensure compatibility with your bung)
- **Temperature Range:** 0-1000°C (1832°F)

**Alternative Budget EGT:**
| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **EGT Sensor Kit (Thermocouple + Amplifier)** | Complete Kit | 1 | $15.99 | [Amazon](https://www.amazon.com/dp/B07ZPBTMM4) |

---

### Pressure Sensor

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **3-Bar MAP Sensor** | 0-3 bar absolute, 0.5-4.5V output | 1 | $18.99 | [Amazon](https://www.amazon.com/dp/B07DWGRYXW) |

**Specifications:**
- **Range:** 0-3 bar absolute (0-29 PSI gauge)
- **Output:** 0.5V @ 0 bar → 4.5V @ 3 bar
- **Thread:** 1/8" NPT or barbed nipple
- **Connector:** Pigtail included

---

### Audio Alert

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Piezo Buzzer** | Active, 3-5V, 2kHz | 1 | $6.99 (5-pack) | [Amazon](https://www.amazon.com/dp/B07DBYF4ND) |

---

## 🔌 Wiring & Connectors

### Wire

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **22 AWG Stranded Wire Spool** | 6 colors, 25ft each | 1 | $15.99 | [Amazon](https://www.amazon.com/dp/B01LH1FUE8) |
| **20 AWG Automotive Wire** | For power, 25ft | 1 | $9.99 | [Amazon](https://www.amazon.com/dp/B071D6JQM3) |

### Connectors & Hardware

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Screw Terminal Blocks** | 5mm pitch, 2-12 pos | 1 set | $11.99 | [Amazon](https://www.amazon.com/dp/B07C7VTLFH) |
| **DuPont Jumper Wires** | Male-Male, 40-pin | 1 | $6.99 | [Amazon](https://www.amazon.com/dp/B01EV70C78) |
| **DuPont Jumper Wires** | Male-Female, 40-pin | 1 | $6.99 | [Amazon](https://www.amazon.com/dp/B01EV70C78) |
| **Heat Shrink Tubing Kit** | Assorted sizes | 1 | $8.99 | [Amazon](https://www.amazon.com/dp/B084GDLSCK) |
| **Weatherpack Connectors** | 2/3/4-pin automotive | 1 kit | $16.99 | [Amazon](https://www.amazon.com/dp/B07PPLGZ6D) |
| **Inline Fuse Holder** | ATC/ATO blade fuse | 2 | $8.99 | [Amazon](https://www.amazon.com/dp/B07L3QNWRJ) |
| **3A Blade Fuses** | ATC, 10-pack | 1 | $5.99 | [Amazon](https://www.amazon.com/dp/B07L9QXJJ4) |

### Prototyping Supplies

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Breadboard (830-point)** | For prototyping | 1 | $6.99 | [Amazon](https://www.amazon.com/dp/B082KBF7MM) |
| **Breadboard Jumper Wire Kit** | Pre-cut wires | 1 | $6.99 | [Amazon](https://www.amazon.com/dp/B01EV70C78) |

---

## 🔩 Electronic Components

### Resistors & Capacitors

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Resistor Kit** | 1/4W, assorted values | 1 | $10.99 | [Amazon](https://www.amazon.com/dp/B08FD1XVL6) |
| **2.2kΩ Resistors** | 1/4W (for NTC dividers) | 10 | Included in kit above |
| **10kΩ Resistors** | 1/4W (pull-ups) | 10 | Included in kit above |
| **Ceramic Capacitors (0.1µF)** | 50V, power filtering | 10 | $5.99 (100-pack) | [Amazon](https://www.amazon.com/dp/B07PBQXQNQ) |

---

## 📦 Enclosure & Mounting

### Enclosures

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Arduino Mega Enclosure** | IP65 rated, clear lid | 1 | $16.99 | [Amazon](https://www.amazon.com/dp/B07WJM2YCM) |
| **Display Mounting** | 3D printed or custom | 1 | DIY | - |

### Mounting Hardware

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **M3 Standoffs Kit** | Brass, M-F, assorted | 1 | $12.99 | [Amazon](https://www.amazon.com/dp/B07D7828LC) |
| **Cable Glands** | PG7, waterproof | 5 | $9.99 | [Amazon](https://www.amazon.com/dp/B07X3J8XHG) |
| **Velcro Strips** | Industrial strength | 1 | $8.99 | [Amazon](https://www.amazon.com/dp/B00006RSWT) |

---

## 🔧 Tools (If You Don't Have)

| Item | Purpose | Price | Link |
|------|---------|-------|------|
| **Soldering Iron Kit** | 60W adjustable | $22.99 | [Amazon](https://www.amazon.com/dp/B06XZ31W3M) |
| **Wire Strippers** | 10-22 AWG | $12.99 | [Amazon](https://www.amazon.com/dp/B000JNNWQ2) |
| **Crimping Tool** | For DuPont/JST | $15.99 | [Amazon](https://www.amazon.com/dp/B078WPT5M1) |
| **Multimeter** | Digital, with continuity | $19.99 | [Amazon](https://www.amazon.com/dp/B01ISAMUA6) |
| **Heat Gun** | For heat shrink | $16.99 | [Amazon](https://www.amazon.com/dp/B078S5QMFG) |

---

## 💾 Software (All Free!)

| Item | Platform | Link |
|------|----------|------|
| **Arduino IDE** | Mac/Windows/Linux | [arduino.cc](https://www.arduino.cc/en/software) |
| **SquareLine Studio** | Mac/Windows/Linux | [squareline.io](https://squareline.io) (Free version) |
| **LVGL Library** | Arduino Library Manager | Included in IDE |

---

## 📋 Automotive Installation Hardware (Not Included Above)

You'll need these for installing sensors in your vehicle:

| Item | Specs | Purpose | Link |
|------|-------|---------|------|
| **M6 Exhaust Bung** | Weld-on, stainless | EGT sensor mount | [Amazon](https://www.amazon.com/dp/B07VWXMXLQ) |
| **1/8" NPT to Barb Fitting** | Brass, 4mm barb | MAP sensor connection | [Amazon](https://www.amazon.com/dp/B07DWXQCNP) |
| **Vacuum Hose** | 4mm silicone, 6ft | MAP sensor line | [Amazon](https://www.amazon.com/dp/B07XQXZLQM) |
| **Coolant Temp Adapter** | 1/8" NPT T-fitting | Coolant sensor tap | [Amazon](https://www.amazon.com/dp/B01N5OZVXV) |
| **Intercooler Pipe Bosses** | M8 weld-on bungs | IAT sensor mounts | [Amazon](https://www.amazon.com/dp/B07PWRN8S9) |

---

## 📦 Pre-Made Kit Option

**Don't want to source everything separately?**

I recommend buying the following "starter kits" which include many components:

| Kit | Contents | Price | Link |
|-----|----------|-------|------|
| **Arduino Mega Starter Kit** | Mega + wires + breadboard + components | $54.99 | [Amazon](https://www.amazon.com/dp/B01EWNUUUA) |
| **Sensor Kit for Arduino** | Various sensors including thermistors | $34.99 | [Amazon](https://www.amazon.com/dp/B01N79PG4G) |

---

## 🛠️ Complete Build Options

### Budget Build (~$240)

- ELEGOO Mega 2560 ($22)
- Adafruit 2.8" TFT Shield ($45)
- Budget EGT Kit ($16)
- 3-Bar MAP Sensor ($19)
- NTC Thermistors x3 ($40)
- Wire & Connectors ($30)
- Enclosure ($17)
- Buck Converter ($9)
- Misc Components ($20)
- **Tools** (if needed) (+$100)

### Recommended Build (~$310)

- Arduino Mega 2560 (genuine) ($49)
- Adafruit 3.5" TFT Breakout ($55)
- Quality EGT Sensor + MAX31855 ($40)
- 3-Bar MAP Sensor ($19)
- NTC Thermistors x3 ($40)
- Wire & Connectors ($40)
- Enclosure ($17)
- Buck Converter ($9)
- Misc Components ($30)
- **Tools** (if needed) (+$100)

---

## 📦 Order Summary

**Copy this checklist when ordering:**

### Core (Required)
- [ ] Arduino Mega 2560
- [ ] TFT Display (choose one: 2.8" shield OR 3.5" breakout)
- [ ] 12V-5V Buck Converter
- [ ] microSD Card (if using 3.5" display)

### Sensors (Required)
- [ ] K-Type Thermocouple + MAX31855
- [ ] 3-Bar MAP Sensor
- [ ] NTC Thermistors (2x for IAT)
- [ ] NTC Thermistor (1x for coolant)
- [ ] Piezo Buzzer

### Wiring (Required)
- [ ] 22 AWG Wire (multiple colors)
- [ ] DuPont Jumper Wires (M-M and M-F)
- [ ] Screw Terminals
- [ ] Heat Shrink Tubing
- [ ] Weatherpack Connectors
- [ ] Inline Fuse Holders + Fuses

### Components (Required)
- [ ] 2.2kΩ Resistors (3x minimum)
- [ ] 0.1µF Capacitors (10x)

### Enclosure (Required)
- [ ] Arduino Enclosure
- [ ] M3 Standoffs
- [ ] Cable Glands

### Prototyping (Recommended)
- [ ] Breadboard (830-point)
- [ ] Breadboard Jumper Wires

### Tools (If Needed)
- [ ] Soldering Iron
- [ ] Wire Strippers
- [ ] Multimeter
- [ ] Crimping Tool

---

## 🚚 Shipping Tips

1. **Adafruit Products**: Order all Adafruit items together to save on shipping
2. **Amazon**: Use Prime for free 2-day shipping
3. **Arduino**: Consider buying from local distributor for faster shipping
4. **Bulk Discounts**: Wire and connectors are cheaper in larger quantities

---

## 💡 Cost-Saving Tips

1. **Buy Arduino-compatible clone** instead of genuine ($22 vs $49)
2. **Get starter kits** instead of individual components
3. **Use 2.8" shield** instead of 3.5" breakout (saves wiring time)
4. **Share wire spools** with friends (you won't use all 25ft)
5. **Skip breadboard** if you're confident in direct soldering

---

## ⚠️ Important Notes

- All prices current as of **November 2024** (may vary)
- Links are to popular retailers (Amazon, Adafruit, Arduino official)
- **Adafruit products** are high quality and well-documented (worth the premium)
- **Arduino genuine** boards support open-source development
- **Tools** are one-time purchase (useful for future projects)

---

**Total Cost Summary:**

| Component | Budget | Recommended |
|-----------|--------|-------------|
| Electronics | $67 | $104 |
| Sensors | $62 | $80 |
| Wiring | $30 | $40 |
| Enclosure | $17 | $17 |
| Components | $20 | $30 |
| **Subtotal** | **$196** | **$271** |
| **Tools** (if needed) | **+$100** | **+$100** |
| **Grand Total** | **$296** | **$371** |

---

**Ready to order? Check out the wiring diagrams next!**

See: [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md) for prototyping connections.

---

**Last Updated:** 2025-11-08
