# Migrating from Nextion to Adafruit TFT Display

**Switching from Nextion Smart Display to Adafruit TFT for Mac Compatibility**

This guide explains how to migrate your JNKR Gauge system from the Nextion HMI display to an Adafruit TFT touchscreen display, which can be programmed entirely from a Mac using the Arduino IDE.

---

## 🎯 Overview

### Why Switch?

The Nextion Editor (used to design the GUI) is primarily Windows-based, which creates a barrier for Mac users. The Adafruit TFT display approach uses Arduino libraries and code, making it **fully cross-platform** - you can develop on Mac, Windows, or Linux.

### Proposed Hardware

| Component | Part Number | Purpose |
|-----------|-------------|---------|
| **Display** | [Adafruit 5806](https://www.adafruit.com/product/5806) | 2.8" TFT LCD 320x240 with Capacitive Touch |
| **Driver** | [Adafruit 5800](https://www.adafruit.com/product/5800) | TFT Display FeatherWing (if using Feather) |

**⚠️ IMPORTANT:** Product 5800 is a **FeatherWing**, designed for Adafruit Feather boards, NOT Arduino Mega!

---

## 🔄 Architecture Comparison

### Current: Nextion Display

```
┌──────────────┐       Serial UART        ┌────────────────────┐
│  Arduino     │ ────────────────────────→│  Nextion Display   │
│  Mega 2560   │  (simple text commands)  │  (Smart Display)   │
│              │                          │  - Renders graphics│
│              │                          │  - Handles touch   │
└──────────────┘                          │  - Built-in GUI    │
                                          └────────────────────┘
```

**Communication:**
```cpp
// Send simple commands
Serial2.print("nBoost.txt=\"15.5\"");  // Update text
Serial2.write(0xFF); Serial2.write(0xFF); Serial2.write(0xFF);  // Terminator
```

**Pros:**
- ✅ Minimal Arduino overhead
- ✅ Fast updates
- ✅ Professional GUI editor (drag-and-drop)
- ✅ Touch handling built-in

**Cons:**
- ❌ Nextion Editor requires Windows (VM or Boot Camp on Mac)
- ❌ Separate tool for GUI design
- ❌ Proprietary protocol

---

### Proposed: Adafruit TFT Display

```
┌──────────────┐        SPI Bus           ┌────────────────────┐
│  Arduino     │ ────────────────────────→│  Adafruit TFT      │
│  Mega 2560   │  (pixel data, commands)  │  (Dumb Display)    │
│              │                          │  - Shows pixels    │
│ Renders all  │                          │  - Reports touch   │
│ graphics     │                          │                    │
└──────────────┘                          └────────────────────┘
```

**Communication:**
```cpp
// Arduino draws everything
tft.fillRect(x, y, w, h, COLOR);
tft.setCursor(x, y);
tft.setTextColor(ST77XX_GREEN);
tft.print("15.5 PSI");
```

**Pros:**
- ✅ **100% Mac compatible** - all code in Arduino IDE
- ✅ Complete control over GUI appearance
- ✅ No separate tools needed
- ✅ Open-source libraries
- ✅ More flexible customization

**Cons:**
- ❌ **Significant Arduino overhead** - drawing is slow
- ❌ More complex code
- ❌ Must implement all GUI elements in code
- ❌ Slower screen updates
- ❌ Uses more Arduino pins (SPI + control pins)

---

## 🔌 Hardware Connections for Arduino Mega

### Option 1: Direct TFT Connection (Recommended)

If you're using the standalone Adafruit 2.8" TFT (product 5806 or similar):

#### Pin Connections

| TFT Pin | Arduino Mega Pin | Function |
|---------|------------------|----------|
| **VIN** | 5V | Power (200-300mA) |
| **GND** | GND | Ground |
| **SCK** | 52 | SPI Clock |
| **MISO** | 50 | SPI Data In (for touch) |
| **MOSI** | 51 | SPI Data Out |
| **CS** | 10 (or any digital) | TFT Chip Select |
| **DC** | 9 (or any digital) | Data/Command select |
| **RST** | 8 (or any digital) | Reset |
| **LITE** | 3.3V or PWM pin | Backlight control |

**Touch Controller (if separate):**
| Touch Pin | Arduino Mega Pin | Function |
|-----------|------------------|----------|
| **T_CS** | 11 (or any digital) | Touch Chip Select |
| **T_IRQ** | 12 (or any digital, optional) | Touch Interrupt |

**Updated Pin Assignments:**
```cpp
// In config.h - ADD THESE DEFINITIONS
#define PIN_TFT_CS          10   // TFT Chip Select
#define PIN_TFT_DC          9    // TFT Data/Command
#define PIN_TFT_RST         8    // TFT Reset (move buzzer to pin 7)
#define PIN_TFT_MOSI        51   // SPI MOSI (hardware)
#define PIN_TFT_MISO        50   // SPI MISO (hardware)
#define PIN_TFT_SCK         52   // SPI SCK (hardware)
#define PIN_TOUCH_CS        11   // Touch controller CS
#define PIN_TOUCH_IRQ       12   // Touch interrupt (optional)
#define PIN_BUZZER          7    // MOVED from pin 8
```

### Option 2: FeatherWing Adapter (Not Recommended for Mega)

The Adafruit 5800 is a **FeatherWing** designed for **Feather boards** (ARM Cortex-M based). It won't plug directly into an Arduino Mega. You'd need:
- An Adafruit Feather M4 or similar (replace Arduino Mega)
- OR extensive wiring/adapters (not practical)

**Recommendation:** Use a **breakout TFT display** designed for Arduino, not a FeatherWing.

---

## 🎨 GUI Customization - How to Design Your Display

### Good News: **FULL Customization in Code!**

Unlike Nextion's visual editor, you write **all** the GUI in Arduino C++ code. This gives you complete control.

### Required Libraries

```cpp
// Install via Arduino Library Manager:
#include <Adafruit_GFX.h>        // Core graphics library
#include <Adafruit_ST7789.h>     // Hardware-specific library (for ST7789 controller)
#include <Adafruit_FT6206.h>     // Capacitive touch library
```

**Library Manager Steps:**
1. Arduino IDE → Tools → Manage Libraries
2. Search: "Adafruit GFX"
3. Search: "Adafruit ST7789"
4. Search: "Adafruit FT6206"
5. Install all (with dependencies)

### Example GUI Code Structure

**Initialize Display:**
```cpp
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

Adafruit_ST7789 tft = Adafruit_ST7789(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

void Display_Init() {
  tft.init(240, 320);  // 2.8" display is 240x320
  tft.setRotation(3);  // Landscape mode (320x240)
  tft.fillScreen(ST77XX_BLACK);  // Clear screen
  
  // Draw title
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(40, 10);
  tft.print("JNKR GAUGE");
  
  // Draw labels
  tft.setTextSize(2);
  tft.setCursor(10, 60);
  tft.print("Boost:");
  tft.setCursor(10, 100);
  tft.print("IAT Pre:");
  tft.setCursor(10, 140);
  tft.print("IAT Post:");
  tft.setCursor(10, 180);
  tft.print("EGT:");
}
```

**Update Values:**
```cpp
void Display_UpdateBoost(float boostPSI) {
  // Determine color based on value
  uint16_t color = ST77XX_GREEN;
  if (boostPSI > BOOST_CRITICAL_PSI) {
    color = ST77XX_RED;
  } else if (boostPSI > BOOST_WARNING_PSI) {
    color = ST77XX_YELLOW;
  }
  
  // Clear previous value (draw black rectangle)
  tft.fillRect(150, 60, 100, 30, ST77XX_BLACK);
  
  // Draw new value
  tft.setTextSize(3);
  tft.setTextColor(color);
  tft.setCursor(150, 60);
  tft.print(boostPSI, 1);  // 1 decimal place
  tft.print(" PSI");
}
```

**Draw Gauge (Optional - Advanced):**
```cpp
void drawGauge(int x, int y, int radius, float value, float maxValue) {
  // Draw gauge arc background
  tft.drawCircle(x, y, radius, ST77XX_WHITE);
  
  // Calculate needle angle
  float angle = map(value * 100, 0, maxValue * 100, -120, 120);
  
  // Draw needle
  int needleX = x + radius * cos(radians(angle));
  int needleY = y + radius * sin(radians(angle));
  tft.drawLine(x, y, needleX, needleY, ST77XX_RED);
}
```

**Handle Touch:**
```cpp
#include <Adafruit_FT6206.h>

Adafruit_FT6206 touch = Adafruit_FT6206();

void Display_ProcessTouch() {
  if (touch.touched()) {
    TS_Point point = touch.getPoint();
    
    // Check if touch is in button area
    if (point.x > 50 && point.x < 150 && point.y > 200 && point.y < 230) {
      // Button pressed!
      handleButtonPress();
    }
  }
}
```

---

## 📊 Display Layout Examples

### Simple Text Display (Fast)
```
┌─────────────────────────────────────┐
│         JNKR GAUGE SYSTEM           │
├─────────────────────────────────────┤
│                                     │
│  Boost:       15.5 PSI   [GREEN]   │
│  IAT Pre:     125 °C     [YELLOW]  │
│  IAT Post:    45 °C      [GREEN]   │
│  EGT:         625 °C     [ORANGE]  │
│  Coolant:     88 °C      [GREEN]   │
│                                     │
│  Status: WARNING - High EGT         │
└─────────────────────────────────────┘
```

### Gauge Display (Slower, More Visual)
```
┌─────────────────────────────────────┐
│            JNKR GAUGE               │
├─────────────────────────────────────┤
│   ╭───────╮      ╭───────╮         │
│   │ BOOST │      │  EGT  │         │
│   │  / \  │      │  / \  │         │
│   │ 15.5  │      │  625  │         │
│   ╰───────╯      ╰───────╯         │
│                                     │
│  IAT: 45°C  Coolant: 88°C          │
│  ⚠️  WARNING: High Exhaust Temp     │
└─────────────────────────────────────┘
```

---

## 🔧 Code Modifications Required

### 1. Update `config.h`

Replace Nextion pin definitions with TFT pins:

```cpp
// OLD - Remove these
// #define NEXTION_SERIAL      Serial2
// #define NEXTION_BAUD        115200

// NEW - Add these
#define PIN_TFT_CS          10
#define PIN_TFT_DC          9
#define PIN_TFT_RST         8    // Move buzzer to pin 7!
#define PIN_TOUCH_CS        11
#define PIN_BUZZER          7    // CHANGED from 8
```

### 2. Rewrite `display.cpp`

The display implementation will be **completely different**. Instead of:
```cpp
// Nextion approach (simple)
Display_SendCommand("nBoost.txt=\"15.5\"");
```

You'll need:
```cpp
// TFT approach (more complex)
tft.fillRect(150, 60, 100, 30, ST77XX_BLACK);  // Clear old value
tft.setTextColor(ST77XX_GREEN);
tft.setCursor(150, 60);
tft.print(boostPSI, 1);
```

### 3. Add Graphics Buffering (Optional but Recommended)

To reduce flicker:
```cpp
// Only redraw when value changes
static float lastBoost = -999;
if (abs(boostPSI - lastBoost) > 0.1) {  // Only update if changed by 0.1 PSI
  Display_UpdateBoost(boostPSI);
  lastBoost = boostPSI;
}
```

### 4. Optimize Update Rate

TFT displays are **slower** than Nextion. You may need to reduce update frequency:

```cpp
// In config.h
#define DISPLAY_UPDATE_INTERVAL 500     // Update every 500ms (was 200ms)
```

---

## ⚡ Performance Comparison

| Feature | Nextion | Adafruit TFT |
|---------|---------|--------------|
| **Screen Updates** | 50-100 Hz | 5-20 Hz (depends on complexity) |
| **Arduino CPU Usage** | <5% | 30-60% |
| **RAM Usage** | ~100 bytes | 1-3 KB (for buffering) |
| **Code Complexity** | Low (simple commands) | High (draw primitives) |
| **GUI Design** | Visual editor | Code-based |
| **Mac Compatible** | No (Windows editor) | ✅ Yes! |
| **Customization** | Limited to editor | Unlimited (code) |
| **Touch Support** | Built-in | Requires library |

---

## 📦 Recommended Adafruit Products for Arduino Mega

Instead of the FeatherWing (5800), consider these **Arduino-compatible** displays:

### Option 1: Breakout Display (Best for Mega)
- **[Adafruit 2.8" TFT LCD with Touchscreen Breakout](https://www.adafruit.com/product/1770)**
  - Part #: 1770
  - 240x320 resolution
  - Resistive touch
  - SPI interface
  - **Works directly with Arduino Mega**

### Option 2: Shield (Easiest)
- **[Adafruit 2.8" TFT Touch Shield for Arduino](https://www.adafruit.com/product/1651)**
  - Part #: 1651
  - Plugs directly onto Arduino Mega
  - No wiring needed
  - Capacitive touch
  - microSD slot

### Option 3: Larger Display
- **[Adafruit 3.5" TFT LCD with Touchscreen Breakout](https://www.adafruit.com/product/2050)**
  - Part #: 2050
  - 320x480 resolution (more room for gauges)
  - Resistive touch
  - Closer to Nextion 3.5" size

---

## 🎯 Migration Strategy

### Phase 1: Hardware Setup
1. ✅ Order Adafruit TFT display (shield or breakout)
2. ✅ Install Arduino libraries (GFX, ST7789/ILI9341, FT6206)
3. ✅ Connect display to Arduino Mega
4. ✅ Run test sketch to verify display works

### Phase 2: Basic Display
1. ✅ Modify `config.h` - update pin definitions
2. ✅ Rewrite `Display_Init()` - initialize TFT
3. ✅ Implement `Display_UpdateGauges()` - simple text display
4. ✅ Test with live sensor data

### Phase 3: Enhanced GUI
1. ✅ Add color-coded warnings
2. ✅ Implement alert messages
3. ✅ Add graphical gauges (optional)
4. ✅ Implement touch buttons (if needed)

### Phase 4: Optimization
1. ✅ Reduce flicker (buffering)
2. ✅ Optimize update frequency
3. ✅ Fine-tune layout

**Estimated Time:**
- Basic display: 2-4 hours
- Enhanced GUI: 4-8 hours
- Optimization: 2-4 hours

---

## 📝 Example: Complete Display Rewrite

Here's a simplified example of how `display.cpp` would look with Adafruit TFT:

```cpp
#include "display.h"
#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Initialize display object
Adafruit_ST7789 tft = Adafruit_ST7789(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);

// Screen layout positions
#define TITLE_Y        10
#define BOOST_Y        60
#define IAT_PRE_Y      100
#define IAT_POST_Y     140
#define EGT_Y          180
#define COOLANT_Y      220
#define LABEL_X        10
#define VALUE_X        150

void Display_Init() {
  tft.init(240, 320);
  tft.setRotation(3);  // Landscape
  tft.fillScreen(ST77XX_BLACK);
  
  // Draw title
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(80, TITLE_Y);
  tft.print("JNKR GAUGE");
  
  // Draw labels (only once)
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  
  tft.setCursor(LABEL_X, BOOST_Y);
  tft.print("Boost:");
  
  tft.setCursor(LABEL_X, IAT_PRE_Y);
  tft.print("IAT Pre:");
  
  tft.setCursor(LABEL_X, IAT_POST_Y);
  tft.print("IAT Post:");
  
  tft.setCursor(LABEL_X, EGT_Y);
  tft.print("EGT:");
  
  tft.setCursor(LABEL_X, COOLANT_Y);
  tft.print("Coolant:");
}

void Display_UpdateBoost(float boostPSI) {
  static float lastValue = -999;
  if (abs(boostPSI - lastValue) < 0.1) return;  // Skip if unchanged
  lastValue = boostPSI;
  
  // Determine color
  uint16_t color = ST77XX_GREEN;
  if (boostPSI > BOOST_CRITICAL_PSI) color = ST77XX_RED;
  else if (boostPSI > BOOST_WARNING_PSI) color = ST77XX_YELLOW;
  
  // Clear old value
  tft.fillRect(VALUE_X, BOOST_Y, 160, 30, ST77XX_BLACK);
  
  // Draw new value
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(VALUE_X, BOOST_Y);
  tft.print(boostPSI, 1);
  tft.print(" PSI");
}

void Display_UpdateExhaustTemp(float temp) {
  static float lastValue = -999;
  if (abs(temp - lastValue) < 1.0) return;
  lastValue = temp;
  
  uint16_t color = ST77XX_GREEN;
  if (temp > EGT_DANGER) color = ST77XX_RED;
  else if (temp > EGT_CRITICAL) color = ST77XX_ORANGE;
  else if (temp > EGT_WARNING) color = ST77XX_YELLOW;
  
  tft.fillRect(VALUE_X, EGT_Y, 160, 30, ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(color);
  tft.setCursor(VALUE_X, EGT_Y);
  tft.print((int)temp);
  tft.print(" C");
}

// ... Similar functions for other sensors ...

void Display_ShowAlert(const char* message, uint16_t color) {
  // Draw alert at bottom of screen
  tft.fillRect(0, 280, 320, 40, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(color);
  tft.setCursor(10, 285);
  tft.print(message);
}
```

---

## ✅ Recommendation

### **YES, you can proceed with Adafruit TFT!**

**Best Choice for Mac Users:**
- ✅ Adafruit 2.8" TFT Touch **Shield** (Part #1651) - easiest hardware
- ✅ OR Adafruit 3.5" TFT **Breakout** (Part #2050) - larger, similar to Nextion

### **What You'll Gain:**
- ✅ 100% Mac-compatible development
- ✅ Complete control over GUI design
- ✅ No separate tools needed
- ✅ Learning experience with graphics programming
- ✅ Open-source, community support

### **What You'll Sacrifice:**
- ⚠️ More complex code
- ⚠️ Slower screen updates
- ⚠️ More Arduino CPU/RAM usage
- ⚠️ Manual GUI design (no drag-and-drop editor)

### **Time Investment:**
- Initial setup: 1-2 hours
- Basic display: 2-4 hours
- Refined GUI: 4-8 hours
- **Total: 8-15 hours** to replicate current functionality

---

## 🚀 Next Steps

1. **Order Hardware:**
   - [Adafruit 2.8" TFT Shield #1651](https://www.adafruit.com/product/1651) ← Recommended
   - OR [Adafruit 3.5" TFT Breakout #2050](https://www.adafruit.com/product/2050)

2. **Install Libraries** (in Arduino IDE):
   ```
   Tools → Manage Libraries → Search and install:
   - Adafruit GFX Library
   - Adafruit ST7735 and ST7789 Library  (or ILI9341 depending on display)
   - Adafruit FT6206 Library (for touch)
   ```

3. **Test Hardware:**
   - Run example: File → Examples → Adafruit ST7789 → graphicstest
   - Verify display works

4. **Modify Code:**
   - I can help you rewrite the display module for TFT!

---

## 💬 Questions?

Would you like me to:
1. ✅ Rewrite your `display.cpp` for Adafruit TFT?
2. ✅ Create a specific display layout design?
3. ✅ Help you choose the best display model?
4. ✅ Provide pin wiring diagram?

Just let me know! 🚀

---

**Last Updated:** 2025-11-08
