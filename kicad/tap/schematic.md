# ECU Tap Board — Schematic / Netlist

60 × 40 mm board in the tap box (drawing KZI-104). It reads the stock
ECU's signals through the [ECU interposer](../../hardware/ecu_interposer/)
and logs them. It never drives an ECU line in Phase 1. The Phase 2 parts
(DAC, relays) are at the end, marked **PH2**. They can be left unfitted.

The architecture and firmware plan are in
[`docs/ECU_TAP_ARCHITECTURE.md`](../../docs/ECU_TAP_ARCHITECTURE.md).
The pin-by-pin list of what is tapped is in
[`pinout_kzj95_at.csv`](../../hardware/ecu_interposer/pinout_kzj95_at.csv).
Every divider value on this page is checked by
[`frontend_check.py`](frontend_check.py). Run it after changing any of them.

Items marked **[VERIFY]** need a datasheet check before layout.

## Rule one: the ECU must never see the tap

Every tap lead gets a **47 kΩ 1 % resistor at its splice inside the
interposer chamber**, on the interposer and not on this board. A shorted
lead, a dead board or a board at 0 V then loads an ECU line through at
least 47 kΩ. All front-end values below include that 47 kΩ.

There are two exceptions, and both are on E8-12 (+B):

- The **power lead** takes F1 (PTC, 500 mA) at the splice in place of the 47 kΩ.
- The **+B monitor lead** takes the normal 47 kΩ.

BATT (E8-1) is **not tapped**. It is live with the key off, and through
the input clamps it would back-feed the board and drain the battery.

## Block summary

```
 E5 tap harness (J1) ── NE±, TDC±, SP2± ──► MAX9926 ×2 (VR) ──┐
                     ── SPV, TCV (CAP-HV) ──┐                 │
                     ── 12 × DIG ───────────┤                 │
                     ── E01 (power GND)     │                 │
 E6 tap harness (J2) ── 9 × ANA ──► 100k + 10M ─► MCP6004 ─► 3.3k/4.7k ─► MCP3208 ×2 ─SPI─┐
                     ── IDL, VF (DIG) ──────┤                 │            (REF3030 3.0 V) │
                     ── E2 (sensor GND = AGND)                │                            │
 E8 tap harness (J3) ── TAC, SP1 (CAP) ─────┤                 │                            │
                     ── 17 × DIG ───────────┤                 │                            │
                     ── +B mon ─► 56k/22k ──┼─────────────────┼──► MCP3208 ──────────────┤
                     ── +B power ─► D1 ─► D2 TVS ─► U1 buck 5 V ─► D3 ─► U2 LDO 3.3 V   │
                                            ▼                 ▼                            ▼
                         10k + 1M + 5.1 V zener + 22 pF ─► 74LVC14 ×7 ─┬─► MCPWM capture (6)
                                                                       ├─► PCNT (SP2)
                                                                       └─► 74HC165 ×4 ─► GPIO
                                                         ESP32-S3-WROOM-1-N16R8
                                   ├── SDMMC 1-bit ─► microSD (J7)
                                   ├── TWAI ─► SN65HVD230 ─► CAN bus (J5)
                                   ├── USB-C (J6), flash and log download
                                   └── I²C ─► PH2: PCA9554 relay drivers, PCA9306 ─► MCP4728 ×2 DAC
```

## Power tree nets

| Net | Source | Sinks |
| --- | ------ | ----- |
| `+B_TAP` | J3 pin 1 (E8-12 through F1 in the chamber) | D1 anode |
| `+B_PROT` | D1 cathode | D2 (TVS to GND), C1, C2, U1 VIN |
| `+5V` | U1 VOUT | U7–U12 VDD, U13–U14 VCC, REF3030 VIN, U27 VREF2 side, U29–U30 VDD, D3 anode A, Q7 source (PH2), R192 |
| `VBUS` | J6 VBUS | D5 (ESD), D3 anode B |
| `+5V_LDO` | D3 common cathode | U2 VIN, C4 |
| `+3V3` | U2 VOUT | U3, U4/U5 VDD, U15–U25, U26, U27 VREF1 side, U28, U31, J7 VDD, pull-ups |
| `VREF3V0` | U6 OUT | U4/U5 VREF |
| `+5V_COIL` (PH2) | Q7 drain | K1–K5 coils |
| `GND` | J1 pin 1 (E01, E5-13) | Digital and power return, single pour on In2/B.Cu |
| `AGND` | J2 pin 1 (E2, E6-9) | U4/U5 AGND, U6 GND, the ANA 10 MΩ and 4.7 kΩ legs, C101–C114. Joins `GND` once, at NT1 under U4 |

