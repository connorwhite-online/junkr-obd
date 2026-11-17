# Ordering a fully assembled board (JLCPCB turnkey)

You upload three files to JLCPCB and get back boards with every part
soldered on, through-hole relays and the harness header included. The only
thing left for you is the three shadow-mode tails (J2-J4), and you only
need those if you run shadow mode.

Nothing has been ordered. Prices below are estimates from 2026-11-15;
JLCPCB's quote page gives the real number.

## Why JLCPCB

- Does 6-layer boards and assembly in one order, including through-hole parts.
- 90 of the 92 BOM lines are in its own parts library (LCSC), in stock. The
  other two can be bought into the order through its Global Sourcing service.
- The files are already in its format.

PCBWay is the fallback if you would rather they source every part for you
(their "turnkey" sources from Digi-Key/Mouser). It is slower and usually
dearer.

## Files

All in `kicad/standalone_ecu/fab/`:

| Upload as | File |
| --- | --- |
| Gerber | `junkr_ecu_gerbers.zip` |
| BOM | `jlcpcb/bom_jlcpcb.csv` (or `jlcpcb_lean/`) |
| CPL (pick and place) | `jlcpcb/cpl_jlcpcb.csv` (or `jlcpcb_lean/`) |

`bom.csv` is the full BOM with the exact MPN and a buy link for every line.
`tools/sourcing.csv` is where the part choices live; `gen_fab.py`
regenerates all of the above from the board.

## Order settings

