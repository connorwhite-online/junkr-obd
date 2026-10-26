# Build Stages and Bills of Materials

The build in three stages. Each one works on its own before the next
starts:

1. **Observe.** The ECU tap box logs and shows everything the stock ECU
   sees, on your phone.
2. **Gauge.** The round gauge in the clock hole shows it on the dash.
3. **Sensors.** The engine-bay box adds what the ECU doesn't have: EGT,
   real boost, outside air, more temps, later wideband.

Arming the piggyback tune comes after stage 3, because the tuner's fuel
authority depends on EGT being fitted.

| Stage | New hardware | What you get | Approx. parts cost |
| --- | --- | --- | --- |
| 1. Observe | Interposer (3 plugs), tap board, tap box, baro/cabin sensor | Live data + logs in a web app, trouble codes, baseline for calibration | $250–450 |
| 2. Gauge | Qualia + 2.1" round display, CAN wire to the dash | Boost, temps, rpm and alerts on the dash | $90–140 |
| 3. Sensors | Engine-bay node, EGT probe, 3-bar MAP, NTCs, (wideband) | EGT, independent boost, outside air, extra temps | $270–420, +$200–300 for wideband |

Costs are rough, from the repo's BOMs and typical US prices, not fresh
quotes. Tools (crimper, multimeter, iron, bench supply) are extra and
mostly bought once, in stage 1.

```
            engine bay  ┆  cabin (behind glovebox and dash)
                        ┆
                        ┆    ┌──────────────┐          ┌────────────────────┐
                        ┆    │  Stock ECU   │          │ Phone web app  (1) │
                        ┆    │  read only   │          │ live, logs, codes  │
                        ┆    └──────┬───────┘          └─────────▲──────────┘
                        ┆   interposer, 3 plugs                  ┆ Wi-Fi
 ┌────────────────────┐ ┆    ┌──────┴───────┐                    ┆
 │ Engine-bay node (3)│ ┆    │ Tap box  (1) ├┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘
 │ EGT, 3-bar MAP,    ├─CAN──┤ SD, Wi-Fi,   ├──CAN──┐  ┌──────────────────┐
 │ outside air, temps │ ┆    │ BME280 baro  │       └──┤ Round gauge  (2) │
 └────────────────────┘ ┆    └──────────────┘          └──────────────────┘
```

The tap box is the hub: it reads the ECU, logs, and serves the phone in
stage 1. The gauge and the engine-bay node just join its CAN bus later.

## Stage 1: Tap the ECU and watch it

Stage 1 puts the [interposer](../hardware/ecu_interposer/) between the
ECU and its three plugs and reads every signal through 47 kΩ resistors,
so the ECU can't tell the box is there. Nothing is driven back into the
ECU. The tap box behind the glovebox logs to microSD and serves a web app
over its own Wi-Fi, so your phone becomes the display.

### What you'll see

All of this is already on the harness, so it needs no new sensors: rpm,
actual injection timing (°BTDC), injection quantity (spill-valve degrees),
TCV duty, boost from the stock PIM sensor, coolant, intake air, fuel and
ATF temps, pedal %, speed, battery volts (1 kHz while cranking), glow,
starter, lock-up, gear switches and the check-engine lamp. That's enough
to baseline the engine: warm-up times, cranking health, spool rpm, boost
vs rpm at full pedal, and where the ECU puts timing and fuel.

### Can we read codes? Yes, two ways

- **Free, today:** the 1KZ-TE is OBD1 with flash codes. Key on, engine
  off, bridge TE1 to E1 in the diagnostic check connector under the hood,
  and count the check-engine lamp flashes (steady 0.25 s blinking means no
  codes). Worth doing now, before any of this is installed, so you know
  the starting state.
- **In the web app:** the tap already logs the check-engine lamp live, so
  every time it comes on is recorded with what the engine was doing. A
  "Read codes" button adds one lead on TE1 (E6-15) plus one small relay
  that shorts it to E1 for 20 s while the tap decodes the flashes.
  Firmware only allows it with the engine stopped (no NE pulses) and speed
  zero, because TE1 grounded changes how the ECU behaves. This is the only
  thing in stage 1 that touches an ECU pin, and it's optional; recommended.

There is no live data stream to read (no OBD-II), and clearing codes
stays manual (pull the EFI fuse for 10 s).

### Ambient temperature and baro on the tap box

**Baro: yes, put it on the tap box.** The cabin isn't sealed, so a
pressure sensor anywhere inside reads true baro to within a few pascals.
Today the only baro is PIM read at key-on, which goes stale as soon as you
drive up or down a hill, and the tuner's biggest predicted gains are at
altitude ([LEARNING_TUNER.md](LEARNING_TUNER.md)). A continuous baro
channel fixes that. The stock PIM sensor is absolute, so the two can be
cross-checked at every key-on.

