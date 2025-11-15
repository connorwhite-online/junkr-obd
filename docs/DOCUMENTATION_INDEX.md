# JNKR Adaptive ECU - Complete Documentation Index

**📚 Your Complete Guide to Building a Professional Piggyback ECU**

---

## 🎯 Start Here!

### New to this project?
**👉 [Quick Start Guide](ADAPTIVE_ECU_QUICK_START.md)** - Build Gauge Mode in one afternoon!

### Want the big picture?
**👉 [System Overview](ADAPTIVE_ECU_SYSTEM.md)** - Complete architecture and features

### Choosing installation method?
**👉 [Piggyback Harness](ADAPTIVE_ECU_PIGGYBACK_HARNESS.md)** - Professional plug-and-play (Option A)  
**👉 [Splice/Tap Method](ADAPTIVE_ECU_SPLICE_METHOD.md)** - Fast and economical (Option B)

---

## 📁 Documentation Structure

```
/workspace/docs/
│
├── ADAPTIVE_ECU_SYSTEM.md ⭐ START HERE
│   └── Master overview document
│       • System architecture
│       • Hardware requirements  
│       • Installation options
│       • Safety overview
│       • Development roadmap
│
├── ADAPTIVE_ECU_QUICK_START.md ⭐ BUILD GUIDE
│   └── Get running fast!
│       • Shopping list
│       • Build steps
│       • First test drive
│       • Troubleshooting
│
├── ADAPTIVE_ECU_PIGGYBACK_HARNESS.md (Option A)
│   └── Professional harness design
│       • 3D printed housing
│       • OEM connector integration
│       • Internal PCB design
│       • Assembly instructions
│
├── ADAPTIVE_ECU_SPLICE_METHOD.md (Option B)
│   └── Direct tap installation
│       • Voltage divider theory
│       • ECU pin identification
│       • Safe tapping techniques
│       • Wiring diagrams
│
├── ADAPTIVE_ECU_PCB_DESIGN.md
│   └── Signal modifier circuits
│       • Schematic diagrams
│       • PCB layout
│       • Component selection
│       • Assembly guide
│
├── ADAPTIVE_ECU_BOOST_CONTROL.md
│   └── Electronic boost control
│       • Solenoid selection
│       • Plumbing diagrams
│       • PID tuning
│       • Failsafe logic
│
├── ADAPTIVE_ECU_SENSORS.md
│   └── Sensor installation
│       • Wideband O2 setup
│       • EGT installation
│       • MAP sensor mounting
│       • Calibration procedures
│
├── ADAPTIVE_ECU_CALIBRATION.md
│   └── Sensor calibration
│       • Two-point calibration
│       • Base map creation
│       • Verification methods
│       • Troubleshooting
│
├── ADAPTIVE_ECU_TUNING.md
│   └── Tuning guide
│       • Adaptive learning theory
│       • Map interpretation
│       • AFR targets for diesel
│       • Dyno tuning tips
│
├── ADAPTIVE_ECU_ASSEMBLY.md
│   └── Step-by-step build
│       • Component layout
│       • Wiring diagrams
│       • Testing procedures
│       • Vehicle installation
│
├── ADAPTIVE_ECU_TESTING.md
│   └── Validation procedures
│       • Bench testing
│       • Sensor validation
│       • Mode switching tests
│       • Data logging verification
│
├── ADAPTIVE_ECU_SAFETY.md ⚠️ READ BEFORE ECU MODE
│   └── Safety procedures
│       • Pre-flight checks
│       • Emergency procedures
│       • Fault handling
│       • Risk mitigation
│
├── ADAPTIVE_ECU_TROUBLESHOOTING.md
│   └── Common issues & solutions
│       • Diagnostic procedures
│       • Error codes
│       • FAQ
│       • Community support
│
└── OEM_ECU_DATA_ACCESS.md
    └── Background: ECU sensor reading
        • OBD-II access
        • CAN bus access
        • Analog intercept theory
        • 1KZ-TE specifics
```

---

## 🛠️ Hardware Documentation

