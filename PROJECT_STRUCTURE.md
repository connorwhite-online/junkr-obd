# JNKR Gauge System - Project Structure

**Complete Repository Layout and File Guide**

> 📢 **Platform Update**: This project now uses **Qualia ESP32-S3** + **2.1" Round RGB Display**
> 
> Some documentation marked as "Legacy" refers to the previous Arduino Mega platform.
> See [MIGRATION_SUMMARY.md](MIGRATION_SUMMARY.md) for details.

This document explains the organization of the JNKR Gauge System repository and describes each file's purpose.

---

## 📁 Repository Structure

```
jnkr-gauge/
├── README.md                    ← Start here! Main documentation
├── LICENSE                      ← MIT License
├── PROJECT_STRUCTURE.md         ← This file
├── .gitignore                   ← Git ignore rules
│
├── jnkr-gauge.ino              ← Main Arduino sketch
├── config.h                     ← Configuration and pin assignments
│
├── sensors.h                    ← Sensor library header
├── sensors.cpp                  ← Sensor library implementation
│
├── display.h                    ← Nextion display library header
├── display.cpp                  ← Nextion display library implementation
│
├── alerts.h                     ← Alert system header
├── alerts.cpp                   ← Alert system implementation
│
└── docs/                        ← Documentation folder
    │
    ├── ⭐ CURRENT PLATFORM DOCS (ESP32-S3):
    ├── COMPLETE_BOM.md          ← ⭐ Parts list for ESP32-S3 build
    ├── ESP32_SETUP.md           ← ⭐ Arduino IDE setup for ESP32-S3
    ├── BREADBOARD_WIRING.md     ← ⭐ Wiring guide for ESP32-S3
    ├── SQUARELINE_LVGL_GUIDE.md ← ⭐ GUI design with LVGL
    ├── CALIBRATION.md           ← Sensor calibration (platform-agnostic)
    │
    ├── 📦 LEGACY DOCS (Arduino Mega - Reference Only):
    ├── BOM.md                   ← [Legacy] Arduino Mega parts list
    ├── WIRING.md                ← [Legacy] Arduino Mega wiring
    ├── ASSEMBLY.md              ← [Legacy] Arduino Mega assembly
    ├── NEXTION.md               ← [Legacy] Nextion display guide
    ├── SHIELD.md                ← [Legacy] Arduino shield design
    ├── PCB_SCHEMATIC.md         ← [Legacy] Arduino shield PCB
    │
    └── shield/                  ← [Legacy] Shield design files
        └── schematic-description.md  ← [Legacy] Circuit explanations
```

---

## 📄 File Descriptions

### Root Files

#### README.md
**Purpose:** Main project documentation  
**Contents:**
- Feature overview
- Hardware requirements
- Installation instructions
- Usage guide
- Troubleshooting
- Links to detailed docs

**Start here if you're new to the project!**

#### LICENSE
**Purpose:** Open-source license terms  
**License:** MIT License  
**Allows:** Personal and commercial use, modification, distribution  
**Requirements:** Include copyright notice  

#### PROJECT_STRUCTURE.md
**Purpose:** Repository organization guide  
**This file** - explains what each file does

#### .gitignore
**Purpose:** Git version control exclusions  
**Excludes:**
- Build artifacts
- IDE files
- Temporary files
- OS-specific files

---

### Arduino Code Files

#### jnkr-gauge.ino
**Purpose:** Main Arduino sketch  
**Contents:**
- `setup()` function - initializes system
- `loop()` function - main control loop
- System state management
- Timing and scheduling
- Debug output

**This is the entry point - upload this file to Arduino**

#### config.h
**Purpose:** System configuration  
**Contents:**
- Pin assignments (dual IAT sensors)
- Sensor calibration values
- Temperature thresholds (pre/post intercooler IAT)
- Pressure limits
- Alert levels
- Display settings
- Filter coefficients

**Customize this file for your vehicle**

---

### Sensor Library

#### sensors.h
**Purpose:** Sensor interface definitions  
**Provides:**
- Function declarations
- Public API
- Documentation comments

#### sensors.cpp
**Purpose:** Sensor implementation  
**Handles:**
- NTC thermistor reading (dual IAT: pre/post intercooler, coolant)
- EGT reading via MAX31855
- MAP/boost pressure reading
- Signal filtering
- Fault detection
- Calibration offsets (separate for each IAT sensor)

