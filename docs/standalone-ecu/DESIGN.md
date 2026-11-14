# JUNKR standalone ECU (rev A): design

A plug-in replacement engine and transmission computer for the 1996 KZJ95
Prado: 1KZ-TE with the Denso ECD-V3 electronic distributor pump and the
A343F 4-speed automatic. It plugs into the stock E5 / E6 / E8 harness
plugs, so nothing in the car is cut.

It is built to be safe to try. It ships in **shadow mode**: the stock ECU
stays in the car (on the optional tails) and keeps driving the engine,
while this board reads every sensor, watches every stock output, and
learns the stock calibration. It can only take over when the stock ECU
and this board have agreed for a long run, and the RUN switch on the dash
is on. Drop the switch and the stock ECU is back in charge within one relay
time (about 10 ms).

> Status: designed, simulated and unbuilt. Every number tagged `[VERIFY]`
> below is an engineering estimate that must be measured before takeover.
> See [BENCH_TEST.md](BENCH_TEST.md).

![Board, top](../../kicad/standalone_ecu/renders/board_top.png)

## What it controls

| Function | Stock actuator | Pin | How |
| --- | --- | --- | --- |
| Fuel quantity | Spill valve (SPV, 1-2 Ω) | E5-11 | peak and hold, low side, scheduled in NE pump angle |
| Injection timing | Timing control valve (TCV, 10-14 Ω) | E5-12 | PWM, closed loop on the TDC vs NE phase |
| SPV supply | Spill valve relay (SVR) | E8-13 | relay coil, cut on overspeed |
| Shifts | A343F S1, S2 | E8 | low side, solenoid table below |
| Lock-up | SL | E8 | low side, 4th gear only |
| EGR, intake throttle (S/TH1, S/TH2), S-REL | VSVs | E8 | low side |
| Main relay | M-REL | E5 | high side, diode-ORed with the stock ECU |
| Lamps, tach, VF, IMO, TT, ACT, A/C | | | open drain, wired-OR with the stock ECU |

A343F solenoids: 1st S1 on S2 off, 2nd on/on, 3rd off/on, OD off/off. With
both off the valve body gives OD in D, 3rd in 2 and 1st in L, so a dead
board still drives the truck home.

## How the pump is driven

The ECD-V3 pump has no injectors to time. The NE sensor reads a 52-tooth
wheel on the pump's roller ring: 64 slots per pump turn (720 deg CA), with
3 teeth missing at 4 places. Each 180 deg segment is one cylinder: 13 teeth,
11.25 deg apart, then a gap.

- **Fuel:** the SPV is normally open. Energized, it closes and the plunger
  pressurizes fuel; de-energizing it spills and ends injection. Quantity is
  therefore the NE angle at which the SPV opens. The firmware converts the
  requested Q (mm³/stroke) into "tooth N plus D µs", with the valve's
  response time taken off, and TIM5 fires the edge.
- **Timing:** the timer piston turns the roller ring, so the NE teeth move
  against the crank. The TDC sensor is on the crank, so where the TDC edge
  falls inside the NE segment *is* the actual advance. A PI loop drives the
  TCV duty to hold the target advance. Cranking, stall and lost TDC fall
  back to a fixed duty, exactly as the stock ECU does (and set DTC 14).
- **Failure is no fuel.** Anything that goes wrong leaves the SPV open:
  no sync, ARM low, supervisor trip, overspeed, no power. The SVR relay
  then removes SPV power as a second layer.

## Board

200 × 140 mm, 6 layers (signal / GND plane / 3 inner signal / signal; GND poured on every layer), 353 parts on
10 schematic sheets. The harness header (TE 176122-6, the Toyota ECU
header) sits on the top edge; the three tail footprints (rev C interposer
geometry) behind it carry the stock ECU's side in shadow mode.

| Block | Parts |
| --- | --- |
| Engine MCU | STM32H743ZIT6 (480 MHz M7, LQFP-144), 25 MHz crystal, REF3030 3.000 V ADC reference |
| Comms MCU | ESP32-S3-WROOM-1-N16R8: Wi-Fi tuner app, logging to microSD, CAN gauge, flashes the STM32 (ROM bootloader on USART1) and the ATtiny (UPDI) |
| Safety | ATtiny1616 supervisor, 74LVC1G08 ARM chain, RUN switch input |
| VR inputs | 2× MAX9926 (NE, TDC, SP2, spare), mode A2, 10k + 1 nF |
| Analog | 12 channels: MCP6004 followers on 5 V, 47k in, 10k/15k to the ADC |
| Slow digital | 3× CD74HC4067, 48 channels through 100k/22k dividers |
| Edge sensing | LM2903 comparators on harness SPV, TCV, SP1, TAC |
| Low-side outputs | 5× DRV8803 (20 channels) |
| SPV stage | UCC27517 gate driver, IPD50N10S3L, 20 mΩ shunt + INA240A1, SUD50P10 + SS310 recirculation, SMCJ58A clamp |
| TCV stage | IPD50N10S3L, freewheel to +B, 50 mΩ shunt + INA240A1 |
| Takeover | 3× G5LE (SPV, SVR, TCV), 4× G5V-2 (S1/S2, SL/EGR, S/TH1/S/TH2, S-REL) |
| Power | TPS54360B 5 V buck from +B / IGSW (diode-OR), PTC + SMCJ33A load dump, 2× AP2112K 3.3 V, TPS7B4253 sensor VC tracking an LM4040 5.000 V |
| I/O | USB-C, microSD, CAN (SN65HVD230), Micro-Fit 2×6 AUX, SWD, UPDI |