```
/workspace/hardware/adaptive_ecu/
│
├── 3d_models/
│   ├── piggyback_harness.scad ⭐ PARAMETRIC DESIGN
│   ├── enclosure_bottom.stl
│   ├── enclosure_top.stl
│   └── mounting_bracket.stl
│
├── pcb/
│   ├── signal_modifier/
│   │   ├── schematic.pdf
│   │   ├── pcb_layout.pdf
│   │   ├── bom.csv
│   │   └── gerbers/
│   │       └── (files for PCB fab)
│   │
│   └── sensor_module/ (from I2C project)
│       └── ...
│
├── schematics/
│   ├── system_architecture.pdf
│   ├── wiring_diagram_piggyback.pdf
│   ├── wiring_diagram_splice.pdf
│   ├── boost_controller.pdf
│   └── power_distribution.pdf
│
└── datasheets/
    ├── ESP32-S3_datasheet.pdf
    ├── MCP4725_DAC.pdf
    ├── ADS1115_ADC.pdf
    └── (component datasheets)
```

---

## 💻 Software Documentation

```
/workspace/firmware/adaptive_ecu/
│
├── adaptive_ecu_main.ino ⭐ MAIN FIRMWARE
│   └── Core system code
│
├── config.h
│   └── User configuration
│       • Sensor calibration
│       • Target AFR tables
│       • Safety limits
│       • Pin assignments
│
├── src/
│   ├── sensors.cpp/h
│   │   └── Sensor reading & filtering
│   │
│   ├── display.cpp/h
│   │   └── LVGL display interface
│   │
│   ├── ecu_interface.cpp/h
│   │   └── ECU signal read/modify
│   │
│   ├── adaptive_learning.cpp/h
│   │   └── Map learning algorithms
│   │
│   ├── boost_control.cpp/h
│   │   └── PID boost controller
│   │
│   ├── data_logging.cpp/h
│   │   └── SD card logging
│   │
│   ├── safety.cpp/h
│   │   └── Safety checks & limits
│   │
│   └── mode_manager.cpp/h
│       └── Mode switching logic
│
├── lib/
│   └── (required libraries)
│
└── README.md
    └── Firmware documentation
```

---

## 📖 Example Code

```
/workspace/examples/
│
├── ecu_analog_intercept.ino
│   └── Analog sensor reading example
│
├── ecu_obd2_bluetooth.ino
│   └── OBD-II via ELM327 example
│
├── wideband_o2_reading.ino
│   └── AEM X-Series wideband example
│
├── boost_controller_test.ino
│   └── Boost solenoid PWM test
│
├── adaptive_learning_demo.ino
│   └── Simple learning algorithm demo
│
└── display_test.ino
    └── LVGL display test
```

---

## 📚 Reference Documents

### Automotive Background
- **[OEM ECU Data Access](OEM_ECU_DATA_ACCESS.md)** - How to read ECU sensors
- **[Automotive Sensors Guide](AUTOMOTIVE_SENSORS.md)** - OEM vs generic sensors
- **[Automotive Power Guide](AUTOMOTIVE_POWER.md)** - Vehicle power integration
- **[Complete Wiring Guide](WIRING.md)** - Original direct wiring method

### Hardware Design
- **[I2C Sensor Module](I2C_SENSOR_MODULE.md)** - Engine bay sensor module
- **[PCB Schematic](PCB_SCHEMATIC.md)** - Shield PCB design
- **[BOM](COMPLETE_BOM.md)** - Complete parts list

### Display Options
- **[SquareLine + LVGL Guide](SQUARELINE_LVGL_GUIDE.md)** - Visual GUI design
- **[Nextion HMI Guide](NEXTION.md)** - Alternative display

### Testing & Calibration
- **[Bench Testing Guide](BENCH_TESTING.md)** - Test before installation
- **[Calibration Guide](CALIBRATION.md)** - Sensor calibration procedures
- **[Assembly Guide](ASSEMBLY.md)** - General assembly instructions

---

## 🗺️ Suggested Reading Order

