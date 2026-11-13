# PCB Interposer (proposal, rev C)

One board that plugs in between the ECU and all three harness plugs, with
the ESP32 already on it. Solder it on the bench, plug it into the car,
plug a USB-C cable into it. It tucks behind the glovebox next to the ECU.

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
 E5/E6/E8 harness plugs ==> TE 176122-6 header ==board== OEM female plugs ==> ECU
  (unchanged)               (the ECU's own part)   │      (150 mm tails)
                                                   └─ 17 dividers ─▶ ESP32-S3-DevKitC (on the board)
```

- **Harness side:** a real **TE 176122-6**, the same right-angle header
  the ECU uses (about $10 to $14, in production). The harness plugs click
  into genuine TE plastic with the real latch, so the FDM/SLS mating
  tolerance question goes away.
- **ECU side:** nobody sells a board-mount female for these terminals, so
  the board takes the three OEM female plugs on short tails. The easiest
  source is a junkyard Toyota ECU harness: cut the plugs off with 150 mm of
  wire and solder the wires in. The long tails let the board sit where
  the ECU's header is now, so the stiff harness doesn't move; the ECU
  moves back about 5 cm on its tails instead. Nothing printed goes into the ECU.
- **Taps and ESP32:** 17 dividers and an ESP32-S3-DevKitC-1 sit on the
  board. The 12 stage 1 channels (including E2) plus five from E5: the
  A/T shift solenoids S1 and S2 (gear), the lock-up solenoid SL, and the
  transfer case 4L and neutral switches. No tap box, no Dupont leads.

| Harness side | Exploded |
| --- | --- |
| ![Harness side](pcb-interposer/assembly_harness_side.png) | ![Exploded](pcb-interposer/exploded.png) |

![Board and tails, lid off](pcb-interposer/board_ecu_side.png)

## Why there are so many traces

64 of them are the pass-throughs: every pin has to reach the ECU, so each
header pin gets one short straight trace to its tail pad. That's the same
set of wires a printed interposer would need, just flat. The other 35 are
the taps: 17 pins to their divider, 17 dividers to the ESP32, one ground.

## Compared with the printed interposer

| | Printed (E6 + E8) | PCB rev C (E5 + E6 + E8) |
| --- | --- | --- |
| Mating plastic, harness side | printed copy of the header | **real TE 176122-6** |
| Mating plastic, ECU side | printed plug, resin | **OEM plugs** (junkyard or TE) |
| Crimps and splices | 76 crimps, 12 splices | 0 crimps with junkyard plugs, 128 solder joints |
| ESP32 | DevKitC in a separate box on Dupont leads | **DevKitC soldered on the board** |
| Channels | 12 | **17** (adds gear, lock-up, 4L, transfer neutral) |
| Size | two stacks about 60 mm long, plus a tap box | one 158 × 63 mm L-shaped board in a tray, about 24 mm tall |
| Harness plugs move back by | about 60 mm | **0 mm** (the ECU moves back about 51 mm on its tails instead) |
| Rough cost | $158 print quote + terminals + DevKitC | **about $75 to $120** |

Cost: board about $15 to $30 for five from JLCPCB (2 oz copper) plus
shipping. Header $10 to $14. Junkyard pigtail $20 to $40. DevKitC about
$15 to $20. Resistors $3. Tray and lid printed at home in PETG, or about
$20 to $35 in SLS PA12.

## What it costs, honestly

- **Everything the engine needs runs through it.** E5 carries the spill
  valve and timing valve drives (SPV, TCV) and the power grounds. They
  pass straight through on copper with nothing on them, but that's 128
  solder joints on wires the engine needs. Each one gets a continuity
  check before the engine starts.
- **Power pins.** The 070 pins carry ECU power, grounds and the SPV/TCV
  drives. Their traces are 1.4 mm on both layers where there's room, and
  0.5 to 0.8 mm per layer for a few mm where they pass between pads. With
  2 oz copper that's fine for the ECU's few amps. The peak SPV current
  isn't measured yet [VERIFY].
- **Crank and pump speed (NE, TDC) aren't tapped.** They're AC sensor
  signals that need a VR conditioner chip, not a divider. They pass
  through, so a later board revision can add them.
- **The ECU-side plugs float on their tails.** That's on purpose: each
  plug finds its own cavity in the ECU, the way a harness plug does.
- **Depth, and the stiff loom.** The board plus the plugs in a row add
  about 51 mm. With 150 mm tails that goes on the ECU side, not the
  harness side: the board sits where the ECU's header was, the harness
  plugs in without moving, and the ECU moves back about 5 cm on its
  bracket. The renders still show short tails.

## Open checks before ordering

1. Check the 176122-6 pad numbering against a real header with a meter
   (see [`kicad/interposer/README.md`](../kicad/interposer/README.md)).
2. Measure an OEM plug's length and the ECU face around the header. The
   board stops about 4 mm short of the header mouth.
3. Measure your DevKitC's row spacing (drawn at 22.86 mm).
4. Choose where the ECU-side plugs come from: a junkyard pigtail
   (recommended) or new TE housings plus 64 receptacles.

Side view, harness on the left and ECU on the right:

![Side](pcb-interposer/side.png)
