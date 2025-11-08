# Adafruit Product 5806 Display Specifications

**2.8" Round Rectangle TFT Display with Capacitive Touch**

Based on the Adafruit product link provided, this document details the specifications for product #5806.

---

## 🔍 Product Information

**Adafruit Product #5806**
- **Link:** https://www.adafruit.com/product/5806

**Display Type:** 2.8" Round Rectangle TFT LCD with Capacitive Touch

---

## 📊 Specifications (To Be Confirmed)

### Display Characteristics

| Parameter | Specification | Notes |
|-----------|---------------|-------|
| **Size** | 2.8" diagonal | Round rectangle (rounded corners) |
| **Resolution** | 240 x 320 pixels | Standard portrait |
| **Display Driver** | ST7789 or ILI9341 | Need to confirm from datasheet |
| **Interface** | SPI | 4-wire SPI |
| **Touch Type** | Capacitive | FT6206 or similar controller |
| **Backlight** | LED | PWM dimmable |
| **Viewing Angle** | 160° | IPS or similar |
| **Colors** | 262K (RGB565) | 16-bit color |

### Electrical

| Parameter | Specification |
|-----------|---------------|
| **Supply Voltage** | 3.3V or 5V | Check datasheet |
| **Logic Level** | 3.3V | May need level shifter for 5V Arduino |
| **Current Draw** | ~100-150mA | With backlight |
| **Backlight Current** | ~50-80mA | LED backlight |

### Physical

| Parameter | Specification |
|-----------|---------------|
| **Dimensions** | ~70mm x 70mm | Approximate, verify from datasheet |
| **Thickness** | ~8-10mm | Including PCB |
| **Mounting** | 4x M2.5 holes | Typical Adafruit breakout |
| **Connector** | Soldered header pins | Or STEMMA QT |

---

## 🔌 Pin Connections

**Typical Adafruit TFT Breakout Pinout:**

### Power Pins
- **VIN** - Power supply (3.3V or 5V)
- **GND** - Ground
- **3.3V** - 3.3V output (if using 5V input)

### SPI Display Pins
- **SCK** - SPI Clock
- **MOSI** - SPI Data Out (Master Out Slave In)
- **MISO** - SPI Data In (Master In Slave Out) - may not be present
- **CS** - Chip Select (display)
- **DC** - Data/Command select
- **RST** - Reset (can connect to Arduino reset)

### Touch Pins (Capacitive)
- **SDA** - I2C Data (for touch controller)
- **SCL** - I2C Clock (for touch controller)
- **IRQ** - Touch interrupt (optional)

### Backlight
- **LITE** - Backlight control (connect to 3.3V or PWM pin)

---

## ⚠️ IMPORTANT NOTES

### 1. Verify Exact Specifications

**Before wiring, please verify from the Adafruit product page:**
- Exact resolution
- Display controller chip (ST7789 vs ILI9341)
- Voltage requirements (3.3V vs 5V tolerant)
- Pin assignments
- Touch controller type

### 2. Arduino Mega Compatibility

**If display is 3.3V only:**
- Arduino Mega uses 5V logic
- Will need **logic level shifter** for SPI pins
- Can power display from 3.3V pin (limited current ~50mA)
- OR use external 3.3V regulator

**If display is 5V tolerant:**
- Can connect directly to Arduino Mega
- Power from 5V pin

### 3. Touch Interface Difference

**Capacitive touch uses I2C, not SPI:**
- Separate from display SPI interface
- Uses Arduino I2C pins (SDA/SCL on Mega: pins 20/21)
- Different library needed (Adafruit_FT6206 or similar)

---

## 📚 Required Arduino Libraries

### For Display
```cpp
#include <Adafruit_GFX.h>        // Core graphics
#include <Adafruit_ST7789.h>     // If using ST7789 chip
// OR
#include <Adafruit_ILI9341.h>    // If using ILI9341 chip
```

### For Capacitive Touch
```cpp
#include <Wire.h>                // I2C library
#include <Adafruit_FT6206.h>     // Capacitive touch (if FT6206)
// OR check product page for exact touch controller
```

### For LVGL
```cpp
#include <lvgl.h>                // LVGL graphics library
```

---

## 🔧 Arduino Mega Wiring (Preliminary)