USB power (bench) reaches `+3V3` through D3 only. `+5V` stays dead, so the
analog front ends and the relays are off on USB alone. That is intended:
flashing and log download only.

## Designators (reference list)

| Ref | Part | Notes |
| --- | ---- | ----- |
| F1 | 1812L050/30MR [VERIFY] PTC, 0.5 A hold, 30 V | **In the chamber** at the E8-12 splice, not on this board. Listed so it gets ordered |
| R_SPLICE ×51 | 47 kΩ 1 % 0603 | **In the chamber**, one per tap lead except the +B power lead |
| J1, J2, J3 | Molex Pico-Clasp 2×13, 1.00 mm, vertical [VERIFY PN] | Tap harnesses E5 / E6 / E8. One part number for all three |
| J4 (PH2) | Molex Pico-Clasp 2×5 [VERIFY PN] | Bypass pigtails, H and E per intercepted line |
| J5 | JST GH 4-pin (SM04B-GHS-TB) | CAN bus out through the Ø6.5 gland |
| J6 | USB-C receptacle, GCT USB4105-GF-A | USB 2.0 only |
| J7 | microSD push-push, Hirose DM3AT-SF-PEJM5 | SDMMC 1-bit |
| D1 | SS34 | Reverse polarity, in series |
| D2 | SMBJ24A | Load-dump TVS on `+B_PROT` |
| C1 | 47 µF / 35 V electrolytic, 6.3 mm | `+B_PROT` bulk |
| C2 | 100 nF / 50 V 0805 | `+B_PROT` HF |
| U1 | Pololu D24V10F5 | 5 V 1 A buck module (0.1″ pins). 1 A, not the D24V5F5: Wi-Fi peaks plus PH2 coils reach ~480 mA |
| C3 | 22 µF 1206 | `+5V` bulk |
| D3 | BAT54C (common cathode) | OR of `+5V` and `VBUS` into the LDO |
| U2 | AP2112K-3.3 | 600 mA LDO |
| C4, C5 | 1 µF, 10 µF 0805 | LDO in / out |
| U3 | ESP32-S3-WROOM-1-N16R8 | Antenna over the board edge, keep-out per Espressif |
| C6, C7 | 22 µF 1206, 100 nF | `+3V3` at U3 |
| R1, C8 | 10 kΩ, 1 µF | EN pull-up / RC delay |
| SW1, SW2 | 3 × 4 mm tact | BOOT (GPIO0), RESET (EN) |
| D4, R2 | Green LED 0603, 1 kΩ | Status, GPIO14 |
| R3, R4 | 5.1 kΩ | USB-C CC1/CC2 to GND |
| D5 | USBLC6-2SC6 | USB ESD |
| U4, U5 | MCP3208-CI/SL | 12-bit 8-ch SPI ADC. VDD 3.3 V |
| U6 | REF3030AIDBZR | 3.000 V reference, VIN from `+5V` |
| C9, C10 | 1 µF, 100 nF | REF3030 output. Plus 100 nF at each ADC VREF pin (C11, C12) |
| U7–U10 | MCP6004-I/SL | ANA followers, 5 V. 14 used, 2 spare |
| U11, U12 (PH2) | MCP6004-I/SL | DAC buffers, 5 V. 5 used, 3 spare |
| U13, U14 | MAX9926UAEE+ [VERIFY] | Dual VR interface. 3 channels used, 1 spare |
| U15–U21 | 74LVC14APW | Schmitt inverters. Inputs 5.5 V tolerant, also with VCC = 0 |
| U22–U25 | 74HC165PW | 31 DIG inputs into 3 GPIO |
| U26 | SN65HVD230DR | CAN transceiver, 3.3 V |
| R404, JP1 | 120 Ω, 2-pin jumper | CAN termination, fit only if this node is a bus end |
| R401–R403 | 10 kΩ | SD CMD, D0, D3 pull-ups |
| R101–R114 | 100 kΩ 1 % | ANA series |
| R121–R134 | 10 MΩ 1 % | ANA pull-down to AGND |
| R141–R154 | 3.3 kΩ 1 % | ANA output divider top |
| R161–R174 | 4.7 kΩ 1 % | ANA output divider bottom (to AGND) |
| C101–C114 | 100 nF C0G/X7R 0603 | At each ADC pin |
| R181–R185 (PH2) | 47 kΩ 1 % | On-board splice resistor for the ECU-side readback (ANA 10–14) |
| R190, R191, C190 | 56 kΩ, 22 kΩ 1 %, 100 nF | +B monitor |
| R192, R193, C191 | 10 kΩ, 10 kΩ 1 %, 100 nF | `+5V` monitor |
| R201–R235 | 10 kΩ | DIG / CAP / CAP-HV series |
| R301–R335 | 1 MΩ | DIG / CAP / CAP-HV pull-down |
| D201–D235 | BZX84C5V1 | 5.1 V clamp at the gate input |
| C201–C235 | 22 pF C0G | At the gate input |
| R241–R243 | 10 kΩ | MAX9926 output pull-ups to `+5V` |
| U27 (PH2) | PCA9306DCUR | I²C level shift 3.3 V ↔ 5 V for the DACs |
| U28 (PH2) | PCA9554PW, addr 0x20 | Relay driver gates |
| U29, U30 (PH2) | MCP4728-E/UN, addr 0x60 / 0x61 [VERIFY address option] | 4-ch 12-bit DAC, VDD ref (0–5 V) |
| U31 (PH2) | 74LVC1G123DCT | Retriggerable one-shot: the dead-man timer |
| R508, C501 (PH2) | 200 kΩ, 100 nF C0G/film | U31 timing, ≈ 20 ms [VERIFY K] |
| Q1–Q5 (PH2) | 2N7002 | Relay coil drivers |
| R501–R505 (PH2) | 10 kΩ | Q1–Q5 gate pull-downs (beat the PCA9554 internal pull-ups at power-up) |
| D11–D15 (PH2) | 1N4148W | Coil flyback |
| K1–K5 (PH2) | Panasonic TQ2-5V | DPDT, both poles paralleled |
| R511–R515 (PH2) | 1 kΩ | DAC buffer to relay NO, current limit |
| Q6, R506 (PH2) | 2N7002, 10 kΩ | Coil-rail enable, gate pull-down |
| Q7, R507 (PH2) | AO3401A, 10 kΩ | Coil-rail P-FET, gate pull-up to `+5V` |
| R521–R524 (PH2) | 4.7 kΩ | I²C pull-ups, two on each side of U27 |
| R525 (PH2) | 200 kΩ | U27 EN pull-up to VREF2 |
| NT1 | Net tie | `AGND` to `GND`, under U4 |
| — | 100 nF 0603 at every IC supply pin | Not listed one by one |

