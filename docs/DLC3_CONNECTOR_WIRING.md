# DLC3 Connector Wiring Guide

## Physical Connection to DLC3 (21-pin OP3)

### The Challenge

The DLC3 connector (21-pin OP3) is a **Toyota proprietary connector** under the dash. You need to connect to it, but it's not a standard connector you can buy off-the-shelf.

---

## Connection Options

### Option 1: Backprobe/Test Pins (Easiest for Testing)

**What:** Use test pins or backprobe connectors to access the DLC3 pins without unplugging anything.

**Pros:**
- ✅ No modification to vehicle
- ✅ Easy to test
- ✅ Can remove easily

**Cons:**
- ⚠️ Not permanent
- ⚠️ May be loose
- ⚠️ Not weatherproof

**How:**
- Use backprobe pins (like T-pins or test leads)
- Insert into back of DLC3 connector
- Connect to your K-Line module
- Secure with zip ties

---

### Option 2: Make Adapter Cable (Recommended for Permanent)

**What:** Create a "Y-cable" adapter that plugs into DLC3 and provides a breakout connector.

**Pros:**
- ✅ Permanent installation
- ✅ Professional appearance
- ✅ Can unplug easily
- ✅ Doesn't modify vehicle wiring

**Cons:**
- ⚠️ Need to source DLC3 connector (may be hard to find)
- ⚠️ More complex to build

**How:**
1. Find/source DLC3 connector (male, to plug into vehicle)
2. Wire to breakout connector (Deutsch, Molex, etc.)
3. Create adapter cable
4. Plug into DLC3, connect your module to breakout

---

### Option 3: Tap into Wires (If Connector Not Available)

**What:** Tap into the wires going to DLC3 connector (before the connector).

**Pros:**
- ✅ Works if you can't find connector
- ✅ Permanent installation

**Cons:**
- ⚠️ Requires cutting/tapping wires
- ⚠️ More invasive
- ⚠️ Harder to remove later

**How:**
1. Locate DLC3 connector
2. Identify wires (K-Line, +12V, GND)
3. Tap into wires with scotchlok or T-tap connectors
4. Run wires to your K-Line module

---

## Required Pins

### Minimum Pins Needed

For K-Line communication, you only need **3 pins**:

| Pin | Function | Wire Color (Typical) | What It Does |
|-----|----------|---------------------|--------------|
| **1** | BATT | Red/Brown | +12V power |
| **2** | GND | Black/Brown | Ground |
| **3** | K-Line | Yellow/Green | Diagnostic data line |

**That's it!** You don't need all 21 pins, just these 3.

---

## Finding the DLC3 Connector

### Location

- **Under dash** (driver or passenger side)
- **Near ECU** (often close to ECU location)
- **May be behind kick panel** or under center console

### Physical Appearance

- **21 pins** in a rectangular connector
- **Toyota proprietary** shape (not standard)
- **May have a cover** or be exposed
- **Usually gray or black** plastic

### How to Identify

1. **Count pins:** Should be 21 pins
2. **Check location:** Under dash, near ECU
3. **Look for markings:** May say "OP3" or "CHECK CONNECTOR"
4. **Service manual:** Check wiring diagram for exact location

---

## Connection Methods

### Method 1: Backprobe Pins (Quick Test)

**Tools needed:**
- Backprobe pins or T-pins
- Multimeter
- Wire (24 AWG)
- Heat shrink

**Steps:**
1. **Locate DLC3 connector** under dash
2. **Identify pins 1, 2, 3** with multimeter:
   - Pin 1: Should be +12V (with ignition on)
   - Pin 2: Should be 0V (ground)
   - Pin 3: K-Line (will show activity with oscilloscope)
3. **Insert backprobe pins** into connector back
4. **Connect wires:**
   - Pin 1 → +12V input of your module
   - Pin 2 → GND of your module
   - Pin 3 → K-Line input of MC33290
5. **Secure with zip ties** or tape

**Pros:** Quick, easy, non-invasive  
**Cons:** Not permanent, may be loose

---

### Method 2: Adapter Cable (Professional)

**Components needed:**
- DLC3 male connector (to plug into vehicle) - **Hard to find!**
- Breakout connector (Deutsch DT, Molex, etc.)
- Wire (20-24 AWG, automotive grade)
- Heat shrink, wire loom

