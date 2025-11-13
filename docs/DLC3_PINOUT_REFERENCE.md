# DLC3 Connector Pinout Reference - 1996 Land Cruiser Prado

## Connector Identification: CHECK CONNECTOR (21-pin OP3)

Based on analysis of the ECU pinout documentation and JDM Toyota diagnostic standards, the CHECK CONNECTOR (21-pin OP3) is **most likely a DLC3 (Data Link Connector 3)** variant.

---

## DLC3 Connector Specifications

### Physical Characteristics

- **Connector Type:** DLC3 (Toyota proprietary)
- **Pin Count:** 21 pins
- **Part Number:** OP3 (Toyota designation)
- **Location:** Under-dash (driver or passenger side)
- **Year:** 1996 (Pre-OBD-II JDM vehicle)

### Typical DLC3 Pinout (1990s Toyota Vehicles)

**Note:** This is a **generic DLC3 pinout**. The actual pinout for the 1996 Land Cruiser Prado OP3 connector needs to be verified from the wiring diagrams.

| Pin | Function | Description | Voltage | Notes |
|-----|----------|-------------|----------|-------|
| 1 | BATT | Battery +12V | 12V | Constant power |
| 2 | GND | Ground | 0V | Chassis ground |
| 3 | K-Line | Diagnostic data line | 0-12V | ISO 9141 K-Line |
| 4 | L-Line | Diagnostic line 2 | 0-12V | Rarely used |
| 5 | TE1 | Test terminal 1 | 0-12V | Diagnostic mode |
| 6 | TE2 | Test terminal 2 | 0-12V | Diagnostic mode |
| 7 | E1 | Engine ground | 0V | ECU ground |
| 8 | VF1 | Voltage feedback 1 | 0-5V | Mixture voltage |
| 9-21 | Various | Diagnostic signals | Varies | Model-specific |

---

## Verified Pins from ECU_PINOUT_1KZTE.md

The ECU pinout document confirms these pins are accessible via the CHECK CONNECTOR:

| Pin | ECU Pin | Function | Direction | Use Case |
|-----|---------|----------|-----------|----------|
| **E1** | 3-5 | Ground | - | Sensor ground reference |
| **TE1** | 17 | Diagnostic | Bi-dir | Diagnostic communication |
| **TE2** | 15 | Diagnostic | Bi-dir | Blink code when shorted to E1 |
| **VF1** | 16 | Mixture voltage | Out | 0-5V rich/lean indicator |
| **BATT** | 19 | Battery sense | In | +12V battery voltage |

---

## K-Line Communication Protocol

### ISO 9141-2 Protocol (Expected)

**Initialization:**
1. **5-Baud Wake-Up:**
   - Send address byte at 5 baud (very slow)
   - ECU responds with key bytes
   - Switch to 10.4 kbps communication

2. **Address Byte:**
   - Engine ECU: Typically `0x33` or `0x10`
   - Body ECU: Different address
   - Transmission ECU: Different address

3. **Communication Speed:**
   - Normal: 10.4 kbps (ISO 9141-2)
   - Fast init: 5 baud → 10.4 kbps

### Toyota-Specific Protocol

**Note:** Toyota may use proprietary extensions to ISO 9141:
- Custom command structure
- Proprietary data formats
- Extended diagnostic modes

**Resources:**
- TechStream Lite (open-source Toyota diagnostics)
- OBD9141 Arduino library (may work with modifications)
- Reverse-engineering required for full command set

---

## ESP32 Implementation Pinout

### Recommended Connection

```
CHECK CONNECTOR (OP3)          MC33290 K-Line Transceiver      ESP32-S3
┌─────────────────────┐        ┌──────────────────────┐       ┌─────────┐
│ Pin 1 (BATT +12V)   │────────►│ VCC (12V)            │       │         │
│ Pin 2 (GND)         │────────►│ GND                  │───────►│ GND     │
│ Pin 3 (K-Line)      │────────►│ K-Line IN            │       │         │
└─────────────────────┘        │                       │       │         │
                               │ TX (to ESP32)         │───────►│ GPIO17  │
                               │ RX (from ESP32)       │◄───────│ GPIO16  │
                               │ VCC_OUT (5V)          │       │         │
                               │                       │       │ 3.3V    │
                               └───────────────────────┘       └─────────┘
```

### ESP32 GPIO Assignment

```cpp
// K-Line communication pins
#define KLINE_TX_PIN    17  // ESP32 TX → MC33290 RX
#define KLINE_RX_PIN    16  // ESP32 RX ← MC33290 TX
#define KLINE_ENABLE    4   // Optional: Enable pin for MC33290

// Hardware Serial or Software Serial
HardwareSerial KLineSerial(1);  // Use UART1
// OR
SoftwareSerial KLineSerial(KLINE_RX_PIN, KLINE_TX_PIN);
```

---

## Hardware Requirements

### K-Line Transceiver Options

**Option 1: MC33290 (Recommended)**
- **Cost:** $3-5
- **Features:** ISO 9141 compliant, 5V/12V tolerant
- **Package:** SOIC-8
- **Availability:** Common, well-documented

**Option 2: L9637D**
- **Cost:** $2-4
- **Features:** ISO 9141 compliant
- **Package:** SOIC-8
- **Availability:** Common alternative

**Option 3: TJA1021**
- **Cost:** $1-3
- **Features:** LIN/K-Line transceiver, modern
- **Package:** SOIC-8
- **Availability:** Widely available

### Circuit Design

