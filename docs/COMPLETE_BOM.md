# Complete Bill of Materials - JNKR Gauge System

**Everything You Need to Build Your Engine Monitoring Gauge with ESP32-S3**

This is the complete parts list with exact product links, part numbers, and prices. All links are current as of November 2024.

**Platform:** Adafruit Qualia ESP32-S3 + Round RGB Display

---

## 🛒 Complete Shopping List

### Summary

| Category | Estimated Cost |
|----------|---------------|
| Core Electronics | $95-110 |
| Sensors | $70-90 |
| Wiring & Connectors | $30-40 |
| Enclosure & Hardware | $30-50 |
| **TOTAL** | **$225-290** |

💰 **Cheaper than Arduino Mega setup** and much more powerful!

---

# Core Electronics

## 🎯 ESP32-S3 Development Board + Display

### **Main Controller & Display System**

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **Adafruit Qualia ESP32-S3 RGB-666 Dev Board** | #5800 | 1 | $39.95 | [Adafruit](https://www.adafruit.com/product/5800) |
| **2.1" Round RGB TTL Display (480x480)** | TL021WVC02-B1323B* | 1 | $34.95 | [Adafruit](https://www.adafruit.com/product/5806) |
| **40-pin FFC Cable** | Usually included | 1 | (Included) | - |

\* *Product name: "Round RGB TTL TFT Display - 2.1" 480x480 - No Touchscreen"*

**Specifications:**
- **Processor:** ESP32-S3 dual-core @ 240 MHz
- **RAM:** 8MB PSRAM (1000x more than Arduino Mega!)
- **Flash:** 16MB
- **Display:** 2.1" round, 480x480 pixels, RGB-666 parallel interface
- **Graphics:** 60 FPS smooth updates
- **Connectivity:** WiFi + Bluetooth built-in
- **Programming:** Arduino IDE or CircuitPython
- **Interface:** 40-pin connector (simple plug-in)

---

### **I2C ADC Module (For Extra Analog Inputs)**

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **Adafruit ADS1015 12-Bit ADC** | #1083 | 1 | $9.95 | [Adafruit](https://www.adafruit.com/product/1083) |
| **Stemma QT Cable (100mm)** | #4210 | 1 | $0.95 | [Adafruit](https://www.adafruit.com/product/4210) |

**Why needed:** ESP32-S3 Qualia has 2 analog pins available after display. The ADS1015 adds 4 more analog inputs via I2C for thermistors.

**Specifications:**
- 4 single-ended or 2 differential inputs
- 12-bit resolution (0-4095 range)
- I2C interface (Stemma QT connector)
- Programmable gain amplifier

**Core Electronics Subtotal:** ~$86

---

### Power Supply

| Item | Part Number | Qty | Price | Link |
|------|-------------|-----|-------|------|
| **12V to 5V DC-DC Buck Converter (3A)** | LM2596 Module | 1 | $8.99 | Search: "LM2596 DC-DC Buck Converter 12V to 5V 3A" |
| **USB-C Cable (for programming/testing)** | 6ft USB-C | 1 | $7.99 | Search: "USB C Cable 6 feet USB 2.0" |

**Note:** 
- Buck converter provides 5V to ESP32-S3 via USB-C or VIN pin
- ESP32-S3 has onboard 3.3V regulator for logic
- For bench testing, USB-C cable to computer works fine

---

## 🌡️ Sensors