**Temperature: yes for cabin, no for ambient.** Behind the glovebox on an
RHD truck sits the heater and blower box, so that spot reads heater, sun
and electronics, not outside air. The ECU's own intake air sensor (THA)
heats up with boost and underhood heat, so it isn't ambient either. The
plan:

- Stage 1: a BME280 (pressure, temperature, humidity) on a 10–15 cm lead
  outside the tap box, so the board's own heat doesn't skew it. Log it as
  `baro` and `cabin_temp`.
- Stage 3: a sealed outside-air sensor behind the grille, on the
  engine-bay node's spare NTC input (J2). That's the real ambient number.

The tap board needs one small change for this: a 4-pin STEMMA QT / Qwiic
socket on its I²C bus. The board isn't laid out yet, so this is cheap to
add now.

### Work in this stage

1. Print the interposers in MJF nylon or resin and crimp them up with TE
   terminals. Build E6 first (all small 040 terminals, carries the
   sensors), then E8 (power, check-engine lamp, speed, tach), then E5
   (crank/pump sensors and spill valve, which give timing and fuel).
2. Lay out and order the [tap board](../kicad/tap/) with the Phase 2 parts
   (DAC, relays, bypass header) left off. Add the I²C socket and the
   optional TE1 relay.
3. Bench test it against the sensor simulator, as in
   [BENCH_TESTING.md](BENCH_TESTING.md).
4. Write the tap firmware: edge capture, the two ADCs, SD logging, Wi-Fi
   access point, live WebSocket feed. Only the piggyback lookup
   (`src/tap/piggyback.h`) exists today.
5. Write the web app: live dashboard, log list and download (CSV), events
   (crank, warm-up, boost runs), and the Read codes button.
6. Fit it, check every ECU-side voltage is unchanged with the box plugged
   and unplugged, then log a week of normal driving.

One caveat on the web app: an installable offline PWA needs HTTPS, and
the box can't serve that on its own Wi-Fi. It still adds to the home
screen and opens full-screen while you're connected to the truck.
Reviewing logs away from the truck means downloading them first, or a
later hosted viewer.

### Stage 1 BOM

| Item | Part / spec | Qty | Approx. $ |
| --- | --- | --- | --- |
| Interposer prints | MJF PA12 or tough resin: female plug, male shroud, chamber top + bottom, ×3 plugs | 1 set | 50–100 |
| 040 terminals | TE 175061-1 receptacle + 173682-1 tab, ~48 positions, order 60 + 60 | 120 | 30–60 |
| 070 terminals | TE 173631-1 receptacle + 173645-1 tab, ~16 positions, order 20 + 20 | 40 | 15–30 |
| Splice resistors + fuse | 47 kΩ 1 % 0603 ×60, 0.5 A PTC (1812L050/30MR) | 1 lot | 5 |
| Wire | 0.3–0.5 mm² and 0.85 mm² TXL, heat-shrink | 1 lot | 15–25 |
| Tap harness connectors | Molex Pico-Clasp 2×13 housings + terminals ×3 | 3 | 15–25 |
| Tap board PCB | 60 × 40 mm, 4 layers, 5 boards, partial assembly | 1 order | 40–80 |
| Tap board parts (Phase 1 only) | ESP32-S3-WROOM-1-N16R8, MCP3208 ×2, REF3030, MCP6004 ×4, MAX9926 ×2, 74LVC14 ×7, 74HC165 ×4, SN65HVD230, Pololu D24V10F5, microSD socket, USB-C, passives (see [`kicad/tap/bom.csv`](../kicad/tap/bom.csv)) | 1 | 60–90 |
| Baro / cabin sensor | BME280 breakout with STEMMA QT / Qwiic (Adafruit or SparkFun) + 15 cm cable | 1 | 12–18 |
| TE1 code-reader relay | TQ2-5V + 2N7002 + 1N4148W (optional) | 1 | 4 |
| microSD card | Industrial / high-endurance, 16–32 GB | 1 | 10–20 |
| Tap box | `tapbox_base` + `tapbox_lid`, FDM PETG or ASA | 1 | 5 |
| **Tools (one-time)** | Open-barrel crimper for TE 040/070, terminal extraction tool, multimeter, iron, bench supply (see [BENCH_BOM.md](BENCH_BOM.md)) | | 80–250 |

### Done when

