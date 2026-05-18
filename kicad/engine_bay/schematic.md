# Engine-Bay Shield — Schematic / Netlist

Modules-on-headers shield. ESP32-C3 SuperMini drives a MAX31855 breakout
(SPI), an ADS1115 breakout (I²C), and a TJA1051 CAN transceiver
breakout, all socketed on 0.1″ female headers. Power conditioning,
sensor signal-conditioning, and the Deutsch harness connector are on
the shield itself.

## Block summary

```
        +12 V harness in (DT04-4P pin 1)
                │
              D1 (SS34, reverse-polarity, series)
                │
              F1 (1 A polyfuse)
                │
        ┌───────┴───────┐
        │               │
       D2 TVS         (rail to regulators)
       (SMBJ24A)       ├──► U1 buck 12→3.3 V ──► +3V3 rail
                       └──► U2 buck 12→5 V   ──► +5V rail (MAP sensor only)

        +3V3 rail powers: ESP32-C3, MAX31855, ADS1115, TJA1051
        +5V  rail powers: MAP sensor (via J3)

        ESP32-C3  ── SPI ──► MAX31855 ── K-type ──► onboard terminal block on the breakout
                  ── I²C ──► ADS1115  ── CH0  ◄── NTC divider ──► J2 (coolant connector)
                  ── I²C ──► ADS1115  ── CH1  ◄── MAP 2:1 divider ──► J3 (MAP connector)
                  ── TWAI ──► TJA1051 ── CAN_H/L ──► DT04-4P pins 3,4
                                       └─ 120 Ω term jumper
```

## Power tree nets

| Net | Source | Sinks |
| --- | ------ | ----- |
| `+12V_RAW` | J1 pin 1 | D1 anode |
| `+12V_PROT` | D1 cathode → F1 | F1 → TVS anode (clamped), U1 VIN, U2 VIN |
| `+3V3` | U1 VOUT | U3 VIN, U4 VDD, U5 VIN, U6 VCC (TJA1051), all decoupling caps |
| `+5V` | U2 VOUT | J3 pin 1 (MAP sensor V+), C4 decoupling |
| `GND` | J1 pin 2 | Everything's ground reference (single GND pour on B.Cu) |

## Designators (reference list)

| Ref | Part | Notes |
| --- | ---- | ----- |
| J1 | Deutsch DT04-4P male receptacle | Harness in (4-pin) |
| D1 | SS34 Schottky | Reverse-polarity block, in series on +12V |
| F1 | 1812L100 polyfuse, 1 A hold | Resettable input fuse |
| D2 | SMBJ24A TVS | Across `+12V_PROT` to GND |
| C1 | 100 µF / 35 V electrolytic | `+12V_PROT` bulk |
| C2 | 100 nF ceramic 0805 | `+12V_PROT` HF bypass |
| U1 | Pololu D24V5F3 | 12→3.3 V buck module, 500 mA |
| U2 | Pololu D24V5F5 | 12→5 V buck module, 500 mA (MAP supply) |
| C3 | 10 µF ceramic 1206 | `+3V3` bulk |
| C4 | 10 µF ceramic 1206 | `+5V` bulk |
| C5–C7 | 100 nF ceramic 0805 | `+3V3` decoupling near U3/U4/U6 |
| U3 | ESP32-C3 SuperMini (module) | MCU, on 2× 1×8 socket strips |
| U4 | Adafruit MAX31855 breakout (PID 269) | Thermocouple amp, on 1× 1×6 socket strip |
| U5 | Adafruit ADS1115 breakout (PID 1085) | 16-bit ADC, on 1× 1×10 socket strip |
| U6 | TJA1051T/3 breakout (e.g., HW-194 / generic SOIC-8 module) | CAN transceiver, on 1× 1×6 socket strip |
| R1 | 120 Ω 1 % 0805 | CAN bus termination |
| R2 | 10 kΩ 1 % 0805 | NTC bottom-leg resistor |
| R3, R4 | 10 kΩ 1 % 0805 each | MAP 2:1 voltage divider |
| C8 | 100 nF ceramic 0805 | NTC signal filter |
| C9 | 100 nF ceramic 0805 | MAP signal filter |
| J2 | JST-XH 2-pin or screw terminal | Coolant NTC connector |
| J3 | JST-XH 3-pin | MAP sensor connector (V+, SIG, GND) |
| JP1 | 1×2 pin header + shunt | CAN termination enable jumper |
| LED1 | 0805 green LED | Power-good indicator |
| R5 | 470 Ω 0805 | LED1 current limit |
| H1–H4 | M3 mounting holes | Non-plated, board corners |

