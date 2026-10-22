# Learning tuner: watch, learn, map

An open-source loop that watches the 1KZ-TE through the ECU interposer,
learns how the truck responds, and builds piggyback correction maps for
performance and economy across altitude, temperature and load. The stock
ECU stays in charge of fuel and timing. We only reshape what it *sees* on
two low-current inputs (see [ECU_TAP_ARCHITECTURE.md](ECU_TAP_ARCHITECTURE.md)).

Status: **simulation only.** The learner, the safety logic and the
firmware lookup are written and tested against a simulator of the engine,
the stock ECU and the truck. Nothing has run on the truck yet, and the
simulator's numbers are not a promise of real gains.

```
 sensors ──▶ tap board (ESP32-S3) ──▶ ECU sees pim_seen, thw_seen
                │  logs every channel + the dither it applied
                ▼
            drive logs ──▶ python -m tuner (laptop) ──▶ new tables
                                                         │
             src/tap/tables_gen.h  ◀── python -m tuner export
```

## 1. What it can bend

| Lever | Signal faked | What the ECU does with it | Limits |
| --- | --- | --- | --- |
| `pim_off` | PIM (boost pressure) | The smoke limiter allows fuel in proportion to the air it thinks it has. **+** = more fuel when smoke-limited; **−** = keeps a raised boost under the ~15 psi fuel cut. | −30 … +25 kPa, then capped every sample (below) |
| `thw_off` | THW (coolant temp) | The timing map carries a cold-advance term. A small **−** borrows some of it. | −15 … 0 °C, warm engine only (THW ≥ 75 °C) |

The ECU doesn't control boost (the wastegate is pneumatic), and we don't
touch the spill valve or timing valve. Both were ruled out on purpose: see
the piggyback decision in [ECU_INTERPOSER_RESEARCH.md](ECU_INTERPOSER_RESEARCH.md).

Each lever is a 4-D table over **baro × intake temp × rpm × pedal**
(3 × 3 × 10 × 8 cells), with multilinear interpolation. Altitude and
temperature are table axes, so one map covers sea level to ~3000 m and
winter to desert.

## 2. Sensor modules

The core needs nothing beyond what the interposer already taps (rpm, pedal,
PIM, THA, THW, speed, spill-valve angle). Each extra module unlocks
something specific, and **the learner may only push a quantity as far as it
can measure the consequence.**

| Module | Channels | What it adds |
| --- | --- | --- |
| `wideband` | λ (LSU 4.9 via a rusEFI-style CAN controller, 0x190, λ 0.5–2.0) | Smoke-limit model and the λ ratio cap. Works on diesel (lean λ range). |
| `egt` | EGT (K-type, MAX31855) | Thermal constraint |
| `baro` | absolute pressure | Better altitude axis (otherwise estimated from PIM at key-on) |
| `ambient`, `iat2` | ambient temp, post-intercooler temp | Exogenous temperature covariate |
| `gps` | speed, altitude, grade | Grade-corrected power |
| `imu` | longitudinal accel | Wheel-power estimate (the performance signal) |
| `fuel_flow` | L/h | Real economy signal (otherwise a spill-valve proxy) |

**Authority by capability** (`tuner/channels.py`):

| Fitted | Max extra fuel |
| --- | --- |
| wideband **and** EGT | 20 % |
| one of them | 8 % |
| neither | **0 %**. The learner can still lower PIM (boost-cut protection) and use the THW lever. |

## 3. How it learns

The method is **safe extremum seeking** with a Bayesian local model per
table cell.

1. **Dither.** While learning, the tap board adds a small square wave to
   each lever: PIM ±2 kPa every 2 s, THW ±3 °C every 24 s. The periods
   differ, so their effects can be separated. Each lever has a settle time
   after every flip (0.4 s / 3 s), and samples inside it are ignored.
2. **Regress on the dither, not the absolute value.** For every sample the
   learner knows exactly what offset it applied (`u − tbl`), so the
   response to that offset is a clean experiment, even while the table
   changes between rounds. Covariates are exogenous only: ambient
   temperature and baro, not THA, because THA is downstream of what we
   change.
3. **Per-cell Bayesian least squares.** Each sample is spread over the 16
   surrounding cells by its interpolation weight. Each cell keeps a small
   normal-equations accumulator (7 features) for the objective and for EGT.
   Old data is forgotten each round (factor 0.6). Sparse cells are pooled
   toward their environment neighbours.
4. **Objective.** A pedal-weighted blend: wheel power (from IMU + GPS) at
   high pedal, energy per litre (from fuel flow) at part load. `--mode`
   slides the blend from −1 (economy) to +1 (power).
5. **Step only on confident improvement.** A cell moves one or two steps
   only if the *lower* confidence bound of the gain is positive
   (z = 1.5). It backs off if the evidence turns against it.
6. **Constraints are pessimistic.** EGT is checked at mean + 2σ against
   700 °C, and only when the step adds fuel.

### The λ ratio cap