### Phase 1: Research & Planning (Day 1)
1. [System Overview](ADAPTIVE_ECU_SYSTEM.md) - Understand the system
2. [Quick Start Guide](ADAPTIVE_ECU_QUICK_START.md) - See what's involved
3. [Safety Procedures](ADAPTIVE_ECU_SAFETY.md) - Understand the risks
4. Choose: [Piggyback Harness](ADAPTIVE_ECU_PIGGYBACK_HARNESS.md) or [Splice Method](ADAPTIVE_ECU_SPLICE_METHOD.md)

### Phase 2: Ordering Parts (Day 2-3)
1. [Quick Start Guide](ADAPTIVE_ECU_QUICK_START.md) - Shopping list
2. [Sensors Guide](ADAPTIVE_ECU_SENSORS.md) - Sensor selection
3. [BOM](COMPLETE_BOM.md) - Complete parts list with links

### Phase 3: Building (Week 1)
1. [Assembly Instructions](ADAPTIVE_ECU_ASSEMBLY.md) - Build the hardware
2. [PCB Design](ADAPTIVE_ECU_PCB_DESIGN.md) - Signal modifier circuits
3. [Boost Controller](ADAPTIVE_ECU_BOOST_CONTROL.md) - Boost control setup
4. [Testing & Validation](ADAPTIVE_ECU_TESTING.md) - Bench testing

### Phase 4: Installation (Week 2)
1. [Sensors Guide](ADAPTIVE_ECU_SENSORS.md) - Install sensors
2. Your chosen method: [Piggyback](ADAPTIVE_ECU_PIGGYBACK_HARNESS.md) or [Splice](ADAPTIVE_ECU_SPLICE_METHOD.md)
3. [Calibration Guide](ADAPTIVE_ECU_CALIBRATION.md) - Calibrate sensors
4. [Testing & Validation](ADAPTIVE_ECU_TESTING.md) - Vehicle testing

### Phase 5: Gauge Mode Operation (Weeks 3-4)
1. [Quick Start Guide](ADAPTIVE_ECU_QUICK_START.md) - First test drive
2. Drive 100-200 miles in Gauge Mode
3. Analyze logged data
4. Validate all sensors

### Phase 6: ECU Mode (If Proceeding) (Months 2-3)
1. [PCB Design](ADAPTIVE_ECU_PCB_DESIGN.md) - Build signal modifiers
2. [Safety Procedures](ADAPTIVE_ECU_SAFETY.md) - Review before enabling!
3. [Tuning Guide](ADAPTIVE_ECU_TUNING.md) - Adaptive learning
4. [Calibration Guide](ADAPTIVE_ECU_CALIBRATION.md) - Fine-tuning
5. Professional dyno session (recommended)

---

## 🎓 By Topic

### For Electronics Beginners
1. Start with basic concepts: [OEM ECU Data Access](OEM_ECU_DATA_ACCESS.md)
2. Understand voltage dividers: [Splice Method](ADAPTIVE_ECU_SPLICE_METHOD.md)
3. Practice on breadboard first: [Bench Testing](BENCH_TESTING.md)

