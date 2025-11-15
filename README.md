# Simple Gauge for Qualia ESP32-S3

This repository contains a minimal three-parameter automotive gauge running on an Adafruit **Qualia ESP32-S3** and its built-in IPS display.

## Parameters Displayed

| Sensor | Range | Notes |
| ------ | ----- | ----- |
| EGT (thermocouple) | up to 1800 °F | MAX31855 SPI amplifier |
| Coolant Temperature | –40 °F – 300 °F | 10 kΩ NTC thermistor, Steinhart–Hart |
| Boost / MAP | 0 – 29 psi | MPX2200GP analog pressure |

## Engine-Bay PCB Block Diagram

```text
+---------------------+
| 12 V In             |
|  ↓  Buck Regulator  |
| 3.3 V Out           |
+-----------+---------+
            |
            |-- MAX31855 (SPI) ← EGT probe
            |
            |-- ADS1115 (I²C)
            |      ├─ CH0 ← Coolant thermistor
            |      └─ CH1 ← MPX2200GP pressure
            |
            +-- Deutsch DT04-08 connector → harness
```

## Harness Pinout (DT04-08)

| Pin | Signal | Dir | Notes |
| --- | ------ | --- | ----- |
| A | 3.3 V | → | Power rail |
| B | GND | — | Common ground |
| C | SPI_MISO | ← | MAX31855 data |
| D | SPI_CLK | → | Shared SCK |
| E | SPI_CS_EGT | → | GPIO34 |
| F | I²C_SDA | ↔︎ | GPIO3 |
| G | I²C_SCL | ↔︎ | GPIO4 |
| H | (reserved) | — | Future |

## ESP32-S3 Pin Map

| Function | Pin |
| -------- | --- |
| SPI SCK | GPIO36 |
| SPI MOSI (unused) | GPIO35 |
| SPI MISO | GPIO33 |
| EGT CS | GPIO34 |
| TFT DC | GPIO37 |
| TFT RST | GPIO38 |
| I²C SDA | GPIO3 |
| I²C SCL | GPIO4 |

## Calibration Constants

Thermistor β: **3950 K**  |  Series resistor: **10 kΩ @ 25 °C**  |  MAP: 0.2 V → 0 psi, 4.7 V → 29 psi

## Power Supply

Feed both boards from a switched **12 V** source (cigarette-lighter or add-a-fuse). Use a 12 V→3.3 V buck on the engine-bay PCB, common ground, and a TVS diode for load-dump protection.

## Build / Flash

```bash
pio run -t upload
```

Reboot the device — numeric display should appear instantly.