## Detailed net-by-net wiring

### Power input

```
J3.1 (+B_TAP) ── D1 A │ D1 K ── +B_PROT
+B_PROT ── D2 K (D2 A ── GND)
+B_PROT ── C1+ (C1− ── GND), C2 ── GND
+B_PROT ── U1 VIN │ U1 GND ── GND │ U1 VOUT ── +5V ── C3 ── GND
+5V ── D3 A1 │ VBUS ── D3 A2 │ D3 K ── +5V_LDO
+5V_LDO ── U2 VIN, U2 EN │ U2 GND ── GND │ U2 VOUT ── +3V3
J1.1 (E01) ── GND (wide trace, ≥ 0.5 mm)
J2.1 (E2)  ── AGND
AGND ── NT1 ── GND
```

U1 leaves its EN pin floating (enabled).

### ESP32-S3 GPIO allocation

| GPIO | Net | Peripheral |
| ---- | --- | ---------- |
| 0 | `BOOT` | SW1 to GND, 10 kΩ pull-up inside the module |
| 1 / 2 | `CAN_TX` / `CAN_RX` | TWAI → U26 D / R |
| 4 | `CAP_NE` | MCPWM0 CAP0 |
| 5 | `CAP_TDC` | MCPWM0 CAP1 |
| 6 | `CAP_SPV` | MCPWM0 CAP2 |
| 7 | `CAP_TCV` | MCPWM1 CAP0 |
| 8 | `CAP_TAC` | MCPWM1 CAP1 |
| 18 | `CAP_SP1` | MCPWM1 CAP2 |
| 21 | `CNT_SP2` | PCNT unit 0 |
| 9 / 10 | `ADC2_CS` / `ADC1_CS` | SPI2 CS (U5 / U4) |
| 11 / 12 / 13 | `SPI_MOSI` / `SPI_SCLK` / `SPI_MISO` | SPI2 → U4, U5 |
| 14 | `LED` | R2 → D4 → GND |
| 15 / 16 / 17 | `SR_LD` / `SR_CLK` / `SR_QH` | 74HC165 chain |
| 19 / 20 | `USB_D−` / `USB_D+` | J6 through D5 |
| 38 / 39 / 40 | `SD_CMD` / `SD_CLK` / `SD_D0` | SDMMC slot 1, 1-bit |
| 41 / 42 | `I2C_SDA` / `I2C_SCL` | PH2: U28, U27 → U29/U30 |
| 47 | `ARM` | PH2: U31 /CLR, 100 kΩ pull-down |
| 48 | `KICK` | PH2: U31 B, toggled by the control loop |
| EN | `EN` | R1 to `+3V3`, C8 to GND, SW2 to GND |