Everything is generated from one file, `kicad/standalone_ecu/tools/circuit.py`:
the schematic (`gen_sch.py`), the board (`gen_pcb.py` placement, `route.py`
routing), the firmware pin map (`gen_pins.py` → `src/ecu/board_pins.h`) and
the fab outputs (`gen_fab.py`). KiCad's own netlist export is checked
against `circuit.py` with `check_netlist.py` (0 mismatches).

## Shadow mode and takeover

```
            harness  ──┬───────────────► stock ECU (tails)       inputs: paralleled, high-Z on our side
                       └───────────────► this board

  output:   harness H_x ── relay COM
                           relay NC ──► stock ECU S_x            relay off (default): stock drives
                           relay NO ──► our driver O_x           relay on: we drive
```

- **Inputs** are shared. Our analog front end is 47k into a follower, so
  the stock ECU sees no load. Our own sensor pull-ups and VC are off unless
  no stock ECU is detected (VC present at boot means the stock ECU is
  there).
- **Takeover outputs** go through Form-C relays in four groups: FUEL (SPV,
  SVR), TIME (TCV), TRANS (S1, S2, SL) and AIR (EGR, S/TH, S-REL). Relay coil
  power is VRLY, which only exists while ARM is up. So a crashed MCU, a
  supervisor trip or the RUN switch dropping all put the stock ECU back.
- **Lamps, tach and the like** are wired-OR (open drain), so either ECU can
  light them and the stock ECU's diagnostics keep working.
- **Learning:** for every segment the firmware records where the stock ECU
  opened the SPV, the stock TCV duty and the measured advance, binned by
  rpm and pedal (`ShadowLearner`). That becomes the starting map, so
  takeover begins from what Toyota did, not from guesses.
- **Agreement gate:** our own plan is compared with the stock SPV edge on
  every segment. Takeover (`T` from the app) is refused until 20,000
  consecutive segments agree within 1.5 deg (about 15 minutes of driving).

## Safety chain

`ARM = ARM_REQ (STM32) AND SAFE_OK (ATtiny) AND RUN_SW (dash)`, in logic
gates, not firmware. ARM gates SPV, TCV and SVR commands and powers the
takeover relays.

The ATtiny1616 supervisor (`src/ecu_safety`) drops SAFE_OK and latches
until the engine stops when:

- the STM32 heartbeat stops for 50 ms
- its own NE count says more than 5300 rpm (the STM32 cuts fuel at 4900 and
  the SVR at 5200)
- the gated SPV command stays on for more than 8 NE teeth (90 deg), or
  400 ms: a stuck-on spill valve is the runaway case for a diesel
- the harness says the SPV is energized while our command is off (a
  shorted driver), checked only when we own the harness

The STM32 adds sync-loss fuel cut, pedal plausibility (VA range, VA vs IDL
switch), sensor range checks with limp defaults (coolant 80 °C, no boost
credit) and the OEM stall behaviour.

## Firmware

| Path | What |
| --- | --- |
| `src/ecu/core/` | Engine core, header-only, no heap, host-testable: NE/TDC decoder, fuel and SPV planner, timing PI, A343F, safety, shadow learner, pump/engine simulator |
| `src/ecu/stm32/main.cpp` | STM32H743 HAL layer (Arduino STM32 core): TIM2 input capture on all timing edges, TIM5 SPV scheduler, TIM1 SPV peak/hold, TIM8 TCV, muxed inputs, ESP32 link. Builds for the board and for a NUCLEO-H743ZI: `pio run -e ecu` |
| `src/ecu_safety/` | ATtiny1616 supervisor: `pio run -e ecu-safety` |
| `test/test_ecu/` | 18 tests against the simulator: sync and rpm at 150-4500 rpm, noise rejection, advance measurement, delivered Q, cold start to idle, rev limiter, supervisor cut, stall, pedal and sensor faults, A343F shifts, shadow learning, supervisor trips: `pio test -e native -f test_ecu` |

The simulator closes every loop the firmware has (crank inertia, torque per
mm³ with a timing-efficiency penalty, a first-order timer piston, real SPV
response delay), so "start, idle at 700, hold the limiter" is tested on
every build.

## What is not known yet `[VERIFY]`

| Item | Why it matters | How to find out |
| --- | --- | --- |
| Spill table (Q vs SPV angle) | fuel quantity | shadow mode learns stock SPV angles; dyno or smoke check after |
| TDC offset and sign, TDC teeth per rev | timing loop | shadow mode against stock TCV, timing light on #1 |
| SPV peak time, hold duty, clamp | valve life and response | scope the stock SPV current (INA240 on our side reads it in shadow mode) |
| TCV frequency and duty range | timing loop | scope the stock TCV in shadow mode |
| PIM, VA and thermistor curves | sensor scaling | compare with stock readings via the existing tap / diagnostic |
| IDL switch polarity, range switch logic | limp and shifts | meter on the car |
| Immobiliser (IMI / IMO) | standalone start on immobilised cars | KZJ95 JDM may have none; shadow mode passes it through |
| A/C, ACT direction | A/C cut | meter on the car |
| Relay coil power (~2 W at 14 V) | heat | measure on the bench |
| SUD50P10 availability | SPV recirculation | alternate: any 100 V, < 50 mΩ P-FET in DPAK |
| COMP1 on PB0 | SPV current chop in hardware | firmware v0 uses timer peak/hold instead |
| Space behind the glovebox for 210 × 150 × 40 mm | fit | measure |
| Header shroud height (24 mm assumed) | enclosure | measure the TE header |
