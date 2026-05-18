# Engine-Bay Shield — Placement & Routing Notes

## Board outline

Recommended size: **75 × 60 mm**, rounded corners (R 2 mm).
Mounting holes at the four corners, 4 mm inset from edge.
Orientation: long edge = "harness side" (J1 on the right edge).

## Placement zones

```
+-----------------------------------------------------------+
|                                                           |
|  ZONE A: INPUT POWER                  ZONE D: CONNECTORS  |
|  J1, D1, F1, D2, C1                   J2, J3              |
|                                                           |
|  ZONE B: REGULATORS                                       |
|  U1, U2, C3, C4                                           |
|                                                           |
|  ZONE C: MCU + DIGITAL                                    |
|  U3 (ESP32-C3 SuperMini, on headers)                      |
|                                                           |
|  ZONE E: MIXED SIGNAL                                     |
|  U4 (MAX31855), U5 (ADS1115),                             |
|  R2, R3, R4, C8, C9 — close to U5                         |
|                                                           |
|  ZONE F: CAN                                              |
|  U6 (TJA1051), R1, JP1 — close to J1.3/J1.4               |
|                                                           |
+-----------------------------------------------------------+
                                            ↑ J1 (right edge)
```

## Detailed placement guidance

- **J1 on the right edge**, oriented so the Deutsch shell exits the
  board cleanly. Keep solid copper around J1 pin 2 (GND) for harness
  strain-relief soldering to chassis ground if needed.
- **D1 / F1 / D2 / C1 in a straight line** from J1.1 inward, ≤ 5 mm
  apart. This keeps the load-dump clamp path short — TVS impedance to
  the cap is critical.
- **U1 and U2 side-by-side** below the input chain. Pololu modules
  have generous mounting; don't tuck anything underneath them.
- **U3 (ESP32-C3) centered**, with the USB-C edge facing an accessible
  side of the board (likely the opposite edge from J1) so you can
  reflash on the bench without unwiring.
- **U4 (MAX31855) in its own thermal island.** Leave a ≥ 5 mm copper
  gap on all sides. The breakout's onboard thermocouple terminal block
  should face outward (an edge), so the K-type wire exits straight off
  the board.
- **U5 (ADS1115) adjacent to R2/R3/R4** so the analog input traces are
  short. Place C8 directly on the NTC tap node, C9 directly on the MAP
  tap node — both within 5 mm of U5.
- **U6 (TJA1051) immediately adjacent to J1 pins 3/4.** CAN_H and
  CAN_L traces should be as short and as parallel as possible — total
  length from U6 CANH/L pads to J1.3/J1.4 pads under 25 mm.
- **JP1 + R1 in the CAN path** near U6, somewhere the jumper shunt is
  reachable when the board is mounted (don't tuck it under another
  module).

## Routing constraints

Per [`DESIGN_RULES.md`](../DESIGN_RULES.md), plus:

1. **Power flow on F.Cu** — `+12V_PROT` and `+5V`/`+3V3` run on the top
   layer with the wider trace widths (0.6 / 0.4 mm). Return current on
   the B.Cu GND pour, never on a routed B.Cu trace.
2. **CAN as a coupled pair** — `CAN_H` and `CAN_L` between U6 and J1
   should be routed together, 0.25 mm wide with 0.15 mm spacing. Cross
   to the other layer only if you have to, and use two stitching vias
   per crossover (one each side, GND-connected).
3. **Thermocouple isolation** — under U4's terminal block, do **not**
   pour copper. Leave an unfilled rectangle ≥ 5 mm wide centered on the
   terminal block, both layers. This is to keep self-heating from
   nearby copper out of the cold-junction sensor.
4. **Analog signal trace separation** — `NTC_TAP` and `MAP_TAP` should
   not run parallel to any digital trace (SPI, I²C, CAN_TX/RX) for more
   than 10 mm. If you have to cross, cross perpendicular.
5. **GND stitching** — once routed, stitch the GND pour with vias every
   ~5 mm along high-current return paths (regulators → caps) and
   underneath U6's CAN xcvr footprint.
6. **No vias under the MAX31855 breakout** — leave the footprint area
   clear so heat conduction from elsewhere on the board doesn't reach
   the cold-junction reference.

## Pre-fab checks

Before you send the gerbers out, run through:

- [ ] DRC clean.
- [ ] All silkscreen reference designators readable (no overlap, no
      tucked under components).
- [ ] All connector pin numbering matches `schematic.md` (especially
      J1's Deutsch ordering — pin 1 ≠ pin A in some libraries).
- [ ] Mounting holes are non-plated (KiCad: footprint property "Through
      hole, non-plated").
- [ ] Board edge → component clearance ≥ 1 mm.
- [ ] Verify the SuperMini footprint pin map against your specific
      SuperMini PCB — there are at least 3 variants in the wild with
      different GPIO orderings on the header.

## Test plan (post-fab)

1. **Power up bare board** — apply +12 V to J1.1 / GND to J1.2 with no
   modules installed. Confirm 3.3 V at U1 VOUT and 5 V at U2 VOUT.
   Confirm no smoke on D2 (load-dump TVS).
2. **Insert ESP32-C3** — confirm LED1 lights and the C3 enumerates over
   USB.
3. **Insert MAX31855** — short the T+/T− terminal with a copper wire
   loop (cold-junction reads room temp); confirm CAN ID `0x100`
   transmits a sensible °F value when the engine-bay firmware runs.
4. **Insert ADS1115** — connect a 10 kΩ resistor between J2.1 and J2.2;
   should read ~77 °F coolant temp.
5. **Insert TJA1051** — connect cabin shield, confirm cabin display
   updates.
