# ECU Interposer (1KZ-TE, KZJ95)

A printable, modular pass-through that plugs in between the 1KZ-TE engine
ECU and its harness. Nothing on the truck gets cut. Each of the ECU's three
plugs (A22 / B16 / C26) gets its own independent section, so you can
install only the ones carrying the signals you want.

![C26 section, exploded](preview_C26_exploded.png)

```
 ECU header ◀── female_plug ─┬─ chamber (2 halves) ─┬─ male_shroud ◀── OEM harness plug
               (Multilock     │   pigtails + taps    │  (Multilock tabs,
                receptacles)  │   ribbon out ──▶ tap box (ESP32-S3)
```

- **Research and pinout:** [`docs/ECU_INTERPOSER_RESEARCH.md`](../../docs/ECU_INTERPOSER_RESEARCH.md)
- **Electronics and logging:** [`docs/ECU_TAP_ARCHITECTURE.md`](../../docs/ECU_TAP_ARCHITECTURE.md)

## ⚠ Read before printing

Every connector dimension is an **estimate** from TE Multilock catalogue
data. None of it has been measured on the truck yet. The workflow:

1. Print **`stl/fit_coupon.stl`**. It's small: 040 and 070 female and male
   blocks.
2. Push real terminals in. Check that the tab enters, the body slides, the
   lance clicks and holds, and that the pitch matches the OEM plug.
3. Edit `cad/params.scad` (`clr`, `T040`, `T070`) and
   `cad/sections.scad` (pitch, row pitch, key positions, pin map) until the
   coupon feels like the OEM part.
4. Run `cad/export_all.sh`, then print the sections.

## Files

| Path | What |
| --- | --- |
| `cad/params.scad` | All tunable dimensions (process clearance, terminal envelopes, latch, chamber, tap box) |
| `cad/sections.scad` | Per-connector definitions: grid, pitch, pin map (040/070/empty), keys |
| `cad/interposer.scad` | Library: `female_plug`, `male_shroud`, `chamber_half`, `tapbox_base/lid`, `section_assembly` |
| `cad/main.scad` | Top level. Choose with `-D part=... -D sec_i=...` |
| `cad/fit_coupon.scad` | Terminal fit test |
| `cad/export_all.sh` | Regenerates every STL into `stl/` (needs OpenSCAD ≥ 2021) |
| `stl/` | Pre-exported STLs at the current (unverified) defaults |

## Printing

- **Terminal-holding parts** (`female_plug`, `male_shroud`, coupon): the
  walls between cavities come out around **0.3 mm** at 2.5 mm pitch, and
  OpenSCAD echoes a warning about it. That's normal for OEM Multilock but
  beyond 0.4 mm-nozzle FDM. Use **resin (tough/ABS-like)** or order
  **MJF PA12**. Print orientation is already set: the plug prints front
  face down, the shroud prints floor down.
- **Chamber halves and tap box:** FDM is fine. PETG or ASA. The ECU sits
  behind the glovebox, so cabin temperatures, not engine-bay.
- Hardware: 4 × M2 × 10 self-tappers per section, 4 × M2.5 × 5 for the tap
  PCB.

## Assembly (per section)

1. Crimp a ~40 mm pigtail for each pin: a receptacle on one end and a tab on
   the other, using the same wire gauge as OEM (0.3–0.5 mm² signal, 0.85 mm²
   for 070 pins).
2. Load the receptacles into `female_plug` and the tabs into `male_shroud`.
   **Keep pin positions mirrored correctly:** A-1 on the plug must connect
   to A-1 on the shroud. The pin-1 dots on both flanges sit on the same
   corner.
3. For each tapped pin, solder a **47 kΩ+ series resistor** at the pigtail
   (see the architecture doc), then add ribbon to the tap box.
4. Close the chamber halves over both flanges, screw them together, and
   route the ribbon out of the side slot.
5. With the key off: unplug the OEM plug from the ECU, plug the interposer
   into the ECU, then plug the OEM plug into the interposer.
6. **Before starting the engine**, check every pin end to end with a meter
   (OEM side ↔ ECU side).
