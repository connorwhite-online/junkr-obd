# Shared Design Rules

Apply these in KiCad's **Board Setup → Design Rules** for both boards.

## Layer stack

Both boards are **2-layer FR-4, 1.6 mm, 1 oz copper** (cheapest tier at
JLCPCB / OSH Park / PCBWay). No buried vias, no impedance control.

| Layer | Use |
| ----- | --- |
| F.Cu | Signals + 3.3 V routing |
| B.Cu | GND pour + power return + crossover routing |

## Trace widths

| Net class | Min width | Notes |
| --------- | --------- | ----- |
| Power 12 V | **0.6 mm** (24 mil) | From input pad to TVS, polyfuse, and the 5 V/3.3 V buck regulator inputs. ~3 A worst case. |
| Power 5 V / 3.3 V | **0.4 mm** (16 mil) | All on-board rails. |
| Signal | **0.2 mm** (8 mil) | Default for SPI, I²C, GPIO. |
| CAN_H / CAN_L | **0.25 mm** (10 mil), routed as a coupled pair | Edge-to-edge spacing 0.15 mm, length-match within 5 mm. |

## Clearances

| Class pair | Min clearance |
| ---------- | ------------- |
| Default | 0.2 mm |
| 12 V net to anything else | **0.5 mm** |
| Thermocouple inputs (TC+/TC−) to anything | **1.0 mm** isolation gap (no copper underneath the MAX31855 cold-junction reference area, or under the thermocouple terminal block) |

## Vias

- Via diameter: 0.6 mm
- Drill: 0.3 mm
- Stitching vias every 5 mm along GND pour edges

## Copper pours

- **B.Cu**: full GND pour, 0.5 mm clearance to non-GND copper.
- **F.Cu**: GND pour around sensitive signals (thermocouple lines, MAP
  analog) — use thermal relief connections to GND pins.

## Component clearance

- Keep MAX31855 + its terminal block at least **5 mm** from regulators
  and any large copper area (minimizes thermal gradient that biases the
  cold-junction comp).
- Mounting holes: M3, pad diameter 6.4 mm, drill 3.2 mm, **non-plated**
  (these are mechanical only).

## ERC / DRC

- Enable all default ERC rules.
- Disable “Different netclass on connected net” — Pololu module pins
  intentionally bridge classes.
- Run DRC with **Test fill** and **Check footprint courtyards** on.

## Manufacturing

- Surface finish: HASL (cheap) or ENIG (slightly nicer; required if you
  end up reflowing the chip-down variant later).
- Solder mask: green or whatever; matte black if you want to be cool.
- Silkscreen: white, both sides. Reference designators + value for
  every passive.
- Panelize per supplier rules; nothing on these boards needs to be
  panelized unless you order 10+ at once.