Not used: 35, 36, 37 (octal PSRAM), 3, 45, 46 (strapping), 43, 44 (UART0
console, brought to test pads TP1/TP2).

### ANA front ends (×14)

Per channel *n*, with tap lead **T** (47 kΩ already in the chamber):

```
T ── R10n (100k) ──┬── U7–U10 IN+  (follower: OUT ── IN−)
                   └── R12n (10M) ── AGND
OUT ── R14n (3.3k) ──┬── ADC CHx
                     ├── R16n (4.7k) ── AGND
                     └── C10n (100 nF) ── AGND
```

The ECU line sees 10.15 MΩ: 0.02 % error on a 2 kΩ thermistor pull-up.
A full 5.1 V input reaches the ADC at ≤ 2.98 V, under the 3.0 V reference.
The corner is 821 Hz. Values are checked by `frontend_check.py`.

| n | Signal | Source | Op-amp | ADC |
| - | ------ | ------ | ------ | --- |
| 1 | VA | J2.8 (E6-11) | U7A | U4 CH0 |
| 2 | PIM | J2.3 (E6-2) | U7B | U4 CH1 |
| 3 | THW | J2.5 (E6-4) | U7C | U4 CH2 |
| 4 | THA | J2.4 (E6-3) | U7D | U4 CH3 |
| 5 | THF | J2.6 (E6-5) | U8A | U4 CH4 |
| 6 | VRP | J2.7 (E6-6) | U8B | U4 CH5 |
| 7 | VRT | J2.10 (E6-14) | U8C | U4 CH6 |
| 8 | VC | J2.2 (E6-1) | U8D | U4 CH7 |
| 9 | THO | J2.9 (E6-13) | U9A | U5 CH0 |
| 10 | PIM_E (PH2 readback) | K1 COM via R181 | U9B | U5 CH2 |
| 11 | VA_E (PH2) | K2 COM via R182 | U9C | U5 CH3 |
| 12 | VRP_E (PH2) | K3 COM via R183 | U9D | U5 CH4 |
| 13 | VRT_E (PH2) | K4 COM via R184 | U10A | U5 CH5 |
| 14 | THF_E (PH2) | K5 COM via R185 | U10B | U5 CH6 |

U10C and U10D are spare: IN+ to AGND, OUT to IN−. Without PH2 fitted,
tie the IN+ of n = 10–14 to AGND through R12n alone (leave R181–R185 off).

### Monitors

```
J3.2 (+B mon, 47k in chamber) ── R190 (56k) ──┬── U5 CH1
                                              ├── R191 (22k) ── AGND
                                              └── C190 ── AGND
+5V ── R192 (10k) ──┬── U5 CH7
                    ├── R193 (10k) ── AGND
                    └── C191 ── AGND
```