- [ ] Codes read manually once with TE1–E1 and written down
- [ ] All three interposers fitted, engine starts and idles exactly as before
- [ ] Phone shows live rpm, boost, temps and timing from the box
- [ ] One log per key cycle on the SD card, downloadable from the app
- [ ] Baro on the box agrees with PIM at key-on within 1 kPa
- [ ] A week of baseline logs with no tune

## Stage 2: Round gauge in the clock hole

Stage 2 is mostly hardware and fitting, because the gauge firmware
already exists (`src/cabin/`) and renders all five pages. The tap box
already sends rpm, boost, temps, volts and the check-engine lamp on CAN,
so the gauge works fully on day one without stage 3. EGT and the
independent boost reading show dashes until the engine-bay box arrives.

### What you'll see

Drive page: real boost on one 270° arc with the ~15 psi fuel-cut tick,
plus coolant and intake temps (EGT blank for now). Temps page with the
10-minute coolant trend, Engine page (rpm, timing, injection quantity,
pedal, TCV, lock-up), Tune page (shows STOCK until the piggyback is
armed), and alert overlays for coolant, ATF, overboost, check engine, low
volts and lost bus. See [SENSORS_GAUGE_LOGGING.md](SENSORS_GAUGE_LOGGING.md) §5.

### Work in this stage

1. Measure the clock hole (diameter, depth behind it) and probe the clock
   connector against the wiring manual. It very likely has ignition 12 V,
   illumination (for night dimming) and ground, which means no new power
   wire.
2. Bench first: gauge plus tap box on the bench, replaying a stage 1 log
   onto CAN.
3. Run one twisted pair (CAN H/L) plus ground from the tap box to the
   dash.
4. Print a bezel/mount for the clock hole and fit the gauge, the small
   [CAN board](../kicad/cabin/) and a 12 V→5 V USB-C supply behind it.
5. One page button: reuse the clock's H/M buttons if they survive,
   otherwise a small momentary switch.

**Termination changes with stage 3.** A CAN bus needs 120 Ω at each
physical end. In stage 2 the ends are the tap box and the gauge, so close
the tap board's termination jumper (JP1). In stage 3 the engine-bay node
becomes the far end: move termination there and open the tap box jumper.

### Stage 2 BOM

| Item | Part / spec | Qty | Approx. $ |
| --- | --- | --- | --- |
| Gauge MCU | Adafruit Qualia ESP32-S3 for RGB-666 displays | 1 | 25–30 |
| Display | Adafruit 2.1" round 480×480 TL021WVC02 (5806 no touch, or 5792 touch) | 1 | 30–45 |
| CAN transceiver | TJA1051T/3 breakout (or SN65HVD230 module) + 120 Ω + jumper | 1 | 5–10 |
| Power | 12 V→5 V USB-C buck, automotive-rated, ≥1 A, with inline 2 A fuse | 1 | 10–15 |
| Harness | Deutsch DT04-4P / DT06-4S pair + contacts, 20 AWG twisted pair + 18 AWG ground, ~2 m, loom | 1 | 15–25 |
| Button | Momentary pushbutton (if clock buttons are gone) | 1 | 3 |
| Bezel | 3D-printed mount for the clock hole (ASA/PETG, heat in the dash) | 1 | 5 |
| **Tool (optional)** | USB-CAN sniffer (CANable v2) for bench debugging | 1 | 30–60 |

### Done when

- [ ] Gauge runs on the bench from a replayed stage 1 log
- [ ] Gauge in the dash, powered from the clock connector, dims with the headlights
- [ ] All pages live while driving, no bus-lost alerts
- [ ] Alerts tested (unplug the CAN link: gauge shows dashes and BUS LOST)

## Stage 3: Engine-bay sensor box

Stage 3 adds the numbers the ECU never had, which are the ones tuning
depends on: EGT first, then an independent 3-bar boost sensor, true
outside air temperature, and extra water, oil and air temps. It's one
small IP65 box on the firewall or inner fender (not on the engine), joined
to the same CAN bus. The board design and its firmware already exist
([`kicad/engine_bay/`](../kicad/engine_bay/), `src/engine_bay/`); this
stage builds it, adds a second ADC for the extra temps, and fits the
sensors. Everything it measures lands in the same log file and on the
gauge automatically.

### Sensors and where they go