---

### Display Library

#### display.h
**Purpose:** Nextion display interface  
**Provides:**
- Display function declarations
- Component naming conventions

#### display.cpp
**Purpose:** Nextion display implementation  
**Handles:**
- UART communication with display
- Gauge updates
- Color management
- Page navigation
- Alert messages
- Status indicators

---

### Alert System

#### alerts.h
**Purpose:** Alert system interface  
**Provides:**
- Alert level definitions
- Function declarations

#### alerts.cpp
**Purpose:** Alert system implementation  
**Handles:**
- Threshold monitoring
- Alert level determination
- Buzzer control
- Beep patterns
- Alert acknowledgment
- Audio enable/disable

---

## 📚 Documentation Files

### docs/WIRING.md
**For:** Beginners and experienced builders  
**Contents:**
- Complete wiring instructions
- Voltage divider explanations
- Pin-by-pin connections
- Safety guidelines
- Tools and materials
- Testing procedures
- Wiring diagrams
- Color coding reference
- Troubleshooting

**Read this before starting assembly!**

### docs/ASSEMBLY.md
**For:** Complete build process  
**Contents:**
- 7-phase assembly guide
- Enclosure preparation
- Component mounting
- Wiring steps
- Software upload
- Bench testing
- Vehicle installation
- Final testing

**Follow this step-by-step guide**

### docs/BOM.md
**For:** Parts procurement  
**Contents:**
- Complete parts list
- Part numbers
- Specifications
- Quantity needed
- Pricing information
- Supplier recommendations
- Cost breakdowns
- Shopping strategies
- Alternatives

**Use this to order all parts**

### docs/CALIBRATION.md
**For:** Sensor accuracy  
**Contents:**
- Calibration procedures
- Reference methods
- Offset calculation
- Code modifications
- Testing verification
- Troubleshooting
- Calibration log

**Use after assembly if readings are off**

### docs/NEXTION.md
**For:** Custom display design  
**Contents:**
- Nextion Editor guide
- Display specifications
- Design tutorial
- Component naming
- Upload procedures
- Arduino integration
- Examples and tips

**Read if customizing the display**

### docs/SHIELD.md
**For:** PCB shield assembly  
**Contents:**
- Shield features
- Schematic overview
- PCB layout
- Component BOM
- Assembly instructions
- Testing procedures
- PCBWay ordering
- Modifications

**Use if building the shield PCB**

### docs/shield/schematic-description.md
**For:** Circuit understanding  
**Contents:**
- Detailed circuit explanations
- Component specifications
- Voltage calculations
- Power budget
- Protection features
- Design considerations
- Testing points

**For understanding the electronics**

---

## 🔧 Usage Workflow

### For First-Time Builders

**1. Planning (Day 1):**
```
README.md → BOM.md → Order parts
```

**2. Learning (While waiting for parts):**
```
WIRING.md → ASSEMBLY.md → CALIBRATION.md
```

**3. Assembly (Weekend 1):**
```
ASSEMBLY.md Phase 1-3 → Build enclosure and wire
```

**4. Software (Weekend 1):**
```
Upload jnkr-gauge.ino → Test on bench
```

**5. Installation (Weekend 2):**
```
ASSEMBLY.md Phase 6-7 → Install in vehicle
```

**6. Calibration (Weekend 2):**
```
CALIBRATION.md → Fine-tune sensors
```

### For Shield PCB Builders

**1. Design Review:**
```
SHIELD.md → schematic-description.md
```

**2. Ordering:**
```
SHIELD.md "Ordering from PCBWay" section
Order components from BOM in SHIELD.md
```

**3. Assembly:**
```
SHIELD.md "Assembly Instructions" section
```

**4. Testing:**
```
SHIELD.md "Testing" section
```

**5. Integration:**
```
Continue with ASSEMBLY.md Phase 4+
```

### For Display Customization

**1. Learn Nextion:**
```
NEXTION.md → Install Nextion Editor
```

**2. Design:**
```
NEXTION.md "Creating a Simple Gauge Display"
```

**3. Upload:**
```
NEXTION.md "Uploading to Display"
```

**4. Integrate:**
```
NEXTION.md "Arduino Integration"
Modify display.cpp if needed
```