**Steps:**
1. **Source DLC3 connector:**
   - Check Toyota parts suppliers
   - Check junkyards (cut from wrecked vehicle)
   - Check online (eBay, etc.)
   - May need to use part number from service manual

2. **Wire adapter:**
   ```
   DLC3 Connector (Vehicle Side)
   ├── Pin 1 (BATT) ──► Wire 1 ──► Breakout Pin 1
   ├── Pin 2 (GND)  ──► Wire 2 ──► Breakout Pin 2
   └── Pin 3 (K-Line) ──► Wire 3 ──► Breakout Pin 3
   ```

3. **Create breakout:**
   - Use Deutsch DT connector (recommended)
   - Or Molex connector
   - Or simple screw terminal block

4. **Assemble:**
   - Crimp terminals to DLC3 connector
   - Run wires through loom
   - Terminate at breakout connector
   - Heat shrink all connections

**Pros:** Professional, permanent, easy to disconnect  
**Cons:** Need to source DLC3 connector (may be difficult)

---

### Method 3: Wire Tap (If Connector Not Available)

**Tools needed:**
- Multimeter
- T-tap or scotchlok connectors
- Wire (20-24 AWG)
- Wire stripper

**Steps:**
1. **Locate DLC3 connector** under dash
2. **Identify wires** going to connector:
   - Use multimeter to find +12V wire
   - Find ground wire
   - Find K-Line wire (may need to check with service manual)
3. **Tap into wires:**
   - Use T-tap connectors (non-invasive)
   - Or scotchlok connectors
   - **Don't cut wires!** Just tap into them
4. **Run wires to module:**
   - Route wires to your K-Line module location
   - Use wire loom for protection
   - Secure with zip ties

**Pros:** Works if connector unavailable  
**Cons:** More invasive, harder to remove

---

## Recommended Approach

### For Testing: Backprobe Pins

**Start here:**
- Quick and easy
- No permanent changes
- Test if DLC3 works
- Verify K-Line communication

### For Permanent: Adapter Cable

**If you can find DLC3 connector:**
- Professional installation
- Easy to disconnect
- No wire tapping
- Clean appearance

### If Connector Not Available: Wire Tap

**Last resort:**
- Tap into wires before connector
- Use T-tap connectors (non-invasive)
- Can remove later if needed

---

## DLC3 Connector Part Number

### Finding the Part Number

**Check:**
- 1996 Land Cruiser Prado service manual
- Toyota parts catalog
- Wiring diagram (should show connector part number)
- Online parts databases

**Typical format:**
- Toyota connector part numbers: `90980-#####`
- May be listed as "OP3" or "CHECK CONNECTOR"

**Where to buy:**
- Toyota dealership parts department
- Online Toyota parts suppliers
- Junkyards (cut from wrecked vehicle)
- eBay (used connectors)

---

## Breakout Connector Options

### Option 1: Deutsch DT (Recommended)

**Why:**
- ✅ Automotive-grade
- ✅ Weatherproof
- ✅ Professional appearance
- ✅ Easy to disconnect

**Part number:** Deutsch DT06-3S (3-pin, male)
**Mating:** Deutsch DT06-3P (3-pin, female)

**Cost:** ~$10-15 per pair

---

### Option 2: Molex Connector

**Why:**
- ✅ Common and available
- ✅ Easy to crimp
- ✅ Inexpensive

**Part number:** Molex 2.54mm pitch (standard)
**Cost:** ~$2-5 per pair

---

### Option 3: Screw Terminal Block

**Why:**
- ✅ Simplest
- ✅ No special tools
- ✅ Easy to connect/disconnect

**Part number:** 3-position terminal block
**Cost:** ~$2-5

---

## Wiring Diagram

### Complete Connection

