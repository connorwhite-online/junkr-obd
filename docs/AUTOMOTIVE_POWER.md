# Automotive Power Guide - JNKR Gauge System

**How to Properly Power Your Gauge from the Vehicle Electrical System**

This guide explains exactly where and how to draw power from your vehicle to run the JNKR Gauge System safely and reliably.

---

## 📋 Table of Contents

- [Power Requirements](#power-requirements)
- [Understanding Automotive Electrical Systems](#understanding-automotive-electrical-systems)
- [Switched vs Constant Power](#switched-vs-constant-power)
- [Where to Tap Power](#where-to-tap-power)
- [Grounding](#grounding)
- [Wire Sizing](#wire-sizing)
- [Fuse Selection](#fuse-selection)
- [Installation Methods](#installation-methods)
- [Toyota 1KZTE Specific Guide](#toyota-1kzte-specific-guide)
- [Troubleshooting](#troubleshooting)

---

## ⚡ Power Requirements

### System Power Consumption

Let's calculate what your gauge system actually needs:

| Component | Voltage | Current Draw | Power |
|-----------|---------|--------------|-------|
| **Qualia ESP32-S3** | 5V | 150-300 mA | 0.75-1.5 W |
| **2.1" Round Display** | 5V | 200-400 mA | 1.0-2.0 W |
| **ADS1115 ADC** | 5V | 5-10 mA | 0.025-0.05 W |
| **MCP9600 EGT Amp** | 5V | 5-10 mA | 0.025-0.05 W |
| **Buzzer (active)** | 5V | 20-50 mA | 0.1-0.25 W |
| **Buck Converter Loss** | 12V | ~15% loss | 0.3-0.6 W |
| **TOTAL @ 5V** | 5V | **0.4-0.8 A** | **2.0-4.0 W** |
| **TOTAL @ 12V** | 12V | **0.17-0.35 A** | **2.0-4.2 W** |

### Recommended Specifications

- **Fuse Rating**: 3A (provides safety margin)
- **Wire Gauge**: 20 AWG minimum (18 AWG recommended)
- **Buck Converter Rating**: 3A output minimum
- **Power Source**: Switched 12V @ 500 mA peak

**Why these numbers matter:**
- At 500 mA draw from 12V, you're pulling about 6 watts
- This is less than a dome light
- Modern alternators produce 70-150 amps, so this is negligible
- Battery drain when off: ZERO (if wired to switched power)

---

## 🔋 Understanding Automotive Electrical Systems

### The Basics

Your vehicle has two electrical systems:

**1. 12V System (What You'll Use)**
- Battery voltage: 12.6V when off, 13.8-14.4V when running
- Everything in your car runs on this
- Regulated by voltage regulator in alternator
- Protected by fuses in fuse box(es)

**2. High Voltage System (Hybrid/EV only)**
- Don't touch this! 
- 200-400V for electric motors
- We're not using this

### Where Does 12V Come From?

```
┌──────────────┐
│   Battery    │  12.6V when engine off
│   12V/60Ah   │  Provides starting power & backup
└──────┬───────┘
       │
       ├────→ Starter Motor (200+ amps when starting)
       │
       ├────→ Fuse Box ──→ All Vehicle Systems (lights, radio, etc.)
       │            │
       │            └────→ YOUR GAUGE (0.5A max) ← We tap here!
       │
       └────→ Alternator (charges battery when running)
                  ↓
               13.8-14.4V when engine running
```

### Constant vs Switched Circuits

Your vehicle has two types of 12V circuits:

| Type | Behavior | Examples | Battery Drain Risk |
|------|----------|----------|-------------------|
| **Constant (Always On)** | Live 24/7, even when key is off | Dome light, radio memory, alarm | **YES - Can kill battery!** |
| **Switched (Ignition)** | Only live when ignition is ON | Radio, accessories, gauges | **NO - Turns off with key** |

---

## 🔑 Switched vs Constant Power

### Why You MUST Use Switched Power

**✅ Use Switched Power (Ignition-Controlled)**

**Advantages:**
1. **Automatic On/Off**: Gauge turns on with ignition, off when you remove key
2. **Zero Battery Drain**: Can't accidentally kill your battery
3. **No Switch Needed**: No extra on/off switch to install
4. **Professional**: How OEM gauges are wired
5. **Safe**: No risk of leaving gauge on overnight

**Disadvantages:**
1. Slightly more complex to find (but we'll show you how)

### When You Might Use Constant Power

**⚠️ Use Constant Power (Always On) ONLY IF:**

1. **You add a manual power switch** AND
2. **You'll remember to turn it off** AND
3. **You want gauge to work with engine off** (e.g., showing coolant temp while parked)

**WARNING**: If you forget to turn it off, you'll drain your battery!

**Our Recommendation**: **Always use switched power** unless you have a very specific reason not to.

---

## 🎯 Where to Tap Power

### Option 1: Fuse Box with Add-A-Circuit (Recommended) ⭐

**Why This is Best:**
- ✅ Professional installation
- ✅ Fuse protected (double protection)
- ✅ Reversible (no permanent modifications)
- ✅ Clean and hidden
- ✅ Uses existing circuits safely

**What You Need:**
- Add-A-Circuit fuse tap (blade style matching your fuse box)
- 3A blade fuse (for gauge system)
- Original fuse that you're tapping into
- 20 AWG wire (red for positive)

**How It Works:**

```
┌─────────────────────────────────────┐
│         Fuse Box                    │
│                                     │
│   Original Fuse (e.g., 15A Radio)  │
│            │                        │
│   ┌────────┴────────┐              │
│   │  Add-A-Circuit  │              │
│   │  ┌───┐  ┌───┐   │              │
│   │  │15A│  │3A │   │ ← Your fuse  │
│   │  └─┬─┘  └─┬─┘   │              │
│   │    │      │     │              │
│   │    │      └──────┼──→ Red wire to gauge (switched 12V)
│   │    │            │              │
│   │    └── Original circuit (radio)
│   └─────────────────┘              │
└─────────────────────────────────────┘
```

**Step-by-Step:**

1. **Find your fuse box** (usually under dashboard or under hood)
2. **Check your owner's manual** for fuse diagram
3. **Find a SWITCHED circuit** (radio, accessories, cigarette lighter)
4. **Test with multimeter**:
   - Key OFF: Should read 0V
   - Key ON (ACC or RUN): Should read 12-14V
5. **Remove original fuse**
6. **Insert Add-A-Circuit**:
   - Bottom slot: Original fuse
   - Top slot: 3A fuse for gauge
7. **Connect red wire** to Add-A-Circuit wire
8. **Test again** with multimeter

### Option 2A: Replace Cigarette Lighter Socket (Excellent for Older Vehicles!) ⭐

**If you don't use your cigarette lighter, this is perfect:**

**Pros:**
- ✅ Uses existing switched 12V wiring (already fused!)
- ✅ Professional, hidden installation
- ✅ Perfect power capacity (typically 10-15A circuit)
- ✅ Already has ground wire
- ✅ Hole in dash for mounting gauge or running wires
- ✅ 100% reversible (keep the socket to reinstall later)

**Cons:**
- Lose cigarette lighter function (but who uses these anymore?)
- Need to remove trim to access

**How to Do It:**

**Step 1: Locate and Remove Cigarette Lighter**

1. **Find the cigarette lighter socket** (usually in center console or dashboard)
2. **Remove surrounding trim** (pry gently with plastic trim tool)
3. **Unplug lighter socket** from behind
4. **Note the wiring**:
   - Usually 2 wires: Red/Yellow (+12V) and Black (Ground)
   - Sometimes 3 wires: +12V, Ground, and Illumination (ignore the illumination wire)

**Step 2: Test the Wiring**

Use multimeter:
```
Key OFF → Should read 0V (switched power - perfect!)
Key ACC → Should read 12-14V
```

If it reads 12V with key off, it's constant power (some vehicles). You can still use it, but consider adding a switch.

**Step 3A: Direct Connection Method**

If you're mounting gauge near the lighter location:

1. **Connect directly**:
   ```
   Lighter +12V wire → [Inline 3A fuse] → Buck converter +12V
   Lighter Ground wire → Buck converter GND
   ```

2. **Mount gauge in or near lighter opening**
3. **Buck converter goes behind dash (hidden)**
4. **Run sensor wires through existing lighter hole**

**Step 3B: Wire Extension Method**

If gauge is going elsewhere:

1. **Splice extension wires**:
   ```
   Lighter +12V → [Solder + heat shrink] → 20AWG red wire → Buck converter
   Lighter GND → [Solder + heat shrink] → 20AWG black wire → Buck converter
   ```

2. **Add 3A inline fuse** within 6" of splice point
3. **Route wires to gauge location**
4. **Use lighter hole for wire pass-through**
5. **Cap off or plug original lighter socket hole** (blank plate, or reinstall socket with no connection)

**Step 4: Clean Installation**

Option A - **Gauge in Lighter Location:**
- Mount gauge directly where lighter was
- Perfect size for 2.1" round display!
- Ultra clean, looks factory

Option B - **Use Lighter Hole for Wiring:**
- Run wires through hole
- Install blank plate over hole (available at auto parts stores)
- Or keep lighter socket installed but disconnected (looks stock)

**Why This is Great for 1KZTE Vehicles:**

Most 1990s-2000s Toyota trucks with 1KZTE:
- Have cigarette lighter that's rarely used
- It's already switched power (turns off with key)
- Circuit is 10-15A (more than enough)
- Easy to access under center console
- Perfect for custom gauge installation

**Pro Tip:** Keep the lighter socket and wiring intact! Just:
1. Unplug it
2. Tap into the harness BEFORE the socket
3. Keep socket stored in glove box
4. 100% reversible for resale

### Option 2B: Plug Into Cigarette Lighter (Temporary Only)

**Pros:**
- Very easy
- No modification to vehicle
- Removable

**Cons:**
- Unprofessional appearance
- Takes up outlet
- Cable visible in cabin

**How to Do It:**

Use a 12V cigarette lighter adapter with bare wire output:
1. Get a lighter plug with wire leads
2. Cut off the plug end
3. Strip wires (center pin = +12V, side = ground)
4. Connect to buck converter input
5. Plug into outlet

**Best For**: Testing and bench testing only

### Option 3: Accessory Wire Behind Dash

**Pros:**
- Clean installation
- Professional appearance
- Direct connection

**Cons:**
- Requires dashboard removal
- Need to identify correct wire
- Slightly harder to reverse

**How to Do It:**

1. **Find radio or accessory wire harness** behind dash
2. **Identify red/yellow accessory wire** (usually red with stripe)
3. **Use T-tap connector** or **Posi-Tap connector**:
   - Clips onto existing wire
   - Pierces insulation
   - Makes connection without cutting wire
4. **Add 3A inline fuse** within 6 inches of tap point
5. **Route wire to gauge location**

### Option 4: Ignition Switch Harness

**Pros:**
- Guaranteed switched power
- Very reliable

**Cons:**
- Critical component (be careful!)
- Dashboard removal required
- Incorrect wire can affect starting

**How to Do It:**

1. **Locate ignition switch behind steering column**
2. **Find ACC (accessory) wire** using wiring diagram
3. **Use T-tap connector only** (never cut!)
4. **Add inline fuse immediately**
5. **Test thoroughly before reassembly**

---

## 🔌 Grounding

**Ground is just as important as power!** A poor ground causes:
- Erratic readings
- Display flickering
- System resets
- Noise in sensors
- Complete failure

### Good Ground Points

**✅ Best Locations:**

1. **Chassis Ground Bolt**
   - Bolt directly to metal chassis
   - Existing ground point (near fuse box)
   - Clean metal contact

2. **Dashboard Support Bracket**
   - Metal dash frame
   - Behind instrument cluster
   - Must be painted metal removed

3. **Seat Belt Anchor Bolt**
   - Large, secure bolt
   - Direct chassis connection
   - Easy to access

**❌ Bad Ground Points:**

- Plastic surfaces (no conductivity)
- Painted metal (insulator)
- Loose bolts
- Dirty/rusty metal
- Aluminum (corrodes, increases resistance)

### How to Make a Good Ground

**Step-by-Step:**

1. **Choose location** near gauge (shorter = better)

2. **Prepare surface**:
   - Sand away paint with sandpaper
   - Expose bare metal (shiny!)
   - Clean with isopropyl alcohol
   - Dry completely

3. **Prepare wire**:
   - Strip 1/4" of insulation
   - Crimp on ring terminal
   - Size terminal for bolt (M6 or M8 typical)

4. **Install**:
   - Place ring terminal on bolt
   - Add star washer (bites into metal)
   - Tighten securely
   - Tug wire to test

5. **Protect**:
   - Spray with electrical contact cleaner
   - Optional: Dielectric grease
   - Prevents corrosion

6. **Test**:
   - Multimeter continuity test
   - Ground to battery negative: should be 0Ω
   - If >1Ω, find better ground point

---

## 📏 Wire Sizing

### Why Wire Size Matters

**Undersized wire causes:**
- Voltage drop (gauge gets less than 12V)
- Wire heating (fire risk!)
- Erratic operation
- Shortened component life

### Wire Gauge Selection Table

For 12V automotive use, length from power source to gauge:

| Wire Length | Current Draw | Recommended Gauge | Voltage Drop |
|------------|--------------|-------------------|--------------|
| **0-5 feet** | 0.5A | 22 AWG | 0.08V |
| **5-10 feet** | 0.5A | 20 AWG | 0.10V |
| **10-15 feet** | 0.5A | 18 AWG | 0.09V |
| **15-20 feet** | 0.5A | 16 AWG | 0.08V |

**For JNKR Gauge System:**
- **Recommended**: 20 AWG for runs under 10 feet
- **Recommended**: 18 AWG for runs over 10 feet
- **Minimum**: 22 AWG (only for very short runs <3 feet)

### Wire Type

**✅ Use Automotive Wire:**
- Stranded copper (flexible)
- PVC or cross-linked polyethylene insulation
- Temperature rated -40°C to +105°C
- Oil and fuel resistant

**❌ Don't Use:**
- Solid core wire (breaks from vibration)
- Household electrical wire (wrong insulation)
- Speaker wire (too thin, wrong insulation)

**Where to Buy:**
- Auto parts stores (labeled "automotive primary wire")
- Marine supply stores (even better - marine grade)
- Online: TEMCo, Del City, Amazon

---

## 🛡️ Fuse Selection

### Why Two Fuses?

**You'll have TWO fuses in series:**

1. **Circuit Source Fuse** (already in fuse box)
   - Protects entire vehicle circuit
   - 10-20A typical

2. **Gauge System Fuse** (you add this)
   - Protects only your gauge
   - 3A recommended

**Why This is Good:**
- If gauge shorts, only your 3A fuse blows
- Vehicle circuit stays protected
- Original fuse stays intact
- Easy to diagnose

### Choosing Fuse Size

**Formula:**
```
Fuse Rating = (Maximum Current × 1.25) rounded up
            = (0.5A × 1.25) = 0.625A
            → Use 1A, 2A, or 3A fuse
```

**Recommendation**: **3A fuse**
- Provides safety margin
- Handles inrush current (display turn-on)
- Won't nuisance blow
- Still protects from shorts

**Fuse Types:**

| Type | Use Case | Our Choice |
|------|----------|------------|
| **Blade Fuse** | Fuse box (Add-A-Circuit) | ✅ Use this |
| **Inline Fuse** | In-line on wire | ✅ Use this |
| **Glass Tube** | Old school | ❌ Avoid (corrodes) |

### Fuse Placement

**Add inline fuse within 6 inches of tap point!**

```
Power Source ──┬── [3A Fuse] ───→ Buck Converter ───→ Gauge
               │   ↑
               │   └── Within 6" of tap point
               │
         (existing circuit continues)
```

**Why Close to Tap Point?**
- If wire shorts to ground, fuse blows immediately
- Protects the wire between tap and gauge
- Professional standard

---

## 🔧 Installation Methods

### Method A: Add-A-Circuit at Fuse Box (Recommended)

**Difficulty**: Easy  
**Time**: 15 minutes  
**Reversibility**: 100%

**Tools Needed:**
- Add-A-Circuit fuse tap
- 3A blade fuse
- Wire strippers
- Multimeter
- Electrical tape

**Full Instructions:**

**1. Locate Fuse Box**
   - Check owner's manual
   - Usually under dashboard driver side
   - Or under hood near battery

**2. Find Switched Circuit**
   
   Good candidates for switched power:
   - **Radio/Audio** (5-20A fuse)
   - **Accessory Port** (15-20A fuse)
   - **Power Windows** (20-30A fuse)
   - **Interior Lights** (10-15A fuse, but may be constant)

**3. Test Before Tapping**
   
   Use multimeter in voltage mode:
   ```
   Key OFF → Fuse terminals should read 0V
   Key ACC → Fuse terminals should read 12-14V
   Key RUN → Fuse terminals should read 12-14V
   ```

**4. Install Add-A-Circuit**
   
   ```
   Remove original fuse
   ↓
   Insert Add-A-Circuit (check orientation!)
   ↓
   Bottom slot: Original fuse (e.g., 15A)
   Top slot: Your 3A fuse
   ↓
   Connect red wire to Add-A-Circuit wire lead
   ↓
   Secure connection (crimp or solder)
   ↓
   Heat shrink or electrical tape
   ```

**5. Route Wire**
   
   - Follow existing wire harness
   - Use zip ties every 6-12 inches
   - Avoid hot surfaces (exhaust, engine)
   - Avoid moving parts (pedals, steering)
   - Through firewall grommet to engine bay (if needed)

**6. Test Installation**
   
   ```
   Key OFF → Gauge should be off
   Key ACC → Gauge should power on
   Engine Running → Gauge should operate normally
   ```

### Method B: Cigarette Lighter Plug

**Difficulty**: Very Easy  
**Time**: 5 minutes  
**Reversibility**: 100%

**Parts:**
- 12V cigarette lighter plug with wire leads
- Or: cut the end off a car charger

**Instructions:**

1. **Get a lighter plug adapter**
   - Look for "12V DC power plug with bare wires"
   - Or cut plug off old car charger
   
2. **Identify wires**:
   - Center pin = +12V (usually red)
   - Outer barrel = Ground (usually black)
   - Use multimeter to confirm
   
3. **Connect to buck converter**:
   - Red → Buck converter +12V input
   - Black → Buck converter GND input
   
4. **Plug in and test**

**Limitations:**
- Visible cable in cabin
- Takes up power outlet
- Not professional appearance
- Fine for testing!

### Method C: Direct Battery Connection (Not Recommended)

**⚠️ WARNING: This is CONSTANT power (always on)**

Only do this if:
- You're adding a manual on/off switch
- You want gauge to work with engine off
- You understand battery drain risk

**If you must:**

1. **Positive connection**:
   - Add 3A inline fuse within 6" of battery positive
   - Use ring terminal on battery terminal
   - Route wire through firewall

2. **Negative connection**:
   - Connect to chassis ground OR battery negative
   - Use ring terminal

3. **Add manual switch** (highly recommended):
   ```
   Battery (+) ──[Fuse]── [Switch] ──→ Gauge
   ```

---

## 🚗 Toyota 1KZTE Specific Guide

Since you're building this for a 1KZTE turbodiesel, here's specific guidance:

### Fuse Box Location (1KZ-TE Vehicles)

**Most Toyota trucks/SUVs with 1KZ-TE:**

1. **Main Fuse Box**: Under hood, driver side, near battery
2. **Interior Fuse Box**: Under dashboard, driver side, left of steering wheel

### Recommended Tap Points for 1KZ-TE

**Best Options (in order of preference):**

| Circuit | Fuse Location | Fuse Rating | Why It's Good | Best For |
|---------|--------------|-------------|---------------|----------|
| **1. Cigarette Lighter Wiring** | Behind dash (or center console) | 10-15A | Already wired, switched, has ground! | If you don't use lighter - EASIEST! |
| **2. Radio/Audio** | Interior fuse box | 15A | Switched, low current, easy access | Standard installation |
| **3. Cigarette Lighter Fuse** | Interior fuse box | 15A | Switched, purpose-built for accessories | Using Add-A-Circuit method |
| **4. Power Windows** | Interior fuse box | 20-30A | Switched, high capacity circuit | Alternate if radio not available |
| **5. Accessory** | Interior or engine fuse box | 10-15A | Switched, dedicated accessory power | Alternate option |

### Step-by-Step for 1KZ-TE

#### Method 1: Use Cigarette Lighter Wiring (Recommended if Available!)

**Why This is Perfect for 1KZ-TE Trucks:**
- Most 1990s-2000s Toyota trucks have cigarette lighter in center console
- It's rarely used (everyone uses USB now)
- Already switched power (confirmed on most 1KZ-TE Land Cruisers/Hilux)
- Perfect location for gauge mounting
- 2.1" round display fits nicely in lighter opening!

**Procedure:**

1. **Locate cigarette lighter** (usually center console)
2. **Remove center console trim** (4-6 screws typically)
3. **Unplug lighter from behind** (squeeze tab and pull connector)
4. **Test with multimeter**:
   - Key OFF: 0V (switched - perfect!)
   - Key ON: 12-14V
5. **Connect your gauge**:
   ```
   Lighter plug red/yellow wire → [3A inline fuse] → Buck converter 12V+
   Lighter plug black wire → Buck converter GND
   ```
6. **Options for mounting**:
   - **A**: Mount gauge IN the lighter opening (clean!)
   - **B**: Use lighter hole to route wires, install blank plate
   - **C**: Keep lighter plugged in but tap wires before connector (100% reversible)

**Result**: Cleaner than fuse box tap, fewer wires, looks factory!

#### Method 2: Fuse Box Tap (Standard Method)

**1. Access Interior Fuse Box**
   - Open driver door
   - Look under dashboard, left side
   - Remove fuse box cover (may have clips or screws)

**2. Find Radio Fuse**
   - Check fuse box cover for diagram
   - Look for "RADIO", "AUDIO", or "ACC" label
   - Usually 15A or 20A blade fuse

**3. Test Fuse**
   - Turn key to OFF
   - Touch multimeter probes to metal tabs on TOP of fuse
   - Should read 0V
   - Turn key to ACC position
   - Should now read 12-14V
   - ✅ If it switches with ignition, you found the right one!

**4. Install Add-A-Circuit**
   - Pull out radio fuse
   - Get correct size Add-A-Circuit (mini or standard blade)
   - Install per instructions above

**5. Route Wire to Gauge**

   **If gauge is in cabin (dashboard/A-pillar):**
   - Route along existing harness under dash
   - Keep away from pedals and steering
   - Use wire loom or split tubing
   - Secure with zip ties

   **If gauge is in engine bay:**
   - Route through firewall grommet
   - Look for existing harness pass-through
   - Usually behind engine on firewall
   - Use rubber grommet or seal entry point

**6. Grounding Points on 1KZ-TE**

   Good ground locations:
   - **Chassis ground stud** (near fuse box, under dash)
   - **Dashboard support bracket bolt** (metal frame behind dash)
   - **Firewall ground bolt** (if gauge in engine bay)
   - **Engine block ground** (if gauge in engine bay, but noisier)

### Power Requirements for I2C Sensor Module

**If using the I2C sensor module in engine bay:**

The module needs power from the same buck converter:

```
Vehicle 12V (switched)
    ↓
[3A Fuse]
    ↓
Buck Converter (in cabin with gauge)
    ↓
5V Output ──┬──→ Qualia ESP32-S3
            ├──→ Display
            └──→ I2C Module in engine bay (via harness)
```

**Alternatively (cleaner for I2C setup):**

```
Vehicle 12V (switched) ──[3A Fuse]── Buck Converter ──→ 5V to I2C module
                                                     |
                                                     └──→ Single 4-wire harness to cabin
                                                          (5V, GND, SCL, SDA)
```

This way the buck converter is in the engine bay with the sensor module, and you only run:
- Low-voltage 5V power (safer)
- Digital I2C signals (noise immune)
- Single small 4-wire harness through firewall

---

## 🔍 Testing Your Power Installation

### Pre-Installation Tests

**Before connecting gauge:**

1. **Verify voltage at tap point**:
   - Key OFF: 0V
   - Key ACC: 12-14V
   - Engine running: 13.8-14.4V

2. **Verify ground**:
   - Continuity from ground point to battery negative
   - Should be 0Ω or very close

3. **Test under load**:
   - Connect test load (LED + resistor, or multimeter in current mode)
   - Verify circuit can supply 0.5-1A without voltage drop

### Post-Installation Tests

**After connecting gauge:**

1. **Voltage at buck converter input**:
   - Should match vehicle voltage (12-14V)
   - No significant drop under load

2. **Voltage at buck converter output**:
   - Should be steady 5.0V
   - No ripple or fluctuation

3. **Voltage at ESP32 5V pin**:
   - Should be 5.0V ± 0.1V
   - No drop when display turns on

4. **Current draw measurement**:
   - Total system: 0.4-0.8A @ 5V
   - From vehicle 12V: 0.17-0.35A
   - Within expected range

### Functional Tests

1. **Key OFF**: Gauge should be completely off
2. **Key ACC**: Gauge should power on and initialize
3. **Engine Running**: Gauge should operate normally
4. **Long Term**: Leave gauge on for 30 minutes, check for:
   - No hot wires
   - No burning smell
   - Stable operation
   - No voltage drop

---

## ⚠️ Troubleshooting

### Gauge Doesn't Turn On

**Possible Causes:**

| Symptom | Diagnosis | Fix |
|---------|-----------|-----|
| No power at buck converter input | Check fuse, check tap point voltage | Replace fuse, reconnect wire |
| Power at input, none at output | Buck converter failed or not configured | Check/replace buck converter |
| 5V present but gauge doesn't start | Wiring issue or component fault | Check all connections |

**Diagnostic Steps:**

1. Turn key to ACC
2. Check voltage at fuse tap point (should be 12V)
3. Check voltage at buck converter input (should be 12V)
4. Check voltage at buck converter output (should be 5V)
5. Check voltage at ESP32 5V pin (should be 5V)
6. Check ground continuity (should be 0Ω)

### Gauge Turns On But Resets/Flickers

**Likely Causes:**
- Poor ground connection
- Voltage drop under load
- Wire too thin or too long
- Buck converter undersized

**Fixes:**
1. Improve ground connection (clean, tighten)
2. Use heavier gauge wire (go from 22AWG to 18AWG)
3. Shorten wire run if possible
4. Upgrade to higher capacity buck converter (3A → 5A)

### Gauge Works But Battery Dies Overnight

**Cause**: Connected to constant power instead of switched!

**Fix**:
1. Verify tap point is truly switched (test with multimeter)
2. Move to different circuit if needed
3. Or add manual on/off switch

### Fuse Keeps Blowing

**Possible Causes:**

| Fuse Blows | Likely Cause | Fix |
|-----------|--------------|-----|
| Immediately when installed | Short circuit in wiring | Check all connections for shorts |
| When gauge turns on | Inrush current too high | Use larger fuse (5A instead of 3A) |
| After some minutes | Component failure or overheating | Check buck converter and all components |
| Randomly | Loose connection or intermittent short | Secure all connections, inspect wiring |

---

## 📋 Installation Checklist

Use this checklist during your installation:

### Planning Phase
- [ ] Decided on power source (fuse box recommended)
- [ ] Located fuse box(es) in vehicle
- [ ] Identified good ground point near gauge location
- [ ] Measured wire run length
- [ ] Selected appropriate wire gauge (20 AWG minimum)
- [ ] Purchased Add-A-Circuit fuse tap
- [ ] Have 3A blade fuse
- [ ] Have inline fuse holder

### Preparation Phase
- [ ] Read vehicle owner's manual fuse diagram
- [ ] Battery disconnected (negative terminal)
- [ ] Identified switched circuit to tap
- [ ] Tested circuit with multimeter (0V off, 12V on)
- [ ] Prepared ground location (cleaned, sanded bare metal)

### Installation Phase
- [ ] Installed Add-A-Circuit with 3A fuse
- [ ] Connected red wire to Add-A-Circuit
- [ ] Added inline fuse within 6" of tap point
- [ ] Routed wire along existing harness
- [ ] Secured wire with zip ties every 6-12"
- [ ] Connected red wire to buck converter +12V input
- [ ] Installed ring terminal on black wire
- [ ] Bolted black wire to clean ground point
- [ ] Connected black wire to buck converter GND input
- [ ] Checked all connections are tight
- [ ] No bare wire exposed

### Testing Phase
- [ ] Visual inspection (no shorts, no loose wires)
- [ ] Multimeter test: 12V at buck input (key ON)
- [ ] Multimeter test: 5V at buck output
- [ ] Reconnected battery negative terminal
- [ ] Key OFF: Gauge is off
- [ ] Key ACC: Gauge powers on
- [ ] Gauge operates normally
- [ ] No hot wires after 30 min operation
- [ ] No flickering or resets

### Documentation Phase
- [ ] Took photos of installation
- [ ] Noted which fuse was tapped
- [ ] Labeled wires (helps future troubleshooting)
- [ ] Updated vehicle maintenance records
- [ ] Kept receipt for Add-A-Circuit and fuses

---

## 💡 Pro Tips

1. **Take Photos**: Before and after every connection. Helps with troubleshooting later.

2. **Label Everything**: Use a label maker or masking tape to identify:
   - Which fuse you tapped
   - Wire polarity (+ and -)
   - Wire destinations

3. **Keep Receipts**: Save packaging from Add-A-Circuit and fuses. Makes returns easier.

4. **Test Twice, Install Once**: Always verify circuit is switched before connecting gauge.

5. **Leave Slack**: Don't pull wires tight. Leave a little extra length for vibration and movement.

6. **Use Quality Connectors**: Cheap crimp connectors fail. Use quality marine-grade or automotive connectors.

7. **Protect Wires at Firewall**: Use rubber grommets or edge protectors where wires pass through metal.

8. **Add a Service Loop**: Leave a coil of extra wire near gauge. Makes future service easier.

9. **Future-Proof**: Run an extra unused wire while you're at it. Might need it later!

10. **Document Your Work**: Write down everything you did. You'll forget in 6 months!

---

## 🎓 Understanding Buck Converters

Since you'll be using a buck converter to step down 12V to 5V, here's what you need to know:

### What is a Buck Converter?

A buck converter efficiently reduces voltage:
- Input: 12V from vehicle
- Output: 5V for gauge components
- Efficiency: ~85-95%

### Buck Converter Specifications

**Minimum Requirements:**
- Input: 7-30V DC (covers vehicle range)
- Output: 5V DC, adjustable ± 0.1V
- Current: 3A continuous minimum (5A better)
- Efficiency: >85%
- Protection: Over-current, over-temp, short circuit

**Recommended Models:**
- LM2596 module (cheap, reliable)
- XL4015 module (higher power)
- Pololu D24V10F5 (very efficient, compact)

### Setting Output Voltage

**⚠️ CRITICAL: Set voltage BEFORE connecting to gauge!**

**Procedure:**

1. **Wire buck converter to 12V power supply**:
   - Red wire to +12V
   - Black wire to GND
   - Don't connect output yet!

2. **Connect multimeter to OUTPUT terminals**:
   - Red probe to + output
   - Black probe to - output

3. **Power on 12V supply**

4. **Adjust output voltage**:
   - Find small potentiometer (screw) on module
   - Turn clockwise to increase voltage
   - Turn counter-clockwise to decrease
   - Set to exactly 5.00V

5. **Power off, disconnect multimeter**

6. **Now safe to connect to gauge**

---

## 📞 Need Help?

If you're stuck or unsure:

1. **Take photos** of your fuse box and setup
2. **Measure voltages** at key points with multimeter
3. **Open an Issue** on GitHub with:
   - Vehicle year/make/model
   - Which circuit you're trying to tap
   - Voltage measurements
   - Photos of installation

We'll help you figure it out!

---

## ✅ Summary

**For the JNKR Gauge System, you have two excellent options:**

### Option A: Fuse Box Tap (Most Common)

1. **Power Source**: Interior fuse box, radio circuit, using Add-A-Circuit
2. **Fuse**: 3A blade fuse in Add-A-Circuit
3. **Wire**: 20 AWG automotive wire
4. **Ground**: Chassis ground bolt near fuse box
5. **Buck Converter**: 3A rated, set to 5.0V output
6. **Result**: Gauge turns on with key, turns off when you remove key, zero battery drain

**Total Time**: 30 minutes  
**Total Cost**: $10-20 for Add-A-Circuit, fuse, and wire  
**Skill Level**: Basic (if you can use a multimeter, you can do this)

### Option B: Cigarette Lighter Wiring (If You Don't Use It) ⭐

1. **Power Source**: Existing cigarette lighter wiring (already switched and fused!)
2. **Fuse**: Add 3A inline fuse for gauge protection
3. **Wire**: May not need any - direct connection possible
4. **Ground**: Use lighter's ground wire
5. **Mounting**: Can mount gauge IN the lighter location, or use hole for wiring
6. **Result**: Ultra-clean installation, looks factory, 100% reversible

**Total Time**: 20-30 minutes  
**Total Cost**: $5-10 for inline fuse and connectors  
**Skill Level**: Basic (even easier than fuse box tap!)  
**Why It's Great**: Perfect for older 1KZTE trucks where lighter isn't used, gauge fits perfectly in lighter opening

---

**Now you have complete clarity on how to power your gauge system!**

Ready to proceed with installation? Check out:
- [WIRING.md](WIRING.md) - Sensor and component wiring
- [ASSEMBLY.md](ASSEMBLY.md) - Full assembly guide

---

**Last Updated:** 2025-11-08
