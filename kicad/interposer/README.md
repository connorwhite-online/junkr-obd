# PCB interposer (rev C, draft)

One 2-layer board that plugs in between the ECU and all three harness
plugs (E5, E6, E8), with an ESP32-S3-DevKitC-1 soldered on. Solder it on
the bench, plug it in, plug a USB-C cable into the DevKitC. It replaces the
printed plug + chamber + shroud stacks, the tap box and the Dupont leads.
The why and the trade-offs are in
[`docs/PCB_INTERPOSER.md`](../../docs/PCB_INTERPOSER.md).

| Top (harness edge up) | Back (mirrored) |
| --- | --- |
| ![top](img/board_top.png) | ![back](img/board_bottom.png) |

- **J1: TE 176122-6**, the ECU's own right-angle header, used whole. All
  three harness plugs click into it.
- **J2 to J4: tail pads** for the 150 mm wires of the E8, E6 and E5
  female plugs that go into the ECU. Upper-row tails land on the row
  nearer the header.
- **64 pass-throughs.** Every header pin runs straight to its tail pad on
  both copper layers. Nothing else touches 47 of them, including the
  spill valve and timing valve drives (SPV, TCV), which are never tapped.
- **17 dividers on the back.** Each is a 100k top resistor and a bottom
  resistor of 82k, 18k, 27k or 150k, with an optional 10 nF cap.
- **U1: ESP32-S3-DevKitC-1 N16R8** on its own pin headers, in the L-shaped
  end of the board. The antenna end has no copper under it. The USB end
  sticks out next to the harness plugs, so a cable plugs straight in.
  TAP_GND is the ESP32's ground and never connects to the ECU's grounds.
- **Board:** 158.5 × 63 mm overall, L-shaped (the harness plugs sit in the
  notch), 2 oz copper.

## U1 pinout

The 12 stage 1 channels keep the GPIO map from
[`docs/BUILD_STAGES.md`](../../docs/BUILD_STAGES.md).

| Pin | Signal | Bottom R | GPIO | | Pin | Signal | Bottom R | GPIO |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| E6-2 | PIM (boost) | 82k | 1 | | E8-12 | +B (battery) | 18k | 9 |
| E6-4 | THW (coolant) | 82k | 2 | | E8-5 | W (check engine) | 27k | 3 |
| E6-3 | THA (intake air) | 82k | 4 | | E8-18 | TAC (rpm) | 27k | 11 |
| E6-5 | THF (fuel temp) | 82k | 5 | | E8-9 | SP1 (speed) | 150k | 12 |
| E6-13 | THO (ATF temp) | 82k | 6 | | E5-1 | SL (lock-up solenoid) | 27k | 13 |
| E6-11 | VA (pedal) | 82k | 7 | | E5-2 | S1 (shift solenoid 1) | 27k | 14 |
| E6-1 | VC (5 V ref) | 82k | 8 | | E5-15 | S2 (shift solenoid 2) | 27k | 15 |
| E6-9 | E2 (sensor ground) | 82k | 10 | | E5-7 | L4 (transfer low) | 27k | 16 |
| | | | | | E5-8 | TFN (transfer neutral) | 27k | 17 |

GPIO 1 to 10 are ADC1, which still works with Wi-Fi on. The E5 inputs are
on/off (0 or about 12 V); the 27k bottom keeps them under 3.3 V. Gear
comes from S1 + S2, lock-up from SL. The crank and pump speed sensors
(NE, TDC) pass through but aren't tapped: they need a VR conditioner.

## Files

| File | What |
| --- | --- |
| `gen_board.py` | Builds the board: outline, footprints, nets, the 64 locked pass-throughs, the divider cells, the DevKitC |
| `route.py` | Routes the 35 tap connections (a small two-layer maze router), pours TAP_GND, writes `interposer.kicad_pcb` and the footprint library |
| `export_scene.py` | Writes `hardware/ecu_interposer/cad/pcb_board_data.scad` for the 3D renders and the tray/lid CAD |
| `interposer.kicad_pcb` / `.kicad_pro` | The board (KiCad 7). It has no schematic: the netlist is the pinout CSV plus the table above |
| `junkr_interposer.pretty/` | Footprints, including the 176122-6 drawn from the measured rusEFI pad positions |
| `gerbers/` | Gerbers and the drill file, ready for JLCPCB/PCBWay (2 oz copper) |
| `bom.csv` | Parts with links |

Rebuild everything:

```sh
cd kicad/interposer
python3 gen_board.py && python3 route.py && python3 export_scene.py
kicad-cli pcb export gerbers -o gerbers/ -l F.Cu,B.Cu,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts interposer.kicad_pcb
kicad-cli pcb export drill -o gerbers/ interposer.kicad_pcb
../../hardware/ecu_interposer/cad/render_pcb.sh
```

DRC (KiCad 7, through `pcbnew.WriteDRCReport`): **0 errors, 0 unconnected.**
The only warnings left are footprint-vs-library mismatches.

## Copper

- **070 pins** (power, grounds, SPV, TCV): 1.4 mm on both layers for the
  upper row. The lower row is 0.5 to 0.8 mm per layer between pads, a few
  mm long. Order **2 oz** copper.
- **040 pins**: 1.0 mm (upper) and 0.6 mm (lower) on both layers.
- **Clearance**: 0.2 mm minimum, 0.25 mm on the pass-throughs.

## Before ordering: check these

1. **Pad numbering.** The rusEFI footprint numbers E8 = 1 to 22,
   E6 = 23 to 38 and E5 = 39 to 64, with the far row as the upper cavity
   row. That matches the Toyota numbering in `pinout_kzj95_at.csv`, but
   check it on a real header with a meter. It only moves which pins the
   dividers sit on; the pass-throughs are correct either way.
2. **DevKitC row spacing** is drawn at 22.86 mm (0.9 in) with pin 1 at the
   antenna end. Measure your DevKitC before ordering.
3. **176122-6 board pegs.** Drawn as 3.5 mm unplated holes, copied from
   the rusEFI footprint.
4. **Clearance at the ECU face.** The board stops about 4 mm short of the
   ECU's header mouth with an OEM-length plug, and the DevKitC's antenna
   end reaches 1 mm past the board edge. Measure the plug and the ECU face.
5. **Print one 1:1 paper copy** of `img/board_top.png` and lay a real
   header and DevKitC on it.
