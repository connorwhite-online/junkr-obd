# What Data is Available from DLC3 Diagnostic Port?

## Quick Answer

**DLC3 gives you access to MUCH MORE than just sensor values!**

You get:
- ✅ **Sensor values** (coolant temp, IAT, MAP, etc.)
- ✅ **ECU-calculated values** (RPM, fuel injection timing, etc.)
- ✅ **Diagnostic codes** (error codes, fault history)
- ✅ **Status flags** (engine state, system status)
- ✅ **Calibration data** (injection timing, boost control, etc.)
- ✅ **Live data streams** (real-time parameter monitoring)

**Much more comprehensive than analog intercept!**

---

## Detailed Breakdown

### 1. Sensor Values (Same as Analog Intercept)

**Available via DLC3:**
- ✅ Coolant temperature (THW)
- ✅ Intake air temperature (THA)
- ✅ Manifold absolute pressure (MAP/boost)
- ✅ Throttle position (TPS)
- ✅ Battery voltage
- ✅ Vehicle speed
- ✅ Engine RPM

**Note:** These are the same values you'd get from analog intercept, but via diagnostic protocol.

---

### 2. ECU-Calculated Values (NOT Available via Analog Intercept)

**These are calculated by the ECU and only available via diagnostics:**

- ✅ **Fuel injection quantity** - How much fuel is being injected
- ✅ **Injection timing** - When fuel is injected (degrees before TDC)
- ✅ **Boost pressure target** - What the ECU wants boost to be
- ✅ **Boost control duty cycle** - Wastegate control percentage
- ✅ **EGR valve position** - Exhaust gas recirculation status
- ✅ **Idle speed control** - ISCV position/duty cycle
- ✅ **Glow plug status** - Pre-heat system state
- ✅ **Turbo boost status** - Turbo system health
- ✅ **Fuel pump timing** - Injection pump synchronization
- ✅ **Engine load** - Calculated engine load percentage
- ✅ **Air-fuel ratio** - Calculated mixture (from VF1 and other sensors)
- ✅ **Timing advance** - Ignition/injection timing advance

**These are ECU-internal calculations - you CAN'T get them from analog intercept!**

---

### 3. Diagnostic Codes and Faults

**Available via DLC3:**
- ✅ **Current DTCs** - Active diagnostic trouble codes
- ✅ **Pending DTCs** - Intermittent faults
- ✅ **Stored DTCs** - Historical fault codes
- ✅ **Freeze frame data** - Snapshot when fault occurred
- ✅ **System status** - Which systems are active/faulty

**Example codes for 1KZ-TE:**
- P0100-P0199: Fuel/air metering
- P0200-P0299: Fuel injector circuit
- P0300-P0399: Ignition/misfire
- P0400-P0499: EGR system
- P0500-P0599: Vehicle speed/idle control
- P0600-P0699: ECU/internal faults

**This is diagnostic-only - analog intercept can't tell you about fault codes!**

---

### 4. System Status and Flags

**Available via DLC3:**
- ✅ **Engine running status** - Is engine on/off/cranking
- ✅ **Warm-up status** - Is engine in warm-up mode
- ✅ **Boost control active** - Is wastegate being controlled
- ✅ **EGR system status** - Is EGR active/inactive
- ✅ **Glow plug status** - Pre-heat system state
- ✅ **Immobilizer status** - Security system state
- ✅ **Transmission status** - Gear position (auto trans)
- ✅ **AC request** - Is AC requesting more idle speed
- ✅ **Power steering load** - Is PS pump loaded
- ✅ **Battery charging status** - Alternator output

**System state information - not available via analog intercept!**

---

### 5. Calibration and Tuning Parameters

**Available via DLC3 (read-only, but visible):**
- ✅ **Injection timing maps** - Base timing values
- ✅ **Boost control maps** - Wastegate control curves
- ✅ **Fuel quantity maps** - Fuel delivery curves
- ✅ **Idle speed targets** - Target RPM values
- ✅ **Temperature thresholds** - ECU calibration points
- ✅ **Pressure thresholds** - Boost/vacuum limits

**Note:** These are typically read-only, but seeing them helps understand ECU behavior.

---

### 6. Live Data Streams

**Available via DLC3:**
- ✅ **Real-time parameter monitoring** - All values updating live
- ✅ **Data logging** - Record all parameters over time
- ✅ **Graphing** - Plot multiple parameters simultaneously
- ✅ **Triggered logging** - Record when conditions are met

**Much more powerful than single-value reading!**

---

## Comparison: DLC3 vs Analog Intercept

| Data Type | DLC3 | Analog Intercept |
|-----------|------|------------------|
| **Basic Sensors** | ✅ Yes | ✅ Yes |
| **ECU-Calculated Values** | ✅ Yes | ❌ No |
| **Diagnostic Codes** | ✅ Yes | ❌ No |
| **System Status** | ✅ Yes | ❌ No |
| **Calibration Data** | ✅ Yes (read) | ❌ No |
| **Live Data Logging** | ✅ Yes | ⚠️ Manual |
| **Fault History** | ✅ Yes | ❌ No |
| **Freeze Frame Data** | ✅ Yes | ❌ No |