### Temperature Sensors

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **K-Type Thermocouple (EGT)** | M6 x 1.0 Thread, 0-1000°C | 1 | $24.99 | Search: "K-Type Thermocouple EGT M6 x 1.0 thread 1000C exhaust gas temperature sensor" |
| **MAX31855 Thermocouple Amplifier** | SPI Interface, K-Type | 1 | $14.95 | [Adafruit #269](https://www.adafruit.com/product/269) |
| **NTC Thermistor (Intake Air Temp)** | 2.2kΩ @ 25°C, M8 Sensor | 2 | $12.99/ea | Search: "M8 NTC Thermistor Temperature Sensor 2.2K ohm 25C automotive" |
| **NTC Thermistor (Coolant Temp)** | 2.2kΩ @ 25°C, 1/8" NPT | 1 | $14.99 | Search: "1/8 NPT NTC Thermistor Temperature Sensor 2.2K ohm coolant" |

**EGT Sensor Details:**
- **Probe Length:** 50-100mm (choose based on exhaust pipe size)
- **Thread:** M6 x 1.0 or 1/8" NPT (ensure compatibility with your bung)
- **Temperature Range:** 0-1000°C (1832°F)

**Alternative Budget EGT:**
| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **EGT Sensor Kit (Thermocouple + Amplifier)** | Complete Kit | 1 | $15.99 | Search: "K-Type Thermocouple EGT Kit MAX6675 MAX31855 exhaust gas temperature module" |

---

### Pressure Sensor

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **3-Bar MAP Sensor** | 0-3 bar absolute, 0.5-4.5V output | 1 | $18.99 | Search: "3 Bar MAP Sensor 0.5-4.5V output manifold absolute pressure 1/8 NPT automotive" |

**Specifications:**
- **Range:** 0-3 bar absolute (0-29 PSI gauge)
- **Output:** 0.5V @ 0 bar → 4.5V @ 3 bar
- **Thread:** 1/8" NPT or barbed nipple
- **Connector:** Pigtail included

---

### Audio Alert

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Piezo Buzzer** | Active, 3-5V, 2kHz | 1 | $6.99 (5-pack) | Search: "Active Piezo Buzzer 5V 2kHz Arduino electronic alarm beeper" |

---

## 🔌 Wiring & Connectors

### Wire

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **22 AWG Stranded Wire Spool** | 6 colors, 25ft each | 1 | $15.99 | Search: "22 AWG Stranded Hook Up Wire Kit 6 colors 25ft silicone flexible" |
| **20 AWG Automotive Wire** | For power, 25ft | 1 | $9.99 | Search: "20 AWG Automotive Primary Wire stranded copper 25 feet" |

### Connectors & Hardware

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Screw Terminal Blocks** | 5mm pitch, 2-12 pos | 1 set | $11.99 | Search: "5mm Pitch Screw Terminal Block Connector 2-12 position PCB mount" |
| **DuPont Jumper Wires** | Male-Male, 40-pin | 1 | $6.99 | Search: "DuPont Jumper Wire Male to Male 40pin 20cm breadboard cables" |
| **DuPont Jumper Wires** | Male-Female, 40-pin | 1 | $6.99 | Search: "DuPont Jumper Wire Male to Female 40pin 20cm Arduino cables" |
| **Heat Shrink Tubing Kit** | Assorted sizes | 1 | $8.99 | Search: "Heat Shrink Tubing Kit Assorted Sizes Wire Wrap 2:1 ratio" |
| **Weatherpack Connectors** | 2/3/4-pin automotive | 1 kit | $16.99 | Search: "Weatherpack Connector Kit 2 3 4 pin automotive waterproof sealed" |
| **Inline Fuse Holder** | ATC/ATO blade fuse | 2 | $8.99 | Search: "Inline ATC ATO Blade Fuse Holder 12V 14 AWG automotive waterproof" |
| **3A Blade Fuses** | ATC, 10-pack | 1 | $5.99 | Search: "ATC 3 Amp Blade Fuses Automotive 10 pack standard" |

### Prototyping Supplies

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Breadboard (830-point)** | For prototyping | 1 | $6.99 | Search: "830 Point Solderless Breadboard MB-102 Prototyping" |
| **Breadboard Jumper Wire Kit** | Pre-cut wires | 1 | $6.99 | Search: "Breadboard Jumper Wire Kit Pre-cut Pre-formed Arduino" |

---

## 🔩 Electronic Components

### Resistors & Capacitors

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Resistor Kit** | 1/4W, assorted values | 1 | $10.99 | Search: "Resistor Kit 1/4W Assorted Values 1 ohm - 1M ohm Carbon Film 600pcs" |
| **2.2kΩ Resistors** | 1/4W (for NTC dividers) | 10 | Included in kit above |
| **10kΩ Resistors** | 1/4W (pull-ups) | 10 | Included in kit above |
| **Ceramic Capacitors (0.1µF)** | 50V, power filtering | 10 | $5.99 (100-pack) | Search: "Ceramic Capacitor 0.1uF 104 50V 100 pieces multilayer MLCC" |

---

## 📦 Enclosure & Mounting

### Enclosures

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **Arduino Mega Enclosure** | IP65 rated, clear lid | 1 | $16.99 | Search: "Arduino Mega Enclosure Box IP65 Waterproof Clear Lid Project Case" |
| **Display Mounting** | 3D printed or custom | 1 | DIY | - |

### Mounting Hardware

| Item | Specs | Qty | Price | Link |
|------|-------|-----|-------|------|
| **M3 Standoffs Kit** | Brass, M-F, assorted | 1 | $12.99 | Search: "M3 Brass Standoff Kit Male Female Assorted Hex Spacer PCB" |
| **Cable Glands** | PG7, waterproof | 5 | $9.99 | Search: "PG7 Cable Gland Waterproof IP68 Nylon Wire Connector 3-6.5mm" |
| **Velcro Strips** | Industrial strength | 1 | $8.99 | Search: "Industrial Strength Velcro Strips Heavy Duty Adhesive Black" |

---

## 🔧 Tools (If You Don't Have)

| Item | Purpose | Price | Link |
|------|---------|-------|------|
| **Soldering Iron Kit** | 60W adjustable | $22.99 | Search: "Soldering Iron Kit 60W Adjustable Temperature Welding Tool Stand" |
| **Wire Strippers** | 10-22 AWG | $12.99 | Search: "Wire Strippers Klein Tools 10-22 AWG Self Adjusting Automatic" |
| **Crimping Tool** | For DuPont/JST | $15.99 | Search: "Crimping Tool Dupont JST Molex 2.54mm Terminals Ratcheting SN-28B" |
| **Multimeter** | Digital, with continuity | $19.99 | Search: "Digital Multimeter Auto Ranging Voltage Current Resistance Continuity" |
| **Heat Gun** | For heat shrink | $16.99 | Search: "Heat Gun 1500W Dual Temperature Hot Air Gun Shrink Wrap" |

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
| **M6 Exhaust Bung** | Weld-on, stainless | EGT sensor mount | Search: "M6 x 1.0 Exhaust Bung Weld-on Stainless Steel O2 Sensor EGT Bung" |
| **1/8" NPT to Barb Fitting** | Brass, 4mm barb | MAP sensor connection | Search: "1/8 NPT to 4mm Barb Fitting Brass Hose Adapter Vacuum Fuel" |
| **Vacuum Hose** | 4mm silicone, 6ft | MAP sensor line | Search: "4mm Silicone Vacuum Hose 6 feet High Temp Boost Pressure Line" |
| **Coolant Temp Adapter** | 1/8" NPT T-fitting | Coolant sensor tap | Search: "1/8 NPT T-Fitting Tee Adapter Brass Coolant Temperature Sensor Port" |
| **Intercooler Pipe Bosses** | M8 weld-on bungs | IAT sensor mounts | Search: "M8 x 1.25 Weld-On Bung Aluminum Intercooler Pipe Boss Sensor Mount" |

---

## 🛠️ Complete Build Cost

### **ESP32-S3 Qualia Build** (Recommended)

| Component Category | Cost |
|-------------------|------|
| **Core Electronics:** | |
| - Qualia ESP32-S3 Board | $39.95 |
| - 2.1" Round Display | $34.95 |
| - ADS1015 I2C ADC | $9.95 |
| - Stemma QT Cable | $0.95 |
| - Buck Converter | $8.99 |
| - USB-C Cable | $7.99 |
| **Sensors:** | |
| - EGT Sensor + MAX31855 | $39.94 |
| - 3-Bar MAP Sensor | $18.99 |
| - NTC Thermistors (3x) | $40.97 |
| - Piezo Buzzer | $6.99 |
| **Wiring & Components:** | |
| - Wire, resistors, caps, etc. | $30.00 |
| **Enclosure & Hardware:** | |
| - Enclosure, standoffs, etc. | $30.00 |
| **SUBTOTAL** | **$269.67** |
| **Tools** (if needed) | +$100.00 |
| **TOTAL** | **~$270-370** |

**💡 Comparison:** Arduino Mega setup would be $280-360 before tools, but with:
- ❌ Slower graphics (5-10 FPS vs 60 FPS)
- ❌ Less memory (8KB vs 8MB)
- ❌ Rectangular display vs round
- ❌ No WiFi/Bluetooth

---

## 📦 Order Checklist

**Copy this checklist when ordering:**

### Core Electronics (Required)
- [ ] **Adafruit Qualia ESP32-S3 Board** (#5800)
- [ ] **2.1" Round RGB Display** (TL021WVC02-B1323B)
- [ ] **ADS1015 I2C ADC Module** (#1083)
- [ ] **Stemma QT Cable** (100mm, #4210)
- [ ] **12V-5V Buck Converter** (3A, LM2596)
- [ ] **USB-C Cable** (for programming)

### Sensors (Required)
- [ ] **K-Type Thermocouple** (M6 or 1/8" NPT thread)
- [ ] **MAX31855 Module** (Adafruit #269)
- [ ] **3-Bar MAP Sensor** (0.5-4.5V output)
- [ ] **NTC Thermistors** (2x for IAT, 2.2kΩ @ 25°C)
- [ ] **NTC Thermistor** (1x for coolant, 2.2kΩ @ 25°C)
- [ ] **Piezo Buzzer** (active, 3-5V)

### Wiring & Components (Required)
- [ ] **22 AWG Wire** (multiple colors)
- [ ] **DuPont Jumper Wires** (M-M and M-F)
- [ ] **Screw Terminals** (5mm pitch)
- [ ] **Heat Shrink Tubing** (assorted sizes)
- [ ] **Weatherpack Connectors** (automotive)
- [ ] **2.2kΩ Resistors** (3x for voltage dividers)
- [ ] **0.1µF Capacitors** (10x for filtering)
- [ ] **Inline Fuse Holders + 3A Fuses**

### Enclosure & Hardware (Required)
- [ ] **Project Enclosure** (for ESP32 board)
- [ ] **M2.5/M3 Standoffs Kit**
- [ ] **Cable Glands** (waterproof)

### Prototyping (Highly Recommended)
- [ ] **Breadboard** (830-point)
- [ ] **Breadboard Jumper Wires**

### Tools (If You Don't Have)
- [ ] **Soldering Iron Kit** (60W)
- [ ] **Wire Strippers** (10-22 AWG)
- [ ] **Multimeter** (digital)
- [ ] **Crimping Tool** (for connectors)
- [ ] **Heat Gun** (for heat shrink)

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

| Component | ESP32-S3 Build |
|-----------|---------------|
| Core Electronics | $103 |
| Sensors | $107 |
| Wiring & Components | $35 |
| Enclosure | $25 |
| **Subtotal** | **$270** |
| **Tools** (if needed) | **+$100** |
| **Grand Total** | **$270-370** |

**💰 Value:** For the same price as Arduino Mega setup, you get:
- ✅ 10x faster processor
- ✅ 1000x more RAM
- ✅ Beautiful round display
- ✅ 60 FPS smooth graphics
- ✅ WiFi + Bluetooth built-in
- ✅ Future expandability

---

## 🚀 Next Steps

**After ordering:**

1. ✅ **Install Arduino IDE** and ESP32 board support
   - See: [ESP32_SETUP.md](ESP32_SETUP.md)

2. ✅ **Install SquareLine Studio** for GUI design
   - See: [SQUARELINE_LVGL_GUIDE.md](SQUARELINE_LVGL_GUIDE.md)

3. ✅ **Build breadboard prototype**
   - See: [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md)

4. ✅ **Design your GUI** visually
   - See: [SQUARELINE_LVGL_GUIDE.md](SQUARELINE_LVGL_GUIDE.md)

5. ✅ **Test and calibrate sensors**
   - See: [CALIBRATION.md](CALIBRATION.md)

---

**Last Updated:** 2025-11-08