## Detailed net-by-net wiring

### Power input

| Net | From | To |
| --- | ---- | -- |
| `+12V_RAW` | J1.1 | D1 anode |
| `+12V_PROT` | D1 cathode | F1 pin 1, then to D2 anode, C1+, C2 pin 1, U1.VIN, U2.VIN |
| `GND` | J1.2 | D2 cathode, C1−, C2 pin 2, U1.GND, U2.GND, every GND pin in this list |

### 3.3 V rail

| Net | From | To |
| --- | ---- | -- |
| `+3V3` | U1.VOUT | U3 (ESP32-C3) 3V3 pin, U4 (MAX31855) VIN, U5 (ADS1115) VDD, U6 (TJA1051) VCC, R2 top, R3 top (via MAP supply chain? **No** — R3 top is on `MAP_SIG_RAW`, not `+3V3`. See MAP section), C3+, C5/C6/C7 pin 1, LED1 anode via R5 |

### 5 V rail

| Net | From | To |
| --- | ---- | -- |
| `+5V` | U2.VOUT | J3.1 (MAP V+), C4+ |

### MCU bus connections (ESP32-C3 SuperMini)

Use the SuperMini pinout — pins numbered as silkscreen labels (`G6` =
GPIO6, etc.). Map pin physical positions per the SuperMini reference
diagram (not all GPIOs are on every header position).