### For Experienced Tuners
1. Skip to: [System Overview](ADAPTIVE_ECU_SYSTEM.md)
2. Focus on: [Tuning Guide](ADAPTIVE_ECU_TUNING.md)
3. Deep dive: [Adaptive Learning](ADAPTIVE_ECU_TUNING.md#adaptive-learning)

### For 3D Printing Enthusiasts
1. [Piggyback Harness Design](ADAPTIVE_ECU_PIGGYBACK_HARNESS.md)
2. OpenSCAD file in `/hardware/3d_models/`
3. Parametric design - customize for your ECU!

### For Software Developers
1. Firmware in `/workspace/firmware/adaptive_ecu/`
2. [Tuning Guide](ADAPTIVE_ECU_TUNING.md) - Algorithm details
3. Contribute: Improve learning algorithms, add features

---

## 🔧 Quick Reference Cards

### Pin Assignments (ESP32-S3)

| Pin | Function | Connection |
|-----|----------|------------|
| **GPIO21** | I2C SDA | MCP9600 (EGT) + BMP280 (Baro) |
| **GPIO22** | I2C SCL | MCP9600 + BMP280 |
| **GPIO27** | Mode Switch | Toggle: Gauge ↔ ECU |
| **GPIO33** | TPS Input | Throttle position (from ECU) |
| **GPIO34** | High MAP | AEM 3.5-bar sensor |
| **GPIO35** | Wideband O2 | AEM X-Series (0-5V) |
| **GPIO36** | ECU Coolant | THW signal (via voltage divider) |
| **GPIO39** | ECU IAT | THA signal (via voltage divider) |
| **GPIO25** | Boost PWM | Solenoid valve control |
| **SPI** | SD Card | CS=5, MOSI=23, MISO=19, SCK=18 |
| **RGB** | Display | 40-pin ribbon cable |

### Safety Limits (Default)

| Parameter | Warning | Critical | Danger | Action |
|-----------|---------|----------|--------|--------|
| **EGT** | 600°C | 650°C | 700°C | Reduce fuel/boost |
| **AFR** | 19:1 | 18:1 | 17:1 | Lean out (smoke!) |
| **Boost** | 1.0 bar | 1.3 bar | 1.5 bar | Limit boost |
| **Coolant** | 100°C | 105°C | 110°C | Alert driver |

### Mode Switch Quick Reference

| Mode | Switch Position | Display | ECU Signals |
|------|----------------|---------|-------------|
| **GAUGE** | Left | "MODE: GAUGE 📊" | Pass-through (unmodified) |
| **ECU** | Right | "⚠️ MODE: ECU TUNING ⚠️" | Modified (learning active) |

---

## 📞 Getting Help

### Documentation Issues
- Typo or unclear instructions? Open issue on GitHub
- Missing information? Create feature request

### Build Problems
1. Check: [Troubleshooting Guide](ADAPTIVE_ECU_TROUBLESHOOTING.md)
2. Search: GitHub Issues (someone may have had same problem)
3. Ask: GitHub Discussions

### Tuning Questions
1. Read: [Tuning Guide](ADAPTIVE_ECU_TUNING.md)
2. Check: Logged data (is something abnormal?)
3. Forum: TurboDieselRegister, IH8MUD (diesel-specific)

### Software Bugs
1. Provide: Serial monitor output
2. Include: Data logs (CSV files)
3. Describe: Steps to reproduce
4. Open: GitHub Issue with details

---

## 🤝 Contributing to Documentation

**Found an error?** Please fix it!
1. Fork repository
2. Edit markdown files
3. Submit pull request

**Have better photos/diagrams?** Add them!
- Place in `/workspace/docs/images/`
- Reference in markdown: `![Description](images/photo.jpg)`

**Created a guide for your vehicle?** Share it!
- Create new doc: `ADAPTIVE_ECU_VEHICLE_SPECIFIC_MYCAR.md`
- Submit PR with your specific ECU pinouts/maps

---

## 📊 Documentation Statistics

**Total Documentation:**
- 📄 15+ comprehensive guides
- 💻 Complete firmware codebase
- 🔧 Hardware schematics and PCB designs
- 🎨 3D printable models
- 📝 100+ pages of documentation

**Estimated Reading Time:**
- Quick Start: 30 minutes
- Core Documentation: 4-6 hours
- Complete System: 12-15 hours

**Build Time Estimates:**
- Gauge Mode: 1-2 days
- ECU Connection: 1 day (splice) or 1 week (harness)
- ECU Mode: 1-2 weeks
- Tuning & Optimization: Ongoing

---

## 🎉 You're Ready!

You now have access to:
- ✅ Complete system architecture
- ✅ Two installation methods (detailed)
- ✅ Full firmware source code
- ✅ Hardware schematics
- ✅ 3D printable designs
- ✅ Safety procedures
- ✅ Tuning guides
- ✅ Troubleshooting help

**Start with:** [Quick Start Guide](ADAPTIVE_ECU_QUICK_START.md)

**Questions?** Check the appropriate guide above!

**Ready to build?** Let's go! 🚀

---

**Last Updated:** 2025-11-11  
**Documentation Version:** 2.0  
**Project:** JNKR Adaptive ECU System

**Built with ❤️ for the turbodiesel community!**
