# 1KZ-TE ECU Interposer: Research Notes

Phase 1 research for a zero-cut, plug-in "splice harness" that sits between
the engine ECU and its harness on a **1996–2002 KZJ95 Land Cruiser Prado
(1KZ-TE, 3.0 L turbo-diesel)**.

> Status: desk research only. Nothing below has been measured on the
> truck yet. Items marked **[VERIFY]** need the workshop manual or a
> caliper/multimeter on the real ECU before parts are printed or wired.

---

## 1. Which 1KZ-TE is this?

The KZJ95's 1KZ-TE is the **electronically controlled** version (Toyota
"EFI-diesel" / Denso **ECD-V3**), not the mechanical-pump 1KZ-T:

- The 1KZ-T uses a fully mechanical injection pump. The 1KZ-TE replaced
  that with an electronically controlled distributor pump
  ([Wikipedia: Toyota KZ engine](https://en.wikipedia.org/wiki/Toyota_KZ_engine)).
- Owners' diagnostics on KZJ95s reference the Denso ECD-V3 pump, its
  **spill control valve (SPV)**, **timing control valve (TCV)**, two
  **NE sensors** and the pump **correction resistors**
  ([IH8MUD: 1KZ-TE no start](https://forum.ih8mud.com/threads/1kzte-no-start.1328476);
  [TaT: 2001 KZJ95R no spill-valve feed](https://www.tat.net.au/tatsafact/5162)).
- Toyota's **1KZ-TE Engine Supplement RM790E** covers KZN165 Hilux,
  KZN185 4Runner and **KZJ90/KZJ95 Land Cruiser Prado**. Its ECU terminal
  table is on pages **DI-18 to DI-23**
  ([RM790E excerpt](https://pdfcoffee.com/1kz-te-engine-sup-rm790e-pdf-free.html)).

What that means: the ECU decides **both fuel quantity and injection
timing**. Fuel quantity comes from when the SPV closes and opens during each
pump stroke. Timing comes from the TCV's duty cycle, which moves the pump's
timer piston. So fuel and timing both show up as electrical signals on the
ECU connector, which is good news for this project.

**[VERIFY]** Read the sticker on Connor's ECU: Toyota `89661-xxxxx` and
Denso `175800-xxxx`. That tells us A/T vs M/T and the market (JDM KZJ95W,
EU "Colorado", or AU KZJ95R).

## 2. ECU location and connectors

- **Location:** behind the glovebox, on the passenger side. RM790E's
  knock-sensor check tells you to remove the glove compartment to reach the
  ECU connector **[VERIFY on RHD vs LHD]**.
- **Connectors:** three harness plugs, **E5 = 26 pin, E6 = 16 pin,
  E8 = 22 pin, 64 pins in total** (KZJ95 EWD269F p.210; the A/T ECU is the
  "Engine and ECT ECU"). Pin functions are in §3.
- **Connector family:** a 64-way **TE/AMP Multilock hybrid** header,
  **TE 176122-6** ("hybrid cap assy 64P"). It is also used on 1994 Camry
  and MR2 Turbo ECUs ([rusEFI forum](https://rusefi.com/forum/viewtopic.php?p=33084);
  [rusEFI OEM connectors wiki](https://github.com/rusefi/rusefi/wiki/OEM-connectors)).

### 2.1 Header dimensions (measured)

rusEFI publishes a KiCad footprint and a 3D model of 176122-6
([rusefi/kicad-libraries](https://github.com/rusefi/kicad-libraries)). We
measured the mesh (±0.05 mm) and cross-checked it against TE's published
5.2 mm row spacing on the mating plug 175110-6. Every number in
`hardware/ecu_interposer/cad/sections.scad` comes from this.

| Feature | Value |
| --- | --- |
| Overall length (with mounting ears) | 117.8 mm |
| Body | 108.96 × 18.5 mm |
| Section order (left to right, header seen from its mouth) | E5 (26), E6 (16), E8 (22) |
| Cavity lengths | E5 **42.44**, E6 **25.20**, E8 **35.42** mm |
| Wall between sections | 5.95 mm (centre gap); outer walls 1.48 mm |
| Cavity height | 11.82 mm (floor to rib undersides); slots between ribs reach **15.02 mm** |
| Cavity depth (mouth to floor) | 14.5 mm |
| Row spacing | **5.2 mm** (two rows per section) |
| 040 rows | 1.85 / 7.05 mm above the floor; **3.0 mm** column pitch; tab 0.98 × 0.62 mm, tip 7.09 mm proud of the floor |
| 070 rows | 1.0 mm higher (2.85 / 8.05 mm); **3.5 mm** column pitch; tab 1.78 × 0.62 mm, tip 7.99 mm proud |
| 070 → 040 column step | 3.1 mm |
| Polarising rails | on both end walls of each cavity, mid-height (C/A: 5.26–5.94 mm, 0.8 proud; B: 6.12–7.08 mm, 0.5 proud) |
| Keying | a different rib/slot pattern on the top wall of each section (listed in `sections.scad`) |
| Floor grooves | two per section, 1.5 wide × 1.0 deep, full depth, symmetric about the centre: E5 ±11.35–12.85, E6 ±6.25–7.75, E8 ±9.25–10.75 mm (added after the STEP test fit below) |

Column layout, header seen from its mouth: **E5** = 070 ×3 | 040 ×8 | 070 ×2.
**E6** = 040 ×8. **E8** = 070 ×1 | 040 ×8 | 070 ×2. (The manual's plug-face
drawings show the mirror image of this.)

![E5 (26-pin) cross-section: printed male shroud (top) and female plug (bottom)](../hardware/ecu_interposer/preview_E5_section.png)

**Still [MEASURE]:** the exact receptacle body sizes inside the plug
(`T040` / `T070` in `params.scad`).

### 2.1a The plug latch (from TE's 175110-6 STEP)

TE's customer-view STEP of the 22-pin plug (`C-175110-6`, Pro/E, 2011,
on Connor's Desktop; not committed, TE copyright) settles the latch. We
sectioned it with OpenCascade's point classifier at 0.1 mm steps. In the
plug's own frame (front face z = 0, body ±5.8 mm tall, centred):

| Feature | Value |
| --- | --- |
| Where | on the section centre line, on the slotted (rib) side, in the two slots either side of the header's 2 mm centre rib. E5 and E6 have the same centre slot pair, so the same latch fits all three. |
| Lock arm | two tines at \|x\| = 1.3–4.1 mm, rooted in a solid block at the front (z 0 to −3.5), free towards the back, joined by a crossbar with a press button at z −15.5 to −18.5 (outside the header) |
| Tine | about 1.1 mm thick, top 7.7 mm above the plug centre at the hook (8.75 on the outer part, \|x\| ≥ 2.8); 0.5–1.25 mm gap under it for deflection |
| Hook | on the inner part of each tine (\|x\| = 1.4–2.8): 45° ramp facing the front from z −9.25 to −10.45, flat top at 8.95, square lock face at **z = −11.45** facing the back. Hook height 1.25 mm. |
| Header lock (inferred) | a bar at the header mouth between the tines (\|x\| < 2.8), underside between 7.7 and 8.95 above the plug centre, inner face no deeper than **3.05 mm** inside the mouth. The rusEFI header model leaves it out. |
| Other plug features the header model was missing | two bottom keys at \|x\| = 9.4–10.6, 0.9 mm deep (they ride in the floor grooves, which the rusEFI model *does* have; our first pass missed them) and root fillets on the top ribs (0.35 chamfers on the slot edges) |
| TE spec 108-5280 | locking strength ≥ 98 N; 22-pin unmating force ≤ 118 N; 26-pin ≤ 137 N |

What the CAD does with it:

- **`male_shroud`** gets a lock bar at the mouth (\|x\| ≤ 2.55, underside
  13.91 mm above the cavity floor, 2.9 mm deep with a 0.5 mm lead-in), plus
  the floor grooves and slot chamfers. A virtual test fit of TE's STEP in
  the E8 shroud shows **zero interference** when seated, and the hook's lock
  face lands 0.15 mm behind the bar.
- **`female_plug`** gets its own copy of the lock: 1.0 mm tines kept below
  13.45 mm (under any position the real bar could be), hooks to 14.8 mm,
  lock face 11.1 mm from the plug front (3.2 mm inside the mouth, vs
  3.05 for the OEM), a press tab outside the mouth, and a 0.6 mm relief in
  the body under the tines so they can drop 1.4 mm. Bottom keys for the
  floor grooves too. The tines bend about 2 % at full deflection, which
  wants tough resin or MJF PA12; the zip-tie stays as a backup.

![E8 latch: OEM plug in the printed shroud (left), printed plug in a header copy (right)](../hardware/ecu_interposer/preview_E8_latch.png)

### 2.2 Part numbers

| Part | TE number | Notes |
| --- | --- | --- |
| ECU header (on the board) | **176122-6** | what we copy for `male_shroud` |
| Harness plug, 22 pin (E8) | **175110-6** | 31.4 × 36.1 × 21.0 mm |
| Harness plug, 16 pin (E6) | **175109-6** | |
| Harness plug, 26 pin (E5) | **175111-x** (inferred from the series); 175567-1 is a possible alternative | [VERIFY] from the plug's moulding |
| 040 receptacle (goes in our `female_plug`) | **175061-1**, 175062-1, 173681-6 | 24–20 AWG |
| 070 receptacle | **173631-1** (or -8) | 20–16 AWG, insulation Ø 2.0–2.6 mm |
| 040 tab (goes in our `male_shroud`) | **173682-1** (175206-1 is the older, not-for-new-designs part) | |
| 070 tab | **173645-1** | |
| Application spec | 114-8005 | crimp heights and pull-out forces |
| Hand crimp tool | 58521-1 / 58521-2 | a generic open-barrel crimper works with care |

Sources: [TE 173631-1 datasheet](https://www.te.com.cn/chn-zh/product-173631-1.datasheet.pdf),
[TE Multilock receptacle family](https://www.te.com/en/product-CAT-M9194-T273.html),
TE product pages for each number above.

### Buying terminals rather than printing them

The printed housings hold real metal terminals: receptacles in the part
that plugs into the ECU, tabs in the part the OEM plug goes into. A
sacrificial donor ECU (1KZ-TE, 3S-GTE or 1994 Camry, about US$30–60 at a
wrecker) is still worth having. It gives a real 176122-6 header to test
the printed plug against before going near the truck.

## 3. Pinout (1KZ-TE engine and ECT ECU, automatic)

**Source: Connor's 1996 Land Cruiser Prado wiring manual, EWD269F**
(RZJ/VZJ/KZJ/LJ90-95), pages 206–209 (Engine Control, 1KZ-TE),
220–223 (ECT and A/T Indicator, 1KZ-TE) and 210 (connector face views).
Connor's truck is an automatic, so this is the "Engine and ECT ECU (A/T)".
The machine-readable copy is
[`hardware/ecu_interposer/pinout_kzj95_at.csv`](../hardware/ecu_interposer/pinout_kzj95_at.csv).

**Connector names.** The KZJ95 manual calls the plugs **E5 (26 pin)**,
**E6 (16 pin)** and **E8 (22 pin)**, lettered A, B, C in that order. The
KZN130 tables used earlier letter them the other way (A = 22, C = 26), so
the CAD and this doc now go by the manual's codes.

**Numbering.** The manual draws each harness plug from its mating face.
Top row is 1…n left to right, bottom row continues n+1…2n. The wide 070
terminals sit at both ends:

```
E5 (26)   [ 1][ 2] 3  4  5  6  7  8  9 10 [11][12][13]
          [14][15]16 17 18 19 20 21 22 23 [24][25][26]
E6 (16)    1  2  3  4  5  6  7  8
           9 10 11 12 13 14 15 16
E8 (22)   [ 1][ 2] 3  4  5  6  7  8  9 10 [11]
          [12][13]14 15 16 17 18 19 20 21 [22]
                                   [ ] = 070 (power) terminal
```

The CAD frame looks at the ECU header from its mouth, which is the mirror
image, so manual column 1 is the rightmost column in `sections.scad`
(`pin_no()` does the mapping). The terminal sizes this implies line up
with the circuits: grounds, SPV, TCV, the A/T solenoids, +B and BATT all
land on 070 terminals. The STEP model puts the latch on the slotted side,
which is the CAD's upper row; **[VERIFY]** that the manual's top row is the
latch side too (Toyota face views normally draw the lock at the top).

**E5, 26 pin**

| Pin | Name | Function | Terminal | Wire | Tap class |
| --- | --- | --- | --- | --- | --- |
| E5-1 | SL | A/T lock-up solenoid | 070 |  | DIG |
| E5-2 | S1 | A/T shift solenoid No.1 | 070 |  | DIG |
| E5-3 | SP2+ | A/T output speed sensor No.2 (+) | 040 | W-R | VR |
| E5-4 | **NE+** | Pump speed sensor (+) | 040 | B | VR |
| E5-5 | **TDC+** | Crankshaft (TDC) position sensor (+) | 040 | R | VR |
| E5-6 | | *not used in the EWD* | 040 | | |
| E5-7 | L4 | Transfer L4 position switch | 040 | LG | DIG |
| E5-8 | TFN | Transfer neutral position switch | 040 | L | DIG |
| E5-9 | S/TH2 | Intake constrictor VSV No.2 | 040 | GR-B | DIG |
| E5-10 | S/TH1 | Intake constrictor VSV No.1 | 040 | GR | DIG |
| E5-11 | **SPV** | Magnetic spill valve drive (fuel quantity) | 070 | R | CAP-HV |
| E5-12 | **TCV** | Timing control valve drive (injection timing) | 070 | G-W | CAP-HV |
| E5-13 | E01 | Power ground | 070 | BR | GND |
| E5-14 | E1 | ECU signal ground | 070 | BR | GND |
| E5-15 | S2 | A/T shift solenoid No.2 | 070 |  | DIG |
| E5-16 | SP2- | A/T output speed sensor No.2 (-) | 040 | Y-R | VR |
| E5-17 | **NE-** | Pump speed sensor (-) | 040 | W | VR |
| E5-18 | **TDC-** | Crankshaft (TDC) position sensor (-) | 040 | G | VR |
| E5-19 | | *not used in the EWD* | 040 | | |
| E5-20 | L | Shift lever L position | 040 |  | DIG |
| E5-21 | 2 | Shift lever 2 position | 040 |  | DIG |
| E5-22 | SNWL | Snow (2nd start) indicator lamp | 040 | Y-G | DIG |
| E5-23 | SNW | Snow (2nd start) switch | 040 |  | DIG |
| E5-24 | EGR | EGR VSV | 070 |  | DIG |
| E5-25 | | *not used in the EWD* | 070 | | |
| E5-26 | E02 | Power ground | 070 | BR | GND |

**E6, 16 pin**

| Pin | Name | Function | Terminal | Wire | Tap class |
| --- | --- | --- | --- | --- | --- |
| E6-1 | VC | 5 V sensor reference | 040 | G-Y | ANA |
| E6-2 | **PIM** | Turbo pressure sensor | 040 | B-Y | ANA |
| E6-3 | THA | Intake air temperature | 040 | L-Y | ANA |
| E6-4 | THW | Coolant temperature | 040 | G | ANA |
| E6-5 | THF | Fuel temperature (pump) | 040 | W | ANA |
| E6-6 | **VRP** | Pump correction resistor No.1 (quantity) | 040 | B-R | ANA |
| E6-7 | TE2 | Diagnostic test terminal 2 | 040 | W-G |  |
| E6-8 | VF | Diagnostic output | 040 | B-O | DIG |
| E6-9 | E2 | Sensor ground | 040 | BR | GND |
| E6-10 | | *not used in the EWD* | 040 | | |
| E6-11 | **VA** | Accelerator position | 040 | Y-R | ANA |
| E6-12 | IDL | Idle switch | 040 | G-W | DIG |
| E6-13 | THO | A/T fluid temperature | 040 |  | ANA |
| E6-14 | **VRT** | Pump correction resistor No.2 (timing) | 040 | L-B | ANA |
| E6-15 | TE1 | Diagnostic test terminal 1 | 040 | R-Y |  |
| E6-16 | TT | A/T diagnostic terminal | 040 | LG |  |

**E8, 22 pin**

| Pin | Name | Function | Terminal | Wire | Tap class |
| --- | --- | --- | --- | --- | --- |
| E8-1 | BATT | Constant battery (keep-alive) | 070 | B-Y | V-mon |
| E8-2 | S-REL | Sub glow relay | 070 | G-R | DIG |
| E8-3 | M-REL | ECD main relay control | 040 | L-B | DIG |
| E8-4 | STP | Stop light switch | 040 | G-W | DIG |
| E8-5 | W | Check engine lamp | 040 | Y-R | DIG |
| E8-6 | OIL-W | A/T oil temperature warning lamp | 040 | Y-B | DIG |
| E8-7 | G-IND | Glow indicator lamp | 040 | R | DIG |
| E8-8 | IMO | Immobiliser output | 040 | BR-R | DIG |
| E8-9 | SP1 | Vehicle speed (meter) | 040 | V-W | CAP |
| E8-10 | A/C | A/C magnetic clutch signal | 040 | B-Y | DIG |
| E8-11 | STA | Starter signal | 070 | R | DIG |
| E8-12 | +B | ECU main power (via ECD relay) | 070 | B-R | V-mon |
| E8-13 | SVR | Spill valve relay | 070 | GR | DIG |
| E8-14 | IGSW | Ignition switch | 040 | B-O | DIG |
| E8-15 | P | ECT pattern select (PWR) | 040 | G-Y | DIG |
| E8-16 | OD2 | Overdrive switch | 040 | V-G | DIG |
| E8-17 | IMI | Immobiliser input | 040 | P-G | DIG |
| E8-18 | TAC | Tachometer output | 040 | B | CAP |
| E8-19 | ACT | A/C cut (from A/C amplifier) | 040 | L-B | DIG |
| E8-20 | HSW | Idle-up switch | 040 | P-L | DIG |
| E8-21 | | *not used in the EWD* | 040 | | |
| E8-22 | NSW | Neutral start switch | 070 | B-W | DIG |

SPV and TCV are driven straight from E5-11 and E5-12 (pages 207-3/207-4),
and the ECU also switches a separate **spill valve relay** through E8-13
(SVR). The interposer only ever taps these.

The tap classes (V-mon, ANA, DIG, CAP, CAP-HV, VR) are defined in
[ECU_TAP_ARCHITECTURE.md](ECU_TAP_ARCHITECTURE.md).

## 4. What a passive interposer can see, and what it can change

### Observe (high-impedance taps; the ECU can't tell we're there)

| Quantity | From | How |
| --- | --- | --- |
| Engine RPM | TDC or TAC | period between pulses |
| Pump speed and angle | NE± | VR edge timestamps |
| **Actual injection timing** | **NE vs TDC phase** | the ECU itself computes actual timer advance from this phase, so we can do the same calculation |
| **Commanded timing** | TCV duty | PWM capture |
| **Fuel per stroke** (proxy) | SPV close→open time vs NE angle | edge capture. Calibrate against fuel use or smoke to turn it into mm³/stroke |
| Driver demand | VA | ADC |
| Boost | PIM | ADC |
| Temperatures | THW, THA, THF | ADC + Toyota NTC curve |
| Pump calibration | VRP, VRT | ADC at key-on |
| Speed, idle, A/C, starter, glow, CEL | SP1, IDL, A/C, STA, G-IND, W | digital |

This is far more than the truck's diagnostic port gives. A JDM-era
1KZ-TE has **no OBD-II data stream**, only flash codes through TE1–E1 on
the DLC. The interposer becomes the truck's data logger.

### Change (only by deliberately breaking a pin's pass-through)

A truly passive tap can't change anything. To change fuelling or timing,
the interposer has to **intercept** a line: route it through a circuit
instead of straight through. That still needs zero cuts, because the
interposer owns both ends of every pin. The useful levers, ordered from
safest to riskiest:

1. **PIM (boost signal) scaling.** The ECU's smoke limiter caps fuel
   based on boost. If we rescale or clamp what the ECU sees, the fuel
   ceiling moves. This is the classic diesel "boost box" approach. It's a
   low-current analog line, and failing to stock is easy.
2. **Correction resistors VRP/VRT.** These resistors tell the ECU how the
   pump was calibrated. People already tweak fuelling by changing them.
   An interceptor can switch between resistor values (digital pot or relay
   bank). The ECU reads them at key-on, so changes take effect on the next
   start.
3. **THF / THA / THW spoofing.** These are second-order fuel and timing
   trims. Useful for experiments, but it confuses cold-start and warm-up
   logic.
4. **VA scaling.** Changes pedal feel and response, not maximum power.
5. **TCV takeover (timing): out of scope.** We would drive the timing
   valve ourselves. The ECU watches NE/TDC phase and expects it to match
   its target, so a mismatch sets **code 14 (timing control)** and can put
   it in limp mode.
6. **SPV takeover (fuel quantity): out of scope. ⚠ Don't.** The SPV is a
   high-current inductive solenoid switching once per injection. If our
   circuit fails with the SPV *closed*, the pump injects for the full
   stroke: maximum fuel, overheating, possible runaway. Read it, don't
   drive it.

> **Decision (Connor, 2026-10-22): piggyback only.** The stock ECU stays
> in charge of fuel and timing. The interposer logs everything and reshapes
> only the low-current *inputs* in levers 1–4 (PIM, VRP/VRT, temperatures,
> VA). Timing changes come only from what the ECU itself does with those
> inputs. SPV and TCV are tap-only and never intercepted.

What we **can't** do from the connector: rewrite the ECU's maps. The
mid-90s Denso diesel ECUs keep calibration in on-board ROM. Changing
the maps means desoldering or reading the chip, which is a separate,
bench-only project.

**Design rule that follows from this:** every interceptable line uses a
**normally-closed** bypass (relay or analog switch). Unpowered, faulted,
or with the ESP32 crashed, the truck runs stock. The CAD keeps every pin
as a straight pigtail. Phase 2 adds an "intercept" pigtail option for the
handful of pins in levers 1–4.

## 5. Manuals and documents to obtain

| Doc | Why | Where |
| --- | --- | --- |
| **RM790E 1KZ-TE Engine Supplement** (covers KZJ90/95) | ECU terminal table (DI-18..23), standard voltages, ECD-V3 pump circuits | [pdfcoffee excerpt](https://pdfcoffee.com/1kz-te-engine-sup-rm790e-pdf-free.html), [Scribd](https://www.scribd.com/document/365360972/1KZ-TE-ENGINE-SUP-RM790E-pdf), [ManualsLib](https://www.manualslib.com/manual/1101762/Toyota-1kz-Te.html?page=22) |
| KZJ90/95 Electrical Wiring Diagram (EWD269F) | connector face views, wire colours, ECU pinout | **Have it** (Connor's Desktop); used for §3 |
| Denso ECD-V3 service doc | SPV/TCV/NE theory, correction-resistor tables | referenced on IH8MUD |
| Hilux Surf 1KZ-TE terminal tables | already used above | [hiluxsurf.co.uk](https://hiluxsurf.co.uk/filedata/fetch?id=1295023) |

## 6. Open questions for Connor

1. ~~A/T or M/T?~~ Automatic. The `89661-` / `175800-` ECU numbers would
   still help when buying a donor ECU.
2. ~~Which manual?~~ Done: the KZJ95 EWD (EWD269F) is on Connor's Desktop and
   the pinout above comes from it.
3. Photos of the three plug faces next to a ruler (mating side and wire
   side). Header and latch now come from TE's models, so these would only
   confirm the 26-pin plug's part number. Not needed.