**⚠️ VERIFY VOLTAGE REQUIREMENTS FIRST!**

### If 5V Tolerant (Direct Connection)

| Display Pin | Arduino Mega Pin | Function |
|-------------|------------------|----------|
| **VIN** | 5V | Power |
| **GND** | GND | Ground |
| **SCK** | 52 | SPI Clock |
| **MOSI** | 51 | SPI Data Out |
| **MISO** | 50 | SPI Data In |
| **CS** | 10 | Display Chip Select |
| **DC** | 9 | Data/Command |
| **RST** | 8 | Reset |
| **LITE** | 5V or PWM | Backlight |
| **SDA** | 20 | I2C Data (touch) |
| **SCL** | 21 | I2C Clock (touch) |
| **IRQ** | 12 | Touch Interrupt (optional) |

### If 3.3V Only (Needs Level Shifter)

**Option 1: Use Logic Level Shifter**
- Get 4-channel bidirectional shifter
- Shift: SCK, MOSI, CS, DC, RST
- Power display from 3.3V regulator
- I2C pins (SDA/SCL) also need shifting

**Option 2: Use Adafruit Feather M4 Instead**
- Native 3.3V logic
- More powerful processor
- Better for LVGL graphics

---

## 🎨 SquareLine Studio Configuration

### Project Settings

When creating project in SquareLine Studio:

- **Board:** Arduino
- **Display:** TFT
- **Resolution:** 240 x 320 (portrait) or 320 x 240 (landscape)
- **Color Depth:** 16-bit (RGB565)
- **LVGL Version:** 9.x
- **Rotation:** Set based on mounting orientation

---

## 📋 Action Items Before Proceeding

### 1. Check Adafruit Product Page

Visit: https://www.adafruit.com/product/5806

**Confirm:**
- [ ] Exact display size (2.8"?)
- [ ] Resolution (240x320?)
- [ ] Controller chip (ST7789? ILI9341?)
- [ ] **Voltage: 3.3V or 5V tolerant?** ⚠️ CRITICAL
- [ ] Touch controller type (FT6206? CST816?)
- [ ] Pin assignments
- [ ] Datasheet link

### 2. Download Resources

- [ ] Schematic/pinout diagram
- [ ] Arduino library name
- [ ] Example sketches
- [ ] Datasheet

### 3. Verify Compatibility

- [ ] Check if 5V tolerant (for Arduino Mega)
- [ ] Check current requirements (Arduino 5V pin: 800mA max)
- [ ] Verify SPI compatibility
- [ ] Check LVGL support

---

## 🔗 Resources to Check

### Adafruit Product Page
- Technical specifications
- Pinout diagram
- Downloads section (Arduino libraries, datasheets)
- Schematic
- Learn guide

### Arduino Libraries
Once confirmed, install via Library Manager:
```
Tools → Manage Libraries → Search:
- "Adafruit GFX"
- "Adafruit ST7789" or "Adafruit ILI9341"
- "Adafruit FT6206" (if capacitive touch)
- "lvgl"
```

---

## 🚨 Critical Questions to Answer

1. **Is this display 3.3V or 5V tolerant?**
   - If 3.3V only → Need level shifter or different Arduino
   - If 5V tolerant → Direct connection OK

2. **What is the exact display controller chip?**
   - ST7789 → Use Adafruit_ST7789 library
   - ILI9341 → Use Adafruit_ILI9341 library

3. **What is the touch controller?**
   - FT6206 → Use Adafruit_FT6206 library
   - Other → Check Adafruit product page

4. **Current draw?**
   - If >200mA with backlight → May need separate power supply
   - Arduino 5V pin limited to ~800mA total

---

## 📝 Next Steps

1. ✅ **Visit the Adafruit product page and note:**
   - Voltage requirements
   - Controller chips
   - Pin assignments
   - Any special requirements

2. ✅ **Download resources:**
   - Adafruit Learn guide
   - Schematic
   - Arduino libraries

3. ✅ **Update documentation:**
   - Once specs confirmed, I'll update all wiring diagrams
   - Update BOM with correct product
   - Update code examples

**Please share the key specifications from the product page and I'll update all the documentation accordingly!**

---

**Last Updated:** 2025-11-08
