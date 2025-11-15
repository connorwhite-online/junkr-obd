# Simple Gauge for Qualia ESP32-S3

This sub-project implements a minimal three-parameter automotive gauge using an Adafruit **Qualia ESP32-S3** and its built-in display.  It is intentionally scoped down so you can validate wiring and calibration before investing effort in a fancy UI.

## Parameters Displayed

| Sensor | Range | Notes |
| ------ | ----- | ----- |
| EGT (thermocouple) | up to 1800 °F | MAX31855 SPI amplifier module |
| Coolant Temperature | –40 °F – 300 °F | 10 kΩ NTC thermistor, Steinhart–Hart calibration |
| Boost / MAP | 0–29 psi | MPX2200GP analog pressure sensor |

All temperatures are shown in **Fahrenheit**, and boost is shown in **psi** by default.

## Engine-Bay PCB Block Diagram

```text
+---------------------+
| 12 V In             |
|  ↓  Buck Regulator  |
| 3.3 V Out           |
+-----------+---------+
            |
            |-- MAX31855 (SPI)  ← EGT probe
            |
            |-- ADS1115 (I²C)
            |      ├─ CH0  ← Coolant thermistor divider
            |      └─ CH1  ← MPX2200GP pressure
            |
            +-- Deutsch DT04-08 connector → harness
```

## Harness Pinout (DT04-08)

| Pin | Signal | Direction | Notes |
| --- | ------ | --------- | ----- |
| A | 3.3 V | PCB → ESP | Power rail |
| B | GND | — | Common ground |
| C | SPI_MISO | PCB → ESP | MAX31855 data |
| D | SPI_CLK | ESP → PCB | SCK shared |
| E | SPI_CS_EGT | ESP → PCB | GPIO34 |
| F | I²C_SDA | Bidirectional | GPIO3 |
| G | I²C_SCL | Bidirectional | GPIO4 |
| H | (reserved) | — | Future use |

## Qualia ESP32-S3 Pin Map

| Function | ESP32 Pin |
| -------- | ---------- |
| SPI SCK  | GPIO36 |
| SPI MOSI (unused) | GPIO35 |
| SPI MISO | GPIO33 |
| EGT CS   | GPIO34 |
| TFT DC   | GPIO37 |
| TFT RST  | GPIO38 |
| I²C SDA  | GPIO3 |
| I²C SCL  | GPIO4 |

## Calibration Constants

Thermistor β (B-value): **3950 K**  
Series resistor: **10 kΩ @ 25 °C**  
MAP transfer: 0.2 V = 0 psi, 4.7 V = 29 psi.

Adjust these in `sensors.cpp` if your hardware differs.

## Power Supply

Both the engine-bay PCB and the Qualia ESP32-S3 require a stable 3.3 V rail.  Feed them from the **same switched 12 V source** so the gauge powers up and shuts down with the ignition.  Two common tap points:

1. Cigarette-lighter socket (accessory line).  
2. Add-a-fuse in the interior fuse box on an IGN-switched circuit.

On the engine PCB use a 12 V→3.3 V buck regulator (e.g. MP1584 or TPS5430) rated for at least 500 mA to power the MAX31855, ADS1115, and logic lines.  The Qualia board’s onboard regulator is sufficient once it receives 5 V on USB-C, but you can instead route the regulated 3.3 V from the PCB through the harness to drive both units if desired.

Make sure to common-ground the two boards and include a transient suppression diode (TVS) on the 12 V input for load-dump protection.

## Build / Flash

1. `pio run -t upload`  
2. Reboot device – numeric display should appear instantly.

## Next Steps

* Implement sensor drivers (`sensors.cpp/h`).
* Add basic alert thresholds.
* Upgrade UI later using LVGL Designer / SquareLine.