Black-box fitting of λ was too uncertain, so the learner uses structure
instead. When the ECU is smoke-limited it targets a fixed air/fuel ratio
for the pressure it is told, so

    real λ = stock λ × pim / pim_seen

From smoke-limited samples the learner estimates `k = λ × pim_seen / pim`
(the stock target) per cell, takes a pessimistic `k − 2σ`, and stores
`pim_rcap = (k − 2σ) / λ_min` with λ_min = 1.20. The **firmware enforces
that ratio every sample**, which also covers turbo-lag transients that a
slow table cannot.

## 4. Real-time safety (firmware)

`src/tap/piggyback.h` runs on the tap board every sample, in this order:

1. table value, clamped to the lever's hard limits
2. learning dither, if enabled
3. PIM: authority cap (fitted sensors) → λ ratio cap → **boost-cut
   margin** (never let the ECU see more than 204 − 6 kPa)
4. THW: no offset below 75 °C

The bypass relay is normally closed, so any fault (power, watchdog, a
sensor out of range) drops it and the ECU sees the real sensors again.
The header is plain C++ and is tested on the host against the Python
version (`tuner/tests/test_firmware.py`).

## 5. Simulator results

`python -m tuner campaign --rounds 15 --drives 4` runs 60 random training
drives in three environments, then scores fixed test cycles against stock.
Faster is better for WOT and hill times. Run on 2026-10-22:

| Fitted modules | Env | WOT 40–100 | Hill climb | Economy | min λ | max EGT |
| --- | --- | --- | --- | --- | --- | --- |
| all | sea level, 25 °C | 0.0 % | +2.4 % | 0.0 % | 1.19 | 662 °C |
| all | **1800 m, 10 °C** | **+4.6 %** | **+15.2 %** | +0.1 % | 1.24 | 649 °C |
| all | hot, 40 °C | +1.0 % | +2.2 % | 0.0 % | 1.24 | 687 °C |
| wideband + IMU + GPS | 1800 m | +4.6 % | +14.0 % | +0.1 % | 1.24 | 651 °C |
| none | all three | 0 | 0 | 0 | ≥ 1.28 | ≤ 683 °C |

No run went past a hard limit (λ < 1.12, EGT > 730 °C, or a boost cut) on
any training sample, and no evaluation cycle hit the fuel cut.

What this says:

- **The gains are at altitude and on spool-up**, where the stock smoke
  limiter is the bottleneck. At sea-level WOT the stock ECU is mostly
  demand-limited, so there's little to unlock. That matches the headroom an
  oracle finds in the same simulator.
- **Without a wideband or EGT it changes nothing that adds fuel**, by
  design.
- **Economy is flat.** The THW lever only works in a narrow band the
  learner doesn't reach yet (the ECU needs to see < 80 °C), and in this
  simulator `--mode -1` and `--mode +1` end up almost the same as balanced.
  Economy gains, if they exist on the truck, will need a better lever.
- min λ dips to 1.19 at sea level, 0.01 under the 1.20 target. That's the
  pessimistic cap working at its edge, well clear of the 1.12 hard limit,
  but worth watching on real logs.

## 6. Honest limits

- The simulator is calibrated to published 1KZ-TE numbers (≈96 kW,
  ≈343 Nm, ~10 psi stock boost, ~15 psi cut). It is not the truck. The
  first real step is logging with learning **off**, then fitting the
  simulator to those logs.
- The stock ECU's real smoke-limit and timing maps are unknown. The
  learner only assumes the smoke limiter is monotonic in PIM.
- More fuel means more heat and more stress on a 25-year-old engine. The
  20 % authority and the 700 °C EGT ceiling are conservative starting
  points, not proven-safe numbers.
- A future lever worth testing is scaling the pump's correction resistor
  (VRP) at key-on, which would raise max fuel globally.

## 7. Using it

```
python -m tuner campaign --rounds 15 --drives 4 --modules all --save state.npz
python -m tuner export state.npz --out src/tap/tables_gen.h --json tables.json
python -m unittest discover -s tuner/tests -t .     # 25 tests, ~20 s, needs numpy (+ g++ for the firmware test)
```

Real-log ingestion (CSV from the tap board → `Learner.observe`) uses the
same channel names as the simulator (`tuner/channels.py`).

## Sources

- Constrained Bayesian optimisation for engine calibration: the review
  literature on safe, constrained online calibration that motivated the
  pessimistic-constraint design.
- HP Academy diesel tuning forum: smoke-limit λ targets around 1.15–1.25,
  and LSU 4.9 wideband sensors used on diesels.
- IH8MUD and newhilux.net 1KZ-TE threads: ECU boost cut ≈15 psi, and a
  common EGT guideline under 1000 °F (≈540 °C post-turbo; our 700 °C
  ceiling assumes a pre-turbo probe).
- rusEFI wideband controller: CAN ID 0x190, λ 0.5–2.0.
- Open-source diesel "tuning box" write-ups (EEVblog forum, StrikeEngine)
  confirming that sensor-signal piggybacks (rail or boost) are how
  commercial boxes work.