1. jlcpcb.com, Order now, upload `junkr_ecu_gerbers.zip`.
2. Layers **6**, 200 x 140 mm (read from the Gerbers), thickness 1.6 mm,
   material FR4 **TG155** (heat under the dash), surface finish **ENIG**,
   outer copper 1 oz, inner 0.5 oz, specify stackup **No** (nothing on the
   board needs controlled impedance), via covering **Epoxy Filled &
   Capped** (JLC's 6-layer default), min via hole/diameter
   **0.2mm/(0.3/0.35mm)** (the smallest vias are 0.2 mm holes, 0.45 mm
   pads; all vias are through-hole), board outline tolerance regular.
3. Quantity 5 (the minimum). **PCB Assembly** on, **Standard** PCBA,
   top side, assemble 2 to 5 of them.
4. Upload `bom_jlcpcb.csv` and `cpl_jlcpcb.csv`.
5. On the parts page:
   - **J1 TE 176122-6** and **K1-K3 Omron G5LE-1-DC12** show 0 stock. Use
     Global Sourcing to have JLCPCB buy them (Mouser stocks both), or
     leave them unselected and solder them yourself (J1 is 64 pins, K1-K3
     are 5 pins each).
   - Leave J2-J4 out. They are the shadow-mode tails, hand-made.
6. On the placement preview, check rotation and pin 1 of the ICs, both
   SOT-23-5/6 parts, the DPAKs, the relays and J1. JLCPCB's footprint
   origins sometimes differ from KiCad's; fix any that are off in the
   preview before you confirm.
7. Pay only after you've checked the preview. JLCPCB's engineers review
   the order and email if anything needs a decision.

## Two upload zips

| Zip | What it is |
| --- | --- |
| `jlcpcb_order_files.zip` | the board as designed (`fab/jlcpcb/`) |
| `jlcpcb_lean_order_files.zip` | cheaper build (`fab/jlcpcb_lean/`): SS310, BAT54S, BZT52C12 and BSS123 on JLCPCB "preferred" parts with the same value, rating and package; all six status LEDs red (a basic part) instead of green/yellow/blue; SWD (J10) and UPDI (J12) headers left off. Flashing is over USB-C either way; J12 is a plain 3-pin header you can add by hand if a chip ever needs rescue |

Both use the same Gerbers. The lean zip saves only about $10 per order,
for the reasons below.

## Where the money goes

Estimate for Standard PCBA (ESP32 modules are generally among JLCPCB's
"Standard only" parts), using JLCPCB's published fees on 2026-11-15
and LCSC part prices at the order quantity. The PCB price is the
shakiest number: JLCPCB doesn't publish 6-layer prices for a board this
size, so it's scaled from their 100 x 100 mm price. Upload the zip to
their quote page for the real figure.

| | 1 board | 2 boards | 5 boards |
| --- | --- | --- | --- |
| 6-layer PCB, 5 pcs (minimum), ENIG | ~$100 (80-130) | ~$100 | ~$100 |
| Parts (LCSC prices; J1 and K1-K3 via Global Sourcing) | $111 | $218 | $514 |
| Setup $25.56, stencil $8.21, hand-solder $3.58, misc ~$1 | $38 | $38 | $38 |
| Feeder loading, $1.53 for each of 85 SMT part types | $130 | $130 | $130 |
| Joints: 1,463 SMT at $0.0016, 152 through-hole at $0.0164 | $5 | $10 | $24 |
| **Current build** | **~$385** | **~$495** | **~$805** |
| **Lean zip** (81 part types, cheaper parts) | ~$375 | ~$485 | ~$790 |

Shipping and tax are extra, typically $20-40 by DHL.

Why so much, and why the swap barely helps:

- **Parts are about half.** Roughly $110 a board, mostly a few chips:
  MAX9926 x2 ($18), STM32H743 ($11), DRV8803 x5 ($11), INA240 x2 ($10),
  relays ($14), ESP32-S3 ($5). The resistors and capacitors together are
  under $5.
- **Feeder loading is a third.** On Standard assembly JLCPCB charges
  $1.53 for every distinct part type loaded onto the machines, basic or
  extended. This board has 85 types, so about $130 per order, however
  many boards. Swapping to basic parts only saves this fee on Economic
  assembly, which this board can't use because of the ESP32 module.
- **The PCB is fixed.** 5 boards is the minimum whether you assemble 1
  or 5.

## Options

| Option | 2 boards | Trade-off |
| --- | --- | --- |
| A. Current build, `jlcpcb_order_files.zip` | ~$495 | Nothing to solder except the shadow tails. |
| B. Lean build, `jlcpcb_lean_order_files.zip` | ~$485 | All status LEDs are red, and no SWD/UPDI headers (USB-C still flashes everything). |
| C. Assemble just 1 board (either zip, quantity 1) | ~$375-385 | The cheapest way to a working prototype. A second board later repeats the ~$170 setup and feeder fees. |
| D. Bare boards + parts, solder it yourself | ~$220 for 1 | You place 353 parts including a 144-pin 0.5 mm chip. Realistic with a stencil and hot plate, but it's a weekend and a half. |
| E. Relays as an add-on | not offered | The relays connect this board's drivers to the harness in both modes, so dropping them needs a board change. It would also remove the hardware fuel cut. Saves ~$15 a board. |
| F. 4-layer board | not offered | At this size it didn't route on 4 layers (150+ connections left over). It would need a bigger board and enclosure, and saves only ~$40-60 on the PCB. |

The bigger savings would come from a rev B that changes chips, at
roughly $30-40 a board. Examples: a cheaper STM32 (H723), cheaper
current-sense amps instead of the INA240s, and smaller relays. Each of
those needs re-checking against the [VERIFY] list, so they aren't done
here.

Parts with thin stock (check before ordering 5): MAX9926 (60 in stock,
10 needed), SUD50P10-43L (35, 5), PTS810 boot switch (51, 5),
ATtiny1616 (335, 5), INA240A1 (438, 10).

## What arrives, and what's left for you

- Fully assembled board, ready to flash over USB-C (BUILD.md step 4).
- For shadow mode only: make the three tails (TE 174516-6, 174514-6,
  174515-6 housings with 173716-1 terminals, one wire per pin) and solder
  them into J2-J4. See INSTALL.md.
- Wash and inspect as in BUILD.md, then BENCH_TEST.md stages 1-4.
