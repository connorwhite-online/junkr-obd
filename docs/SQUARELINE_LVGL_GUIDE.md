# SquareLine Studio + LVGL Display Guide

**Mac-Compatible Visual GUI Design for Your Engine Gauge**

This guide shows you how to use **SquareLine Studio** (visual GUI designer) with **LVGL** (Light and Versatile Graphics Library) to create a professional display for your JNKR Gauge system - all from your Mac, with no Windows required!

---

## 🎯 Why SquareLine Studio + LVGL?

### The Benefits
- ✅ **SquareLine Studio**: Professional visual GUI editor that runs on Mac!
- ✅ **LVGL**: Modern, powerful graphics library
- ✅ **100% Mac compatible**: No Windows, no VM, no Boot Camp needed
- ✅ **Drag-and-drop design**: Professional-looking gauges in minutes

### Feature Comparison

| Feature | SquareLine + LVGL | Notes |
|---------|-------------------|-------|
| **Mac Compatible** | ✅ **YES!** | Native Mac application |
| **Visual Editor** | ✅ **YES!** | Drag-and-drop interface |
| **Fast Updates** | ✅ **Fast** | Optimized rendering |
| **Professional UI** | ✅ **YES!** | Built-in beautiful widgets |
| **Animations** | ✅ **Built-in** | Smooth transitions |
| **Widgets** | ✅ **Rich** | 30+ professional widgets |
| **Arduino Overhead** | ⚠️ **Medium** | More than serial, less than basic drawing |

**Perfect for creating a professional engine monitoring display!**

---

## 📐 Architecture Overview

```
┌─────────────────────┐
│  SquareLine Studio  │  ← Design GUI visually on Mac
│  (Visual Editor)    │
└──────────┬──────────┘
           │ Export
           ↓
┌─────────────────────┐
│   LVGL UI Code      │  ← C code for your UI
│   (C files)         │
└──────────┬──────────┘
           │ Include in project
           ↓
┌─────────────────────┐       SPI        ┌─────────────────┐
│  Arduino Mega 2560  │ ───────────────→ │  Adafruit TFT   │
│  + LVGL Library     │                  │  Display        │
│  + Your Sensor Code │                  │  (320x240)      │
└─────────────────────┘                  └─────────────────┘
```

**Workflow:**
1. Design GUI in **SquareLine Studio** (visual, drag-and-drop)
2. Export to **LVGL C code**
3. Import into **Arduino IDE**
4. Connect to your sensor data
5. Upload to Arduino Mega

---

## 🎨 What is SquareLine Studio?

**SquareLine Studio** is a professional GUI designer for embedded displays.

### Features
- 🖱️ **Drag-and-drop interface** - like Nextion Editor
- 📊 **Built-in widgets**: gauges, charts, meters, buttons, labels, sliders
- 🎨 **Visual styling**: colors, fonts, animations
- 📱 **Responsive layouts**: auto-sizing, positioning
- 💾 **Exports to LVGL C code** - ready for Arduino
- 🍎 **Mac native** - runs perfectly on macOS

