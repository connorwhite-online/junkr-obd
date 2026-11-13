# Standalone ECU: bench and car test plan

Each stage has to pass before the next. Nothing here fuels the engine
until stage 6, and stage 6 only happens with the stock ECU plugged into
the tails and the RUN switch in reach.

**Safety rules for every stage**

- Diesel runaway is the failure that matters. The engine can be stopped
  with the RUN switch (stock ECU takes back over), the key, and, last
  resort, by blocking the intake. Know where each is before cranking.
- Fused bench supply (5 A) for stages 1-4. Never a car battery until
  stage 5.
- Never probe the SPV or TCV stage with a grounded scope on +B side
  nodes; use a differential probe or a current clamp.

## Stage 1: bare board power-up (no MCUs flashed)

1. Board alone on a current-limited 13.8 V supply (limit 300 mA) into
   +B and IGSW on the AUX connector.
2. Check rails: 5 V (V5), 3.3 V (3V3, 3V3_RF), VREF = 3.000 V ± 3 mV,
   VC **off** (VC_OFF has a pull-up: our sensor 5 V must stay off at reset).
3. Idle current should be under 150 mA. Thermal camera or finger: nothing
   warm except the buck.
4. Reverse the supply briefly through the PTC (it must not conduct; the
   SS310 OR-diodes block). Pulse 30 V for 1 s: SMCJ33A clamps, the buck
   survives.

## Stage 2: flash and talk

1. USB-C to a laptop: the ESP32 enumerates. Flash it, then let it flash the
   STM32 (ROM bootloader over USART1) and the ATtiny (UPDI). SWD (J10) is
   there as a fallback.
2. The ESP32 serves the tuner app. The STM32 status line (`D,...`) shows
   up at 50 Hz.
3. SAFE_OK rises 0.5 s after boot (supervisor sees the heartbeat), ARM
   stays low (RUN switch off).

## Stage 3: inputs, with a signal generator

Use a second ESP32 DevKitC running the simulator (`src/ecu/core/sim.h`) as
a stimulator, or a function generator:

1. NE: 52-3×4 pattern into the NE VR input, 30 Hz to 1 kHz tooth rate.
   The app shows rpm within 1 %, and sync never drops over 10 minutes.
2. TDC: one pulse per 180 deg, phase-swept: the measured advance follows.
3. Every analog channel: 0-5 V sweep, check the reading and that the input
   draws under 0.1 mA (47k + follower: the stock ECU must not notice us).
4. Every mux channel: 0 and 12 V.

## Stage 4: outputs into dummy loads

1. SPV: 1.5 Ω 50 W resistor in series with a real spare SPV if possible.
   With ARM up (RUN switch on, takeover forced in bench mode), command
   injections at 750 and 4000 rpm simulated. Check on the scope:
   - peak current and peak time, then hold current `[VERIFY]` values
   - turn-off: clamp at about 58 V, current to zero in < 150 µs
   - ISPV (INA240) reading matches a current clamp
2. Pull RUN: SPV current stops within one segment, relays drop.
3. Kill the STM32 (hold NRST): the ATtiny drops SAFE_OK within 50 ms.
4. Short SPV_IN low while forcing the gate high with a jumper (simulates
   a shorted FET): the ATtiny trips on SPV_SHORT.
5. TCV into 12 Ω: duty 0-95 % tracks the command. Shift solenoids and
   VSV outputs into 12 V bulbs.

## Stage 5: in the car, shadow mode, engine off then running

The stock ECU goes on the tails; this board goes on the harness. RUN
switch off. In this state the stock ECU drives everything through the
relay NC contacts.

1. Key on, engine off: the stock ECU must behave exactly as before (no
   new DTCs, check-engine lamp test normal). Our board reads every sensor.
2. Start and drive. Log for a week of normal use:
   - our rpm vs the stock tach output
   - stock SPV opening angle and TCV duty per rpm/pedal cell (the learner)
   - measured advance
3. Export the learned map. Fill the `[VERIFY]` table in
   [DESIGN.md](DESIGN.md) from the logs: TDC offset and sign, TCV
   frequency, SPV current profile (scope the stock one).
4. The comparator must reach 20,000 agreeing segments in a row.

## Stage 6: first takeover, stationary

Engine warm, idling, parked, handbrake on, someone at the key.

1. Flip RUN on. Idle should not change by more than 30 rpm.
2. Flip RUN off and on ten times: no stumble, no DTCs on the stock ECU.
3. Blip the throttle to 2500: compare our Q with the learned stock value.
4. Check the limiter on the stand: 4600 taper, 4900 cut. Watch for smoke.

## Stage 7: driving

Short trips first, RUN switch in reach, shift logic checked in 2 and L
before D. Lock-up last.

## Pass criteria summary

| Stage | Must show |
| --- | --- |
| 1 | rails right, VC off at reset, idle < 150 mA, survives reverse and 30 V |
| 2 | all three MCUs flashed from USB, SAFE_OK up, ARM down |
| 3 | rpm within 1 %, no sync loss in 10 min, inputs draw < 0.1 mA |
| 4 | SPV current profile right, every trip path drops ARM in < 50 ms |
| 5 | stock ECU unaffected, map learned, 20,000-segment agreement |
| 6 | takeover and handback invisible at idle |
