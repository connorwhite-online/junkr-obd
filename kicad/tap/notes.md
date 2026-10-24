# ECU Tap Board — Layout and Design Notes

## Board

- 60 × 40 × 1.6 mm, 4 layers: signal, GND, power, signal. Four Ø2.7 mm
  holes at 54 × 34 mm centres for M2.5 self-tappers into the KZI-104
  bosses.
- Parts height is 11 mm or less on top. The box gives 15.6 mm inside:
  3 standoff + 1.6 PCB + 11 parts. Nothing goes on the bottom except
  0603 passives.
- Shared rules (trace widths, clearances, DRC) are in
  [`../DESIGN_RULES.md`](../DESIGN_RULES.md).

## Placement zones

```
   front wall: 3 tap-harness inlets                      right wall: USB-C
 ┌──────────────────────────────────────────────────────────────────┐
 │ J1 (E5)        J2 (E6)          J3 (E8)                          │
 │ ┌────────────┐ ┌──────────────┐ ┌──────────────┐ ┌─────────────┐│
 │ │ VR: U13/14 │ │ ANALOG       │ │ DIG/CAP RC   │ │ ESP32-S3    ││
 │ │ DIG RC     │ │ U7–U10, U4,  │ │ U15–U21      │ │ WROOM-1     ││
 │ │            │ │ U5, U6       │ │ U22–U25      │ │ antenna ──► ││
 │ └────────────┘ └──────────────┘ └──────────────┘ │ over edge   ││
 │ POWER: D1 D2 C1 U1 U2 (by J3.1)   SD J7  CAN U26 J5 └───────────┘│
 └──────────────────────────────────────────────────────────────────┘
```

- **Analog** sits under J2 with its own AGND pour on the top layer. It
  joins GND only at NT1, under U4. No digital trace crosses it. The SPI
  lines enter from the ESP32 side.
- **Power** goes next to J3 pin 1, so the +B current never crosses the
  analog area. The E01 return (J1 pin 1) runs on a wide trace to the
  same corner.
- **ESP32-S3**: the antenna hangs 5 mm or more past the board edge, toward
  the cabin. Espressif's keep-out applies: no copper on any layer under
  it. The box wall there is plastic.
- **VR**: route each NE±, TDC± and SP2± pair as a tight differential
  pair from J1 to U13/U14, kept away from the SPV/TCV traces.
- **CAP-HV** (SPV, TCV): their zeners and 10 kΩ go at J1, so the flyback
  energy is dumped at the connector and not across the board.

## PH2 parts and fit

- Five TQ2 relays (about 14 × 9 mm each), two MCP4728, two MCP6004,
  PCA9554, PCA9306 and U31 add roughly 900 mm² of footprint.
  **[VERIFY at layout]** whether they fit on the 60 × 40 board next to
  Phase 1. If not, put PH2 on a 60 × 40 mezzanine, stacked on two 2×5
  1.27 mm headers carrying `+5V`, `+3V3`, GND, I²C, ARM, KICK and the
  five readback nets. Phase 1 boards then ship without it.
- **J4 opens the intercepted lines when unplugged.** The NC relay
  contacts keep the engine stock only while the pigtails are connected.
  Use a latching housing. Keep a stock jumper plug (pins 1–2 … 9–10
  bridged) in the glovebox, and fit it whenever the board is out.
- **THF is a thermistor input.** The ECU pulls it up internally (value
  unknown, probably 2–4 kΩ). While armed, the DAC buffer works against
  that pull-up through the 1 kΩ, so the ECU sees a mix of the two. The
  readback (ADC2 CH6) reads the actual ECU-side node, and firmware
  closes the loop on it. Check that the MCP6004 can sink the pull-up
  current, 2.5 mA at the worst case: **[VERIFY]** on the bench with the
  sensor simulator.
- **Firmware disarm rules (for `src/tap/`):** drop ARM if any readback
  differs from its commanded value by more than 5 %, if +B goes under
  10 V, or if the engine stops (no NE edges for 500 ms).

## Bring-up order

1. Power only: populate D1–D3, U1, U2. Feed 12 V on J3.1 through a
   current-limited supply. Check `+5V` and `+3V3`, and check that USB
   alone leaves `+5V` dead.
2. ESP32 and USB: flash a blink build.
3. ANA: fit U4–U10. Use the sensor simulator
   ([`docs/BENCH_TESTING.md`](../../docs/BENCH_TESTING.md)) with a 47 kΩ
   in each lead. Compare the counts with `frontend_check.py`.
4. DIG and CAP: drive 0 V, 0.8 V, 4.5 V and 14.4 V through 47 kΩ. Check
   the 74HC165 bits and the MCPWM capture.
5. VR: signal generator, 0.2–50 V p-p differential.
6. Only then the car, with the tap harnesses plugged in and the engine
   off first. Check that every ECU-side voltage is unchanged with the
   board plugged in and unplugged.

## Open items

- MAX9926 pinout, output type and input network, from the datasheet.
- Pico-Clasp 2×13 and 2×5 header, housing and terminal part numbers,
  plus a KiCad footprint. The 14 × 4 mm tap slot takes 21 leads of
  0.15 mm² in two layers, which is tight but fits. Widen it to 16 × 4 on
  the next CAD rev if the leads bind.
- PTC F1: a 1812 part (0.5 A hold, 30 V or more) inside the chamber.
  Check that it fits next to the E8-12 splice.
- MCP4728 second address (factory option or reprogramming).
- 74LVC1G123 timing constant at 3.3 V for R508/C501.
