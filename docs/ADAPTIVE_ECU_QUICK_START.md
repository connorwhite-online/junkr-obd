# JNKR Adaptive ECU - Quick Start Guide

**Get up and running in one afternoon!**

---

## 🎯 What You're Building

A professional piggyback ECU that:
- ✅ Monitors all engine parameters (2.1" round display)
- ✅ Logs data to SD card
- ✅ Controls boost electronically  
- ✅ Optionally modifies ECU signals for adaptive tuning
- ✅ Two modes: Safe Gauge Mode + Advanced ECU Mode

---

## 📦 What You Need

### Buy These (Total: ~$430 for Gauge Mode)

**Core ($79):**
- ESP32-S3 DevKit: $10
- 2.1" Round RGB Display (480x480): $30
- SD card module + 32GB card: $11
- Mode toggle switch: $5
- 12V-5V buck converter (3A): $8
- Enclosure: $15

**Sensors ($350):**
- AEM X-Series Wideband O2: $220
- K-type EGT probe + MCP9600: $55
- AEM 3.5-bar MAP sensor: $70
- BMP280 barometric: $5

**ECU Connection - Choose One:**
- Option A (Piggyback): 2× used ECU harness ($40) + 3D printed housing
- Option B (Splice): 100kΩ resistors ($2) + scotchlok taps ($10)

---

## 🚀 Build Steps (Gauge Mode First!)

### Step 1: Assemble Core System (2 hours)

```
1. Flash ESP32 with firmware (see firmware/ folder)
2. Connect display to ESP32 (40-pin ribbon cable)
3. Add SD card module (SPI: CS=5, MOSI=23, MISO=19, SCK=18)
4. Add mode switch (GPIO27 with pullup)
5. Install in enclosure
6. Connect 12V→5V buck converter
7. Bench test: Upload code, verify display works
```

### Step 2: Install Sensors (4 hours)

**Wideband O2:**
- Weld 18mm bung in exhaust (6-12" from turbo)
- Install sensor
- Run 0-5V signal wire to ESP32 GPIO35
- Mount controller in cabin

**EGT:**
- Weld M6 bung in exhaust manifold  
- Install K-type probe
- Connect MCP9600 module to ESP32 I2C (SDA=21, SCL=22)

**High-Range MAP:**
- T-tap into intake manifold
- Mount AEM 3.5-bar sensor
- Connect to ESP32 GPIO34 (0-5V analog)

**Barometric:**
- Mount BMP280 inside enclosure
- Connect to ESP32 I2C (shares with MCP9600)

### Step 3: First Test Drive (Gauge Mode Only!)

```
✅ Turn on ignition
✅ Display shows "GAUGE MODE"
✅ All sensors reading
✅ Drive normally
✅ Monitor AFR, EGT, boost
✅ Log data to SD card
✅ Analyze stock ECU behavior
```

**DO THIS FOR 100-200 MILES BEFORE ECU MODE!**

---

## 🔌 ECU Connection (Choose Your Method)

### Option A: Piggyback Harness (Recommended)

**What to do:**
1. Buy 2× used ECU harnesses with connectors (~$20 each on eBay)
2. 3D print housing (use OpenSCAD file in `/hardware/`)
3. Build internal PCB or wire harness (see schematics)
4. Install between ECU and sensor harness

**Signals to tap:**
- THW (Coolant): Via 100kΩ voltage divider → GPIO36
- THA (IAT): Via 100kΩ voltage divider → GPIO39
- PIM (MAP): Via 100kΩ voltage divider → GPIO34
- TPS (Throttle): Via 100kΩ voltage divider → GPIO33
- E2 (Ground): Direct connection

**Full guide:** [ADAPTIVE_ECU_PIGGYBACK_HARNESS.md](ADAPTIVE_ECU_PIGGYBACK_HARNESS.md)

### Option B: Splice/Tap Method (Faster/Cheaper)

**What to do:**
1. Remove ECU (behind glove box typically)
2. Identify sensor pins with multimeter (see pinout guide)
3. Build voltage dividers (100kΩ + 100kΩ)  
4. Backprobe or use scotchlok taps
5. Route wires to ESP32

**Voltage divider for EACH signal:**
```
ECU Pin → [100kΩ] → ESP32 GPIO → [100kΩ] → GND
```

**Why:** ECU input impedance ~10kΩ, our tap is 200kΩ total = <5% loading

**Full guide:** [ADAPTIVE_ECU_SPLICE_METHOD.md](ADAPTIVE_ECU_SPLICE_METHOD.md)

---

## 🎮 Using The System

### Gauge Mode (Default - SAFE!)

**Physical switch position:** GAUGE  
**Display shows:** "MODE: GAUGE 📊"

**What it does:**
- ✅ Reads all sensors
- ✅ Displays on screen
- ✅ Logs to SD card
- ✅ Controls boost (safety limits only)
- ❌ Does NOT modify ECU signals

**Use this for:**
- Daily driving
- Learning your engine
- Data logging
- Testing new sensors
- Any time you want zero risk

### ECU Mode (Advanced - ACTIVE TUNING!)

**Physical switch position:** ECU  
**Display shows:** "⚠️ MODE: ECU TUNING ⚠️"

**What it does:**
- ✅ All Gauge Mode features PLUS:
- ✅ Reads OEM sensor signals
- ✅ Applies learned corrections
- ✅ Modifies signals to ECU
- ✅ Adaptive learning enabled
- ✅ Advanced boost control

**Use this for:**
- Performance tuning
- Economy optimization
- Custom boost curves
- Adaptive learning

**Requirements to enable:**
- ✅ Engine must be running
- ✅ All sensors reading valid values
- ✅ Coolant temp 40-110°C
- ✅ EGT <600°C
- ✅ Learned maps exist OR learning mode on

---

## 📊 What the Display Shows

```
┌─────────────────────────────────┐
│ MODE: GAUGE 📊                  │ ← Current mode
├─────────────────────────────────┤
│                                 │
│  🔥 EGT:    548°C               │ ← Exhaust temp
│  💨 Boost:  12.3 PSI            │ ← Turbo boost
│  🌡️  Coolant: 87°C              │ ← Coolant temp
│  🌡️  IAT:     45°C              │ ← Intake temp
│  📊 AFR:    19.2:1              │ ← Air/fuel ratio
│  📈 MAP:    1.85 bar            │ ← Manifold pressure
│  🎚️  TPS:     65%               │ ← Throttle position
│  ⚡ RPM:    2850                 │ ← Engine speed
│                                 │
│  ⏺  LOGGING (125 MB)            │ ← SD card status
└─────────────────────────────────┘

Colors change based on thresholds:
🟢 Green: Normal
🟡 Yellow: Warning  
🟠 Orange: Critical
🔴 Red: Danger!
```

---

## 🛡️ Safety Features (Always Active)

**Hardware:**
- Physical mode switch (can't accidentally enable ECU mode)
- Watchdog timer (auto-reset if frozen)
- Fused power supply (3A)

**Software:**
- EGT hard limit: 650°C (immediate intervention)
- AFR smoke limit: 18:1 under boost
- Boost overrun: Max 1.5 bar
- Sensor fault detection
- Auto-revert to Gauge Mode on ANY error

**Emergency:**
- Flip mode switch to GAUGE → instant safe mode
- Disconnect piggyback harness → pure stock ECU

---

## 📈 Adaptive Learning (ECU Mode)

### How It Works

```
1. Start with ZERO corrections (stock behavior)
2. Drive normally while monitoring wideband O2
3. System compares actual AFR to target AFR
4. If off by >0.5 AFR: Apply small correction
5. Save correction to map cell (RPM × Load)
6. Repeat for all driving conditions
7. After 200-500 miles: Fully learned!
```

### What Gets Learned

**Per map cell (12 RPM × 12 Load = 144 cells):**
- Fuel correction (-20% to +20%)
- Timing adjustment (if enabled)
- Boost target (per gear, per RPM)

### Targets (Diesel-Specific)

| Condition | Target AFR | Why |
|-----------|-----------|-----|
| Idle | 25:1 | Lean for clean idle |
| Cruise (light load) | 22:1 | Efficiency |
| Moderate load | 19-20:1 | Balance |
| Full boost | 18:1 | Smoke limit, max power |
| EGT >600°C | 20:1+ | Cool down! |

---

## 🔧 Troubleshooting

### "Can't enable ECU mode"

**Check:**
- Is engine running? (Must be >500 RPM)
- Are all sensors reading valid values?
- Is EGT <600°C?
- Are learned maps loaded? (Or learning mode enabled)

### "Sensor fault" warnings

**Check:**
- Wiring connections (loose wire?)
- Sensor power (5V present?)
- Voltage ranges (within 0.5-4.5V for analog?)
- I2C pullups (for MCP9600, BMP280)

### "Display shows garbage"

**Check:**
- 40-pin ribbon cable fully inserted
- Correct FFC orientation (contacts facing right way)
- Stable 5V power (measure with multimeter)
- Firmware uploaded correctly

### "Wrong readings"

**Check:**
- Sensor calibration (run calibration routine)
- Voltage divider values (100kΩ + 100kΩ)
- ADC attenuation setting (should be 11dB for 0-3.3V)
- Compare to OEM gauge cluster

---

## 📚 Next Steps

**After building Gauge Mode:**

1. **Drive 100-200 miles** in Gauge Mode
   - Log all data to SD card
   - Analyze stock ECU behavior
   - Verify all sensors accurate
   - Learn your engine's characteristics

2. **Analyze the data:**
   - What is stock AFR under boost? (Typically 20-22:1)
   - Where is stock EGT highest? (Usually 3000-3500 RPM, full load)
   - Is there room for improvement? (Leaner than 18:1 under boost = yes!)

3. **Decide:**
   - Is stock tune good enough? → Enjoy your gauge!
   - Want more power/economy? → Build ECU Mode hardware

4. **ECU Mode (if proceeding):**
   - Build signal modifier circuits (DACs + op-amps)
   - Install mode switch
   - Load firmware with ECU mode enabled
   - Start with zero corrections
   - Enable learning mode
   - Drive another 200-500 miles
   - Validate on dyno (optional but recommended)

---

## 💾 Firmware Updates

**Location:** `/workspace/firmware/adaptive_ecu/`

**To update:**
```bash
1. Connect ESP32 via USB
2. Open Arduino IDE or Platform IO
3. Select: Board → ESP32-S3 Dev Module
4. Upload: adaptive_ecu_main.ino
5. Open Serial Monitor (115200 baud)
6. Verify initialization messages
```

**Configuration:** Edit `config.h` for your specific:
- Sensor calibration values
- Target AFR tables
- Boost limits
- Safety thresholds
- Display preferences

---

## 📊 Data Logging

**Files on SD card:**
- `datalog_YYYYMMDD_HHMMSS.csv` - Main sensor log (10-50 Hz)
- `maps_learned.bin` - Learned correction maps
- `config.txt` - Current configuration
- `events.log` - Safety events, mode changes

**Analyzing logs:**
```python
import pandas as pd
import matplotlib.pyplot as plt

# Load data
df = pd.read_csv('datalog_20251111_143000.csv')

# Plot AFR vs RPM
plt.scatter(df['RPM'], df['AFR'], c=df['Boost_PSI'], cmap='viridis')
plt.xlabel('RPM')
plt.ylabel('AFR')
plt.colorbar(label='Boost (PSI)')
plt.title('AFR Map')
plt.show()

# Find lean conditions under boost
lean_boost = df[(df['Boost_PSI'] > 10) & (df['AFR'] > 19)]
print(f"Lean under boost: {len(lean_boost)} samples")
print(f"Average EGT when lean: {lean_boost['EGT'].mean():.0f}°C")
```

---

## 🎓 Learning Resources

**Before building:**
- Read: "Maximum Boost" by Corky Bell
- Watch: Engineering Explained (YouTube) - Turbo basics
- Forum: TurboDieselRegister - 1KZ-TE specific

**During building:**
- ESP32 docs: https://docs.espressif.com
- LVGL docs: https://docs.lvgl.io (for display)
- This repo: All schematics and code included!

**After building:**
- Dyno tuning basics
- PID control theory
- Diesel fueling strategies

---

## ⚠️ Critical Warnings

**DO NOT:**
- ❌ Enable ECU mode without extensive Gauge mode testing first
- ❌ Exceed EGT limit (650°C for extended periods)
- ❌ Run leaner than 17:1 under full boost (smoke/damage risk)
- ❌ Increase boost without validating AFR is safe
- ❌ Modify timing without dyno validation
- ❌ Ignore warning lights or check engine codes

**DO:**
- ✅ Start conservative (Gauge mode for 100+ miles)
- ✅ Monitor wideband O2 constantly in ECU mode
- ✅ Keep EGT below 650°C sustained
- ✅ Target 18-19:1 AFR under boost
- ✅ Log everything
- ✅ Have backup plan if system fails

---

## 🆘 Emergency Procedures

### If engine runs poorly with ECU mode:

1. **Immediate:** Flip mode switch to GAUGE
2. Engine should return to stock behavior
3. Drive to safe location
4. Diagnose issue before re-enabling ECU mode

### If EGT spikes above 650°C:

1. System will auto-reduce boost/fuel
2. If still climbing: Reduce throttle immediately
3. Pull over if exceeds 700°C
4. Let engine idle to cool
5. Check for causes (boost leak, fuel issue, etc.)

### If wideband O2 shows <17:1 under boost:

1. System will attempt to lean out
2. Expect heavy smoke
3. Reduce throttle
4. Check fuel system (stuck injector?)

### Complete system failure:

1. Flip mode switch to GAUGE
2. If still issues: Disconnect piggyback harness
3. Vehicle will run on stock ECU only
4. May have check engine light (clear codes)
5. System is fully reversible!

---

## 📞 Support

**Questions?**
- GitHub Issues: Technical problems
- GitHub Discussions: General questions
- Forums: TurboDieselRegister, IH8MUD

**Found a bug?**
- Open issue with:
  - Description of problem
  - Steps to reproduce
  - Serial monitor output
  - Data logs (if relevant)

**Want to contribute?**
- Fork repo
- Make improvements
- Submit pull request
- Help others in Issues/Discussions

---

## ✅ Pre-Flight Checklist

**Before first test drive:**

- [ ] All sensors bench-tested and reading correctly
- [ ] Display showing reasonable values
- [ ] SD card logging working
- [ ] Mode switch toggles between modes correctly
- [ ] Gauge Mode selected (not ECU mode!)
- [ ] All wiring secure and insulated
- [ ] Fire extinguisher in vehicle (seriously!)
- [ ] Emergency plan if something goes wrong
- [ ] Someone knows where you're going (safety!)

**Before enabling ECU Mode (first time):**

- [ ] Driven 100+ miles in Gauge Mode successfully
- [ ] All sensor readings validated against OEM gauge
- [ ] Logged extensive data on stock ECU behavior
- [ ] Understand target AFRs for diesel
- [ ] Signal modifier circuits tested on bench
- [ ] Conservative base maps loaded (all zeros)
- [ ] Wideband O2 reading correctly
- [ ] Emergency procedure practiced (flip to Gauge)
- [ ] Dyno appointment scheduled (optional but wise)

---

**Ready to build? Let's go!** 🚀

Start with Gauge Mode - it's safe, fun, and educational. ECU Mode is there when you're ready to take it further!

**Questions?** Check the full documentation in `/workspace/docs/`

**Built with ❤️ for the turbodiesel community!**
