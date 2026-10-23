# ECU Interposer (1KZ-TE, KZJ95)

A printable, modular pass-through that plugs in between the 1KZ-TE engine
ECU and its harness. Nothing on the truck gets cut. Each of the ECU's three
plugs (E5 26-pin / E6 16-pin / E8 22-pin, as the KZJ95 wiring manual names
them) gets its own independent section, so you can
install only the ones carrying the signals you want.

![E5 (26-pin) section, exploded](preview_E5_exploded.png)

```
 ECU header ◀── female_plug ─┬─ chamber (2 halves) ─┬─ male_shroud ◀── OEM harness plug
  (176122-6)   (Multilock     │   pigtails + taps    │  (copy of 176122-6,
                receptacles)  │   ribbon out ──▶ tap box (ESP32-S3)   Multilock tabs)
```

- **Research and pinout:** [`docs/ECU_INTERPOSER_RESEARCH.md`](../../docs/ECU_INTERPOSER_RESEARCH.md)
- **Electronics and logging:** [`docs/ECU_TAP_ARCHITECTURE.md`](../../docs/ECU_TAP_ARCHITECTURE.md)

## Dimensions: where they come from

The header side is no longer a guess. The ECU's header is **TE 176122-6**,
and every cavity, rib, slot, rail, row and column position in
`cad/sections.scad` is measured from TE's 3D model of it (via rusEFI). The
details and all the TE part numbers are in
[research §2](../../docs/ECU_INTERPOSER_RESEARCH.md#2-ecu-location-and-connectors).

![E5 (26-pin) cross-section](preview_E5_section.png)

*E5 (26-pin) cut across the mating axis. Top: `male_shroud`, a copy of the
header cavity with its key slots and end rails. Bottom: `female_plug`,
with key ridges that ride in the header's slots and a cavity per pin.*

The **latch** comes from TE's STEP model of the 22-pin harness plug
(175110-6). The OEM plug has two flexible tines with hooks that snap behind
a bar at the header mouth. So `male_shroud` now has that bar (the OEM plug
clicks in and holds), and `female_plug` has its own tines and press tab to
click into the ECU's header. A virtual test fit of TE's model in the
printed shroud shows no interference. Details in
[research §2.1a](../../docs/ECU_INTERPOSER_RESEARCH.md#21a-the-plug-latch-from-tes-175110-6-step).

![E8 latch, lengthwise section](preview_E8_latch.png)

One thing is still an estimate, marked **[MEASURE]** in `params.scad`: the
receptacle and tab **body sizes** inside the printed parts (`T040`,
`T070`), which only matter for how the metal terminals seat.

## Exploded view and cross-section analysis

[`diagram/connector_diagram.pdf`](diagram/connector_diagram.pdf) ([PNG](diagram/connector_diagram.png))
shows the E5 (26-pin) section exploded with every terminal, then two true-scale
sections: across the pins (pitch, rows, keys, webs) and lengthwise through
a 070 and a 040 column (how far each tab engages). Rebuild it with
`python3 diagram/make_diagram.py` (needs OpenSCAD, xvfb-run, matplotlib).

## Printing

1. Print **`stl/fit_coupon.stl`** first: a 4-column slice (070, 040, 040,
   070) of each side at the real pitch. Check that the female block slides
   into the ECU header, that the OEM plug's terminals enter the male block,
   and that real terminals click in and hold.
2. Adjust `cad/params.scad` (`clr`, `slide_clr`, `T040`, `T070`) if needed.
3. Run `cad/export_all.sh` (OpenSCAD ≥ 2021), then print the sections.

- **Terminal-holding parts** (`female_plug`, `male_shroud`, coupon): the
  thinnest wall between cavities is about **0.35 mm** where a 070 column
  meets a 040 column (OpenSCAD echoes a warning). That's the same as OEM
  but too fine for 0.4 mm-nozzle FDM. Print them in **resin
  (tough/ABS-like)** or order **MJF PA12**. Orientation is already set:
  the plug prints front face down, the shroud prints flange down. The
  plug's lock tines then stand vertically, so in FDM they would bend
  across layer lines and snap: another reason for resin or MJF. Press the
  tab outside the header mouth to release.
- **Chamber halves and tap box:** FDM is fine, in PETG or ASA. The ECU sits
  behind the glovebox, so these see cabin temperatures, not engine-bay.
  The chamber halves export split face down.
- **Hardware:** 4 × M2 × 10 self-tappers per section, 4 × M2.5 × 5 for the
  tap PCB, and one 4 mm zip-tie per section to the ECU bracket (backup to the
  latch).

## Files

| Path | What |
| --- | --- |
| `pinout_kzj95_at.csv` | ECU pinout for the automatic, from the KZJ95 wiring manual (EWD269F) |
| `cad/sections.scad` | The measured 176122-6 geometry: per-section cavity length, columns (040/070), key slots, rails, floor grooves |
| `cad/params.scad` | Process clearances, terminal body envelopes **[MEASURE]**, latch (lock bar and tines), chamber and tap box |
| `cad/interposer.scad` | Library: `female_plug`, `male_shroud`, `chamber_half`, `tapbox_base/lid`, `section_assembly` |
| `cad/main.scad` | Top level. Choose with `-D part=... -D sec_i=...` (0 = E5 26-pin, 1 = E6 16-pin, 2 = E8 22-pin) |
| `cad/fit_coupon.scad` | Fit test slice |
| `cad/diagram.scad` | Diagram scene: terminals, pigtails, exploded / cut views (not for printing) |
| `cad/export_all.sh` | Regenerates every STL into `stl/` |
| `stl/` | Pre-exported STLs, print-oriented |

## Assembly (per section)

1. Crimp a ~40 mm pigtail for each pin: a receptacle on one end and a tab on
   the other, using the same wire gauge as OEM (terminal part numbers in research §2.2) (0.3–0.5 mm² signal, 0.85 mm²
   for 070 pins).
2. Load the receptacles into `female_plug` and the tabs into `male_shroud`.
   **Every pigtail runs straight through:** pin N on the plug connects to
   pin N on the shroud, which is the position directly opposite it. The
   pin-1 dots on both flanges mark the same corner (Toyota pin 1, upper
   row). Pin functions are in `pinout_kzj95_at.csv`.
3. For each tapped pin, solder a **47 kΩ+ series resistor** at the pigtail
   (see the architecture doc), then add ribbon to the tap box.
4. Close the chamber halves over both flanges, screw them together, and
   route the ribbon out of the side slot.
5. With the key off: unplug the OEM plug from the ECU, plug the interposer
   into the ECU, then plug the OEM plug into the interposer.
6. **Before starting the engine**, check every pin end to end with a meter
   (OEM side ↔ ECU side).
