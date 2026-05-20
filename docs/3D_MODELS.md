# 3D Models for Enclosure Design

Where every part's 3D model lives, and how to assemble them into an
enclosure-ready reference.

## TL;DR workflow

You almost certainly **don't want to hand-assemble individual component
models**. Instead:

1. Capture the schematics (`kicad/*/schematic.md`) and lay out the PCBs
   in KiCad.
2. Attach 3D models to the footprints that don't already have one (most
   generic passives/connectors ship with KiCad).
3. `File → Export → STEP` exports the **whole populated board** as a
   single solid — correct outline, connector positions, component
   heights.
4. Import that STEP into your enclosure CAD (Fusion / Onshape / FreeCAD)
   and build the box around it.

The per-component models below matter mostly because they populate the
KiCad board before you export it. The two boards that need standalone
models — because they mount *outside* a PCB you're designing (the Qualia
display module, and the Deutsch panel connectors) — are called out.

## What's vendored in this repo

Under [`../hardware/3d_models/`](../hardware/3d_models/), MIT-licensed
from Adafruit (see `ATTRIBUTION.md`):

| File | Part | Use |
| ---- | ---- | --- |
| `ads1115_pid1085.step` | ADS1115 breakout | Drop onto its footprint, or reference for breadboard-stage clearances. |
| `qualia_esp32s3_pid5800.step` | Qualia ESP32-S3 | **Cabin display bezel** — this is the model you build the dash pod around. |

## What you need to download (not redistributable here)

| Component | Source | Format | Notes |
| --------- | ------ | ------ | ----- |
| **Deutsch DT04-4P / DT06-4S** | [te.com](https://www.te.com/en/search.html?q=DT04-4P) → product page → "3D" download | STEP | **Critical** — needed for the harness panel cutout on both enclosures. TE gates these behind a free login. |
| **MAX31855 breakout (PID 269)** | Not in Adafruit's CAD repo. [SnapEDA](https://www.snapeda.com/search/?q=MAX31855) or model the outline (25.4 × 17.8 mm, ~3 mm tall + header). | STEP | Or use the bare MAX31855 SOIC-8 model from the manufacturer + a generic protoboard. |
| **ESP32-C3 SuperMini** | [GrabCAD](https://grabcad.com/library?query=esp32-c3%20supermini) (free account) | STEP / STL | Several community models; verify dimensions against your board (~22.5 × 18 mm). |
| **TJA1051 module** | [GrabCAD](https://grabcad.com/library?query=can%20transceiver%20module) or model the outline | STEP | Cheap generic module; outline modeling is fast if you can't find the exact one. |
| **Pololu D24V5F3 / D24V5F5** | [Pololu resources](https://www.pololu.com/product/2842/resources) — dimensional drawing; STEP via their CAD links or model the 0.4″ × 0.5″ × 0.4″-tall block | STEP / PDF | Tiny; modeling the bounding block is often enough for clearance. |
| **MAP sensor** | Manufacturer site or [GrabCAD](https://grabcad.com/library?query=map%20sensor) for GM/Bosch/MPX parts | STEP | Depends on the part you pick in Bench Stage 6. |
| **K-type thermocouple / compression fitting** | [McMaster-Carr](https://www.mcmaster.com/) — search the probe or fitting, "Product Detail" → CAD download | STEP | McMaster CAD is excellent and free, but login-gated and not scriptable. |
| **NTC coolant sensor** | Manufacturer (varies by part) | STEP / STL | Often only a dimensional drawing exists; model the threaded body. |

## What KiCad already ships (no download needed)

These footprints have 3D models bundled with KiCad's standard library —
they'll appear in the board STEP export automatically:

- 0805 / 1206 resistors and capacitors
- SMA (SS34) and SMB (SMBJ24A) diodes
- Electrolytic caps
- 0.1″ pin headers and sockets
- JST-XH connectors
- M3 mounting holes (no model, but a hole in the board)
- LEDs

## Hardware from McMaster-Carr (for mounting + mechanical)

McMaster is the right source for the *mechanical* bill — and its STEP
files are first-rate. Grab these when you design the enclosures (login
required, downloads are manual):

| Item | McMaster search |
| ---- | --------------- |
| M3 standoffs (board mounting) | "M3 standoff" |
| M3 heat-set inserts (for plastic enclosures) | "heat-set insert M3" |
| M3 socket-head screws | "M3 socket head screw" |
| EGT weld bung / NPT fitting | "thermocouple fitting" / "weld bung" |
| Enclosure raw stock (if machining) | "aluminum box" / "ABS sheet" |
| Cable glands (harness entry) | "cable gland PG7" |

## Network note

This repo's automated tooling could only fetch the publicly,
permissively-licensed Adafruit models (GitHub-hosted, MIT). TE
Connectivity and McMaster-Carr block automated access and prohibit
redistribution, so those must be downloaded manually from their sites —
the links above take you straight to the right search.
