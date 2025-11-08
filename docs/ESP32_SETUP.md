# ESP32-S3 Arduino IDE Setup Guide

**Getting Your Development Environment Ready for Qualia ESP32-S3**

This guide walks you through installing and configuring Arduino IDE for ESP32-S3 development.

---

## 📋 Overview

The Adafruit Qualia ESP32-S3 board can be programmed using Arduino IDE, just like an Arduino Mega. You'll need to:
1. Install Arduino IDE (if not already installed)
2. Add ESP32 board support
3. Install required libraries
4. Configure for Qualia board
5. Test with example sketch

**Time Required:** 15-30 minutes

---

## 🖥️ Step 1: Install Arduino IDE

### Download Arduino IDE

**Version 2.x (Recommended):**
- Go to: https://www.arduino.cc/en/software
- Download for your OS (Mac, Windows, Linux)
- Install following standard procedure

**Minimum Version:** Arduino IDE 1.8.19 or Arduino IDE 2.0+

---

## 🔧 Step 2: Add ESP32 Board Support

### Install ESP32 Arduino Core

1. **Open Arduino IDE**

2. **Open Preferences:**
   - Mac: Arduino IDE → Preferences
   - Windows: File → Preferences
   - Linux: File → Preferences

3. **Add Board Manager URL:**
   
   In the "Additional Boards Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
   
   **If you already have URLs there:**
   - Click the icon next to the field to open the manager
   - Add the ESP32 URL on a new line
   - Click OK

4. **Install ESP32 Boards:**
   - Go to: Tools → Board → Boards Manager
   - Search for: **"esp32"**
   - Find: **"esp32 by Espressif Systems"**
   - Click **Install** (latest version)
   - Wait for installation to complete (~200MB download)

5. **Restart Arduino IDE**

---

## ⚙️ Step 3: Select Qualia Board

### Configure for Adafruit Qualia ESP32-S3

1. **Connect Qualia board** via USB-C cable

2. **Select Board:**
   - Tools → Board → esp32
   - Select: **"Adafruit Feather ESP32-S3 No PSRAM"**
   
   **Note:** Even though Qualia HAS PSRAM, select "No PSRAM" option initially. You can enable PSRAM later if needed.

3. **Select Port:**
   - Tools → Port
   - Select your ESP32's port (usually `/dev/cu.usbmodem*` on Mac, `COM#` on Windows)
   - If you don't see the port, check USB cable and drivers

4. **Configure Settings:**
   ```
   Board: "Adafruit Feather ESP32-S3 No PSRAM"
   USB CDC On Boot: "Enabled"
   USB DFU On Boot: "Disabled"
   Upload Mode: "UART0 / Hardware CDC"
   USB Mode: "Hardware CDC and JTAG"
   PSRAM: "Disabled" (for now)
   Flash Size: "16MB"
   Partition Scheme: "Default 4MB with spiffs"
   Upload Speed: "921600"
   ```

---

## 📚 Step 4: Install Required Libraries

### Core Libraries

Open Library Manager: **Tools → Manage Libraries**

Install the following libraries (search by name):

#### 1. Display & Graphics

| Library | Version | Purpose |
|---------|---------|---------|
| **lvgl** | 9.x | Graphics library for UI |
| **Adafruit GFX Library** | Latest | Core graphics functions |

#### 2. Sensors

| Library | Version | Purpose |
|---------|---------|---------|
| **Adafruit MAX31855** | Latest | EGT thermocouple |
| **Adafruit ADS1X15** | Latest | I2C ADC module |

#### 3. I/O Expansion

| Library | Version | Purpose |
|---------|---------|---------|
| **Adafruit PCA9554** | Latest | I/O expander (built-in on Qualia) |

#### 4. Utilities

| Library | Version | Purpose |
|---------|---------|---------|
| **Adafruit BusIO** | Latest | I2C/SPI helpers (dependency) |

### Installation Steps

For each library:
1. Click "Install All" when prompted for dependencies
2. Wait for installation to complete
3. Close Library Manager when done

---

## 🔧 Step 5: Configure LVGL

### Create `lv_conf.h`

LVGL requires a configuration file in your Arduino libraries folder.

