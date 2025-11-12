# Junkr OBD Hardware Architecture (1KZ-TE Land Cruiser)

**v0.1 – 2025-11-12**

---

## Overview

The sensor-intercept system is split into **three dedicated PCBs** to keep harness length short, improve EMC/temperature resilience, and simplify installation:

| ID | Board | Mounting location | Core silicon | Primary roles |
|----|-------|-------------------|--------------|---------------|
| A  | ECU-Intercept | Behind factory ECU, inline with the two 34-pin Toyota connectors | ADS131M08 (8-ch Σ-Δ ADC) + ISO7842 isolator | • Tap OEM analogue lines (TPS, MAP, THA/THW, VRP/VRT…)  
• Provide isolated SPI stream to Board B  
• Leave all OEM wires pass-through by default |
| B  | Dash I/O + Display Host | Cluster cavity, piggy-backed on Adafruit **Qualia ESP32-S3** (#5800) | ESP32-S3 8 MB PSRAM + PCA9554 IO expander (on Qualia) | • Collect data from Board A  
• Render high-res graphics to TTL RGB-666 TFT via Qualia board  
• User-controls, buzzer, future boost-control output  
• RS-485/CAN link to Board C |
| C  | Engine-bay Sensor Node | LH inner fender (sealed enclosure) | STM32C0 (-40‒125 °C) | • MAX31856 K-type EGT  
• LSU 4.9 wideband controller  
• BMP388 baro/thermo  
• Streams packets back to Board B |

---

## Inter-board buses

``A ↔ B``  Isolated SPI @ 8 MHz (CLK, MISO, MOSI, /CS, /IRQ) through ISO7842 + twisted pair. Fallback: ISO-UART 1 Mbps.

``B ↔ C``  RS-485 half-duplex 1 Mbps, 5-pin sealed connector (Vbat, GND, A, B, IGN_SW). Protocol: framed protobuf (len + msg-id + CRC-16-CCITT).

---

## Power domains

• **+12 V Switched (B-R: GAUGE 10 A fuse)** feeds all three boards.  
• Local DC/DC on each board → 5 V logic → 3.3 V rail.  
• Board A senses VC (5 V ref) but never loads it.  
• Grounds: Board A bonds to ECU sensor ground (E2). Boards B/C use chassis ground but share shield with A via isolated data lines only.

---

## Connectors (draft)

### Board A – ECU-Intercept

| J1 | Toyota 34-pin male (to vehicle harness) |
| J2 | Toyota 34-pin female (to ECU) |
| J3 | 6-pin JST PA to Board B SPI (ISO) |

### Board B – Dash I/O / Qualia Carrier

| J10 | 40-pin RGB-666 ribbon from Qualia to TFT |
| J11 | 6-pin JST PA to Board A (SPI) |
| J12 | 4-pin JST-XH RS-485 to Board C |
| J13 | 8-pin JST PH user I/O (buttons, LED, buzzer) |

### Board C – Engine-bay Node

| J20 | 4-pin TE MicroMatch RS-485 in |
| J21 | 2-pin Mini-K thermocouple |
| J22 | 4-pin Sumitomo LSU4.9 |
| J23 | 4-pin BMP388 STEMMA-QT |

---

## Mechanical notes

* **Qualia ESP32-S3** mounts as a daughtercard on Board B via 2×20 header; no rework to Adafruit PCB required.
* Board A height < 12 mm so the OEM ECU can still bolt into its bracket.
* Board C conformal-coated, gasketed Hammond 1555F IP66.

---

## Next Steps

1. Confirm Toyota ECU connector part numbers & PCB-edge footprint.
2. Finalise signal list to ADS131M08 (≤ 8 channels). Decide if VRP/VRT high-voltage path needs attenuation divider.
3. Choose RS-485 or CAN-FD transceiver pair; update connector pin-out accordingly.
4. Kick-off KiCad schematic for Board A.

