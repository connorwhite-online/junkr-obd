# OEM ECU Sensor Data Access for 1KZ-TE

**Reading Factory Sensors via ECU Instead of Installing Separate Sensors**

This guide explores methods to read engine sensor data directly from your 1KZ-TE ECU rather than installing separate aftermarket sensors. The ECU already monitors water temperature, boost pressure (MAP), and intake air temperature - we just need to tap into that data stream.

---

## 📋 Table of Contents

- [Why Read from the ECU?](#why-read-from-the-ecu)
- [1KZ-TE ECU Overview](#1kz-te-ecu-overview)
- [Method 1: OBD-II Protocol Access](#method-1-obd-ii-protocol-access)
- [Method 2: CAN Bus Direct Access](#method-2-can-bus-direct-access)
- [Method 3: Sensor Signal Intercept (Unichip Style)](#method-3-sensor-signal-intercept-unichip-style)
- [Method 4: K-Line / ISO 9141 Diagnostics](#method-4-k-line--iso-9141-diagnostics)
- [Implementation with ESP32-S3](#implementation-with-esp32-s3)
- [What About EGT?](#what-about-egt)
- [Comparison Table](#comparison-table)
- [Recommended Approach](#recommended-approach)

---

## 🎯 Why Read from the ECU?

### Advantages

✅ **Fewer sensors to install**
- No separate coolant temp sensor
- No separate intake air temp sensors
- No separate MAP/boost sensor
- Only EGT requires dedicated sensor

✅ **Factory-calibrated readings**
- OEM sensors are precisely calibrated
- No need to calibrate thermistors
- Professional-grade accuracy

✅ **Simplified wiring**
- Single connection to ECU/OBD port
- No sensor wiring through firewall
- Cleaner engine bay

✅ **Lower cost** (potentially)
- Eliminates $100-200 in sensors
- May need $10-50 in communication hardware

### Disadvantages

❌ **ECU dependency**
- If ECU fails, gauge system fails
- Can't monitor if ECU is disconnected

❌ **Limited data refresh rate**
- OBD-II typically 10-20Hz (vs 100Hz with direct sensors)
- CAN bus faster but still limited

❌ **Not all parameters available**
- EGT not available from ECU (requires separate sensor)
- Some parameters may not be broadcast
- Pre/post intercooler temps may not be separate

❌ **Protocol complexity**
- Requires understanding of CAN/OBD protocols
- More complex software
- Debugging harder than analog sensors

---

## 🚗 1KZ-TE ECU Overview

### What is the 1KZ-TE?

The **1KZ-TE** is Toyota's 3.0L turbocharged diesel engine used in:
- Land Cruiser 70, 80, 90, 100 series (1993-2007)
- Hilux / 4Runner (1993-2004)
- Prado (various years)

### ECU Types by Year/Region

| Year | Region | ECU Type | Diagnostic Protocol | Notes |
|------|--------|----------|---------------------|-------|
| **1993-1995** | Most | Denso 89661-60xxx | K-Line (ISO 9141) | Pre-OBD-II |
| **1996-1999** | USA/Canada | Denso 89661-60xxx | OBD-II (ISO 9141-2) | Limited OBD-II compliance |
| **1996-2004** | Australia/Europe | Denso 89661-60xxx | K-Line (ISO 9141) | Toyota proprietary |
| **2000-2007** | Japan/Asia | Denso 89661-60xxx | CAN Bus (partial) | Later models only |

**Critical**: Most 1KZ-TE vehicles do NOT have full OBD-II or CAN bus! They use Toyota's proprietary K-Line protocol.

### OEM Sensors Available in 1KZ-TE ECU

| Sensor | ECU Input | Typical Range | Available via OBD? |
|--------|-----------|---------------|-------------------|
| **Coolant Temp (THW)** | Analog (NTC) | -40°C to 120°C | ✅ Yes (PID $05) |
| **Intake Air Temp (THA)** | Analog (NTC) | -40°C to 80°C | ✅ Yes (PID $0F) |
| **MAP / Boost Pressure** | Analog (0-5V) | 0-250 kPa | ✅ Yes (PID $0B) |
| **Engine RPM** | Digital | 0-5000 RPM | ✅ Yes (PID $0C) |
| **Throttle Position** | Analog (0-5V) | 0-100% | ✅ Yes (PID $11) |
| **Vehicle Speed** | Digital | 0-255 km/h | ✅ Yes (PID $0D) |
| **EGT** | **NOT AVAILABLE** | N/A | ❌ No - requires separate sensor |

**Important**: The 1KZ-TE ECU does **NOT** have an EGT sensor input. You will still need a separate K-type thermocouple + MAX31855/MCP9600 for exhaust temperature monitoring.

---

## 🔌 Method 1: OBD-II Protocol Access

### Overview

Read standardized OBD-II PIDs (Parameter IDs) from the diagnostic port. This is the **easiest** method if your vehicle has OBD-II compliance.

### Hardware Required

**Option A: ELM327 OBD-II Adapter** ($10-30)
- Bluetooth or WiFi version
- Connects to OBD-II port under dash
- ESP32 communicates via Bluetooth/WiFi
- **Pro**: Plug-and-play, cheap, widely available
- **Con**: Slow (5-10Hz), Bluetooth latency, cheap clones unreliable

**Option B: OBD-II to UART Module** ($15-40)
- OBD-II to TTL serial converter
- Connects directly to ESP32 UART pins
- Examples: STN1110, OBD2UART
- **Pro**: Faster than Bluetooth, no wireless issues
- **Con**: Requires wiring, more expensive

**Option C: ESP32 + MCP2515 CAN Module** ($8-15)
- Read CAN bus directly (if vehicle has CAN)
- Bypass OBD-II protocol overhead
- **Pro**: Fastest, most flexible
- **Con**: Requires CAN protocol knowledge

### Wiring Diagram - ELM327 Bluetooth

```
┌─────────────────────┐
│  OBD-II Port        │
│  (Under Dash)       │
└──────┬──────────────┘
       │
       │ OBD-II Cable
       │
┌──────▼──────────────┐
│  ELM327 Bluetooth   │
│  OBD Adapter        │
└──────┬──────────────┘
       │
       │ Bluetooth
       │ (SPP Profile)
       │
┌──────▼──────────────┐
│  ESP32-S3           │
│  (Your Gauge)       │
│                     │
│  - Bluetooth Serial │
│  - Parse OBD PIDs   │
│  - Display on gauge │
└─────────────────────┘
```

### Available OBD-II PIDs for 1KZ-TE

| PID | Mode | Description | Formula | Unit |
|-----|------|-------------|---------|------|
| **$05** | 01 | Engine Coolant Temp | A - 40 | °C |
| **$0B** | 01 | Intake Manifold Pressure | A | kPa (absolute) |
| **$0C** | 01 | Engine RPM | ((A×256)+B)/4 | RPM |
| **$0D** | 01 | Vehicle Speed | A | km/h |
| **$0F** | 01 | Intake Air Temperature | A - 40 | °C |
| **$11** | 01 | Throttle Position | A×100/255 | % |

**Example OBD-II Request/Response:**

```
Request:  01 05       (Mode 01, PID 05 = Coolant Temp)
Response: 41 05 8F    (41 = response, 05 = PID, 8F = data)
Decode:   0x8F = 143 decimal
          143 - 40 = 103°C
```

### ESP32 Code Example - OBD-II via Bluetooth

```cpp
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

// OBD-II PID definitions
#define PID_COOLANT_TEMP   0x05
#define PID_INTAKE_PRESSURE 0x0B
#define PID_ENGINE_RPM     0x0C
#define PID_INTAKE_AIR_TEMP 0x0F

void setup() {
  Serial.begin(115200);
  SerialBT.begin("JNKR_Gauge"); // Bluetooth device name
  
  // Wait for ELM327 connection
  while (!SerialBT.available()) {
    delay(100);
  }
  
  // Initialize ELM327
  sendATCommand("ATZ");      // Reset
  delay(1000);
  sendATCommand("ATE0");     // Echo off
  sendATCommand("ATH1");     // Headers on
  sendATCommand("ATSP0");    // Auto protocol
}

void loop() {
  // Read coolant temperature
  float coolantTemp = readOBDPID(PID_COOLANT_TEMP);
  Serial.print("Coolant: "); Serial.println(coolantTemp);
  
  // Read intake air temperature
  float intakeTemp = readOBDPID(PID_INTAKE_AIR_TEMP);
  Serial.print("IAT: "); Serial.println(intakeTemp);
  
  // Read manifold pressure (boost)
  float manifoldPressure = readOBDPID(PID_INTAKE_PRESSURE);
  float boostPSI = (manifoldPressure - 101.325) * 0.145038; // Convert kPa to PSI gauge
  Serial.print("Boost: "); Serial.println(boostPSI);
  
  delay(200); // 5Hz refresh rate
}

float readOBDPID(uint8_t pid) {
  // Send OBD-II request
  char cmd[10];
  sprintf(cmd, "01%02X\r", pid);
  SerialBT.print(cmd);
  
  // Wait for response
  String response = "";
  unsigned long timeout = millis() + 1000;
  while (millis() < timeout) {
    if (SerialBT.available()) {
      char c = SerialBT.read();
      if (c == '>') break; // ELM327 prompt
      response += c;
    }
  }
  
  // Parse response: "41 05 8F" -> extract 8F
  int dataStart = response.indexOf("41") + 6;
  if (dataStart > 6) {
    String hexValue = response.substring(dataStart, dataStart + 2);
    int rawValue = strtol(hexValue.c_str(), NULL, 16);
    
    // Convert based on PID
    switch (pid) {
      case PID_COOLANT_TEMP:
      case PID_INTAKE_AIR_TEMP:
        return rawValue - 40.0; // Temperature (°C)
      
      case PID_INTAKE_PRESSURE:
        return rawValue; // Pressure (kPa absolute)
      
      case PID_ENGINE_RPM:
        // RPM is 2 bytes: ((A*256)+B)/4
        return 0; // TODO: parse 2 bytes
      
      default:
        return rawValue;
    }
  }
  
  return -999.0; // Error
}

void sendATCommand(const char* cmd) {
  SerialBT.println(cmd);
  delay(100);
}
```

### Pros and Cons

**Pros:**
- ✅ Easiest method (plug-and-play)
- ✅ No permanent wiring changes
- ✅ Standardized protocol (works on many vehicles)
- ✅ Cheap hardware ($10-30)

**Cons:**
- ❌ Slow refresh rate (5-10Hz typical)
- ❌ Bluetooth latency and reliability issues
- ❌ Many cheap ELM327 clones are unreliable
- ❌ **1KZ-TE may not fully support OBD-II** (especially pre-1996)

### Does 1KZ-TE Support OBD-II?

**Depends on year and market:**

✅ **1996+ USA/Canada models:** Should have OBD-II compliance
- Port location: Under dash, driver side
- Protocol: ISO 9141-2 or ISO 14230 (KWP2000)
- PIDs available: Basic set (coolant, IAT, MAP, RPM)

⚠️ **1993-1995 or Non-US models:** No OBD-II
- May have Toyota diagnostic port (not OBD-II)
- Requires proprietary Toyota scan tool protocol
- ELM327 may not work

❌ **Australian, European, Asian models (most):** No OBD-II
- Toyota K-Line proprietary protocol
- Requires Method 4 (K-Line direct access)

**How to check:** Look under your dash. If you see a standard 16-pin OBD-II port, try an ELM327. If not, you need Method 3 or 4.

---

## 🌐 Method 2: CAN Bus Direct Access

### Overview

Tap directly into the vehicle's CAN (Controller Area Network) bus to intercept messages between ECU and other modules. This is **faster and more reliable** than OBD-II, but requires protocol knowledge.

### Does 1KZ-TE Have CAN Bus?

**Short answer: Usually NO** ❌

| Vehicle Year | CAN Bus? | Notes |
|--------------|----------|-------|
| **Pre-2000** | ❌ No | Uses analog sensors + K-Line diagnostics |
| **2000-2004** | ⚠️ Maybe | Some late models have partial CAN |
| **2005-2007** | ⚠️ Partial | CAN bus for comfort systems, not engine |

**Critical**: Most 1KZ-TE vehicles **do not have engine CAN bus**. They use direct analog sensor wiring to the ECU. This means Method 2 is **not viable** for most 1KZ-TE applications.

### How to Check if You Have CAN Bus

1. **Check OBD-II port pins:**
   - Pin 6 = CAN High (CAN-H)
   - Pin 14 = CAN Low (CAN-L)
   - If these pins have wires, you likely have CAN

2. **Check ECU connector:**
   - Look for twisted pair wires (yellow/green typically)
   - CAN wires are always twisted pair for noise immunity

3. **Use multimeter:**
   - Measure voltage between Pin 6 and Pin 14
   - CAN-H should read ~2.5-3.5V
   - CAN-L should read ~1.5-2.5V
   - Differential voltage ~2V

### Hardware Required (If You Have CAN)

**MCP2515 CAN Controller Module** ($5-15)
- SPI interface to ESP32
- 8MHz crystal
- TJA1050 or MCP2551 CAN transceiver
- **Connection:** CAN-H to Pin 6, CAN-L to Pin 14 of OBD port

### Wiring Diagram - MCP2515 to ESP32

```
┌─────────────────────┐
│  OBD-II Port        │
│                     │
│  Pin 6 (CAN-H) ─────┼──┐
│  Pin 14 (CAN-L) ────┼──┤
│  Pin 16 (12V) ──────┼──┤
│  Pin 4,5 (GND) ─────┼──┤
└─────────────────────┘  │
                         │
                    ┌────▼─────────────┐
                    │  MCP2515 Module  │
                    │  (CAN Controller)│
                    │                  │
                    │  CANH ◄─ Pin 6   │
                    │  CANL ◄─ Pin 14  │
                    │  VCC  ◄─ 5V      │
                    │  GND  ◄─ GND     │
                    │                  │
                    │  SPI Interface:  │
                    │  SCK  ◄─ GPIO18  │
                    │  MISO ◄─ GPIO19  │
                    │  MOSI ◄─ GPIO23  │
                    │  CS   ◄─ GPIO5   │
                    └──────────────────┘
                            │
                            │ SPI
                            │
                    ┌───────▼──────────┐
                    │   ESP32-S3       │
                    │   (Your Gauge)   │
                    └──────────────────┘
```

### ESP32 Code Example - CAN Bus

```cpp
#include <SPI.h>
#include <mcp2515.h>

// MCP2515 pins (adjust for your ESP32)
#define MCP2515_CS    5   // Chip Select
#define MCP2515_INT   4   // Interrupt (optional)

MCP2515 mcp2515(MCP2515_CS);

// Toyota CAN message IDs (examples - need to be sniffed)
#define CAN_ID_ENGINE_DATA_1   0x0B0  // Hypothetical: RPM, coolant temp
#define CAN_ID_ENGINE_DATA_2   0x0B4  // Hypothetical: IAT, MAP
#define CAN_ID_ENGINE_DATA_3   0x0B8  // Hypothetical: throttle, speed

void setup() {
  Serial.begin(115200);
  SPI.begin();
  
  // Initialize MCP2515
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // Toyota typically uses 500 kbps
  mcp2515.setNormalMode();
  
  Serial.println("CAN Bus initialized");
}

void loop() {
  struct can_frame frame;
  
  // Check for CAN message
  if (mcp2515.readMessage(&frame) == MCP2515::ERROR_OK) {
    // Parse based on CAN ID
    switch (frame.can_id) {
      case CAN_ID_ENGINE_DATA_1:
        parseEngineData1(&frame);
        break;
      
      case CAN_ID_ENGINE_DATA_2:
        parseEngineData2(&frame);
        break;
      
      default:
        // Unknown message - log for analysis
        Serial.print("CAN ID: 0x");
        Serial.print(frame.can_id, HEX);
        Serial.print(" Data: ");
        for (int i = 0; i < frame.can_dlc; i++) {
          Serial.print(frame.data[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
        break;
    }
  }
  
  delay(10);
}

void parseEngineData1(struct can_frame* frame) {
  // Example parsing (actual format needs to be determined by CAN sniffing)
  // Byte 0-1: Engine RPM (16-bit, little endian, scale 0.25)
  uint16_t rawRPM = (frame->data[1] << 8) | frame->data[0];
  float rpm = rawRPM * 0.25;
  
  // Byte 2: Coolant temp (8-bit, offset -40°C)
  float coolantTemp = frame->data[2] - 40.0;
  
  Serial.print("RPM: "); Serial.print(rpm);
  Serial.print(" | Coolant: "); Serial.println(coolantTemp);
}

void parseEngineData2(struct can_frame* frame) {
  // Example parsing
  // Byte 0: Intake air temp (8-bit, offset -40°C)
  float intakeTemp = frame->data[0] - 40.0;
  
  // Byte 1-2: Manifold pressure (16-bit, scale 0.1 kPa)
  uint16_t rawPressure = (frame->data[2] << 8) | frame->data[1];
  float pressureKPa = rawPressure * 0.1;
  float boostPSI = (pressureKPa - 101.325) * 0.145038;
  
  Serial.print("IAT: "); Serial.print(intakeTemp);
  Serial.print(" | Boost: "); Serial.println(boostPSI);
}
```

### Required: CAN Bus Reverse Engineering

**Problem:** Toyota does not publish CAN message formats. You must **reverse engineer** the protocol by:

1. **CAN Sniffing:**
   - Connect MCP2515 to CAN bus
   - Log all messages while driving
   - Correlate data with known values (e.g., RPM from tachometer)

2. **Pattern Analysis:**
   - Look for values that change with engine conditions
   - Coolant temp increases = find byte that increases
   - RPM increases = find 2-byte value that increases proportionally

3. **Tools:**
   - SavvyCAN (open source CAN analysis tool)
   - Wireshark with SocketCAN
   - Custom Python scripts

**This is time-consuming** (10-40 hours of work) but provides the fastest, most reliable data access.

### Pros and Cons

**Pros:**
- ✅ Fast refresh rate (50-100Hz)
- ✅ No protocol overhead (direct data)
- ✅ Most reliable method
- ✅ Can access **all** ECU parameters

**Cons:**
- ❌ **1KZ-TE usually doesn't have engine CAN bus**
- ❌ Requires reverse engineering (time-consuming)
- ❌ CAN message format undocumented
- ❌ Risk of causing issues if transmitting wrong messages

**Verdict for 1KZ-TE:** ❌ **Not recommended** - Most 1KZ-TE vehicles don't have engine CAN. Try Method 3 or 4 instead.

---

## 🔧 Method 3: Sensor Signal Intercept (Unichip Style)

### Overview

This method **taps the analog sensor signals** between the sensors and the ECU. It's how piggyback tuning modules like Unichip, Apexi, and AEM FIC work. You're essentially "listening in" on the sensor wires without modifying them.

**This is the MOST VIABLE method for 1KZ-TE!** ✅

### How It Works

The 1KZ-TE ECU receives analog voltage signals from sensors:
- **Coolant Temp Sensor (THW):** NTC thermistor, 2-pin connector
- **Intake Air Temp (THA):** NTC thermistor, 2-pin connector
- **MAP Sensor:** 3-wire sensor (5V, Signal, GND)

We tap into these signal wires using **high-impedance connections** (voltage dividers or op-amp buffers) so we don't affect the signal going to the ECU.

### Wiring Locations (1KZ-TE Specific)

#### ECU Location
- **Land Cruiser 80:** Behind glove box, passenger side
- **Hilux/4Runner:** Behind passenger kick panel
- **Prado:** Behind glove box

#### Sensor Wiring at ECU Connector

**Toyota 1KZ-TE ECU Connector (Typical Pinout):**

| ECU Pin | Wire Color | Sensor | Signal Type | Voltage Range |
|---------|------------|--------|-------------|---------------|
| **THW** | Green/Red | Coolant Temp | NTC to Ground | 0.5-4.5V |
| **THA** | Yellow/Black | Intake Air Temp | NTC to Ground | 0.5-4.5V |
| **PIM** | Blue/White | MAP Sensor Signal | Analog | 0.5-4.5V |
| **VC** | Pink | Sensor 5V Supply | 5V | 5.0V (regulated) |
| **E2** | Brown | Sensor Ground | Ground | 0V |

**⚠️ Warning:** Pinouts vary by year and market. **Always verify with a multimeter before tapping!**

### How to Identify Sensor Wires

**Method 1: ECU Pinout Diagram**
- Get factory service manual for your year/model
- Look up ECU connector pinout
- Identify THW, THA, and PIM pins

**Method 2: Multimeter Backprobing**

1. **Coolant Temp (THW):**
   - Cold engine: ~2.5-3.5V
   - Warm engine (90°C): ~0.5-1.0V
   - As engine warms, voltage decreases

2. **Intake Air Temp (THA):**
   - Ambient temp: ~2.5-3.5V
   - Hot engine bay: ~1.5-2.5V
   - Voltage decreases with temperature

3. **MAP Sensor (PIM):**
   - Engine off: ~1.0-1.5V (atmospheric)
   - Engine idle: ~0.5-0.8V (vacuum)
   - Engine under boost: >1.5V (pressure)

4. **Sensor 5V Supply (VC):**
   - Constant 5.0V (±0.1V)
   - Powers MAP sensor and throttle position sensor

5. **Sensor Ground (E2):**
   - 0V (chassis ground)

### Tapping the Signal Wires

**Option A: High-Impedance Voltage Divider** (Simple, Good Enough)

This method uses a voltage divider to safely tap the signal without loading the circuit.

```
From ECU Sensor Pin (e.g. THW)
         │
         ├─────────────────────► To OEM Sensor (don't disconnect!)
         │
         │
    [100kΩ Resistor]
         │
         ├──────────────────────► To ESP32 Analog Input
         │
    [100kΩ Resistor]
         │
        GND
```

**Why this works:**
- Total impedance: 100kΩ + 100kΩ = 200kΩ
- ECU input impedance: ~10kΩ typical
- Voltage divider draws minimal current (~25µA)
- ESP32 ADC input impedance: >10MΩ (doesn't load circuit)
- Voltage at ESP32 = 50% of ECU signal (scale up in software by 2x)

**Step-by-Step:**

1. **Locate ECU connector** (see above)
2. **Identify sensor pin** (THW, THA, or PIM)
3. **Solder 100kΩ resistor** to a thin wire (24-26 AWG)
4. **Pierce or backprobe** the ECU pin (don't cut the wire!)
5. **Connect to resistor**, other end to ESP32 analog input
6. **Add second 100kΩ resistor** from ESP32 input to GND
7. **Heat shrink and secure** all connections

**Option B: Op-Amp Buffer (Professional, Expensive)**

For zero circuit loading, use a unity-gain op-amp buffer:

```
From ECU Sensor Pin
         │
         ├─────────────────────► To OEM Sensor
         │
         │
         └─────► [Op-Amp Buffer] ─────► To ESP32 Analog Input
                   (LM358 or TL082)
```

This provides **perfect isolation** but requires additional components and power supply.

### ESP32 Wiring Diagram - Sensor Signal Intercept

```
┌─────────────────────────────────────────┐
│           1KZ-TE ECU Connector          │
│                                         │
│  Pin THW (Coolant) ──────┬──────────────┼─ To Coolant Sensor
│                          │              │
│                     [100kΩ]             │
│                          │              │
│                          ├──────────────┼─ To ESP32 GPIO36
│                          │              │
│                     [100kΩ]             │
│                          │              │
│                         GND             │
│                                         │
│  Pin THA (IAT) ──────────┬──────────────┼─ To IAT Sensor
│                          │              │
│                     [100kΩ]             │
│                          │              │
│                          ├──────────────┼─ To ESP32 GPIO39
│                          │              │
│                     [100kΩ]             │
│                          │              │
│                         GND             │
│                                         │
│  Pin PIM (MAP) ──────────┬──────────────┼─ To MAP Sensor
│                          │              │
│                     [100kΩ]             │
│                          │              │
│                          ├──────────────┼─ To ESP32 GPIO34
│                          │              │
│                     [100kΩ]             │
│                          │              │
│                         GND             │
│                                         │
│  Pin E2 (GND) ───────────────────────────┼─ To ESP32 GND
│                                         │
└─────────────────────────────────────────┘
```

### ESP32 Code Example - Analog Signal Reading

```cpp
// Pin definitions for intercepted signals
#define PIN_ECU_COOLANT    36  // ADC1_CH0 (GPIO36)
#define PIN_ECU_IAT        39  // ADC1_CH3 (GPIO39)
#define PIN_ECU_MAP        34  // ADC1_CH6 (GPIO34)

// ADC configuration
#define ADC_RESOLUTION     12  // 12-bit ADC (0-4095)
#define ADC_VREF           3.3 // ESP32 ADC reference voltage

void setup() {
  Serial.begin(115200);
  
  // Configure ADC
  analogReadResolution(ADC_RESOLUTION);
  analogSetAttenuation(ADC_11db);  // 0-3.3V input range
  
  Serial.println("ECU Signal Intercept Ready");
}

void loop() {
  // Read intercepted sensor signals
  float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
  float iatVoltage = readECUSignal(PIN_ECU_IAT);
  float mapVoltage = readECUSignal(PIN_ECU_MAP);
  
  // Convert voltages to engineering units
  float coolantTemp = voltageToTemperature(coolantVoltage);
  float intakeTemp = voltageToTemperature(iatVoltage);
  float boostPressure = voltageToPressure(mapVoltage);
  
  // Display
  Serial.print("Coolant: "); Serial.print(coolantTemp); Serial.print(" C  |  ");
  Serial.print("IAT: "); Serial.print(intakeTemp); Serial.print(" C  |  ");
  Serial.print("Boost: "); Serial.print(boostPressure); Serial.println(" PSI");
  
  delay(100); // 10Hz refresh
}

float readECUSignal(uint8_t pin) {
  // Read ADC value (0-4095)
  int rawADC = analogRead(pin);
  
  // Convert to voltage
  float voltage = (rawADC / 4095.0) * ADC_VREF;
  
  // Multiply by 2 because of voltage divider (100k/100k = 50% attenuation)
  voltage *= 2.0;
  
  return voltage;
}

float voltageToTemperature(float voltage) {
  // Convert voltage to resistance (NTC thermistor with 2.2kΩ pullup)
  float resistance = (voltage * 2200.0) / (5.0 - voltage);
  
  // Steinhart-Hart equation (Beta parameter method)
  // Typical Toyota NTC: 2.45kΩ @ 25°C, Beta 3435K
  float tempK = 1.0 / (
    (1.0 / (25.0 + 273.15)) + 
    ((1.0 / 3435.0) * log(resistance / 2450.0))
  );
  
  return tempK - 273.15; // Convert to Celsius
}

float voltageToPressure(float voltage) {
  // Toyota MAP sensor: typically 0.5V = 0 kPa, 4.5V = 250 kPa
  float pressureKPa = ((voltage - 0.5) / 4.0) * 250.0;
  
  // Convert to PSI gauge pressure (subtract atmospheric)
  float boostPSI = (pressureKPa - 101.325) * 0.145038;
  
  return boostPSI;
}
```

### Calibration Required

Since we're reading OEM sensor signals, we need to match Toyota's calibration:

**Toyota 1KZ-TE Sensor Specs (Typical):**

| Sensor | Type | Resistance @ 25°C | Beta Coefficient |
|--------|------|-------------------|------------------|
| Coolant (THW) | NTC | 2.45 kΩ | 3435K |
| Intake Air (THA) | NTC | 2.45 kΩ | 3435K |
| MAP (PIM) | 3-wire analog | 0.5V = 0 kPa, 4.5V = 250 kPa | Linear |

**How to find YOUR sensor specs:**

1. **Measure resistance directly** at sensor connector
   - Disconnect sensor
   - Measure resistance at known temperature
   - Compare to Toyota service manual

2. **Use known calibration points:**
   - Coolant at 90°C (normal operating temp)
   - IAT at ambient (measure with thermometer)
   - MAP at atmospheric (should be ~101 kPa)

3. **Compare to gauge cluster readings:**
   - OEM coolant gauge should match your reading
   - If off, adjust calibration in software

### Pros and Cons

**Pros:**
- ✅ **Works on ALL 1KZ-TE vehicles** (no OBD/CAN required)
- ✅ Fast refresh rate (100Hz+)
- ✅ Factory-calibrated sensors
- ✅ No modification to OEM wiring (read-only)
- ✅ Cheap ($5 in resistors)
- ✅ Can read sensors the ECU can't see (pre/post IC temp)

**Cons:**
- ❌ Requires finding and tapping ECU connector
- ❌ Must verify pinout for your specific year/model
- ❌ Risk of damaging ECU if done incorrectly
- ❌ Still requires calibration in software
- ❌ No EGT (not available from ECU)

**Verdict for 1KZ-TE:** ✅ **HIGHLY RECOMMENDED** - This is the best method for 1KZ-TE!

---

## 📡 Method 4: K-Line / ISO 9141 Diagnostics

### Overview

Most 1KZ-TE vehicles (especially non-US) use Toyota's K-Line diagnostic protocol. This is similar to OBD-II but uses Toyota's proprietary commands.

### Hardware Required

**ISO 9141 to Serial Adapter** ($20-40)
- K-Line transceiver IC (MC33290 or L9637D)
- TTL UART interface to ESP32
- **Example:** DIY adapter using MC33290 + ESP32

### K-Line Connection Points

**Option A: OBD-II Port (if present)**
- Pin 7 = K-Line (bidirectional data)
- Pin 15 = L-Line (rarely used on Toyota)
- Pin 16 = 12V
- Pin 4,5 = Ground

**Option B: Toyota Diagnostic Port (pre-OBD-II)**
- Typically under hood or behind dash
- 3-wire connector: K-Line, 12V, Ground
- **Service manual required** to locate

### Toyota K-Line Protocol Basics

**Initialization Sequence:**
1. Wake up ECU (5 baud init sequence)
2. Send address byte (0x33 for engine ECU)
3. Receive key bytes (sync response)
4. Communication established at 10.4 kbps

**Example Data Request:**
```
Send: 0x33 0x10 0x00 0xChecksum  (Request engine data)
Recv: 0x33 0x50 0xData1 0xData2 ... 0xChecksum
```

**Problem:** Toyota does not publish K-Line protocol details. You must reverse-engineer or use existing projects:

- **TechStream Lite:** Open-source Toyota diagnostic software
- **OBD9141:** Arduino library for ISO 9141 (may work)

### ESP32 Code Example - K-Line Communication

```cpp
#include <SoftwareSerial.h>

// K-Line connection
#define KLINE_TX   17  // ESP32 TX to K-Line transceiver
#define KLINE_RX   16  // ESP32 RX from K-Line transceiver

SoftwareSerial KLine(KLINE_RX, KLINE_TX);

void setup() {
  Serial.begin(115200);
  
  // Initialize K-Line at 10.4 kbps (ISO 9141-2)
  KLine.begin(10400);
  
  // Perform 5-baud init sequence
  if (klineInit()) {
    Serial.println("K-Line initialized");
  } else {
    Serial.println("K-Line init failed");
  }
}

void loop() {
  // Request engine data (example command - actual format needs research)
  uint8_t request[] = {0x68, 0x6A, 0xF1, 0x01, 0x0D}; // Hypothetical: read RPM
  sendKLineCommand(request, sizeof(request));
  
  // Receive response
  uint8_t response[32];
  int len = receiveKLineResponse(response, sizeof(response));
  
  if (len > 0) {
    // Parse response (format depends on Toyota protocol)
    Serial.print("K-Line response: ");
    for (int i = 0; i < len; i++) {
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
  
  delay(100);
}

bool klineInit() {
  // Perform ISO 9141 5-baud initialization
  // (Complex timing-critical sequence - see ISO 9141 spec)
  // TODO: Implement 5-baud init
  return false; // Placeholder
}

void sendKLineCommand(uint8_t* data, int len) {
  for (int i = 0; i < len; i++) {
    KLine.write(data[i]);
  }
}

int receiveKLineResponse(uint8_t* buffer, int maxLen) {
  int count = 0;
  unsigned long timeout = millis() + 1000;
  
  while (millis() < timeout && count < maxLen) {
    if (KLine.available()) {
      buffer[count++] = KLine.read();
    }
  }
  
  return count;
}
```

### Pros and Cons

**Pros:**
- ✅ Works on pre-OBD-II 1KZ-TE vehicles
- ✅ Can access ECU parameters directly
- ✅ No modification to vehicle wiring
- ✅ More data available than OBD-II

**Cons:**
- ❌ Toyota K-Line protocol is proprietary (undocumented)
- ❌ Requires reverse engineering or existing projects
- ❌ Slower than analog signal intercept
- ❌ Complex initialization sequence
- ❌ May not work on all 1KZ-TE variants

**Verdict for 1KZ-TE:** ⚠️ **Possible, but difficult** - Only attempt if you have experience with automotive protocols.

---

## 🖥️ Implementation with ESP32-S3

### Your Existing Hardware

You already have:
- ✅ Qualia ESP32-S3 (Adafruit #5800)
- ✅ 2.1" Round RGB Display
- ✅ I2C sensor module capability

### Integration Options

**Option 1: OBD-II Bluetooth** (Easiest)
- Add ELM327 Bluetooth adapter ($10-20)
- ESP32 reads via Bluetooth Serial
- **Pros:** No wiring changes
- **Cons:** Slow, unreliable, may not work on 1KZ-TE

**Option 2: MCP2515 CAN Module** (If you have CAN)
- Add MCP2515 to ESP32 SPI bus
- **Pins:** SCK=18, MISO=19, MOSI=23, CS=5
- **Pros:** Fast, reliable
- **Cons:** 1KZ-TE usually doesn't have engine CAN

**Option 3: Analog Signal Intercept** (RECOMMENDED ✅)
- Tap ECU signals with voltage dividers
- Use ESP32 ADC pins (GPIO36, 39, 34, etc.)
- **Pros:** Works on ALL 1KZ-TE, fast, cheap
- **Cons:** Requires finding ECU pins

**Option 4: K-Line Adapter**
- Add MC33290 K-Line transceiver
- Connect to ESP32 UART
- **Pros:** Works on pre-OBD-II vehicles
- **Cons:** Protocol undocumented, complex

### Recommended Pin Assignments (ESP32-S3)

```cpp
// ECU signal intercept pins (if using Method 3)
#define PIN_ECU_COOLANT     36  // ADC1_CH0
#define PIN_ECU_IAT         39  // ADC1_CH3
#define PIN_ECU_MAP         34  // ADC1_CH6

// I2C for engine bay sensor module (existing)
#define PIN_I2C_SDA         21
#define PIN_I2C_SCL         22

// MCP9600 for EGT (existing - still required!)
// Connected via I2C (address 0x60)

// Optional: CAN bus (if vehicle has CAN)
#define PIN_CAN_SCK         18  // SPI SCK
#define PIN_CAN_MISO        19  // SPI MISO
#define PIN_CAN_MOSI        23  // SPI MOSI
#define PIN_CAN_CS          5   // MCP2515 CS

// Optional: K-Line
#define PIN_KLINE_TX        17  // UART TX
#define PIN_KLINE_RX        16  // UART RX
```

### Code Integration

Modify your existing `sensors.cpp` to add ECU data sources:

```cpp
// sensors.cpp additions

#ifdef USE_ECU_SENSORS

// Read from ECU instead of I2C sensor module
void Sensors_Update() {
  // Option 1: Read from OBD-II (if available)
  #ifdef USE_OBD2
    coolantTemp = readOBDPID(PID_COOLANT_TEMP);
    intakeTempPost = readOBDPID(PID_INTAKE_AIR_TEMP);
    boostPressureBar = readOBDPID(PID_INTAKE_PRESSURE) / 100.0; // kPa to bar
  #endif
  
  // Option 2: Read from analog signal intercept (RECOMMENDED)
  #ifdef USE_ANALOG_INTERCEPT
    float coolantVoltage = readECUSignal(PIN_ECU_COOLANT);
    coolantTemp = voltageToTemperature(coolantVoltage);
    
    float iatVoltage = readECUSignal(PIN_ECU_IAT);
    intakeTempPost = voltageToTemperature(iatVoltage);
    
    float mapVoltage = readECUSignal(PIN_ECU_MAP);
    boostPressureBar = voltageToPressure(mapVoltage);
  #endif
  
  // EGT always from separate sensor (ECU doesn't have EGT input)
  exhaustTemp = readEGT(prevExhaustTemp);
  
  // Update previous values
  prevCoolantTemp = coolantTemp;
  prevIntakeTempPost = intakeTempPost;
  prevBoostPressure = boostPressureBar;
  prevExhaustTemp = exhaustTemp;
}

#endif // USE_ECU_SENSORS
```

---

## 🔥 What About EGT?

### Critical: ECU Does NOT Have EGT Input

The 1KZ-TE ECU **does not monitor exhaust gas temperature**. This is true for almost all diesel ECUs (except some modern Euro 6 trucks with DPF systems).

**You MUST install a separate EGT sensor** regardless of which method you choose for other sensors.

### EGT Sensor Options

**Option 1: K-Type Thermocouple + MCP9600** (Your current setup ✅)
- Keep your existing I2C sensor module
- MCP9600 thermocouple amplifier
- K-type probe in exhaust manifold
- **This is the correct approach**

**Option 2: Analog Thermocouple Amplifier**
- MAX31855 or MAX6675 (SPI interface)
- Slightly cheaper but less accurate
- Your I2C module is better

### EGT Installation Location

**Best locations for 1KZ-TE:**

1. **Pre-Turbo** (Most Critical) ⭐
   - Exhaust manifold, 6-12" before turbo inlet
   - Measures true EGT (highest temps)
   - **Recommended for diesel**

2. **Post-Turbo**
   - Downpipe, 6-12" after turbo outlet
   - Measures turbine health
   - Cooler temps (~200°C lower)

3. **Both (Ideal)**
   - Monitor turbo efficiency (pre - post = turbine drop)
   - Requires 2 thermocouples + 2 MCP9600 modules

### Wiring - Keep Your EGT Module

Since ECU doesn't have EGT, **keep your existing I2C sensor module** for EGT monitoring:

```
┌────────────────────────────────┐
│  Engine Bay I2C Module         │
│  (Weatherproof enclosure)      │
│                                │
│  - MCP9600 (EGT thermocouple)  │ ◄─ K-Type Probe in Exhaust
│                                │
│  - Optional: ADS1115 for       │
│    additional analog sensors   │
└────────┬───────────────────────┘
         │
         │ I2C (4-wire harness)
         │ (5V, GND, SCL, SDA)
         │
         │ Through firewall
         ↓
┌────────────────────────────────┐
│  ESP32-S3 in Cabin             │
│                                │
│  Reads ECU sensors:            │
│  - Coolant temp (from ECU)     │
│  - IAT (from ECU)              │
│  - Boost (from ECU)            │
│                                │
│  Reads I2C module:             │
│  - EGT (NOT available from ECU)│
└────────────────────────────────┘
```

---

## 📊 Comparison Table

| Method | Difficulty | Speed | Reliability | 1KZ-TE Compatible? | Cost |
|--------|-----------|-------|-------------|-------------------|------|
| **1. OBD-II Bluetooth** | Easy | Slow (5-10Hz) | Medium | ⚠️ Maybe (1996+ USA) | $10-30 |
| **2. CAN Bus** | Hard | Fast (50-100Hz) | High | ❌ Usually No | $10-20 |
| **3. Analog Intercept** | Medium | Fast (100Hz+) | High | ✅ Yes (All years) | $5-10 |
| **4. K-Line Protocol** | Very Hard | Medium (20Hz) | Medium | ✅ Yes (All years) | $20-40 |

---

## 🏆 Recommended Approach for 1KZ-TE

### For Most Users: **Method 3 - Analog Signal Intercept** ✅

**Why:**
1. ✅ Works on **all** 1KZ-TE vehicles (any year, any market)
2. ✅ Fast refresh rate (100Hz+)
3. ✅ Factory-calibrated sensors
4. ✅ Cheap ($5 in resistors)
5. ✅ Read-only (no risk to ECU)
6. ✅ No protocols to reverse-engineer

**What you need:**
- 6× 100kΩ resistors (for voltage dividers)
- Thin wire (24-26 AWG)
- Heat shrink tubing
- Multimeter (to verify signals)
- Factory service manual (for ECU pinout)

**Sensors you can read:**
- ✅ Coolant temperature (THW)
- ✅ Intake air temperature (THA)
- ✅ Manifold pressure / Boost (PIM)
- ❌ EGT (not available - keep your I2C module!)

**Step-by-Step:**

1. **Locate your ECU** (behind glove box or kick panel)
2. **Get pinout diagram** for your year/model from service manual
3. **Identify sensor signal wires** (THW, THA, PIM)
4. **Verify signals with multimeter:**
   - THW: 0.5-4.5V, decreases as engine warms
   - THA: 0.5-4.5V, decreases as air heats up
   - PIM: 0.5-4.5V, changes with boost/vacuum
5. **Build voltage dividers** (100kΩ + 100kΩ for each signal)
6. **Tap ECU signals** using backprobing or scotchlok connectors
7. **Connect to ESP32 ADC pins** (GPIO36, 39, 34)
8. **Calibrate in software** using Toyota sensor specs
9. **Keep your I2C EGT module** (ECU doesn't have EGT)

### For Advanced Users: **Try OBD-II First** ⚠️

If your vehicle is **1996+ USA/Canada model**, try Method 1 (OBD-II) first:

1. Buy a **quality** ELM327 adapter (not a $5 clone)
2. Test if your ECU responds to OBD-II commands
3. If it works: Use Bluetooth Serial to ESP32
4. If it doesn't: Fall back to Method 3 (analog intercept)

**How to test:**
- Connect ELM327 to OBD port
- Use Torque app on Android
- Try to read coolant temp, IAT, and MAP
- If successful: Implement in your ESP32 code
- If failed: Your ECU doesn't support OBD-II → use Method 3

---

## 🛠️ Next Steps

### 1. Determine Your Vehicle's Protocol

**Check:**
- [ ] Year and model of your vehicle
- [ ] Location of OBD/diagnostic port
- [ ] ECU part number (on ECU label)
- [ ] Does OBD port have pins 6 and 14? (CAN bus)

### 2. Choose Your Method

Based on the checklist above:
- **OBD port + USA 1996+** → Try Method 1 (OBD-II)
- **No OBD or pre-1996** → Method 3 (Analog Intercept)
- **CAN bus detected** → Method 2 (CAN Bus - advanced)
- **Non-US, no OBD-II** → Method 3 (Analog Intercept)

### 3. Gather Hardware

**For Method 1 (OBD-II):**
- [ ] ELM327 Bluetooth adapter ($15-30)
- [ ] OBD-II extension cable (optional)

**For Method 3 (Analog Intercept):** ✅ RECOMMENDED
- [ ] 100kΩ resistors (6 pieces, $1)
- [ ] 24-26 AWG wire
- [ ] Heat shrink tubing
- [ ] Backprobing kit or scotchlok connectors
- [ ] Multimeter
- [ ] Factory service manual (FSM) for your model

**For EGT (ALL methods):**
- [ ] Keep your existing I2C sensor module
- [ ] K-type thermocouple probe
- [ ] MCP9600 module (you already have this ✅)

### 4. Implementation

**Phase 1: Bench Testing**
- Test OBD-II or analog reading on bench
- Verify voltage dividers work correctly
- Calibrate ADC readings

**Phase 2: Vehicle Integration**
- Locate ECU and sensor wires
- Tap signals with voltage dividers
- Route wires to ESP32 in cabin

**Phase 3: Software Integration**
- Modify `sensors.cpp` to read ECU signals
- Add calibration for Toyota sensors
- Test and validate readings

**Phase 4: Final Assembly**
- Install EGT sensor (keep I2C module)
- Secure all wiring
- Test under driving conditions

---

## 📚 Additional Resources

### Toyota 1KZ-TE Service Manuals
- Search: "1KZ-TE service manual PDF"
- Check: Land Cruiser / Hilux workshop manuals
- **Critical:** Need ECU connector pinout for your year/model

### Useful Forums
- **IH8MUD** - Land Cruiser technical forum
- **Pirate4x4** - Off-road and diesel section
- **ToyotaNation** - General Toyota tech

### Open Source Projects
- **TechStream Lite** - Open-source Toyota diagnostics
- **OBD9141** - Arduino library for ISO 9141
- **MCP2515 Arduino Library** - CAN bus library

### Tools
- **ELM327 Scan Tool** - Test OBD-II compatibility
- **SavvyCAN** - CAN bus analysis (if applicable)
- **Torque Pro** - Android OBD-II app

---

## ⚠️ Safety Warnings

1. **Do NOT cut OEM wiring** - Use backprobing or scotchlok taps
2. **Use high impedance connections** - Voltage dividers >100kΩ
3. **Verify polarity** - Reversing 5V and GND can damage ECU
4. **Test with multimeter first** - Confirm signals before connecting ESP32
5. **Disconnect battery** - When working on ECU connector
6. **Use proper tools** - Backprobing kit or qualified crimps
7. **Keep EGT sensor** - ECU does NOT monitor exhaust temperature
8. **Don't transmit on CAN/K-Line** - Read-only mode only (unless you know what you're doing)

---

## 🆘 Troubleshooting

### Problem: OBD-II adapter doesn't connect
- ✅ Try: Check if your vehicle has OBD-II (1996+ USA/Canada)
- ✅ Try: Different ELM327 adapter (many clones are bad)
- ✅ Try: Method 3 (analog intercept) instead

### Problem: CAN bus shows no activity
- ✅ Check: Does your 1KZ-TE have CAN? (most don't)
- ✅ Check: Correct baud rate (500 kbps typical)
- ✅ Try: Method 3 (analog intercept) instead

### Problem: Analog readings are wrong
- ✅ Check: Voltage divider resistor values (should be 100kΩ each)
- ✅ Check: Multiply by 2.0 in software (voltage divider attenuation)
- ✅ Calibrate: Use known temperatures (ice water, boiling water)
- ✅ Compare: To factory gauge cluster readings

### Problem: ECU throws error codes
- ❌ STOP: You may have damaged a sensor circuit
- ✅ Check: High-impedance connection (>100kΩ)
- ✅ Verify: No shorts to ground or 5V
- ✅ Disconnect: Your intercept wiring and retest

---

## 📝 Summary

**Best method for 1KZ-TE:** **Analog Signal Intercept (Method 3)** ✅

**What you get:**
- ✅ Coolant temperature from ECU sensor
- ✅ Intake air temperature from ECU sensor  
- ✅ Boost pressure from ECU MAP sensor
- ✅ Fast refresh rate (100Hz)
- ✅ Factory-calibrated accuracy
- ✅ Works on all 1KZ-TE vehicles

**What you still need:**
- ❌ Separate EGT sensor + MCP9600 (ECU doesn't have EGT input)
- ✅ Keep your existing I2C sensor module for EGT

**Cost:** ~$5 (resistors) + EGT setup you already have

**Time:** 2-4 hours (finding ECU pins, tapping signals, calibrating)

**Difficulty:** Medium (requires finding ECU connector and identifying sensor wires)

---

**Ready to proceed?** 

Start with:
1. Get factory service manual for your year/model
2. Locate ECU and connector
3. Identify THW, THA, and PIM pins with multimeter
4. Build voltage dividers and tap signals
5. Connect to ESP32 and test!

**Questions?** Open an issue on GitHub or ask on Toyota forums!

---

**Last Updated:** 2025-11-11
**Applies to:** 1KZ-TE turbodiesel engines (Land Cruiser, Hilux, Prado, 4Runner)
