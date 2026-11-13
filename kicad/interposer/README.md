# PCB interposer (rev B, draft)

One small 2-layer board, 97 × 46 mm, that plugs in between the ECU and the
**E6 + E8** harness plugs, with the ESP32 soldered on. E5 stays plugged
straight into the ECU. It replaces the printed plug + chamber + shroud
stacks, the tap box and the Dupont leads. The why and the trade-offs are
in [`docs/PCB_INTERPOSER.md`](../../docs/PCB_INTERPOSER.md).

| Top (harness edge up) | Back (mirrored) |
| --- | --- |
| ![top](img/board_top.png) | ![back](img/board_bottom.png) |

- **J1: TE 176122-6**, the ECU's own right-angle header, sawn through the
  wall between E6 and E5. The E6 and E8 harness plugs click into it.
- **J2, J3: tail pads** for the 20 to 25 mm wires of the E8 and E6 female
  plugs that go into the ECU. Upper-row tails land on the row nearer the
  header.
- **38 pass-throughs.** Every header pin runs straight to its tail pad on
  both copper layers. Nothing else touches 27 of them.
- **11 dividers on the back.** Each is a 100k top resistor and a bottom
  resistor of 82k, 18k, 27k or 150k, with an optional 10 nF cap.
- **U1: Seeed XIAO ESP32-S3** on its two 1x7 headers at the left end. Its
  USB-C faces out: a car USB charger powers it, and the same cable flashes
  it. TAP_GND is the XIAO's ground and never connects to the ECU's grounds.

## U1 pinout

| XIAO pin | GPIO | Signal | | XIAO pin | GPIO | Signal |
| --- | --- | --- | --- | --- | --- | --- |
| D0 | 1 | PIM (E6-2) | | D7 | 44 | SP1 (E8-9) |
| D1 | 2 | THW (E6-4) | | D8 | 7 | VC (E6-1) |
| D2 | 3 | THA (E6-3) | | D9 | 8 | +B (E8-12) |
| D3 | 4 | THF (E6-5) | | D10 | 9 | W (E8-5) |
| D4 | 5 | THO (E6-13) | | 3V3 | | not used |
| D5 | 6 | VA (E6-11) | | GND | | TAP_GND |
| D6 | 43 | TAC (E8-18) | | 5V | | not used (USB powers it) |

All the analog channels are on ADC1 (GPIO 1 to 9), which still works with
Wi-Fi on. TAC and SP1 are pulse inputs, so they don't need an ADC pin.
GPIO 43/44 are the XIAO's UART pins; the console stays on USB.

The XIAO has 11 GPIO, so **E2 (E6-9, sensor ground) isn't tapped** any
more. Sensors are read against TAP_GND instead. The DevKitC GPIO table in
[`docs/BUILD_STAGES.md`](../../docs/BUILD_STAGES.md) doesn't apply to this
board; tap firmware should use the table above (no tap firmware reads
pins yet).

## Files

| File | What |
| --- | --- |
| `gen_board.py` | Builds the board: outline, footprints, nets, the 38 locked pass-throughs, the divider cells, the XIAO |
| `route.py` | Routes the 22 tap connections (a small two-layer maze router), pours TAP_GND, writes `interposer.kicad_pcb` and the footprint library |
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
The only warnings left are footprint-vs-library mismatches and silk at the
harness edge, where the header's mouth line runs along the board edge.

## Copper

- **070 pins** (power, grounds): 1.4 mm on both layers for the upper row.
  The lower row is 0.5 to 0.8 mm per layer between pads, a few mm long.
  Order **2 oz** copper.
- **040 pins**: 1.0 mm (upper) and 0.6 mm (lower) on both layers.
- **Clearance**: 0.2 mm minimum, 0.25 mm on the pass-throughs.

## Before ordering: check these

1. **Pad numbering.** The rusEFI footprint numbers E8 = 1 to 22 and
   E6 = 23 to 38, with the far row as the upper cavity row. That matches
   the Toyota numbering in `pinout_kzj95_at.csv`, but check it on a real
   header with a meter. It only moves which pins the dividers sit on; the
   pass-throughs are correct either way.
2. **Sawing the header.** Cut through the 1.48 mm wall between the E6 and
   E5 cavities with a fine razor saw, and pull the 26 E5 pins first. Check
   the E6 cavity wall stays whole so the plug's latch still holds.
3. **176122-6 board pegs.** Drawn as 3.5 mm unplated holes, copied from
   the rusEFI footprint.
4. **XIAO footprint.** Rows 15.24 mm apart, pin 1 (D0) next to the USB-C
   on the top row. Check against the Seeed drawing.
5. **Clearance at the ECU face.** The board stops about 4 mm short of the
   ECU's header mouth with an OEM-length plug. Measure the plug and the
   ECU face.
6. **Print one 1:1 paper copy** of `img/board_top.png` and lay a real
   header and XIAO on it.