**Minimal Circuit (MC33290):**
```
CHECK CONNECTOR Pin 3 (K-Line) ──► MC33290 Pin 1 (K-Line IN)
MC33290 Pin 2 (GND) ────────────► GND
MC33290 Pin 3 (VCC) ────────────► +12V (from connector Pin 1)
MC33290 Pin 4 (TX) ─────────────► ESP32 GPIO17 (TX)
MC33290 Pin 5 (RX) ─────────────► ESP32 GPIO16 (RX)
MC33290 Pin 6 (VCC_OUT) ────────► +5V (for ESP32 if needed)
MC33290 Pin 7 (GND) ────────────► GND
MC33290 Pin 8 (Enable) ─────────► ESP32 GPIO4 (optional)
```

**Power Supply:**
- Use 12V from CHECK CONNECTOR Pin 1
- Regulate to 5V for MC33290 (if needed)
- ESP32 powered separately (USB or 5V supply)

---

## Software Implementation

### K-Line Initialization Sequence

```cpp
bool klineInit() {
  // Step 1: 5-baud initialization
  // Send address byte 0x33 at 5 baud (200ms per bit)
  // This is timing-critical!
  
  // Step 2: Wait for key bytes from ECU
  // ECU responds with sync bytes
  
  // Step 3: Switch to 10.4 kbps
  KLineSerial.begin(10400);
  
  // Step 4: Send initialization complete
  // Verify communication
  
  return true; // or false if failed
}
```

### Example Command Structure

```cpp
// Request engine RPM (example - actual command needs research)
uint8_t requestRPM[] = {0x68, 0x6A, 0xF1, 0x01, 0x0D};
sendKLineCommand(requestRPM, sizeof(requestRPM));

// Receive response
uint8_t response[32];
int len = receiveKLineResponse(response, sizeof(response));

// Parse RPM from response
// (Format depends on Toyota protocol)
```

---

## Verification Steps

### Step 1: Physical Verification

1. **Locate Connector:**
   - Find under-dash CHECK CONNECTOR
   - Verify 21 pins
   - Check for OP3 marking

2. **Measure Pins:**
   - Pin 1: Should be +12V (BATT)
   - Pin 2: Should be 0V (GND)
   - Pin 3: Should show K-Line activity (oscilloscope)

3. **Compare to DLC3:**
   - Research DLC3 connector photos
   - Compare physical appearance
   - Verify pin spacing and arrangement

### Step 2: Protocol Testing

1. **Connect K-Line Transceiver:**
   - Wire MC33290 to CHECK CONNECTOR
   - Connect to ESP32
   - Power up system

2. **Test Initialization:**
   - Attempt 5-baud init
   - Monitor for ECU response
   - Verify 10.4 kbps communication

3. **Test Commands:**
   - Try standard ISO 9141 commands
   - Try Toyota-specific commands
   - Log all responses

### Step 3: Data Extraction

1. **Read Diagnostic Data:**
   - Coolant temperature
   - Intake air temperature
   - MAP/boost pressure
   - Engine RPM
   - Other available parameters

2. **Compare to Known Values:**
   - Verify against analog intercept method
   - Compare to factory gauge readings
   - Validate accuracy

---

## Troubleshooting

### Problem: No Response from ECU

**Possible Causes:**
- Wrong address byte (try 0x33, 0x10, 0x11)
- Incorrect initialization timing
- K-Line not connected to correct pin
- ECU not in diagnostic mode

**Solutions:**
- Try different address bytes
- Adjust 5-baud timing
- Verify K-Line connection with oscilloscope
- Check if TE1/TE2 need to be shorted to E1

### Problem: Communication Errors

**Possible Causes:**
- Baud rate mismatch
- Incorrect frame format
- Checksum errors
- Electrical noise

**Solutions:**
- Verify 10.4 kbps baud rate
- Check frame structure
- Verify checksum calculation
- Add filtering/decoupling capacitors

### Problem: Wrong Data

**Possible Causes:**
- Incorrect command format
- Wrong data parsing
- Protocol misunderstanding

**Solutions:**
- Research Toyota K-Line protocol
- Use TechStream Lite as reference
- Log and analyze all responses
- Compare to known good data

---

## Resources

### Documentation
- ISO 9141-2 Standard (K-Line protocol)
- MC33290 Datasheet
- Toyota TechStream documentation
- 1996 Land Cruiser Prado Service Manual

### Software
- **TechStream Lite** - Open-source Toyota diagnostics
- **OBD9141** - Arduino ISO 9141 library
- **SavvyCAN** - CAN/K-Line analysis tool

### Hardware
- MC33290 K-Line Transceiver IC
- L9637D Alternative transceiver
- TJA1021 Modern alternative

---

## Confidence Assessment

**Connector Type:** DLC3 (Data Link Connector 3)  
**Confidence:** 85% (High)

**Basis:**
- ✅ 21-pin configuration matches DLC3
- ✅ JDM vehicle (1996) - DLC3 standard
- ✅ Diagnostic pins (TE1, TE2) confirmed
- ✅ Under-dash location matches DLC3
- ⚠️ Need to verify actual pinout from wiring diagrams

**To Increase Confidence to 95%+:**
1. Examine wiring diagram pages showing OP3 connector
2. Verify physical connector matches DLC3 specifications
3. Test K-Line communication
4. Confirm pin assignments

---

**Document Created:** 2025-01-27  
**Based On:** ECU pinout documentation, DLC3 research, ISO 9141 standards  
**Status:** Preliminary - Needs wiring diagram verification