---

## What You Can't Do (Read-Only Port)

**DLC3 is READ-ONLY for data access:**
- ❌ **Can't modify ECU settings** - No writing/calibration changes
- ❌ **Can't clear codes** - (Actually, you CAN clear codes - that's a diagnostic function)
- ❌ **Can't adjust timing** - No tuning via diagnostic port
- ❌ **Can't change fuel maps** - No remapping via DLC3

**But you CAN:**
- ✅ Read all available data
- ✅ Clear diagnostic trouble codes (DTCs)
- ✅ Reset adaptations (some ECUs)
- ✅ Activate test modes (some functions)

---

## Specific Data for 1KZ-TE Engine

### Typical Parameters Available (Based on Toyota Diagnostic Standards)

**Engine Parameters:**
- Engine RPM (calculated from crank sensor)
- Coolant temperature (THW sensor)
- Intake air temperature (THA sensor)
- Manifold pressure (MAP sensor)
- Throttle position (TPS sensor)
- Battery voltage
- Vehicle speed

**Fuel System:**
- Fuel injection quantity (calculated)
- Injection timing (calculated)
- Fuel pump timing (synchronization)
- Injection pump position
- Spill valve duty cycle

**Turbo System:**
- Boost pressure (actual)
- Boost pressure target (ECU target)
- Wastegate duty cycle (boost control)
- Turbo status flags

**EGR System:**
- EGR valve position
- EGR duty cycle
- EGR system status

**Idle Control:**
- Idle speed (actual)
- Idle speed target
- ISCV position/duty cycle
- Idle control status

**Diagnostic:**
- All DTCs (current, pending, stored)
- Freeze frame data
- System status flags
- Component test modes

---

## Example: What TechStream Shows

**If you use Toyota TechStream (official tool), you can see:**

1. **Data List:**
   - 50+ parameters updating in real-time
   - All sensor values
   - All calculated values
   - All system status flags

2. **Active Test:**
   - Activate components (EGR valve, ISCV, etc.)
   - Test system functions
   - Verify component operation

3. **Diagnostic Codes:**
   - Read all DTCs
   - Clear DTCs
   - View freeze frame data

4. **Data Logging:**
   - Record all parameters
   - Export to CSV
   - Analyze trends

**Your ESP32 can access the same data!**

---

## What This Means for Your Project

### If You Use DLC3:

**You get:**
- ✅ All basic sensor values (same as analog intercept)
- ✅ **PLUS** ECU-calculated values (injection timing, boost control, etc.)
- ✅ **PLUS** diagnostic codes (know when something's wrong)
- ✅ **PLUS** system status (know what systems are active)
- ✅ **PLUS** comprehensive data logging

**Much more powerful than analog intercept!**

### If You Use Analog Intercept:

**You get:**
- ✅ Basic sensor values only
- ❌ No ECU-calculated values
- ❌ No diagnostic codes
- ❌ No system status
- ⚠️ Manual data logging

**Simpler, but more limited.**

---

## Recommended Approach

### Option 1: DLC3 Only (Recommended if you want comprehensive data)

**Use DLC3 for:**
- All sensor values
- ECU-calculated values
- Diagnostic codes
- System status
- Data logging

**Pros:**
- Most comprehensive data access
- Non-invasive (just plug in)
- Professional diagnostic approach
- Can detect problems (DTCs)

**Cons:**
- Slightly slower (10-20Hz)
- Requires K-Line transceiver
- More complex implementation

### Option 2: Hybrid Approach (Best of Both Worlds)

**Use DLC3 for:**
- Diagnostic codes
- System status
- ECU-calculated values
- Comprehensive logging

**Use Analog Intercept for:**
- High-speed critical sensors (100Hz+)
- Real-time boost monitoring
- Fast response gauges

**Pros:**
- Best of both worlds
- Fast critical data + comprehensive diagnostics
- Redundancy (can verify sensor readings)

**Cons:**
- More complex setup
- More wiring
- Higher cost

### Option 3: Analog Intercept Only

**Use Analog Intercept for:**
- Basic sensor values only
- High-speed monitoring

**Pros:**
- Simplest setup
- Fastest (100Hz+)
- Cheapest

**Cons:**
- Limited data (no ECU calculations)
- No diagnostic codes
- No system status

---

## Bottom Line

**DLC3 gives you MUCH MORE than just sensor values:**

1. ✅ **All sensor values** (same as analog intercept)
2. ✅ **ECU-calculated values** (injection timing, boost control, etc.)
3. ✅ **Diagnostic codes** (know when something's wrong)
4. ✅ **System status** (what systems are active)
5. ✅ **Comprehensive logging** (record everything)

**It's a full diagnostic interface, not just sensor reading!**

**Analog intercept only gives you:**
- Basic sensor voltages
- That's it

**DLC3 is much more powerful if you want comprehensive engine monitoring and diagnostics.**

---

**Document Created:** 2025-01-27  
**Purpose:** Clarify what data is available from DLC3 diagnostic port
