# Nextion Display Design Guide

**Creating Custom Display Interfaces for JNKR Gauge System**

This guide explains how to design and customize the Nextion HMI touchscreen display for your JNKR Gauge System. Whether you want to modify the existing design or create something completely custom, this guide covers everything you need to know.

---

## 📋 Table of Contents

- [Introduction to Nextion](#introduction-to-nextion)
- [Required Software](#required-software)
- [Display Specifications](#display-specifications)
- [Design Basics](#design-basics)
- [Creating a Simple Gauge Display](#creating-a-simple-gauge-display)
- [Advanced Features](#advanced-features)
- [Uploading to Display](#uploading-to-display)
- [Arduino Integration](#arduino-integration)

---

## 🖥️ Introduction to Nextion

### What is Nextion?

Nextion is a Human-Machine Interface (HMI) solution that consists of:
- **Hardware:** Touchscreen TFT LCD display with embedded controller
- **Software:** Nextion Editor for designing interfaces
- **Protocol:** Simple serial UART communication

### Why Nextion for JNKR Gauge?

✅ **Easy to use** - Drag-and-drop interface designer  
✅ **No graphics library needed** - Display handles all rendering  
✅ **Touchscreen** - Interactive buttons and controls  
✅ **Low Arduino overhead** - Simple serial commands  
✅ **Professional appearance** - High-resolution graphics  
✅ **Affordable** - $25-35 for 3.5" display  

---

## 💻 Required Software

### Nextion Editor

1. **Download:**
   - Go to [nextion.tech](https://nextion.tech)
   - Navigate to Downloads → Nextion Editor
   - Select your OS (Windows, Mac, Linux)
   - Current version: 1.65.1 or newer

2. **Install:**
   - Run installer
   - Follow prompts
   - Launch Nextion Editor

3. **First Launch:**
   - Create account (free)
   - Login to editor
   - Ready to design!

### Optional Tools

- **Image editor** (Photoshop, GIMP) - For custom graphics
- **Font editor** - For custom fonts
- **Color picker** - For exact color matching

---

## 📱 Display Specifications

### NX4832T035 (Recommended 3.5" Display)

**Hardware:**
- Screen size: 3.5 inches (diagonal)
- Resolution: 480 x 320 pixels
- Touch: Resistive touchscreen
- Communication: UART at 9600-115200 baud
- Power: 5V, 100-300mA
- Temperature: -10°C to 60°C operating

**Available Models:**
- **Basic:** 4MB flash, 2KB RAM
- **Enhanced:** 16MB flash, 3.5KB RAM

**Recommendation:** Enhanced model for more complex designs

---

## 🎨 Design Basics

### Understanding the Interface

**Nextion Editor Layout:**
```
┌────────────────────────────────────────────────┐
│  Menu Bar  [File] [Edit] [Tools] [Debug]      │
├─────────────┬──────────────────────┬───────────┤
│  Component  │  Canvas (480x320)    │  Props    │
│  Toolbox    │                      │  Panel    │
│             │                      │           │
│  [Button]   │   [Your Design]      │  objname: │
│  [Text]     │                      │  n0       │
│  [Gauge]    │                      │           │
│  [Progress] │                      │  x: 100   │
│  [Picture]  │                      │  y: 50    │
│             │                      │           │
└─────────────┴──────────────────────┴───────────┘
```

### Color Format

Nextion uses RGB565 format (16-bit color):
- 5 bits red (0-31)
- 6 bits green (0-63)
- 5 bits blue (0-31)

**Common Colors:**
| Color | RGB565 | Hex |
|-------|--------|-----|
| Black | 0 | 0x0000 |
| White | 65535 | 0xFFFF |
| Red | 63488 | 0xF800 |
| Green | 2016 | 0x07E0 |
| Blue | 31 | 0x001F |
| Yellow | 65504 | 0xFFE0 |
| Orange | 64512 | 0xFC00 |

### Component Naming

**Important:** Use consistent naming for Arduino communication:

| Component Type | Naming | Example |
|---------------|---------|---------|
| Numeric text | `n` + description | `nBoost`, `nIAT` |
| Text field | `t` + description | `tStatus`, `tAlert` |
| Gauge/Progress | `j` + description | `jBoost`, `jRPM` |
| Button | `b` + description | `bSettings`, `bReset` |
| Picture | `p` + description | `pLogo`, `pWarning` |

---

## 🎯 Creating a Simple Gauge Display

### Step-by-Step Tutorial

#### Step 1: Create New Project

1. Open Nextion Editor
2. File → New
3. Select device:
   - **Model:** NX4832T035
   - **Orientation:** Landscape (480x320)
4. Click OK

#### Step 2: Design Main Page

**Add Background:**

1. Select `page0` in Page panel
2. Set properties:
   - `bco` (background color): 0 (black)
   - `title`: "Main"

**Add Title Text:**

1. Drag **Text** component to canvas
2. Properties:
   - `objname`: `tTitle`
   - `txt`: "JNKR GAUGE"
   - `font`: Select large font (e.g., 32pt)
   - `pco` (text color): 65535 (white)
   - `xcen`: 1 (center X)
   - `ycen`: 1 (center Y)
   - `w`: 480
   - `h`: 40
   - `x`: 0
   - `y`: 10

#### Step 3: Add Boost Gauge

**Numeric Display:**

1. Drag **Number** component
2. Properties:
   - `objname`: `nBoost`
   - `font`: Large bold (24-32pt)
   - `pco`: 2016 (green)
   - `format`: Numeric
   - `length`: 4
   - `x`: 50
   - `y`: 80
   - `w`: 100
   - `h`: 50

**Label:**

1. Drag **Text** component
2. Properties:
   - `objname`: `tBoostLabel`
   - `txt`: "Boost (PSI)"
   - `font`: Medium (16pt)
   - `pco`: 65535 (white)
   - `x`: 50
   - `y`: 140

**Optional: Progress Bar:**

1. Drag **Progress Bar** (`j0`)
2. Properties:
   - `objname`: `jBoost`
   - `x`: 50
   - `y`: 170
   - `w`: 150
   - `h`: 20
   - `maxval`: 25 (0-25 PSI)
   - `val`: 0
   - `pco`: 2016 (green)

#### Step 4: Add Temperature Displays

**Repeat for each temperature:**

**Intake Air Temperature:**
- Number component: `nIAT`
- Label: "Intake (°C)"
- Position: x=250, y=80

**Exhaust Temperature:**
- Number component: `nEGT`
- Label: "Exhaust (°C)"
- Position: x=250, y=160

**Coolant Temperature:**
- Number component: `nCoolant`
- Label: "Coolant (°C)"
- Position: x=250, y=240

#### Step 5: Add Status Display

**Status Text:**

1. Drag **Text** component
2. Properties:
   - `objname`: `tStatus`
   - `txt`: "OK"
   - `font`: Medium bold
   - `pco`: 2016 (green)
   - `xcen`: 1
   - `x`: 0
   - `y`: 290
   - `w`: 480

**Alert Text:**

1. Drag **Text** component
2. Properties:
   - `objname`: `tAlert`
   - `txt`: ""
   - `font`: Small
   - `pco`: 63488 (red)
   - `xcen`: 1
   - `x`: 0
   - `y`: 270
   - `w`: 480
   - `sta`: `crop center` (visibility)

#### Step 6: Compile and Test

1. Click **Compile** button (hammer icon)
2. Check for errors in output panel
3. If successful, .tft file is created

---

## 🚀 Advanced Features

### Page Navigation

**Create multiple pages:**

1. Right-click Page panel → Add page
2. Name pages: Main, Settings, Alerts
3. Add navigation buttons

**Button to change page:**

1. Drag **Button** component
2. Properties:
   - `txt`: "Settings"
   - Touch Release Event: `page Settings`

### Dynamic Text Updates

**From Arduino:**

```cpp
// Update numeric value
Display_SetNumber("nBoost", 15);  // Set to 15 PSI

// Update text
Display_SetText("tStatus", "WARNING");

// Change text color
Display_SetColor("tStatus", "pco", COLOR_RED);
```

### Conditional Colors

**In Nextion (using events):**

Example: Change boost color based on value

1. Add **Timer** component (100ms interval)
2. Timer Event code:
```
if nBoost.val>20 then
  nBoost.pco=63488  // Red
else
  if nBoost.val>15 then
    nBoost.pco=64512  // Yellow
  else
    nBoost.pco=2016  // Green
  endif
endif
```

### Custom Fonts

1. Tools → Font Generator
2. Select font file (.ttf)
3. Choose size and characters
4. Generate and add to project
5. Use in component properties

### Images and Icons

**Add background image:**

1. Import image: Picture → Import
2. Convert to .png, resize to 480x320
3. Drag **Picture** component
4. Select imported image
5. Set as background

**Warning icons:**

1. Create or download icons (32x32, 64x64)
2. Import to project
3. Use Picture component with visibility control

### Touch Events

**Button press actions:**

Component → Events → Touch Release Event

Examples:
```
// Navigate to page
page Settings

// Toggle value
nValue.val=!nValue.val

// Send to Arduino
printh 23 02 54  // Send hex data

// Print text
print "BUTTON_PRESSED"
```

---

## 📤 Uploading to Display

### Method 1: microSD Card (Recommended)

1. **Prepare SD card:**
   - Format as FAT32
   - 16GB or smaller
   - Class 10 recommended

2. **Copy .tft file:**
   - Locate compiled .tft file
   - Copy to root of SD card
   - Eject safely

3. **Upload to display:**
   - Power off display
   - Insert SD card
   - Power on display
   - Display automatically loads .tft file
   - Shows progress bar
   - When complete, remove SD card
   - Power cycle display

### Method 2: USB-to-Serial (Alternative)

1. **Connect USB-UART adapter:**
   ```
   Adapter RX  → Nextion TX (pin 2)
   Adapter TX  → Nextion RX (pin 3)
   Adapter GND → Nextion GND
   ```

2. **In Nextion Editor:**
   - File → Upload
   - Select COM port
   - Select .tft file
   - Click Upload
   - Wait for completion

**Note:** Method 1 (SD card) is faster and more reliable

---

## 🔌 Arduino Integration

### Sending Data to Display

**In Arduino code:**

```cpp
// Update numeric displays
void Display_UpdateBoost(float boostPSI) {
  char buffer[16];
  dtostrf(boostPSI, 0, 1, buffer);
  
  char cmd[32];
  snprintf(cmd, sizeof(cmd), "nBoost.txt=\"%s\"", buffer);
  Display_SendCommand(cmd);
  
  // Change color based on value
  uint16_t color = COLOR_GREEN;
  if (boostPSI > 20) color = COLOR_RED;
  else if (boostPSI > 15) color = COLOR_YELLOW;
  
  Display_SetColor("nBoost", "pco", color);
}
```

### Receiving Data from Display

**Setup:**

In Nextion button touch event:
```
print "BTN_SETTINGS"
```

**In Arduino:**

```cpp
void Display_ProcessMessages() {
  if (NEXTION_SERIAL.available()) {
    String msg = NEXTION_SERIAL.readStringUntil('\n');
    
    if (msg == "BTN_SETTINGS") {
      // Handle settings button press
      Display_SetPage(PAGE_SETTINGS);
    }
  }
}
```

---

## 🎨 Design Tips

### Best Practices

✅ **High contrast** - Dark background, light text  
✅ **Large fonts** - Readable at a glance while driving  
✅ **Color coding** - Green = OK, Yellow = Caution, Red = Danger  
✅ **Minimize pages** - Keep most info on one screen  
✅ **Clear labels** - Abbreviate but keep understandable  
✅ **Touch targets** - Buttons >40x40 pixels  

### Layout Suggestions

**Dashboard Style:**
- Boost pressure: Large, center-top
- Temperatures: Grid layout below
- Status bar at bottom
- Warning messages flash in center

**Gauge Cluster Style:**
- Circular gauges (using picture components)
- Animated needles
- Digital readouts in center
- More complex but impressive

**Minimalist Style:**
- Numbers only
- No decorations
- Very readable
- Fast updates

---

## 📚 Resources

### Official Nextion Resources

- [Nextion Website](https://nextion.tech)
- [Instruction Set](https://nextion.tech/instruction-set/)
- [Wiki](https://wiki.iteadstudio.com/Nextion)
- [Forum](https://forum.nextion.tech)
- [YouTube Tutorials](https://www.youtube.com/c/NextionTechnology)

### Community Resources

- Arduino Forum - Nextion section
- Reddit: r/Nextion
- GitHub: Search "Nextion Arduino"

### Example Projects

Search for inspiration:
- "Nextion car gauge"
- "Arduino Nextion dashboard"
- "Nextion boost gauge"

---

## 🆘 Common Issues

**Display shows "No SD card found":**
- SD card not formatted FAT32
- SD card >32GB
- .tft file not in root directory

**Arduino can't communicate:**
- Check baud rate (115200)
- Verify TX/RX wiring (crossover)
- Check voltage (5V)
- Serial port conflict

**Text doesn't update:**
- Check component name spelling
- Verify terminator bytes (0xFF 0xFF 0xFF)
- Check command syntax

**Touch doesn't work:**
- Check if Basic model (no touch)
- Calibrate touch: page → Calibration
- Verify touch events in code

---

## ✅ Display Design Checklist

- [ ] Project created with correct display model
- [ ] All gauges have clear labels
- [ ] Component names follow convention
- [ ] Colors provide good contrast
- [ ] Fonts are readable size
- [ ] Status messages visible
- [ ] Alert messages prominent
- [ ] Compiled without errors
- [ ] Uploaded to display successfully
- [ ] Tested with Arduino
- [ ] All values update correctly
- [ ] Touch events work (if applicable)

---

**Enjoy designing your custom gauge display!**

The Nextion Editor is powerful and flexible - experiment with different layouts and features to create the perfect display for your vehicle.

---

**Last Updated:** 2025-11-07