1. **Locate Arduino Libraries Folder:**
   - Mac: `~/Documents/Arduino/libraries/`
   - Windows: `Documents\Arduino\libraries\`
   - Linux: `~/Arduino/libraries/`

2. **Create `lv_conf.h` file** in libraries folder (NOT in lvgl folder)

3. **Add this content:**

```cpp
/**
 * LVGL Configuration for ESP32-S3
 * File location: ~/Documents/Arduino/libraries/lv_conf.h
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// ============================================================================
// COLOR SETTINGS
// ============================================================================

#define LV_COLOR_DEPTH 16        // 16-bit RGB565 color
#define LV_COLOR_16_SWAP 0       // Don't swap RGB bytes

// ============================================================================
// MEMORY SETTINGS
// ============================================================================

#define LV_MEM_CUSTOM 0          // Use LVGL's memory management
#define LV_MEM_SIZE (128U * 1024U) // 128KB for LVGL heap (plenty on ESP32-S3!)

// ============================================================================
// DISPLAY SETTINGS
// ============================================================================

#define LV_DISP_DEF_REFR_PERIOD 16   // Refresh every 16ms (~60 FPS)
#define LV_INDEV_DEF_READ_PERIOD 30  // Read input every 30ms

// ============================================================================
// FEATURE ENABLE/DISABLE
// ============================================================================

#define LV_USE_LOG 1             // Enable logging
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN // Only warnings and errors

// ============================================================================
// BUILT-IN WIDGETS
// ============================================================================

#define LV_USE_ARC 1             // Arc widget (gauges)
#define LV_USE_BAR 1             // Progress bar
#define LV_USE_LABEL 1           // Text labels
#define LV_USE_METER 1           // Meter widget (round gauges)
#define LV_USE_CHART 0           // Charts (disable if not needed)
#define LV_USE_SLIDER 0          // Sliders (not needed)
#define LV_USE_ROLLER 0          // Rollers (not needed)
#define LV_USE_KEYBOARD 0        // Keyboard (not needed)
#define LV_USE_TEXTAREA 0        // Text area (not needed)

// ============================================================================
// FONTS
// ============================================================================

#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_32 1

#define LV_FONT_DEFAULT &lv_font_montserrat_16

// ============================================================================
// THEMES
// ============================================================================

#define LV_USE_THEME_DEFAULT 1   // Enable default theme

// ============================================================================
// PERFORMANCE
// ============================================================================

#define LV_USE_REFR_DEBUG 0      // Disable refresh debug overlay
#define LV_USE_PERF_MONITOR 0    // Disable performance monitor

#endif /* LV_CONF_H */
```

4. **Save file**

5. **Restart Arduino IDE** (important!)

---

## ✅ Step 6: Test Installation

### Upload Test Sketch

1. **File → Examples → lvgl → LVGL_Arduino → LVGL_Arduino.ino**

2. **Modify for ESP32-S3** (if example doesn't work out of box):

```cpp
#include <lvgl.h>

void setup() {
  Serial.begin(115200);
  
  // Initialize LVGL
  lv_init();
  
  // You'll add display initialization here later
  
  Serial.println("LVGL initialized!");
}

void loop() {
  lv_timer_handler(); // Let LVGL do its work
  delay(5);
}
```

3. **Upload:**
   - Click Upload button (→)
   - Wait for compilation and upload
   - Check Serial Monitor (115200 baud) for "LVGL initialized!"

---

## 🎯 Step 7: Install Display Library

### Adafruit RGB Matrix Library

The Qualia board uses RGB-666 parallel interface. You'll need Adafruit's library:

```
Tools → Manage Libraries
Search: "Adafruit GFX Library"
Install: Adafruit GFX Library

Search: "Adafruit Protomatter"
Install: Adafruit Protomatter (for RGB matrix displays)
```

**Note:** Full display initialization code will be provided in BREADBOARD_WIRING.md

---

## 🔍 Troubleshooting

### Board Not Found in Port List

**Problem:** ESP32 doesn't appear in Tools → Port

**Solutions:**
1. Check USB-C cable (must be data cable, not charge-only)
2. Press **BOOT** button on Qualia board while uploading
3. Install CH340 or CP2102 USB drivers (Google your OS + driver name)
4. Try different USB port
5. Restart Arduino IDE

### Upload Fails

**Problem:** "A fatal error occurred: Failed to connect"

**Solutions:**
1. Hold **BOOT** button, click Upload, release after "Connecting..."
2. Check Upload Speed (try 115200 instead of 921600)
3. Select different USB Mode in Tools menu

### Library Compilation Errors

**Problem:** Errors mentioning LVGL or missing files

**Solutions:**
1. Verify `lv_conf.h` is in correct location
2. Restart Arduino IDE after creating `lv_conf.h`
3. Update all libraries to latest versions
4. Clear Arduino cache: Delete `~/Library/Arduino15/tmp/` (Mac) or similar

### Out of Memory Errors

**Problem:** "region `dram0_0_seg' overflowed"

**Solutions:**
1. Enable PSRAM: Tools → PSRAM → "QSPI PSRAM"
2. Reduce LV_MEM_SIZE in `lv_conf.h` (try 64KB)
3. Disable unused LVGL widgets
4. Use larger Flash partition

---

## 📋 Final Checklist

Before proceeding to wiring and code:

- [ ] Arduino IDE installed (2.0+ recommended)
- [ ] ESP32 board support installed
- [ ] Board selected: "Adafruit Feather ESP32-S3 No PSRAM"
- [ ] Correct port selected
- [ ] All libraries installed (lvgl, Adafruit MAX31855, ADS1X15, etc.)
- [ ] `lv_conf.h` created in libraries folder
- [ ] Arduino IDE restarted after creating `lv_conf.h`
- [ ] Test sketch compiles and uploads successfully
- [ ] Serial Monitor shows output at 115200 baud

---

## 🎯 Next Steps

**You're ready to start building!**

1. ✅ **Wire sensors to breadboard**
   - See: [BREADBOARD_WIRING.md](BREADBOARD_WIRING.md)

2. ✅ **Design GUI in SquareLine Studio**
   - See: [SQUARELINE_LVGL_GUIDE.md](SQUARELINE_LVGL_GUIDE.md)

3. ✅ **Upload and test code**
   - Full working examples coming soon

---

## 📚 Additional Resources

### ESP32-S3 Documentation
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [ESP32 Arduino Core Docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [Adafruit Qualia Guide](https://learn.adafruit.com/adafruit-qualia-esp32-s3)

### LVGL Resources
- [LVGL Documentation](https://docs.lvgl.io/)
- [LVGL Arduino Examples](https://github.com/lvgl/lvgl/tree/master/examples)
- [SquareLine Studio](https://squareline.io/)

### Community
- [Adafruit Forums](https://forums.adafruit.com/)
- [ESP32 Reddit](https://www.reddit.com/r/esp32/)
- [LVGL Forum](https://forum.lvgl.io/)

---

**Last Updated:** 2025-11-08
