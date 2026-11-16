# EGT and wideband: protection first, then live tuning

Two exhaust sensors, added in order:

1. **EGT probe** (first): a type K thermocouple before the turbo. The ECU
   pulls fuel when the exhaust gets too hot, so a long pull, a heavy tow or a
   hot day can't crack the head or cook the turbo.
2. **Wideband O2** (soon after, kept permanently): the ECU reads lambda and
   tunes the smoke limit itself while you drive. That's the "real-time
   tuning" part: the fuel limit at each boost and rpm cell settles where the
   engine actually makes clean power, on your fuel, at your altitude.

**No board change.** Both plug into the rev A AUX connector (J7), which
already has two buffered 0-5 V inputs, switched 12 V and sensor 5 V. The
code is in `src/ecu/core/exhaust.h`. The tests in `test/test_exhaust/` run
it against the engine simulator, which now models lambda and EGT too.

## Parts

| | Part | Notes |
|---|---|---|
| EGT amplifier | [Reveltronics EGT-K](https://www.reveltronics.com/en/products/egt-k-thermocouple-amplifier-type-k-0-5v) 1-channel, or the [MaxxECU](https://maxxecu.com/store/engine-control-or-electronics/sensors/amplifiers-and-converters/egt-amplifier-0-5v-out) / [Turbozentrum](https://www.turbozentrum.de/EGT-K-Thermocouple-Amplifier-Controller-0-5V-Type-K-1-Channel) equivalent | 0-5 V linear, 0 V = 0 °C, 5 V = 1250 °C (4 mV/°C). Those are the firmware defaults. Runs from 8-32 V, so AUX 12 V. |
| EGT probe | Any type K, 3 mm or 1/8" exposed or fast-response tip, 1/8" NPT compression fitting | Mount it pre-turbo. The no-weld bung options are in the hub plan doc: drill and tap the manifold collector for 1/8" NPT, or use a clamp-on/bolt-in bung. Use type K extension wire only. |
| Wideband controller | 14Point7 Spartan 3 Lite v2 ([manual](https://diyautotune.com/wp-content/uploads/Spartan_3_Lite_v2_User_Manual_Dec_19_2023.pdf)) or similar, with a Bosch LSU 4.9 | Reads λ 0.6-3.4 and has a programmable linear output plus a lean-operation setting, which suits a diesel. |
| Wideband bung | Weld-in or clamp-on O2 bung in the downpipe after the turbo | Mount between 10 and 2 o'clock so condensation runs off the sensor. |

## Wiring to AUX (J7, Micro-Fit 12-way, through the lid)

| J7 pin | Signal | EGT amp | Wideband controller |
|---|---|---|---|
| 3 | AUX 12 V (switched, 0.5 A PTC) | + supply | **no**: the heater needs its own fused 12 V |
| 4 / 6 | GND | ground | signal ground |
| 7 | AUX_AN1 | 0-5 V out | |
| 8 | AUX_AN2 | | linear 0-5 V out |

- The heater pulls 1-2 A, far beyond the AUX PTC. Give the controller its own
  ignition-switched, fused 12 V and a chassis ground. Connect its signal
  ground to J7.
- Put a **1 MΩ resistor from each signal to GND** at the AUX plug. An
  unplugged sensor then reads 0 V and sets a fault instead of floating.
  The board's inputs are op-amp followers, so 1 MΩ doesn't load the
  amplifier. Reveltronics asks for more than 1 MΩ.

## Turning them on

In `src/ecu/core/calib.h`:

```cpp
AuxUse aux_use[2] = {AuxUse::Egt, AuxUse::Wideband};   // AUX_AN1, AUX_AN2
```

An input set to `None` is ignored completely. That's the default, so a board
with nothing on AUX behaves exactly as before.

- **EGT scaling:** `egt_v0/egt_c0/egt_v1/egt_c1`. The defaults (0 V = 0 °C,
  5 V = 1250 °C) match the EGT-K.
- **Wideband scaling:** `wb_v0/wb_l0/wb_v1/wb_l1`. Program the controller's
  linear output to span at least λ 0.7 to 2.0, then enter its 0 V and 5 V
  lambda here. The defaults (0.5 at 0 V, 2.5 at 5 V) are placeholders [VERIFY].
  Lambda is used, not AFR, so the diesel/petrol stoich difference doesn't matter.

## What the ECU does with them

**EGT protection** (any time a probe is set up):

- Below 650 °C nothing happens.
- From 650 °C to 750 °C the fuel limit comes down linearly to 60 %. The truck
  stays drivable, and the EGT stops climbing.
- If a fitted probe reads 0 V or 5 V, or reads under 60 °C after a minute of
  running, it sets DTC `egt`. The fuel limit then holds at 85 % until the
  probe is fixed.

The thresholds are `egt_derate_start_c`, `egt_derate_end_c`, `egt_min_scale`
and `egt_fault_scale`, all marked [VERIFY]. 650 °C pre-turbo is a common
sustained limit for the 1KZ-TE's aluminium head. Start there and move it
only with evidence.

**Wideband live tuning** (only while this ECU is fueling, in takeover):

- **At the smoke limit**, the ECU compares lambda with `lambda_target` for
  that rpm (1.22-1.30 to start). It trims that boost/rpm cell of the smoke
  map:
  - Too rich: the trim comes down at `trim_rate_rich`. That's fast.
  - Lean with margin: the trim goes back up at `trim_rate_lean`. That's
    slow, and only while the EGT is healthy and at least 50 °C below its
    derate point.
- **Trims are bounded:** -15 % to +5 %. Fuel can never go above `q_max`,
  the full-load curve, whatever the wideband says. EGT protection still
  applies on top.
- **Rich floor:** richer than λ 1.10 anywhere (a boost leak, a split
  intercooler hose) pulls up to 30 % of fuel within a fraction of a second.
  The pull eases off once lambda recovers.
- **Shadow mode:** the stock ECU is fueling, so the readings are only
  logged. Nothing is learned from them.
- **No EGT, no lean trim:** if the EGT probe isn't fitted or has failed, the
  wideband can only take fuel away, never add it.

The trims live in RAM for now, so each drive starts from the calibration
again. Every drive's log has them (`smoke_trim`). The "maps coming back"
step (calibration upload, LOGGING.md) is how good trims get saved into the
map, after you've looked at them.

## In the logs

New channels, all at 100 Hz:

| Channel | What it is |
|---|---|
| `egt`, `egt_v` | Exhaust temperature, °C, and its raw voltage |
| `lambda`, `wb_v` | Lambda, and its raw voltage |
| `egt_scale` | How much of the fuel limit the EGT allows (%) |
| `rich_cut` | How much the rich floor is pulling (%) |
| `smoke_trim` | The live trim at the current cell (%) |
| `q_limit` | The final fuel limit |

New flags: `egt_limit`, `wb_valid`, `wb_learn`. New DTCs: `egt`, `wideband`.
`tools/junkrlog.py` decodes them all.

## What the simulator shows (`pio test -e native -f test_exhaust`)

| Test | Result |
|---|---|
| Towing (40 % hotter exhaust) | Unprotected, EGT runs past 760 °C. With protection it never reaches 750 °C, and fuel drops only part way. |
| 15 % less air than the map assumes | The trim settles the smoke limit on the lambda target, within 0.03, and only the cells in use move. |
| Plenty of air | The trim adds a little, at most +5 %, and never goes past `q_max`. |
| Boost leak (λ about 1.0) | Fuel drops more than 8 % within 1.5 s, and lambda is back above the floor afterwards. |
| No EGT fitted | No lean trim. |
| Shadow mode | Nothing learned. |
| Unplugged sensors | Each sets its DTC. |
| Nothing on AUX | Behaves exactly as before. |

## Next

- Bench: feed J7-7/8 from a bench supply, check `egt`/`lambda` in the live
  view (BENCH_TEST.md stage 2).
- On the truck: shadow mode first. Log a few weeks of EGT and lambda with
  the stock ECU fueling, to see where the stock smoke limit actually sits
  before setting `lambda_target`.
- Later: a CAN wideband (rusEFI or AEM X-series CAN) would free AUX_AN2. A
  second EGT (post-turbo) fits on AUX_AN2 if the wideband moves to CAN.