| Sensor | Where | Input on the node |
| --- | --- | --- |
| EGT, K-type ungrounded probe | Pre-turbo, in the manifold collector ~2 in before the turbine flange | MAX31855 |
| 3-bar MAP (absolute) | Firewall, teed into the stock PIM hose | ADS1115 #1 A1 |
| Outside air temp | Behind the grille, out of the radiator airflow and sun | ADS1115 #1 A0 (the spare NTC input) |
| Oil pressure (optional) | Oil pressure switch port with a tee | ADS1115 #1 A2 |
| 5 V sensor supply monitor | On the board | ADS1115 #1 A3 |
| Oil temp | Sump drain-plug adapter or filter sandwich plate | ADS1115 #2 A0 |
| Radiator outlet temp | Inline adapter in the lower hose (with THW this gives the radiator's temperature drop) | ADS1115 #2 A1 |
| Turbo outlet air temp | Pre-intercooler pipe, if the truck has one; otherwise turbo outlet | ADS1115 #2 A2 |
| Spare | | ADS1115 #2 A3 |
| Wideband O2 (later) | Downpipe, post-turbo, 10 o'clock | Its own controller on CAN (0x190) |

Coolant at the head and intake air already come from the ECU's THW and
THA through the tap, so they aren't duplicated.

### Work in this stage

1. Build the engine-bay node and add a second ADS1115 at I²C address 0x49
   for the four extra temps.
2. Firmware: read the new channels, add one CAN frame for them (0x104),
   add them to the log and the gauge's Temps page. Verify the 3-bar MAP
   calibration end points against the datasheet.
3. EGT: pull the turbo (or manifold) so no swarf reaches the turbine,
   drill and tap 1/8" NPT, fit the probe. First check the collector has a
   flat boss and heat-shield clearance.
4. Fit the MAP sensor, the NTCs and the firewall grommet for the CAN pair
   and the sensor leads.
5. Move CAN termination from the tap box to this node.
6. Log another week with the tune still off. This is the baseline the
   piggyback gets compared against.
7. Later: wideband. A rusEFI-style CAN controller fits the frame the
   firmware already expects.

### Stage 3 BOM

| Item | Part / spec | Qty | Approx. $ |
| --- | --- | --- | --- |
| Node MCU | ESP32-C3 SuperMini (buy 2, one spare) | 2 | 8 |
| Thermocouple amp | Adafruit MAX31855 breakout (269) | 1 | 15 |
| ADCs | Adafruit ADS1115 breakout (1085) | 2 | 30 |
| CAN | TJA1051 module + 120 Ω | 1 | 5 |
| Power | Pololu D24V5F3 + D24V5F5, SS34, SMBJ24A, 1 A PTC, caps | 1 | 20 |
| Node PCB | 2-layer, per `kicad/engine_bay/` | 1 order | 10–20 |
| Enclosure | IP65 ABS/aluminium ~80×60×30 mm, cable glands, DT04/DT06-4 pair | 1 | 25–40 |
| EGT probe + lead | K-type ungrounded, 1/8" NPT compression, 1/4" Inconel, open tip + 1.5 m braided K extension | 1 | 35–75 |
| EGT install | 11/32" drill + 1/8"-27 NPT tap (or weld-in bung for post-turbo) | 1 | 15 |
| Boost | GM 3-bar MAP (12569240 / 16254609) + pigtail, 1/8" barbed tee, 3 ft hose | 1 | 30–50 |
| Outside air | Sealed 2-wire NTC air temp sensor (GM-style IAT or OEM ambient sensor) | 1 | 10–20 |
| Oil temp | 2-wire NTC sender + drain-plug or sandwich adapter (measure the thread) | 1 | 20–40 |
| Radiator outlet | 2-wire NTC + inline hose adapter sized to the lower hose | 1 | 20–35 |
| Turbo outlet air | GM-style IAT + weld-on or clamp bung | 1 | 15–25 |
| Wiring | Firewall grommet, 20 AWG twisted pair ~3 m, sensor wire, loom | 1 | 15–25 |
| **Wideband (later)** | Bosch LSU 4.9 + CAN controller (rusEFI wideband, or Spartan 3 / AEM X-Series with CAN) + weld bung | 1 | 200–300 |

### Done when

- [ ] EGT reads ambient with the engine cold and climbs on a pull; no thermocouple fault
- [ ] 3-bar MAP agrees with the stock PIM within 3 kPa across a pull, and with the tap box baro at key-on
- [ ] Outside air within 2 °F of a reference thermometer when parked in shade
- [ ] Extra temps show on the Temps page and in the log
- [ ] A week of baseline logs with everything fitted and the tune off

## Open questions

- Include the TE1 "Read codes" relay in stage 1? Recommended yes: one
  lead, one relay, engine-off only.
- OK to add the I²C socket for the BME280 to the tap board before layout?
- Does the truck have an intercooler? Decides where the turbo outlet air
  sensor goes.
- Measure: clock hole size and depth, clock connector pins (stage 2);
  manifold collector boss and the oil drain-plug thread (stage 3).
- Interposer prints: MJF PA12 from a print service, or resin at home? The
  fit tests so far were FDM.