16 V on +B reads ≤ 2.86 V, and `+5V` at 5.5 V reads 2.75 V.

### ADCs

```
U4/U5: VDD ── +3V3, VREF ── VREF3V0 (+ C11/C12), AGND ── AGND, DGND ── GND
       CLK ── SPI_SCLK, DIN ── SPI_MOSI, DOUT ── SPI_MISO
       U4 /CS ── ADC1_CS (GPIO10), U5 /CS ── ADC2_CS (GPIO9), each with 10 kΩ pull-up
U6:    IN ── +5V, GND ── AGND, OUT ── VREF3V0 ── C9, C10 ── AGND
```

SPI at 1 MHz (the MCP3208 limit at 3.3 V is about 1.2 MHz). 24 clocks per
conversion gives ~40 ksps per chip, against the 8 ksps needed (8 ch × 1 kHz).

### DIG / CAP / CAP-HV front ends (×35)

Per channel *n*, with tap lead **T** (47 kΩ in the chamber):

```
T ── R20n (10k) ──┬── 74LVC14 input
                  ├── R30n (1M) ── GND
                  ├── D20n (BZX84C5V1, K at node) ── GND
                  └── C20n (22 pF) ── GND
```

0.8 V or less on the ECU line reads low, and 4.5 V or more reads high, for
all parts at 1 %. 14.4 V gives 2.4 V at the gate, so no clamp current
flows. A 60 V SPV flyback puts 1.13 mA into the zener. The ECU line sees
1.06 MΩ, so STP draws 0.16 mA. The RC delay is 1.25 µs. The gates invert,
so a `1` in firmware means the line is low.

| Gate | Signal | Source | Output to |
| ---- | ------ | ------ | --------- |
| U15.1 | SPV (CAP-HV) | J1.8 (E5-11) | `CAP_SPV` GPIO6 |
| U15.2 | TCV (CAP-HV) | J1.9 (E5-12) | `CAP_TCV` GPIO7 |
| U15.3 | TAC (CAP) | J3.3 (E8-18) | `CAP_TAC` GPIO8 |
| U15.4 | SP1 (CAP) | J3.4 (E8-9) | `CAP_SP1` GPIO18 |
| U15.5 | NE (from U13 OUT1, no RC network) | — | `CAP_NE` GPIO4 |
| U15.6 | TDC (from U13 OUT2, no RC network) | — | `CAP_TDC` GPIO5 |
| U16.1 | SP2 (from U14 OUT1, no RC network) | — | `CNT_SP2` GPIO21 |
| U16.2–6 | SL, S1, S2, L4, TFN | J1.10–14 | U22 A–E |
| U17.1–6 | S/TH1, S/TH2, L, 2, SNWL, SNW | J1.15–20 | U22 F–H, U23 A–C |
| U18.1–6 | EGR, IDL, VF, S-REL, M-REL, STP | J1.21, J2.11, J2.12, J3.5–7 | U23 D–H, U24 A |
| U19.1–6 | W, OIL-W, G-IND, IMO, A/C, STA | J3.8–13 | U24 B–G |
| U20.1–6 | SVR, IGSW, P, OD2, IMI, ACT | J3.14–19 | U24 H, U25 A–E |
| U21.1–2 | HSW, NSW | J3.20–21 | U25 F–G |
| U21.3–6 | spare | inputs to GND | — |

U25 H is tied to GND (bit 31 always reads 0).

### VR inputs

```
J1.2/J1.3 (NE+/NE−)   ── U13 IN1+/IN1−   OUT1 ── R241 ── +5V, OUT1 ── U15.5 in
J1.4/J1.5 (TDC+/TDC−) ── U13 IN2+/IN2−   OUT2 ── R242 ── +5V, OUT2 ── U15.6 in
J1.6/J1.7 (SP2+/SP2−) ── U14 IN1+/IN1−   OUT1 ── R243 ── +5V, OUT1 ── U16.1 in
U14 IN2± ── tied to the bias pin per the datasheet (unused)
U13/U14 VCC ── +5V, GND ── GND
```

