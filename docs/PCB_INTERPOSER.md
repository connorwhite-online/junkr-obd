# PCB Interposer (proposal, rev A)

One thin board instead of three printed "plug + chamber + shroud" stacks.
The board files are in [`kicad/interposer/`](../kicad/interposer/), and the
tray and lid are in
[`hardware/ecu_interposer/cad/pcb_interposer.scad`](../hardware/ecu_interposer/cad/pcb_interposer.scad).

![Assembled, seen from the ECU side](pcb-interposer/assembly_ecu_side.png)

## What the "spool" in the old render is

The grey box in the middle of each printed interposer is the **chamber**.
In that design every pin gets its own 40 mm wire: a crimped female
terminal on the ECU end and a crimped male tab on the harness end. The
chamber holds all of that coiled wire, plus the 12 tap splices with their
resistors. For E6 + E8 that is 38 wires, 76 crimps and 12 splices, and
two printed connector shells that have to hold OEM tolerances.

## The PCB version

```
 OEM harness plugs ==> TE 176122-6 header ==board== OEM female plugs ==> ECU
  (unchanged)          (the ECU's own part)   │      (20 mm tails)
                                              └─ 12 dividers ─ 2x7 header ─▶ ESP32-S3
```

- **Harness side:** a real **TE 176122-6**, the same right-angle header
  the ECU uses (about $10 to $14, in production). The harness plugs click
  into genuine TE plastic with the real latch, so the FDM/SLS mating
  tolerance question goes away.
- **ECU side:** nobody sells a board-mount female for these terminals, so
  the board takes the three OEM female plugs on short tails. The easiest
  source is a junkyard Toyota ECU harness: cut the three plugs off with
  25 mm of wire and solder the wires in. Nothing printed goes into the ECU.
- **Taps:** both divider resistors for every channel sit on the back of
  the board. A 2x7 header carries 12 safe 0 to 3.3 V signals and ground
  to the DevKitC on Dupont leads. The Dupont-end resistors go away.

| Harness side | Exploded |
| --- | --- |
| ![Harness side](pcb-interposer/assembly_harness_side.png) | ![Exploded](pcb-interposer/exploded.png) |

![Board and tails, lid off](pcb-interposer/board_ecu_side.png)

## Compared with the printed interposer

| | Printed (E6 + E8) | PCB (all three plugs) |
| --- | --- | --- |
| Mating plastic, harness side | printed copy of the header | **real TE 176122-6** |
| Mating plastic, ECU side | printed plug, resin | **OEM plugs** (junkyard or TE) |
| Crimps and splices | 76 crimps, 12 splices | 0 crimps with junkyard plugs (64 with new TE housings), 64 solder joints |
| Divider resistors | 12 in splices, 12 at the Dupont ends | 24 on the board |
| Size | two stacks about 60 mm long, plus a tap box | one 130 × 46 × 24 mm board in a tray |
| Harness plugs move back by | about 60 mm | about **51 mm** |
| Plugs covered | E6, E8 | **E5, E6, E8** (E5 passes straight through, ready for later taps) |
| Rough cost | $158 print quote + terminals | **about $60 to $100** |

Cost for the PCB version: board about $15 to $30 for five from JLCPCB
(2 oz copper) plus shipping. Header $10 to $14. Junkyard pigtail $20 to
$40. Resistors and header $5. Tray and lid printed at home in PETG, or
about $15 to $25 in SLS PA12.

## What it costs in quality, honestly

- **It's one piece over all three plugs.** The header spans E5, E6 and E8,
  so E5 has to pass through too, including the spill valve and timing
  valve drives and the power grounds. It is still a passive copper path
  with no parts in it, but it adds 128 solder joints to wires the engine
  needs. Every joint gets a continuity check before the engine starts.
  If you'd rather leave E5 alone, the header can be sawn through the
  wall between E5 and E6, and a shorter board done. That's not drawn yet.
- **Power pins.** The 070 pins carry ECU power, grounds and the SPV/TCV
  drives. Their traces are 1.4 mm on both layers where there's room, and
  0.5 to 0.8 mm per layer for 3 to 12 mm where they pass between pads.
  With 2 oz copper that's fine for the ECU's few amps of current. The
  peak SPV current isn't measured yet [VERIFY].
- **The ECU-side plugs float on their tails.** That's on purpose: each
  plug finds its own cavity in the ECU, the way a harness plug does.
- **Depth.** It's still about 51 mm. Most of that is the two plastic
  connectors, which are OEM size. The harness needs about 5 cm of slack,
  the same as before.

## Open checks before ordering

1. Check the 176122-6 pad numbering against a real header with a meter
   (see [`kicad/interposer/README.md`](../kicad/interposer/README.md)).
2. Measure an OEM plug's length and the ECU face around the header. The
   board stops about 4 mm short of the header mouth.
3. Choose where the ECU-side plugs come from: a junkyard pigtail
   (recommended) or new TE housings plus 64 receptacles.
4. Decide whether E5 passes through (as drawn) or the header is cut.

Side view, harness on the left and ECU on the right:

![Side](pcb-interposer/side.png)
