# Simple Gauge for Qualia ESP32-S3

This repository contains a minimal three-parameter automotive gauge running on an Adafruit **Qualia ESP32-S3** and its built-in IPS display.

## Parameters Displayed

| Sensor | Range | Notes |
| ------ | ----- | ----- |
| EGT (thermocouple) | up to 1800 °F | MAX31855 SPI amplifier |
| Coolant Temperature | –40 °F – 300 °F | 10 kΩ NTC thermistor, Steinhart–Hart |
| Boost / MAP | 0 – 29 psi | MPX2200GP analog pressure |

## System Block Diagram

Two MCUs, linked by CAN over a 4-pin Deutsch harness. Sensors terminate
locally on the engine-bay board (notably the K-type thermocouple — MAX31855
does cold-junction comp at its own die, so co-locating with the connector
is more accurate than running extension wire to the cabin). The cabin
Qualia is a CAN listener that updates the display.

```text
+--------------------- ENGINE BAY ----------------------+        +---------------- CABIN ----------------+
| 12 V (switched) ── fuse ──┐                          |        |                                       |
|                           ↓                          |        |  Adafruit Qualia ESP32-S3 + IPS       |
|                    12 V → 3.3 V buck                 |        |                                       |
|                           │                          |        |   ┌── TJA1051 / SN65HVD230 (CAN xcvr)│
|                           │  3.3 V rail              |        |   │       │                          │
|                  ┌────────┼────────┐                 |        |   │      120 Ω termination           │
|                  │        │        │                 |        |   │       │                          │
|              ESP32-C3 ─ SPI ─ MAX31855 ← K-type EGT  |        |   │   TWAI_TX / TWAI_RX → ESP32-S3  │
|                  │        │        │                 |        |   │                                  │
|                  │        └─ I²C ─ ADS1115           |        |                                       |
|                  │                  ├─ CH0 ← NTC     |        |                                       |
|                  │                  └─ CH1 ← MAP     |        |                                       |
|                  │                                   |        |                                       |
|              TJA1051 / SN65HVD230 (CAN xcvr)         |        |                                       |
|                  │                                   |        |                                       |
|              120 Ω termination                       |        |                                       |
|                  │                                   |        |                                       |
|              Deutsch DT04-4P ─── twisted pair ───────┼────────┼─── Deutsch DT06-4S                    |
+──────────────────────────────────────────────────────+        +───────────────────────────────────────+
                                  CAN_H / CAN_L
                                  +12 V switched / GND
```

## Harness Pinout (DT04-4P)

| Pin | Signal | Dir | Notes |
| --- | ------ | --- | ----- |
| 1 | +12 V (switched) | → | Fused upstream (3 A), feeds engine-bay buck |
| 2 | GND | — | Chassis ground, single-point at battery |
| 3 | CAN_H | ↔︎ | Twisted pair with CAN_L, 120 Ω term each end |
| 4 | CAN_L | ↔︎ | Twisted pair with CAN_H |

## Cabin ESP32-S3 (Qualia) Pin Map

| Function | Pin |
| -------- | --- |
| TFT SCK | GPIO36 |
| TFT MOSI | GPIO35 |
| TFT MISO | GPIO33 |
| TFT CS | GPIO34 |
| TFT DC | GPIO37 |
| TFT RST | GPIO38 |
| CAN TX (TWAI) | GPIO5 |
| CAN RX (TWAI) | GPIO6 |

## Engine-Bay ESP32-C3 Pin Map

| Function | Pin |
| -------- | --- |
| MAX31855 SCK | GPIO4 |
| MAX31855 MISO | GPIO5 |
| MAX31855 CS | GPIO6 |
| ADS1115 SDA | GPIO8 |
| ADS1115 SCL | GPIO9 |
| CAN TX (TWAI) | GPIO0 |
| CAN RX (TWAI) | GPIO1 |

## Calibration Constants

Thermistor β: **3950 K**  |  Series resistor: **10 kΩ @ 25 °C**  |  MAP: 0.2 V → 0 psi, 4.7 V → 29 psi

## Power Supply

The cabin Qualia runs off USB-C (or its own switched-12 V→5 V adapter). The
engine-bay board is fed switched **12 V** through pin 1 of the Deutsch
harness, fused at ~3 A upstream, and steps it down locally with a 12 V→3.3 V
buck. Single-point ground at the battery; TVS diode on the engine-bay 12 V
input for load-dump protection.

## Firmware Layout

Two PlatformIO environments (current `src/` is the cabin-side firmware and
needs to be refactored to drop the direct sensor reads):

- **`engine-bay/`** (ESP32-C3) — reads MAX31855 + ADS1115, publishes one CAN
  frame per parameter on a fixed cadence (~20 Hz).
- **`src/`** (Qualia ESP32-S3) — TWAI listener that updates the three LVGL
  labels from incoming frames; no direct sensor access.

## Build / Flash

```bash
pio run -t upload
```

Reboot the device — numeric display should appear instantly.