| Signal | ESP32-C3 pin | Goes to |
| ------ | ------------ | ------- |
| `SPI_SCK` | GPIO4 | U4 (MAX31855) CLK |
| `SPI_MISO` | GPIO5 | U4 (MAX31855) DO |
| `MAX_CS` | GPIO10 | U4 (MAX31855) CS |
| `I2C_SDA` | GPIO6 | U5 (ADS1115) SDA |
| `I2C_SCL` | GPIO7 | U5 (ADS1115) SCL |
| `CAN_TX` | GPIO0 | U6 (TJA1051) TXD |
| `CAN_RX` | GPIO1 | U6 (TJA1051) RXD |
| `+3V3` | 3V3 pin | `+3V3` rail |
| `GND` | GND pin (any of two) | `GND` |
| `5V` (USB) | 5V pin | **No-connect** (we don't power the C3 from this rail; it runs off 3.3 V or USB during dev) |
| All other GPIOs | — | No-connect |

### MAX31855 breakout (U4)

| Pin | Net | Notes |
| --- | --- | ----- |
| VIN | `+3V3` | The Adafruit breakout has a 3.3 V LDO; tying VIN to 3.3 V powers the chip directly via the bypass jumper. Alternatively tie 5V available — we use 3.3 V for simplicity. |
| 3Vo | NC | Internal LDO output; not used. |
| GND | `GND` | |
| DO | `SPI_MISO` | |
| CS | `MAX_CS` | |
| CLK | `SPI_SCK` | |
| T+ / T− | On-board screw terminal | K-type thermocouple connects here directly. |

> Cold-junction note: keep U4 thermally isolated from the buck regulators
> (no copper bridges, 5 mm setback). The breakout's onboard chip is the
> cold-junction reference — gradients across it bias EGT readings.

### ADS1115 breakout (U5)

| Pin | Net | Notes |
| --- | --- | ----- |
| VDD | `+3V3` | |
| GND | `GND` | |
| SCL | `I2C_SCL` | I²C pull-up to 3.3 V via the breakout's onboard 10 kΩ resistors — no external pull-ups needed. |
| SDA | `I2C_SDA` | |
| ADDR | `GND` | Selects address `0x48`. |
| ALRT | NC | |
| A0 | `NTC_TAP` | Coolant thermistor divider tap |
| A1 | `MAP_TAP` | MAP voltage divider tap |
| A2 | NC | |
| A3 | NC | |

### TJA1051 transceiver breakout (U6)

Generic HW-194-style module pinout (verify against the exact breakout
you order; the SOIC-8 pinout itself is fixed).

| Pin | Net | Notes |
| --- | --- | ----- |
| VCC | `+3V3` | The /3 variant accepts 3.3 V on its I/O side. Some breakouts have a separate VIO pin — tie VIO to 3.3 V. |
| GND | `GND` | |
| TXD | `CAN_TX` | |
| RXD | `CAN_RX` | |
| CANH | `CAN_H` → J1.3 via 120 Ω term jumper network | |
| CANL | `CAN_L` → J1.4 via 120 Ω term jumper network | |
| S (silent / standby) | `GND` | Normal mode. |

### CAN bus + termination

| Net | From | To |
| --- | ---- | -- |
| `CAN_H` | U6 CANH | J1.3, JP1.1 |
| `CAN_L` | U6 CANL | J1.4, R1 pin 2 |
| (term path) | JP1.2 | R1 pin 1 |

JP1 shorts `CAN_H` through R1 to `CAN_L`, enabling 120 Ω termination
when the shunt is installed. Pull the shunt to remove termination
(useful if you ever multi-drop a third node).

### NTC coolant network

| Net | From | To |
| --- | ---- | -- |
| `+3V3_NTC` | `+3V3` | J2.1 (NTC connector pin 1) |
| `NTC_TAP` | J2.2 (NTC connector pin 2) | R2 top, C8 pin 1, U5 A0 |
| `GND` | R2 bottom, C8 pin 2 | `GND` |

External NTC sits between `+3V3` and `NTC_TAP`. R2 (10 kΩ) sits between
`NTC_TAP` and `GND`. This matches the firmware's
`R_NTC = R_series * (3.3 / V_tap − 1)` formula in
`src/engine_bay/sensors.cpp`.

### MAP sensor network

| Net | From | To |
| --- | ---- | -- |
| `+5V` | U2.VOUT | J3.1 (MAP V+) |
| `MAP_SIG_RAW` | J3.2 (MAP SIG) | R3 top |
| `MAP_TAP` | R3 bottom, R4 top, C9 pin 1, U5 A1 | (junction node) |
| `GND` | J3.3, R4 bottom, C9 pin 2 | `GND` |

R3 + R4 form a 2:1 divider: `MAP_TAP = MAP_SIG_RAW × R4 / (R3 + R4) =
MAP_SIG_RAW / 2`. Sensor's 0.2–4.7 V output becomes 0.1–2.35 V at the
ADC, comfortably inside the ADS1115's 0–3.3 V safe range.

> The firmware constants `MAP_V_ZERO` and `MAP_V_MAX` in
> `src/engine_bay/sensors.cpp` are updated in this PR from 0.2/4.7 to
> 0.1/2.35 to match the divided signal.

### Status LED

| Net | From | To |
| --- | ---- | -- |
| `+3V3` | rail | R5 top |
| (LED) | R5 bottom | LED1 anode |
| `GND` | LED1 cathode | `GND` |

## Connector pinouts (external)

### J1 — Deutsch DT04-4P (harness in)

| Pin | Signal | Direction |
| --- | ------ | --------- |
| 1 | +12 V switched | IN |
| 2 | GND | — |
| 3 | CAN_H | bidir |
| 4 | CAN_L | bidir |

### J2 — Coolant NTC (2-pin)

| Pin | Signal |
| --- | ------ |
| 1 | +3.3 V (from rail) |
| 2 | NTC return → ADC |

### J3 — MAP sensor (3-pin)

| Pin | Signal |
| --- | ------ |
| 1 | +5 V (from on-board buck) |
| 2 | MAP signal (0.2–4.7 V) |
| 3 | GND |
