# KiCad Hardware Designs

Two PCBs:

- **`engine_bay/`** — the 12 V-tolerant board that lives next to the
  sensors. Mounts the ESP32-C3 SuperMini, MAX31855 breakout, ADS1115
  breakout, and TJA1051 CAN transceiver on 0.1″ headers. Adds power
  conditioning (reverse-polarity diode, TVS, polyfuse, dual buck
  regulators for 3.3 V + 5 V rails), CAN bus termination, and the
  signal-conditioning network for the NTC and MAP sensors.

- **`cabin/`** — a small breakout that pairs with the Qualia ESP32-S3.
  Carries only the TJA1051 transceiver, 120 Ω termination jumper, and
  the Deutsch plug. Connects to the Qualia over a short 4-wire flex
  (3.3 V, GND, TWAI_TX, TWAI_RX).

## How to use this

Each board has three deliverables in its directory:

| File | What it is |
| ---- | ---------- |
| `schematic.md` | Net-by-net wiring. Use this as the canonical reference when capturing the schematic in KiCad — every net and every pin is listed. |
| `bom.csv` | The exact parts with KiCad symbol + footprint library references. Import into KiCad's symbol-to-footprint assigner (`cvpcb`). |
| `notes.md` | Placement strategy, routing constraints, manufacturing notes, design checks. |

Shared design rules — trace widths, copper pours, layer stack, clearances,
DRC settings — live in [`DESIGN_RULES.md`](DESIGN_RULES.md).

## KiCad workflow

1. `File → New → Project` inside each board's directory. Name the project
   `junkr-engine-bay` and `junkr-cabin` respectively.
2. In Schematic Editor: add symbols per `schematic.md`. Use only symbols
   from KiCad's stock libraries (referenced by name in the BOM) — no
   custom libraries needed for revision 1.
3. Run ERC. Resolve any unconnected pin warnings using the “No-connect”
   flag where the BOM marks pins as NC.
4. Open `cvpcb` (Tools → Assign Footprints), assign per the BOM's
   `footprint` column.
5. Push to PCB Editor (`Tools → Update PCB from Schematic`).
6. Apply board outline + mounting hole positions per `notes.md`.
7. Place components per the zones described in `notes.md`.
8. Set design rules per `DESIGN_RULES.md`, then route (manual or
   Freerouting).

## Why not generated `.kicad_sch` files

KiCad's schematic format embeds UUIDs and references that depend on the
local KiCad library paths. Generating them blind tends to produce files
that won't open cleanly. The netlist tables in `schematic.md` carry the
same information in a form you can act on in 20–30 minutes per board in
the KiCad GUI, with much lower risk of broken-project regrets.