### Download
- **Website**: [https://squareline.io](https://squareline.io)
- **Free version**: Available (with limitations)
- **Pro version**: $249/year (optional, not needed for this project)
- **Platforms**: Mac, Windows, Linux

**Free Version Limitations:**
- 1 project at a time (fine for us)
- Basic widgets (enough for gauges)
- Export to LVGL (✅ we need this)

---

## 📚 What is LVGL?

**LVGL (Light and Versatile Graphics Library)** is a modern open-source graphics library for embedded systems.

### Why LVGL is Great

**Performance:**
- ✅ Fast rendering (optimized for low-power MCUs)
- ✅ Smart refresh (only updates changed pixels)
- ✅ Frame buffering (smooth animations)
- ✅ Works well on Arduino Mega

**Features:**
- ✅ 30+ built-in widgets (gauges, charts, meters, buttons)
- ✅ Smooth animations
- ✅ Anti-aliased graphics
- ✅ Multiple fonts and styles
- ✅ Touch support

**Arduino Support:**
- ✅ Official Arduino library
- ✅ Works with Adafruit TFT displays
- ✅ Active community
- ✅ Extensive examples

### LVGL Library
- **GitHub**: [https://github.com/lvgl/lvgl](https://github.com/lvgl/lvgl)
- **Docs**: [https://docs.lvgl.io](https://docs.lvgl.io)
- **Arduino Library**: Available in Library Manager

---

## 🔧 Hardware Requirements

### Display Options (Arduino Mega Compatible)

**Recommended:**

| Display | Resolution | Touch | Size | Price | Link |
|---------|-----------|-------|------|-------|------|
| **Adafruit 2.8" TFT Shield** | 240x320 | Resistive | 2.8" | $45 | [#1651](https://www.adafruit.com/product/1651) |
| **Adafruit 3.5" TFT Breakout** | 320x480 | Resistive | 3.5" | $55 | [#2050](https://www.adafruit.com/product/2050) |
| **Adafruit 2.8" Capacitive** | 240x320 | Capacitive | 2.8" | $40 | [#1947](https://www.adafruit.com/product/1947) |

**I recommend the 3.5" (320x480) for more screen real estate.**

### Pin Connections

Same as before - display connects via SPI:

```cpp
// TFT Display (SPI)
#define PIN_TFT_CS          10   // Chip Select
#define PIN_TFT_DC          9    // Data/Command
#define PIN_TFT_RST         8    // Reset
#define PIN_TFT_MOSI        51   // SPI MOSI (hardware)
#define PIN_TFT_MISO        50   // SPI MISO (hardware)
#define PIN_TFT_SCK         52   // SPI SCK (hardware)

// Touch (if separate)
#define PIN_TOUCH_CS        11   // Touch CS
```

---

## 🚀 Setup Guide

### Step 1: Install SquareLine Studio (on Mac)

1. **Download:**
   - Go to [squareline.io](https://squareline.io)
   - Click "Download"
   - Choose macOS version
   - Download DMG file

2. **Install:**
   - Open DMG file
   - Drag SquareLine Studio to Applications
   - Launch app

3. **Create Account:**
   - Launch SquareLine Studio
   - Create free account
   - Login

4. **Activate Free License:**
   - Help → License
   - Select "Free" version
   - Activate

### Step 2: Install Arduino Libraries

In Arduino IDE:

```
Tools → Manage Libraries → Search and install:

1. "lvgl" by lvgl (v9.x)
   - Install ALL dependencies when prompted

2. "Adafruit GFX Library" by Adafruit
   - Core graphics support

3. "Adafruit ILI9341" or "Adafruit ST7789"
   - Choose based on your display chip
   - Check product page for chip type

4. "Adafruit TouchScreen" (for resistive)
   OR "Adafruit FT6206 Library" (for capacitive)
   - Choose based on touch type
```

### Step 3: Configure LVGL for Arduino

LVGL requires configuration. Create `lv_conf.h` in your Arduino libraries folder:

**Location:** `~/Documents/Arduino/libraries/lv_conf.h`

**Contents:**
```cpp
/**
 * LVGL Configuration for Arduino Mega 2560
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Color settings */
#define LV_COLOR_DEPTH 16        // 16-bit color (RGB565)
#define LV_COLOR_16_SWAP 0       // Don't swap bytes

/* Memory settings */
#define LV_MEM_CUSTOM 0          // Use LVGL's built-in memory management
#define LV_MEM_SIZE (32U * 1024U) // 32KB for LVGL heap

/* Display settings */
#define LV_DISP_DEF_REFR_PERIOD 30  // Refresh every 30ms (~33 fps)
#define LV_INDEV_DEF_READ_PERIOD 30 // Read touch every 30ms

/* Feature enable */
#define LV_USE_LOG 1             // Enable logging for debug
#define LV_USE_ASSERT_NULL 1     // Check NULL pointers
#define LV_USE_ASSERT_MEM 1      // Check memory allocation

/* Widgets - Enable what you need */
#define LV_USE_ARC 1             // Arc/gauge widget
#define LV_USE_BAR 1             // Progress bar
#define LV_USE_LABEL 1           // Text label
#define LV_USE_METER 1           // Meter/gauge
#define LV_USE_CHART 1           // Charts (optional)
#define LV_USE_SLIDER 0          // Slider (not needed)
#define LV_USE_ROLLER 0          // Roller (not needed)

/* Fonts - Enable fonts you'll use */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_32 1

/* Default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_16

/* Performance optimization for Arduino Mega */
#define LV_USE_REFR_DEBUG 0      // Disable refresh debug overlay
#define LV_USE_PERF_MONITOR 0    // Disable performance monitor

#endif /* LV_CONF_H */
```

**Important:** Restart Arduino IDE after creating this file!

---

## 🎨 Creating Your GUI in SquareLine Studio

### Step 1: Create New Project

1. **Launch SquareLine Studio**
2. **Create Project:**
   - File → New Project
   - **Board:** Arduino
   - **Display:** TFT
   - **Resolution:** 
     - 2.8" display: 240 x 320
     - 3.5" display: 320 x 480
   - **Color Depth:** 16-bit (RGB565)
   - **LVGL Version:** 9.x
   - **Project Name:** JNKR_Gauge
   - **Project Path:** Choose folder

### Step 2: Design Main Screen

**Layout Example for 320x480 Display:**

```
┌──────────────────────────────────┐
│         JNKR GAUGE SYSTEM        │ ← Title Label
├──────────────────────────────────┤
│                                  │
│   ╭─────────────╮ ╭───────────╮ │
│   │   BOOST     │ │    EGT    │ │ ← Meter Widgets
│   │  ╱───╲      │ │  ╱───╲    │ │
│   │  15.5 PSI   │ │  625°C    │ │
│   ╰─────────────╯ ╰───────────╯ │
│                                  │
│   ╭─────────────╮ ╭───────────╮ │
│   │  IAT PRE    │ │ COOLANT   │ │
│   │  125°C ▂▂▂▆ │ │  88°C ▂▂▆ │ │ ← Bar Indicators
│   ╰─────────────╯ ╰───────────╯ │
│                                  │
│   ╭─────────────╮                │
│   │  IAT POST   │                │
│   │  45°C  ▂▆   │                │
│   ╰─────────────╯                │
│                                  │
│  ⚠️  WARNING: High Exhaust Temp  │ ← Alert Label
└──────────────────────────────────┘
```

#### Add Widgets:

**1. Title Label:**
- Drag **Label** widget
- Text: "JNKR GAUGE SYSTEM"
- Font: Montserrat 24 Bold
- Color: White
- Position: Top center
- Alignment: Center

**2. Boost Gauge (Meter):**
- Drag **Meter** widget
- Configure:
  - Range: 0-25 (PSI)
  - Indicators: Arc + Needle
  - Color zones:
    - 0-12: Green
    - 12-15: Yellow
    - 15-19: Orange
    - 19-25: Red
- Add **Label** below for value
- Object name: `ui_BoostMeter`

**3. EGT Gauge (Meter):**
- Drag **Meter** widget
- Configure:
  - Range: 0-800°C
  - Color zones:
    - 0-550: Green
    - 550-600: Yellow
    - 600-650: Orange
    - 650-800: Red
- Object name: `ui_EGTMeter`

**4. Temperature Bars (Bar widgets):**
- Drag **Bar** widget (horizontal)
- Configure for each temp:
  - IAT Pre: 0-180°C
  - IAT Post: 0-100°C
  - Coolant: 0-120°C
- Object names: `ui_IATPreBar`, `ui_IATPostBar`, `ui_CoolantBar`

**5. Value Labels:**
- Drag **Label** below each gauge/bar
- Font: Montserrat 20
- Object names: `ui_BoostValue`, `ui_EGTValue`, etc.

**6. Alert Label:**
- Drag **Label** at bottom
- Font: Montserrat 16 Bold
- Color: Red
- Initially hidden
- Object name: `ui_AlertLabel`

### Step 3: Configure Widget Properties

For each widget, set the **Object Name** (important!):

| Widget | Object Name | Purpose |
|--------|-------------|---------|
| Boost Meter | `ui_BoostMeter` | Boost gauge |
| Boost Label | `ui_BoostValue` | Boost numeric value |
| EGT Meter | `ui_EGTMeter` | Exhaust temp gauge |
| EGT Label | `ui_EGTValue` | EGT numeric value |
| IAT Pre Bar | `ui_IATPreBar` | Pre-IC temp bar |
| IAT Pre Label | `ui_IATPreValue` | Pre-IC temp value |
| IAT Post Bar | `ui_IATPostBar` | Post-IC temp bar |
| IAT Post Label | `ui_IATPostValue` | Post-IC temp value |
| Coolant Bar | `ui_CoolantBar` | Coolant temp bar |
| Coolant Label | `ui_CoolantValue` | Coolant temp value |
| Alert Label | `ui_AlertLabel` | Alert message |

### Step 4: Export UI Code

1. **File → Export → Export UI Files**
2. **Export To:** Choose folder (e.g., `~/JNKR_Gauge_UI`)
3. **Format:** Arduino
4. Click **Export**

**Exported files:**
```
JNKR_Gauge_UI/
├── ui.c           ← Main UI code
├── ui.h           ← UI header
├── ui_Screen1.c   ← Screen 1 code
└── images/        ← Images (if any)
```

---

## 🔧 Integrating LVGL UI with Arduino

### Step 1: Create Arduino Project Structure

```
jnkr-gauge/
├── jnkr-gauge.ino     ← Main sketch
├── config.h
├── sensors.h
├── sensors.cpp
├── alerts.h
├── alerts.cpp
├── display.h          ← Updated for LVGL
├── display.cpp        ← Updated for LVGL
└── ui/                ← NEW: SquareLine export
    ├── ui.c
    ├── ui.h
    └── ui_Screen1.c
```

### Step 2: Copy UI Files

1. Create `ui` folder in your Arduino sketch folder
2. Copy all exported files from SquareLine Studio into `ui/`

### Step 3: Update `display.h`

```cpp
/*
 * Display Manager - Header
 * LVGL + SquareLine Studio UI
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * Initialize LVGL display and UI
 * Must be called once during setup()
 */
void Display_Init();

/**
 * Update LVGL (call in main loop)
 * This handles rendering and touch
 */
void Display_Update();

// ============================================================================
// GAUGE UPDATES
// ============================================================================

/**
 * Update all gauge values on the display
 */
void Display_UpdateGauges(
  float boostPSI,
  float iatPre,
  float iatPost,
  float exhaustTemp,
  float coolantTemp
);

/**
 * Update boost pressure display
 */
void Display_UpdateBoost(float boostPSI);

/**
 * Update intake temperature display - Pre-Intercooler
 */
void Display_UpdateIntakeTempPre(float temp);

/**
 * Update intake temperature display - Post-Intercooler
 */
void Display_UpdateIntakeTempPost(float temp);

/**
 * Update exhaust temperature display
 */
void Display_UpdateExhaustTemp(float temp);

/**
 * Update coolant temperature display
 */
void Display_UpdateCoolantTemp(float temp);

// ============================================================================
// STATUS AND ALERTS
// ============================================================================

/**
 * Show alert message on display
 */
void Display_ShowAlert(const char* message, uint16_t color);

/**
 * Clear alert message
 */
void Display_ClearAlert();

/**
 * Set alert level (changes background/status)
 */
void Display_SetAlertLevel(uint8_t level);

#endif // DISPLAY_H
```

### Step 4: Update `display.cpp`

```cpp
/*
 * Display Manager - Implementation
 * LVGL + SquareLine Studio UI
 */

#include "display.h"
#include "config.h"
#include <lvgl.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>  // Or ST7789, depending on display
#include "ui/ui.h"              // SquareLine Studio UI

// ============================================================================
// HARDWARE SETUP
// ============================================================================

// Display driver
Adafruit_ILI9341 tft = Adafruit_ILI9341(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// LVGL display buffer (1/10 of screen)
#define LVGL_BUFFER_SIZE (TFT_WIDTH * TFT_HEIGHT / 10)
static lv_color_t lvgl_buf[LVGL_BUFFER_SIZE];

// LVGL objects
static lv_display_t *disp;

// ============================================================================
// LVGL DISPLAY DRIVER
// ============================================================================

/**
 * Flush display buffer to TFT
 * Called by LVGL when screen needs update
 */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.writePixels((uint16_t *)color_p, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

// ============================================================================
// PUBLIC FUNCTIONS - INITIALIZATION
// ============================================================================

void Display_Init() {
  // Initialize TFT display
  tft.begin();
  tft.setRotation(3);  // Landscape
  tft.fillScreen(ILI9341_BLACK);

  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Display: Initializing LVGL..."));
  #endif

  // Initialize LVGL
  lv_init();

  // Create display
  disp = lv_display_create(TFT_WIDTH, TFT_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, lvgl_buf, NULL, LVGL_BUFFER_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Initialize SquareLine Studio UI
  ui_init();

  #if ENABLE_SERIAL_DEBUG
  Serial.println(F("Display: LVGL + UI initialized"));
  #endif
}

void Display_Update() {
  // Let LVGL handle rendering and animations
  lv_timer_handler();
}

// ============================================================================
// PUBLIC FUNCTIONS - GAUGE UPDATES
// ============================================================================

void Display_UpdateGauges(
  float boostPSI,
  float iatPre,
  float iatPost,
  float exhaustTemp,
  float coolantTemp
) {
  Display_UpdateBoost(boostPSI);
  Display_UpdateIntakeTempPre(iatPre);
  Display_UpdateIntakeTempPost(iatPost);
  Display_UpdateExhaustTemp(exhaustTemp);
  Display_UpdateCoolantTemp(coolantTemp);
}

void Display_UpdateBoost(float boostPSI) {
  // Update meter widget (exported from SquareLine)
  lv_meter_set_indicator_value(ui_BoostMeter, 0, (int32_t)boostPSI);
  
  // Update text label
  char buf[16];
  snprintf(buf, sizeof(buf), "%.1f PSI", boostPSI);
  lv_label_set_text(ui_BoostValue, buf);
  
  // Change color based on value
  if (boostPSI > BOOST_CRITICAL_PSI) {
    lv_obj_set_style_text_color(ui_BoostValue, lv_color_hex(0xFF0000), 0);  // Red
  } else if (boostPSI > BOOST_WARNING_PSI) {
    lv_obj_set_style_text_color(ui_BoostValue, lv_color_hex(0xFFFF00), 0);  // Yellow
  } else {
    lv_obj_set_style_text_color(ui_BoostValue, lv_color_hex(0x00FF00), 0);  // Green
  }
}

void Display_UpdateIntakeTempPre(float temp) {
  // Update bar widget
  lv_bar_set_value(ui_IATPreBar, (int32_t)temp, LV_ANIM_OFF);
  
  // Update label
  char buf[16];
  snprintf(buf, sizeof(buf), "%d C", (int)temp);
  lv_label_set_text(ui_IATPreValue, buf);
  
  // Color coding
  if (temp > IAT_PRE_CRITICAL) {
    lv_obj_set_style_bg_color(ui_IATPreBar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  } else if (temp > IAT_PRE_WARNING) {
    lv_obj_set_style_bg_color(ui_IATPreBar, lv_color_hex(0xFFFF00), LV_PART_INDICATOR);
  } else {
    lv_obj_set_style_bg_color(ui_IATPreBar, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
  }
}

void Display_UpdateIntakeTempPost(float temp) {
  lv_bar_set_value(ui_IATPostBar, (int32_t)temp, LV_ANIM_OFF);
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%d C", (int)temp);
  lv_label_set_text(ui_IATPostValue, buf);
  
  // Color coding
  if (temp > IAT_POST_CRITICAL) {
    lv_obj_set_style_bg_color(ui_IATPostBar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  } else if (temp > IAT_POST_WARNING) {
    lv_obj_set_style_bg_color(ui_IATPostBar, lv_color_hex(0xFFFF00), LV_PART_INDICATOR);
  } else {
    lv_obj_set_style_bg_color(ui_IATPostBar, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
  }
}

void Display_UpdateExhaustTemp(float temp) {
  // Update meter
  lv_meter_set_indicator_value(ui_EGTMeter, 0, (int32_t)temp);
  
  // Update label
  char buf[16];
  snprintf(buf, sizeof(buf), "%d C", (int)temp);
  lv_label_set_text(ui_EGTValue, buf);
  
  // Color coding
  if (temp > EGT_DANGER) {
    lv_obj_set_style_text_color(ui_EGTValue, lv_color_hex(0xFF0000), 0);
  } else if (temp > EGT_CRITICAL) {
    lv_obj_set_style_text_color(ui_EGTValue, lv_color_hex(0xFF8000), 0);  // Orange
  } else if (temp > EGT_WARNING) {
    lv_obj_set_style_text_color(ui_EGTValue, lv_color_hex(0xFFFF00), 0);
  } else {
    lv_obj_set_style_text_color(ui_EGTValue, lv_color_hex(0x00FF00), 0);
  }
}

void Display_UpdateCoolantTemp(float temp) {
  lv_bar_set_value(ui_CoolantBar, (int32_t)temp, LV_ANIM_OFF);
  
  char buf[16];
  snprintf(buf, sizeof(buf), "%d C", (int)temp);
  lv_label_set_text(ui_CoolantValue, buf);
  
  // Color coding
  if (temp > COOLANT_DANGER) {
    lv_obj_set_style_bg_color(ui_CoolantBar, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
  } else if (temp > COOLANT_CRITICAL) {
    lv_obj_set_style_bg_color(ui_CoolantBar, lv_color_hex(0xFF8000), LV_PART_INDICATOR);
  } else if (temp > COOLANT_WARNING) {
    lv_obj_set_style_bg_color(ui_CoolantBar, lv_color_hex(0xFFFF00), LV_PART_INDICATOR);
  } else {
    lv_obj_set_style_bg_color(ui_CoolantBar, lv_color_hex(0x00FF00), LV_PART_INDICATOR);
  }
}

// ============================================================================
// PUBLIC FUNCTIONS - STATUS AND ALERTS
// ============================================================================

void Display_ShowAlert(const char* message, uint16_t color) {
  lv_label_set_text(ui_AlertLabel, message);
  lv_obj_set_style_text_color(ui_AlertLabel, lv_color_hex(color), 0);
  lv_obj_clear_flag(ui_AlertLabel, LV_OBJ_FLAG_HIDDEN);  // Show
}

void Display_ClearAlert() {
  lv_obj_add_flag(ui_AlertLabel, LV_OBJ_FLAG_HIDDEN);  // Hide
}

void Display_SetAlertLevel(uint8_t level) {
  // Could change background color, status indicator, etc.
  // Implement based on your UI design
}
```

### Step 5: Update Main Sketch

```cpp
// jnkr-gauge.ino

#include "config.h"
#include "sensors.h"
#include "display.h"
#include "alerts.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize modules
  Sensors_Init();
  Display_Init();      // Initializes LVGL + UI
  Alerts_Init();
  
  Serial.println("System Ready!");
}

void loop() {
  unsigned long now = millis();
  
  // Update sensors (10Hz)
  static unsigned long lastSensor = 0;
  if (now - lastSensor >= 100) {
    lastSensor = now;
    Sensors_Update();
  }
  
  // Update display (20Hz - LVGL needs frequent calls)
  static unsigned long lastDisplay = 0;
  if (now - lastDisplay >= 50) {
    lastDisplay = now;
    Display_Update();  // IMPORTANT: Call LVGL timer handler
  }
  
  // Update gauge values (5Hz)
  static unsigned long lastGauge = 0;
  if (now - lastGauge >= 200) {
    lastGauge = now;
    Display_UpdateGauges(
      Sensors_GetBoostPSI(),
      Sensors_GetIntakeTempPre(),
      Sensors_GetIntakeTempPost(),
      Sensors_GetExhaustTemp(),
      Sensors_GetCoolantTemp()
    );
  }
  
  // Update alerts (2Hz)
  static unsigned long lastAlert = 0;
  if (now - lastAlert >= 500) {
    lastAlert = now;
    // Check alerts and update display...
  }
  
  Alerts_Update();
}
```

---

## 📊 Performance Considerations

### Arduino Mega Memory Usage

**LVGL Requirements:**
- Flash (program): ~50-100 KB (depending on widgets used)
- RAM: ~25-35 KB (for LVGL + buffers)

**Arduino Mega Specs:**
- Flash: 256 KB ✅ (plenty of room)
- RAM: 8 KB ⚠️ (tight fit!)

**Optimization Tips:**

1. **Reduce LVGL buffer size:**
```cpp
// In lv_conf.h
#define LV_MEM_SIZE (24U * 1024U)  // Reduce to 24KB if needed
```

2. **Disable unused widgets:**
```cpp
// In lv_conf.h
#define LV_USE_CHART 0      // Disable if not using
#define LV_USE_KEYBOARD 0   // Disable keyboard
#define LV_USE_TEXTAREA 0   // Disable text area
```

3. **Use simple animations:**
```cpp
// In SquareLine, minimize animations
// Use LV_ANIM_OFF when updating values
lv_bar_set_value(bar, value, LV_ANIM_OFF);
```

4. **Monitor RAM usage:**
```cpp
void printMemoryUsage() {
  extern int __heap_start, *__brkval;
  int free_memory = (int)&free_memory - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
  Serial.print("Free RAM: ");
  Serial.println(free_memory);
}
```

### Display Update Rate

LVGL is more efficient than basic drawing, but still has overhead:

- **Target:** 20-30 FPS (call `lv_timer_handler()` every 30-50ms)
- **Sensor updates:** 5 Hz (every 200ms) is sufficient
- **Smooth animations:** LVGL handles this automatically

---

## ✅ Summary: Why SquareLine + LVGL is Perfect

| Aspect | Rating | Notes |
|--------|--------|-------|
| **Mac Compatible** | ✅✅✅ | 100% native Mac app |
| **Visual Design** | ✅✅✅ | Drag-and-drop like Nextion |
| **Performance** | ✅✅ | Much better than hand-coded |
| **Professional Look** | ✅✅✅ | Beautiful built-in widgets |
| **Learning Curve** | ✅✅ | Moderate (easier than hand-coding) |
| **Arduino Overhead** | ✅✅ | Optimized, but still uses RAM |
| **Customization** | ✅✅✅ | Unlimited flexibility |
| **Cost** | ✅✅✅ | Free version works! |

**This is the BEST solution for your situation!**

---

## 🎯 Next Steps

1. ✅ **Download SquareLine Studio** (Mac version)
2. ✅ **Order Adafruit TFT display** (recommend 3.5" #2050)
3. ✅ **Install LVGL library** in Arduino IDE
4. ✅ **Create lv_conf.h** configuration file
5. ✅ **Design UI** in SquareLine Studio
6. ✅ **Export and integrate** into Arduino project
7. ✅ **Test and refine**

**Want me to help with any of these steps?** I can:
- Provide a ready-to-use `lv_conf.h` template
- Create a complete example with LVGL
- Help troubleshoot memory issues
- Design a specific UI layout

Let me know! 🚀

---

**Last Updated:** 2025-11-08
