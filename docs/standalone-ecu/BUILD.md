# Standalone ECU: build guide

From files to a board in the car. Read [DESIGN.md](DESIGN.md) first and
follow [BENCH_TEST.md](BENCH_TEST.md) after every step that powers it.

## 1. Order the board

Files are in `kicad/standalone_ecu/fab/`:

| File | For |
| --- | --- |
| `junkr_ecu_gerbers.zip` | PCB: 4 layers, 200 × 140 mm, 1.6 mm, 1 oz outer / 0.5 oz inner, ENIG (the header and tails are press-fit sensitive, ENIG keeps the holes flat) |
| `bom.csv` | assembly BOM: Comment, Designator, Footprint, MPN |
| `cpl.csv` | pick and place, top side only |

Have the fab assemble everything marked `SMT` in `bom.csv`. Leave the
`hand solder` lines (relays, the TE header, the tails, JST/pin headers):
they are through-hole and cheaper and better done by hand.

The board is 200 × 140 mm, so expect fab assembly to cost more than the
parts. The STM32H743, ESP32-S3 module, DRV8803 and MAX9926 are the long
lead items; check stock before ordering.

## 2. Hand-solder the through-hole parts

Order matters, low to tall, so the board sits flat on the bench:

1. JST RUN switch connector (J11), UPDI header (J12), SWD (J10 if not
   assembled).
2. Micro-Fit AUX (J7).
3. Relays K1-K7: G5LE (K1-K3) and G5V-2 (K4-K7). Check orientation against
   the silkscreen, coil pins are not symmetric.
4. The TE 176122-6 harness header (J1). Clamp it flat to the board edge
   before soldering, then solder the two end pins, check it is square,
   then the rest. 64 pins, take your time, no bridges.
5. Tails J2-J4 (only for shadow mode): the rev C tail pigtails from the
   interposer project, which carry the stock ECU's side.

Wash flux off, especially around the MAX9926 and the analog front end:
flux residue at 47k impedance shows up as sensor error.

## 3. Print the enclosure

`hardware/standalone_ecu/tray.stl` and `lid.stl`, SLS or MJF PA12 (heat
under the dash, and nylon takes the heat-set inserts). FDM PETG is fine for
a fit check.

1. Press four M3 × 5 heat-set inserts into the tray standoffs.
2. Board in, header facing the window.
3. Lid on, four M3 × 30 screws through the lid bosses into the inserts.
   The bosses clamp the board.

## 4. Flash

Everything is flashed over the one USB-C port:

1. `pio run -e ecu` builds the STM32 image, `pio run -e ecu-safety` the
   ATtiny image.
2. The ESP32 firmware carries both images and flashes the STM32 through
   its ROM bootloader (BOOT0 + NRST + USART1) and the ATtiny over UPDI.
   J10 (SWD) and J12 (UPDI) are fallbacks for a bricked part.

## 5. Bench test

Do stages 1-4 of [BENCH_TEST.md](BENCH_TEST.md) on the bench before the
board goes anywhere near the car.

## 6. Install (shadow mode)

1. Battery negative off.
2. Unplug E5, E6 and E8 from the stock ECU.
3. Plug them into this board's header. Plug the stock ECU into the tails.
4. Route the RUN switch lead to the dash. Start with it **off**.
5. Battery on, key on. The stock ECU must behave exactly as before.

From here, stage 5 onwards in [BENCH_TEST.md](BENCH_TEST.md).

## Tools

Fine-tip iron with a chisel tip for the header, flux, IPA, a multimeter,
a 2-channel scope (current clamp or differential probe for the SPV
stage), a current-limited bench supply, and a spare SPV or a 1.5 Ω 50 W
load resistor.