```
┌─────────────────────┐
│  DLC3 Connector     │
│  (21-pin OP3)       │
│  Under Dash         │
│                     │
│  Pin 1 (BATT +12V)  │──┐
│  Pin 2 (GND)        │──┤
│  Pin 3 (K-Line)     │──┤
└─────────────────────┘  │
                          │
                          │ Adapter Cable
                          │ (3 wires)
                          │
┌─────────────────────────▼──────────────┐
│  Breakout Connector                    │
│  (Deutsch DT, Molex, or Terminal)      │
│                                        │
│  Pin 1: +12V                           │
│  Pin 2: GND                            │
│  Pin 3: K-Line                         │
└────────────┬───────────────────────────┘
             │
             │ Short Cable
             │
┌────────────▼───────────────────────────┐
│  K-Line Module                         │
│  (Small ESP32 + MC33290)              │
│                                        │
│  +12V ──► MC33290 Pin 3 (VCC)         │
│  GND  ──► MC33290 Pin 2 (GND)         │
│  K-Line ──► MC33290 Pin 1 (K-Line IN) │
└────────────────────────────────────────┘
```

---

## Step-by-Step: Backprobe Method (Easiest to Start)

### Step 1: Locate DLC3 (15 minutes)

1. **Check under dash:**
   - Driver side (near steering column)
   - Passenger side (near glove box)
   - Center console area

2. **Look for:**
   - 21-pin connector
   - May have cover
   - Usually gray/black

3. **Verify:**
   - Count pins (should be 21)
   - Check with multimeter (Pin 1 = +12V with ignition on)

### Step 2: Identify Pins (10 minutes)

1. **With multimeter:**
   - Pin 1: Measure voltage (should be +12V with ignition on)
   - Pin 2: Measure to ground (should be 0V, continuity to chassis)
   - Pin 3: K-Line (may show voltage, but hard to identify without scope)

2. **Reference service manual:**
   - Check wiring diagram
   - Verify pin assignments
   - Confirm K-Line pin

### Step 3: Connect Backprobe Pins (10 minutes)

1. **Insert backprobe pins:**
   - Pin 1: Insert backprobe, connect red wire
   - Pin 2: Insert backprobe, connect black wire
   - Pin 3: Insert backprobe, connect yellow/green wire

2. **Secure:**
   - Use zip ties to secure wires
   - Use heat shrink on connections
   - Route wires carefully

### Step 4: Connect to Module (5 minutes)

1. **Wire to MC33290:**
   - Pin 1 wire → MC33290 VCC
   - Pin 2 wire → MC33290 GND
   - Pin 3 wire → MC33290 K-Line IN

2. **Test:**
   - Power on module
   - Check Serial Monitor
   - Should see K-Line initialization

**Total time: ~40 minutes for first-time setup**

---

## Troubleshooting

### Problem: Can't Find DLC3 Connector

**Solutions:**
- Check service manual for exact location
- Look for "CHECK CONNECTOR" or "OP3" in diagrams
- May be behind kick panel or under center console
- May have a cover that needs removal

### Problem: Can't Identify Pins

**Solutions:**
- Use service manual wiring diagram
- Check ECU_PINOUT_1KZTE.md (mentions OP3)
- Use multimeter to find +12V and GND
- K-Line pin may need oscilloscope to identify

### Problem: Can't Source DLC3 Connector

**Solutions:**
- Use backprobe method (easiest)
- Use wire tap method (if connector unavailable)
- Check junkyards for wrecked vehicles
- Check online parts suppliers

### Problem: Connection Loose

**Solutions:**
- Use better backprobe pins
- Secure with zip ties
- Consider adapter cable for permanent install
- Use connector with locking mechanism

---

## Recommended Parts List

### For Backprobe Method

| Item | Quantity | Cost |
|------|----------|------|
| Backprobe pins | 3 | $5-10 |
| Wire (24 AWG, automotive) | 3 ft | $2-3 |
| Heat shrink | 6 pieces | $2 |
| Zip ties | 10 | $1 |
| **Total** | | **$10-16** |

### For Adapter Cable Method

| Item | Quantity | Cost |
|------|----------|------|
| DLC3 connector (if available) | 1 | $10-50 |
| Deutsch DT 3-pin connector | 1 pair | $10-15 |
| Wire (20 AWG, automotive) | 5 ft | $3-5 |
| Heat shrink | 10 pieces | $2 |
| Wire loom | 3 ft | $2 |
| **Total** | | **$27-74** |

---

## Next Steps

1. **Locate DLC3 connector** under dash
2. **Identify pins** with multimeter
3. **Start with backprobe method** (easiest, non-invasive)
4. **Test K-Line communication**
5. **If working, consider adapter cable** for permanent install

---

**Document Created:** 2025-01-27  
**Status:** Connection guide ready  
**Recommendation:** Start with backprobe method for testing
