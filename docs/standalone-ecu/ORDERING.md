# Ordering a fully assembled board (JLCPCB turnkey)

You upload three files to JLCPCB and get back boards with every part
soldered on, through-hole relays and the harness header included. The only
thing left for you is the three shadow-mode tails (J2-J4), and you only
need those if you run shadow mode.

Nothing has been ordered. Prices below are estimates from the parts
library on 2026-11-15; JLCPCB's quote page gives the real number.

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
| BOM | `jlcpcb/bom_jlcpcb.csv` (Comment, Designator, Footprint, LCSC Part #) |
| CPL (pick and place) | `jlcpcb/cpl_jlcpcb.csv` |

`bom.csv` is the full BOM with the exact MPN and a buy link for every line.
`tools/sourcing.csv` is where the part choices live; `gen_fab.py`
regenerates all of the above from the board.

## Order settings

1. jlcpcb.com, Order now, upload `junkr_ecu_gerbers.zip`.
2. Layers **6**, 200 x 140 mm (read from the Gerbers), thickness 1.6 mm,
   surface finish **ENIG**, outer copper 1 oz, inner 0.5 oz, via covering
   tented, minimum via hole 0.2 mm / diameter 0.45 mm (the board's vias
   are 0.45/0.2).
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

## Estimate

| | 2 boards assembled | 5 boards assembled |
| --- | --- | --- |
| Parts from the JLCPCB library | ~$190 | ~$440 |
| J1 + K1-K3 via Global Sourcing | ~$50 | ~$125 |
| 6-layer PCB, 5 pcs, ENIG | ~$150-250 | ~$150-250 |
| Assembly: setup, stencil, 61 extended-part loading fees ($3 each), SMT and THT joints | ~$230 | ~$270 |
| **Total before shipping and tax** | **~$620-720** | **~$985-1085** |

Per board that is roughly $310-360 for two or $200-220 for five. The
extended-part fees are per order, not per board, so the fifth board is
much cheaper than the second. The board has 1,463 SMT joints and 152
through-hole joints (the tails excluded).

Parts in short supply for a 5-board order (check before ordering):
MAX9926 (60 in stock, 10 needed), SUD50P10-43L (35, 5), PTS810 boot
switch (51, 5), ATtiny1616 (335, 5), INA240A1 (438, 10).

## What arrives, and what's left for you

- Fully assembled board, ready to flash over USB-C (BUILD.md step 4).
- For shadow mode only: make the three tails (TE 174516-6, 174514-6,
  174515-6 housings with 173716-1 terminals, one wire per pin) and solder
  them into J2-J4. See INSTALL.md.
- Wash and inspect as in BUILD.md, then BENCH_TEST.md stages 1-4.
