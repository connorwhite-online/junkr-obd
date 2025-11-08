# JNKR Gauge System

**Read-Only Engine Monitoring Gauge for 1KZTE Turbodiesel**

A comprehensive engine monitoring system powered by ESP32-S3 with a beautiful 2.1" round display. Features real-time sensor data, color-coded warnings, audio alerts, and smooth 60 FPS graphics. Perfect for enthusiasts who want to monitor their turbocharged diesel engine without modifying ECU signals.

![Version](https://img.shields.io/badge/version-2.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-red.svg)
![Display](https://img.shields.io/badge/display-2.1%22%20Round%20480x480-green.svg)

---

## 🎯 Features

### Real-Time Monitoring
- **Boost Pressure** - PSI gauge display (0-25 PSI range)
- **Intake Air Temperature (Dual)** - Pre & Post-intercooler monitoring
- **Exhaust Gas Temperature** - K-type thermocouple (up to 1000°C)
- **Coolant Temperature** - Engine operating temperature

### Visual Feedback
- 2.1" round RGB display (480x480 pixels)
- Smooth 60 FPS graphics
- Color-coded warnings (Green → Yellow → Orange → Red)
- Professional automotive gauge appearance
- LVGL-powered GUI with SquareLine Studio design

### Audio Alerts
- Multi-level alert system with distinctive beep patterns:
  - **Info**: 1 beep (informational)
  - **Warning**: 2 beeps (attention needed)
  - **Critical**: 3 beeps (immediate attention)
  - **Danger**: Continuous alarm (pull over immediately)
- Configurable thresholds
- Acknowledgeable alerts

### Safety Features
- Sensor fault detection and reporting
- Out-of-range validation
- Filtered readings to reduce noise
- Comprehensive diagnostic output

---

## 📋 Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [Bill of Materials](#bill-of-materials)
- [Wiring Guide](#wiring-guide)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Shield PCB Design](#shield-pcb-design)
- [Contributing](#contributing)
- [License](#license)

---

## 🔧 Hardware Requirements

### Core Components

| Component | Part Number | Quantity | Purpose |
|-----------|-------------|----------|---------|
| **Qualia ESP32-S3** | Adafruit #5800 | 1 | Main controller (240 MHz, 8MB RAM) |
| **Round RGB Display** | 2.1" 480x480 | 1 | High-res automotive gauge display |
| **I2C ADC Module** | Adafruit ADS1015 | 1 | Extra analog inputs for sensors |
| **MAX31855 Module** | MAX31855 | 1 | K-type thermocouple amplifier |
| **K-Type Thermocouple** | M6 thread | 1 | EGT sensor (0-1000°C) |
| **MAP Sensor** | 0-3 bar | 1 | Boost pressure sensor |
| **NTC Thermistors** | 2.2kΩ @ 25°C | 3 | Temperature sensors (2x IAT, 1x coolant) |
| **Piezo Buzzer** | 2kHz active | 1 | Audio alerts |

**Why ESP32-S3?**
- ✅ **10x faster** than Arduino Mega (240 MHz vs 16 MHz)
- ✅ **1000x more RAM** (8MB vs 8KB) - LVGL runs perfectly
- ✅ **60 FPS graphics** via RGB-666 parallel interface
- ✅ **WiFi + Bluetooth** built-in (future data logging)
- ✅ **Round display** for authentic automotive gauge look
- ✅ **Same sensors, same Arduino IDE**

### Supporting Components

- Resistors: 2.2kΩ, 10kΩ (voltage dividers)
- Capacitors: 0.1µF ceramic (power filtering)
- Screw terminals: 5mm pitch
- Power supply: 12V to 5V buck converter (3A minimum)
- Wiring: 20-22 AWG automotive wire
- Heat shrink tubing and connectors

**Detailed Bill of Materials**: See [docs/BOM.md](docs/BOM.md)

---

## 🔌 Installation Options

### Option A: Traditional Direct Wiring (Original)

All sensors connect directly to ESP32/Arduino with individual wires through firewall.

**Pros:**
- Lower cost (~$190-230)
- Uses readily available modules
- Simpler electronics

**Cons:**
- 15-20 wires through firewall
- Multiple firewall penetrations
- More complex installation
- Limited cable length (3-5 feet)

**Documentation**: [WIRING.md](docs/WIRING.md), [BREADBOARD_WIRING.md](docs/BREADBOARD_WIRING.md)

### Option B: I2C Sensor Module (Recommended) ⭐ **NEW!**

All sensors connect to a module in the engine bay. Single 4-wire harness with one connector runs through firewall to display.

**Pros:**
- ✅ Professional installation (automotive-style)
- ✅ Single connector through firewall (easy removal)
- ✅ Digital I2C communication (noise immune)
- ✅ Longer cable runs possible (10+ feet)
- ✅ Cleaner wire routing

**Cons:**
- Additional cost (+$60-120)
- Requires building sensor module
- Moderate soldering skills needed

**Why choose this?** If you want a clean, professional installation that's easy to service and looks like it came from the factory, this is the way to go. The single connector through the firewall makes removal trivial and eliminates wire management headaches.

**Documentation**:
- **[I2C_SENSOR_MODULE.md](docs/I2C_SENSOR_MODULE.md)** - Design overview and specifications
- **[I2C_SHOPPING_LIST.md](docs/I2C_SHOPPING_LIST.md)** - Parts with direct links
- **[I2C_ASSEMBLY_GUIDE.md](docs/I2C_ASSEMBLY_GUIDE.md)** - Step-by-step build guide
- **[CUSTOM_PCB_DESIGN.md](docs/CUSTOM_PCB_DESIGN.md)** - Custom PCB option

---

## 📐 Architecture

### Option A: Traditional Direct Wiring

```
┌──────────────────┐
│   12V Vehicle    │
│   Electrical     │
└────────┬─────────┘
         │
         ├──→ Buck Converter ──→ 5V ──→ Qualia ESP32-S3 Board
         │                              │
         │                              ├─ 40-pin RGB ───→ 2.1" Round Display
         │                              │                  (480x480, 60 FPS)
         │                              │
         │                              ├─ Analog ────────→ MAP Sensor (Boost)
         │                              ├─ Analog ────────→ NTC (IAT Pre-IC)
         │                              │
         │                              ├─ I2C (Stemma QT)
         │                              │   └─ ADS1015 ADC ─→ NTC (IAT Post-IC)
         │                              │                  └─ NTC (Coolant)
         │                              │
         │                              ├─ SPI ───────────→ MAX31855 ──→ EGT Sensor
         │                              └─ PWM ───────────→ Buzzer (Alerts)
         │
```

### Option B: I2C Sensor Module (Recommended)

```
┌──────────────────┐                    ┌────────────────────────┐
│   12V Vehicle    │                    │  ENGINE BAY I2C MODULE │
│   Electrical     │                    │  (Weatherproof Box)    │
└────────┬─────────┘                    │                        │
         │                              │  ┌─ NTC (IAT Pre-IC)   │
         │                              │  ├─ NTC (IAT Post-IC)  │
         ├──→ Sensors in Engine Bay ───┼──┤  NTC (Coolant)      │
         │                              │  ├─ MAP Sensor (Boost) │
         │                              │  └─ K-Type (EGT)       │
         │                              │         ↓               │
         │                              │    ADS1115 ADC         │
         │                              │    MCP9600 Amp         │
         │                              │         ↓               │
         │                              │    Deutsch Connector   │
         │                              └────────┬───────────────┘
         │                                       │
         │                              4-wire shielded harness
         │                              (5V, GND, SCL, SDA)
         │                                       │
         │                              Single connector
         │                              through firewall
         │                                       ↓
         │
         ├──→ Buck Converter ──→ 5V ──→ Qualia ESP32-S3 Board
                                        │
                                        ├─ 40-pin RGB ───→ 2.1" Round Display
                                        │                  (480x480, 60 FPS)
                                        │
                                        ├─ I2C (GPIO 21/22)
                                        │   └─ Engine Bay Module (via harness)
                                        │
                                        └─ PWM ──────────→ Buzzer (Alerts)
```

---

## 🔌 Pin Assignments

### ESP32-S3 Qualia Board

| Interface | Component | Connection Method |
|-----------|-----------|-------------------|
| **40-pin RGB Connector** | Round Display | Plug-in ribbon cable |
| **Analog GPIO 1** | MAP Sensor | Direct analog input |
| **Analog GPIO 2** | NTC (IAT Pre) | Direct analog input w/ voltage divider |
| **I2C (Stemma QT)** | ADS1015 ADC Module | Stemma QT cable |
| **ADS1015 Ch 0** | NTC (IAT Post) | Via I2C ADC |
| **ADS1015 Ch 1** | NTC (Coolant) | Via I2C ADC |
| **SPI (via expander)** | MAX31855 (EGT) | PCA9554 I/O expander |
| **Digital (via expander)** | Buzzer | PCA9554 I/O expander |

**Complete Wiring**: See [docs/BREADBOARD_WIRING.md](docs/BREADBOARD_WIRING.md)

---

## 🚀 Installation

### Step 1: Hardware Assembly

1. **Prepare the Enclosure**
   - Drill mounting holes for Qualia ESP32-S3 board
   - Install cable glands for sensor wires and display cable
   - Display connects via 40-pin FFC cable (included)

2. **Mount Components**
   - Secure Qualia ESP32-S3 with standoffs
   - Connect round display via 40-pin cable
   - Install buck converter
   - Install screw terminals

3. **Wire Connections**
   - Follow the wiring diagram in [docs/WIRING.md](docs/WIRING.md)
   - Use heat shrink on all connections
   - Label all wires

**Detailed Assembly**: See [docs/ASSEMBLY.md](docs/ASSEMBLY.md)

### Step 2: Software Setup

1. **Install Arduino IDE**
   ```bash
   # Download from: https://www.arduino.cc/en/software
   # Install version 1.8.x or 2.x
   ```

2. **Install Required Libraries**
   - Open Arduino IDE
   - Go to Sketch → Include Library → Manage Libraries
   - Search and install:
     - `SPI` (built-in)
     - `Wire` (built-in)

3. **Upload Code**
   - Open `jnkr-gauge.ino` in Arduino IDE
   - Select **Board**: Adafruit Feather ESP32-S3 No PSRAM
   - Select **Port**: Your ESP32's COM port
   - Click **Upload** (→)

4. **Verify Operation**
   - Open Serial Monitor (115200 baud)
   - Check for initialization messages
   - Verify sensor readings

### Step 3: Sensor Installation

1. **EGT Sensor** (Exhaust)
   - Install in exhaust manifold or downpipe
   - Use M6 or 1/8" NPT bung
   - Position 6-12 inches from turbo outlet
   - Route wiring away from heat

2. **Boost Pressure Sensor**
   - Connect to intake manifold
   - Use vacuum hose (4mm or 5/32")
   - Mount sensor in protected location

3. **Temperature Sensors**
   - **IAT Pre-IC**: Install in intake pipe before intercooler (near turbo outlet)
   - **IAT Post-IC**: Install in intake pipe after intercooler (near manifold)
   - **Coolant**: Use existing coolant temp sensor port or T-adapter

4. **Power Connection**
   - Connect to switched 12V (ignition) - **See [AUTOMOTIVE_POWER.md](docs/AUTOMOTIVE_POWER.md)**
   - Use 3A fuse for protection
   - Connect ground to chassis

**Detailed Installation**: See [docs/ASSEMBLY.md](docs/ASSEMBLY.md)

---

## ⚙️ Configuration

### Temperature Thresholds

Edit `config.h` to customize alert thresholds:

```cpp
// Exhaust Gas Temperature (°C)
#define EGT_WARNING            600.0
#define EGT_CRITICAL           650.0
#define EGT_DANGER             700.0

// Coolant Temperature (°C)
#define COOLANT_WARNING        100.0
#define COOLANT_CRITICAL       105.0
#define COOLANT_DANGER         110.0

// Boost Pressure (PSI)
#define BOOST_WARNING_PSI      14.5
#define BOOST_CRITICAL_PSI     18.8
#define BOOST_DANGER_PSI       21.8
```

### Sensor Calibration

If your sensors read incorrectly, you can apply calibration offsets:

```cpp
// In setup() function:
Sensors_SetIATPreOffset(2.5);   // Add 2.5°C to IAT Pre-IC reading
Sensors_SetIATPostOffset(-1.0); // Subtract 1.0°C from IAT Post-IC reading
Sensors_SetBoostOffset(0.05);   // Add 0.05 bar to boost reading
```

### Display Settings

Adjust update rates and filtering:

```cpp
#define SENSOR_UPDATE_INTERVAL 100   // Read sensors every 100ms
#define DISPLAY_UPDATE_INTERVAL 200  // Update display every 200ms
#define FILTER_ALPHA           0.15  // Smoothing (0.0-1.0)
```

---

## 📱 Usage

### Startup Sequence

1. Turn ignition to ON position
2. Wait for display to initialize (~2 seconds)
3. System performs self-test
4. Display shows "System Ready"
5. Gauges begin updating

### Normal Operation

- **Green values**: All parameters normal
- **Yellow values**: Approaching warning threshold
- **Orange values**: At critical threshold
- **Red values**: Dangerous level - take action!

### Alert Responses

| Alert Level | Display | Sound | Action Required |
|-------------|---------|-------|-----------------|
| **None** | Green text | Silent | Normal operation |
| **Warning** | Yellow text | 2 beeps | Monitor closely |
| **Critical** | Orange text | 3 beeps | Reduce load/speed |
| **Danger** | Red text | Continuous | Pull over immediately |

### Interpreting Readings

**Boost Pressure (PSI)**
- 0-12 PSI: Normal driving
- 12-15 PSI: Moderate boost
- 15-19 PSI: High boost (modified)
- >19 PSI: Overboost - danger!

**Exhaust Gas Temperature (°C)**
- <500°C: Normal cruising
- 500-600°C: Moderate load
- 600-650°C: High load (short duration OK)
- >650°C: Danger - reduce throttle!

**Coolant Temperature (°C)**
- 75-95°C: Normal operating range
- 95-100°C: Warming - check cooling system
- 100-105°C: Overheating - reduce load
- >105°C: Critical - pull over!

---

## 🛠️ Troubleshooting

### Display Issues

**Problem**: Display doesn't turn on
- Check 5V power connection via USB-C or VIN
- Verify 40-pin FFC cable is fully inserted
- Check cable orientation (contacts facing correct direction)

**Problem**: Garbled display or flickering
- Verify 40-pin cable connection
- Check power supply (needs stable 5V/2A minimum)
- Ensure proper LVGL configuration in code

### Sensor Issues

**Problem**: EGT reads 0°C
- Check MAX31855 wiring
- Verify thermocouple connection
- Check SPI pins (10, 50, 52)
- Test thermocouple with multimeter

**Problem**: Temperature readings too high/low
- Apply calibration offset in `config.h`
- Verify thermistor type (2.2kΩ @ 25°C)
- Check reference resistor value (2.2kΩ)
- Verify voltage divider wiring
- For IAT sensors, ensure good thermal contact with intake pipe

**Problem**: Boost pressure incorrect
- Verify MAP sensor wiring
- Check vacuum line for leaks
- Calibrate using known atmospheric pressure
- Ensure sensor is rated for 0-3 bar

### Audio Alerts

**Problem**: No buzzer sound
- Check pin 8 connection
- Verify buzzer polarity
- Test with `Alerts_PlayBeeps(3, 250)`
- Check if audio is disabled

**Problem**: Buzzer always on
- Check for alert condition
- Verify thresholds in `config.h`
- Reset system

### General Issues

**Problem**: ESP32 won't upload
- Check USB-C cable (must be data-capable)
- Select correct board (Adafruit Feather ESP32-S3)
- Select correct COM port
- Try holding BOOT button while uploading

**Problem**: System freezes
- Check power supply (needs 3A minimum)
- Verify all wiring connections
- Check for short circuits
- Monitor Serial output for errors

---

## 🏗️ Shield PCB Design

For a professional, plug-and-play solution, we've designed a custom Arduino Mega shield that simplifies installation significantly.

### Shield Features
- Screw terminals for all sensor connections
- Built-in voltage dividers and filtering
- MAX31855 module socket
- Buzzer mount
- Nextion display connector
- 12V power input with buck converter
- LED status indicators
- Compact design fits in standard enclosure

### PCB Files
- **Schematic**: `docs/shield/schematic.pdf`
- **PCB Layout**: `docs/shield/pcb-layout.pdf`
- **Gerber Files**: `docs/shield/gerbers/` (for PCBWay)
- **Assembly Drawing**: `docs/shield/assembly.pdf`

### Ordering from PCBWay

1. Visit [PCBWay.com](https://www.pcbway.com)
2. Click "Quote Now"
3. Upload gerber ZIP file (`docs/shield/gerbers/jnkr-shield-gerbers.zip`)
4. Select options:
   - **Layers**: 2
   - **PCB Thickness**: 1.6mm
   - **Surface Finish**: HASL
   - **Copper Weight**: 1oz
5. Add to cart and checkout

**Estimated Cost**: $5-15 for 5 PCBs (plus shipping)

**Assembly**: See [docs/SHIELD.md](docs/SHIELD.md) for component placement and soldering guide

---

## 📚 Documentation

### Getting Started
- **[Complete Bill of Materials](docs/COMPLETE_BOM.md) - Exact parts with links** ⭐ **Start Here!**
- **[Breadboard Wiring Guide](docs/BREADBOARD_WIRING.md) - Prototyping with jumper wires** ⭐ **Build Guide**
- [PCB Schematic](docs/PCB_SCHEMATIC.md) - Custom shield design for permanent installation

### I2C Sensor Module (Professional Installation) ⭐ **NEW!**
- **[I2C Sensor Module Overview](docs/I2C_SENSOR_MODULE.md) - Design and specifications**
- **[I2C Shopping List](docs/I2C_SHOPPING_LIST.md) - Parts with Amazon/Adafruit links**
- **[I2C Assembly Guide](docs/I2C_ASSEMBLY_GUIDE.md) - Step-by-step build instructions**
- **[Custom PCB Design](docs/CUSTOM_PCB_DESIGN.md) - Professional PCB option**

### Display Options
- **[SquareLine + LVGL Guide](docs/SQUARELINE_LVGL_GUIDE.md) - Visual GUI design** ⭐ **Recommended** (Mac/Windows/Linux)
- [Nextion HMI Guide](docs/NEXTION.md) - Alternative display option (Windows only)

### Additional Guides
- **[Automotive Power Guide](docs/AUTOMOTIVE_POWER.md) - How to power from vehicle** ⭐ **Essential!**
- [Complete Wiring Guide](docs/WIRING.md) - Detailed automotive connections (traditional)
- [Assembly Guide](docs/ASSEMBLY.md) - Step-by-step build instructions
- [Shield Guide](docs/SHIELD.md) - PCB shield assembly and ordering
- [Calibration Guide](docs/CALIBRATION.md) - Sensor calibration procedures

---

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for:
- Bug fixes
- New features
- Documentation improvements
- Hardware modifications
- Shield design improvements

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## ⚠️ Disclaimer

**IMPORTANT**: This system is for monitoring purposes only. It does not modify any engine control signals.

- Use at your own risk
- Improper installation can damage sensors or vehicle systems
- Always verify readings against known good gauges
- Do not rely solely on this system for critical safety decisions
- Author assumes no liability for damage or injury

---

## 💡 Credits

- Inspired by the [jnkr-ecu](https://github.com/connorwhite-online/jnkr-ecu) project
- Based on Arduino Mega 2560 platform
- Nextion HMI display for visualization
- Thanks to the automotive Arduino community

---

## 📞 Support

For questions, issues, or support:
- Open an [Issue](https://github.com/connorwhite-online/jnkr-gauge/issues)
- Check the [Wiki](https://github.com/connorwhite-online/jnkr-gauge/wiki)
- Join discussions in Issues

---

**Built with ❤️ for the turbodiesel community**

**Version 1.0.0** | Last Updated: 2025-11-07