The 47 kΩ at each splice serves as the datasheet's input resistor. Set
the input filter caps, the bias, and the mode pins (adaptive threshold,
internal hysteresis) per the MAX9926 typical application **[VERIFY: pin
names, output type, and whether 47 kΩ on each leg is within its
input-resistor range]**. R241–R243 are harmless if the outputs are push-pull.

### Shift registers

```
U22–U25: VCC ── +3V3, /PL ── SR_LD, CP ── SR_CLK, /CE ── GND
U22 Q7 ── SR_QH (GPIO17)
U23 Q7 ── U22 DS, U24 Q7 ── U23 DS, U25 Q7 ── U24 DS, U25 DS ── GND
```

Firmware reads the chain at 100 Hz: pulse `SR_LD` low, then 32 clocks.

### microSD (J7)

```
J7 VDD ── +3V3 (10 µF + 100 nF at the socket)
J7 CLK ── SD_CLK, CMD ── SD_CMD (R401), DAT0 ── SD_D0 (R402)
J7 DAT3/CD ── R403 ── +3V3 (holds the card in SD mode), DAT1, DAT2 ── 10 kΩ ── +3V3
```

### CAN (J5)

```
U26 VCC ── +3V3, GND ── GND, D ── CAN_TX, R ── CAN_RX, Rs ── GND (high speed)
U26 CANH ── J5.1, CANL ── J5.2, J5.3 ── GND, J5.4 NC
CANH ── R404 ── JP1 ── CANL
```

### USB (J6)

```
J6 VBUS ── VBUS, D+/D− (both rows joined) ── D5 ── GPIO20/19
J6 CC1 ── R3 ── GND, CC2 ── R4 ── GND, shell ── GND through 1 MΩ ∥ 4.7 nF
```

### PH2: bypass relays, DAC and dead-man

One block per intercepted line *k* = 1–5: PIM, VA, VRP, VRT, THF.

```
J4.(2k−1)  (pigtail H, harness side) ── Kk NC1 + NC2
J4.(2k)    (pigtail E, ECU side)     ── Kk COM1 + COM2 ── R18k (47k) ── ANA readback n = 9 + k
DAC ch ── U11/U12 follower ── R51k (1k) ── Kk NO1 + NO2
Kk coil ── +5V_COIL / Qk drain, Dk across the coil (K at +5V_COIL)
Qk gate ── U28 IOk−1, R50k ── GND, Qk source ── GND
```

DAC channels: U29 A–D drive PIM, VA, VRP and VRT, and U30 A drives THF.
U30 B–D are spare.

```
I²C (3.3 V side): I2C_SDA/SCL ── U28 (A0–A2 ── GND, addr 0x20) ── U27 SDA1/SCL1, R521/R522 to +3V3
I²C (5 V side):   U27 SDA2/SCL2 ── U29, U30, R523/R524 to +5V
U27: VREF1 ── +3V3, VREF2 ── +5V, EN ── VREF2 via R525
U29/U30: VDD ── +5V, /LDAC ── GND, RDY/BSY NC, VREF = VDD
```

**Dead-man.** The coil rail is on only while firmware holds `ARM` high
**and** keeps toggling `KICK`.

```
U31: VCC ── +3V3, A ── GND, B ── KICK (GPIO48), /CLR ── ARM (GPIO47, 100 kΩ pull-down)
     Rext/Cext ── R508 ── +3V3, Cext ── C501 ── Rext/Cext node
     Q ── Q6 gate (R506 ── GND)
Q6 drain ── Q7 gate (R507 ── +5V), Q6 source ── GND
Q7 source ── +5V, Q7 drain ── +5V_COIL
```

KICK is toggled by the core-1 control task at 500 Hz in software, not by
a hardware timer, so a hung task stops it. Each rising edge retriggers
U31. With no edge for about 20 ms, Q drops, the coil rail goes off and
every relay falls back to NC (the stock path) within about 2 ms. `ARM`
low clears U31 straight away. This replaces the TPS3823 shown on the
earlier drawings: its 200 ms reset pulse would turn the relays back on
for 1.6 s at a time while the firmware stayed hung.

## Connector pinouts

Tap harness connectors, board side. "47k" means the resistor is at the
splice in the chamber.

### J1 — E5 tap harness (2×13)