---

## 🔄 File Dependencies

```
jnkr-gauge.ino
    ├─ includes config.h
    ├─ includes sensors.h
    ├─ includes display.h
    └─ includes alerts.h

sensors.cpp
    ├─ includes sensors.h
    ├─ includes config.h
    └─ uses SPI library (for MAX31855)

display.cpp
    ├─ includes display.h
    └─ includes config.h

alerts.cpp
    ├─ includes alerts.h
    └─ includes config.h
```

**Arduino IDE automatically compiles all .cpp files in the sketch folder**

---

## 📝 Editing Guidelines

### Modifying Configuration

**To change pin assignments:**
1. Edit `config.h`
2. Modify `#define PIN_XXX` values (e.g., PIN_IAT_PRE_IC, PIN_IAT_POST_IC)
3. Upload code to Arduino

**To change thresholds:**
1. Edit `config.h`
2. Modify temperature/pressure limits
3. Upload code to Arduino

### Adding New Sensors

**Steps:**
1. Add pin definition in `config.h`
2. Add sensor reading function in `sensors.cpp`
3. Add getter function in `sensors.h`
4. Call function in `jnkr-gauge.ino`
5. Add display update in `display.cpp`

### Creating Custom Alerts

**Steps:**
1. Define threshold in `config.h`
2. Add check in `alerts.cpp` → `Alerts_CheckAll()`
3. Test with Serial Monitor
4. Verify buzzer pattern

---

## 🧪 Testing Files

**To test individual components:**

**Sensors:**
```cpp
// In loop(), add debug output:
Serial.print("IAT Pre: "); Serial.println(Sensors_GetIntakeTempPre());
Serial.print("IAT Post: "); Serial.println(Sensors_GetIntakeTempPost());
```

**Display:**
```cpp
// In setup(), test:
Display_UpdateBoost(15.0);  // Should show 15 PSI
```

**Alerts:**
```cpp
// In setup(), test:
Alerts_SetLevel(ALERT_WARNING);
Alerts_PlayBeeps(3, 250);
```

---

## 📊 File Statistics

| Type | Count | Lines of Code |
|------|-------|---------------|
| Arduino code (.ino, .cpp, .h) | 7 | ~2,500 |
| Documentation (.md) | 9 | ~8,000 |
| Configuration files | 2 | ~50 |
| **Total** | **18** | **~10,500** |

---

## 🆘 Getting Help

**Issue with specific file? Check:**

| Problem | Check File |
|---------|-----------|
| Wiring questions | WIRING.md |
| Assembly steps | ASSEMBLY.md |
| Need to buy parts | BOM.md |
| Sensor calibration | CALIBRATION.md |
| Display design | NEXTION.md |
| Shield PCB | SHIELD.md |
| Pin assignments | config.h |
| Sensor code | sensors.cpp |
| Display code | display.cpp |
| Alert code | alerts.cpp |

**Still stuck?**
1. Check README.md troubleshooting section
2. Review Serial Monitor output
3. Open GitHub Issue with:
   - Which file you're working with
   - What you've tried
   - Serial Monitor output
   - Photos if relevant

---

## 🤝 Contributing

**Want to improve the project?**

**Documentation:**
- Fix typos or unclear instructions
- Add photos or diagrams
- Translate to other languages

**Code:**
- Bug fixes
- New features
- Performance improvements
- Additional sensor support

**Hardware:**
- Shield design improvements
- Enclosure designs
- 3D printed parts
- Mounting solutions

**Submit pull requests via GitHub!**

---

## 🔄 Version History

**Current Version:** 1.0.0

**Changelog:**
- See main README.md for version history
- Each major revision documented
- Breaking changes highlighted

---

## 📞 Support Channels

1. **GitHub Issues** - Bug reports, feature requests
2. **GitHub Discussions** - General questions, show and tell
3. **README.md** - Comprehensive documentation
4. **Individual .md files** - Specific topics

---

**Thank you for using the JNKR Gauge System!**

This project is maintained by the community. Contributions, feedback, and success stories are always welcome.

---

**Last Updated:** 2025-11-08  
**Project Version:** 2.0.0 (ESP32-S3)  
**Previous Version:** 1.0.0 (Arduino Mega - Legacy)
