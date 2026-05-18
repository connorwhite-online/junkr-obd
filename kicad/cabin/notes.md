# Cabin Shield — Placement & Routing Notes

## Board outline

Recommended size: **30 × 25 mm**, rounded corners (R 2 mm). Two M3
mounting holes diagonally opposite. Roughly the size of a piece of gum.

## Placement

```
+-------------------------+
|                         |
|  J1     U1     JP1  R1  |
|                         |
|  C1 C2                  |
|                         |
|                    J2   |
|                         |
+-------------------------+
                          ↑
                   J2 on right edge
                   (harness exit)
```

- **J2 on the right edge**, oriented so the Deutsch shell exits the
  board.
- **U1 (TJA1051) centered**, with CAN_H / CAN_L pads on the J2 side.
- **R1 + JP1 between U1 and J2** so the termination is in the CAN path
  with no detour.
- **J1 on the opposite edge** from J2, so the ribbon back to the Qualia
  exits the opposite direction from the harness.
- C1 directly adjacent to U1's VCC pin.
- C2 anywhere on the 3.3 V trace between J1 and U1.

## Routing constraints

Per [`DESIGN_RULES.md`](../DESIGN_RULES.md), plus:

1. **CAN_H / CAN_L coupled pair** — 0.25 mm wide, 0.15 mm spacing,
   length-match within 2 mm. Total run < 15 mm.
2. **No GND pour split** under the CAN path — the differential pair
   needs a continuous return reference.
3. **+12V_NC pad** — connect J2.1 to a small isolated copper pad so
   it's mechanically terminated. Label "NC – +12V" on silkscreen.
4. **Single-layer if you can** — this board is small enough to route on
   F.Cu alone with GND pour on B.Cu. No vias except for GND stitching.

## Pre-fab checks

- [ ] DRC clean.
- [ ] J1 pinout on silkscreen matches `schematic.md` (3V3 / GND /
      TWAI_TX / TWAI_RX in that order).
- [ ] J2 pin numbering matches engine-bay J1 (so the harness is a
      straight pin-to-pin run, no crossover).

## Test plan

1. Bench-power +3.3 V to J1.1, GND to J1.2. Confirm U1 quiescent
   current is sane (< 10 mA).
2. Pull TXD low (J1.3 to GND via jumper) — confirm CANH rises to
   ~3.5 V and CANL drops to ~1.5 V (dominant state).
3. Wire up to the engine-bay shield with a short DT pair, install the
   termination jumpers on both ends — confirm cabin firmware receives
   the three CAN frames.