| Pin | Net | ECU pin | Pin | Net | ECU pin |
| --- | --- | ------- | --- | --- | ------- |
| 1 | GND (E01, no resistor) | E5-13 | 2 | NE+ 47k | E5-4 |
| 3 | NE− 47k | E5-17 | 4 | TDC+ 47k | E5-5 |
| 5 | TDC− 47k | E5-18 | 6 | SP2+ 47k | E5-3 |
| 7 | SP2− 47k | E5-16 | 8 | SPV 47k | E5-11 |
| 9 | TCV 47k | E5-12 | 10 | SL 47k | E5-1 |
| 11 | S1 47k | E5-2 | 12 | S2 47k | E5-15 |
| 13 | L4 47k | E5-7 | 14 | TFN 47k | E5-8 |
| 15 | S/TH1 47k | E5-10 | 16 | S/TH2 47k | E5-9 |
| 17 | L 47k | E5-20 | 18 | 2 47k | E5-21 |
| 19 | SNWL 47k | E5-22 | 20 | SNW 47k | E5-23 |
| 21 | EGR 47k | E5-24 | 22 | GND (board only) | — |
| 23–26 | NC | — | | | |

Twist each VR pair (NE±, TDC±, SP2±) along the lead.

### J2 — E6 tap harness (2×13)

| Pin | Net | ECU pin |
| --- | --- | ------- |
| 1 | AGND (E2, no resistor) | E6-9 |
| 2 | VC 47k | E6-1 |
| 3 | PIM 47k | E6-2 |
| 4 | THA 47k | E6-3 |
| 5 | THW 47k | E6-4 |
| 6 | THF 47k | E6-5 |
| 7 | VRP 47k | E6-6 |
| 8 | VA 47k | E6-11 |
| 9 | THO 47k | E6-13 |
| 10 | VRT 47k | E6-14 |
| 11 | IDL 47k | E6-12 |
| 12 | VF 47k | E6-8 |
| 13 | AGND (board only) | — |
| 14–26 | NC | — |

On an intercepted line (PH2), the J2 tap comes from the **harness side**
of the split, so it always reads the real sensor.

### J3 — E8 tap harness (2×13)

| Pin | Net | ECU pin | Pin | Net | ECU pin |
| --- | --- | ------- | --- | --- | ------- |
| 1 | +B power (F1, no 47k) | E8-12 | 2 | +B mon 47k | E8-12 |
| 3 | TAC 47k | E8-18 | 4 | SP1 47k | E8-9 |
| 5 | S-REL 47k | E8-2 | 6 | M-REL 47k | E8-3 |
| 7 | STP 47k | E8-4 | 8 | W 47k | E8-5 |
| 9 | OIL-W 47k | E8-6 | 10 | G-IND 47k | E8-7 |
| 11 | IMO 47k | E8-8 | 12 | A/C 47k | E8-10 |
| 13 | STA 47k | E8-11 | 14 | SVR 47k | E8-13 |
| 15 | IGSW 47k | E8-14 | 16 | P 47k | E8-15 |
| 17 | OD2 47k | E8-16 | 18 | IMI 47k | E8-17 |
| 19 | ACT 47k | E8-19 | 20 | HSW 47k | E8-20 |
| 21 | NSW 47k | E8-22 | 22 | GND (board only) | — |
| 23–26 | NC | — | | | |

### J4 — PH2 bypass pigtails (2×5)

| Pin | Net | Pin | Net |
| --- | --- | --- | --- |
| 1 | PIM_H | 2 | PIM_E |
| 3 | VA_H | 4 | VA_E |
| 5 | VRP_H | 6 | VRP_E |
| 7 | VRT_H | 8 | VRT_E |
| 9 | THF_H | 10 | THF_E |

**Unplugging J4 opens the intercepted sensor lines.** The relays only fail
safe while the pigtails are connected. Build a **stock jumper plug**: a
2×5 housing with pins 1–2, 3–4, 5–6, 7–8 and 9–10 bridged. Fit it whenever
the board is out.

### J5 — CAN (JST GH 4)

| Pin | Net |
| --- | --- |
| 1 | CANH |
| 2 | CANL |
| 3 | GND |
| 4 | NC |
