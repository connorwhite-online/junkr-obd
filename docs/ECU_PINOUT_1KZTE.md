# Toyota 1KZ-TE ECU Connector Pin-out (KZJ95 – 1996)

Source: Electrical Wiring Diagram pages 206-208 (images 64-65)

*ECU Part No. 89661-#### (Manual & Auto)* – 34-pin single-row connector

| Cav | Wire Colour | OEM Designation | Signal direction | Junkr tap? | Notes |
|-----|-------------|-----------------|------------------|------------|-------|
| 1   | B-R         | +B              | In  (+12 V batt) | —          | Battery constant power |
| 2   | B-R         | +B1             | In  (+12 V IGN)  | 5 V logic  | Switched via GAUGE fuse |
| 3   | Br          | E01             | GND              | ADC GND    | Sensor ground |
| 4   | Br          | E1              | GND              | ADC GND    | Sensor ground |
| 5   | Br          | E02            | GND              | ADC GND    | Sensor ground |
| 6   | P-K         | VC              | Out (+5 V ref)   | monitor    | Do **not** load |
| 7   | B-R         | VA              | Out (+5 V ref 2) | —          | Un-used on diesel |
| 8   | B-R         | IDL             | In  (TPS idle)   | ADC ch1    | From throttle sensor |
| 9   | B-Y         | VTA             | In (TPS angle)   | ADC ch2    | 0.5-4.5 V |
| 10  | B-Y         | PIM             | In (MAP sensor)  | ADC ch3    | 0.5-4.5 V |
| 11  | LG          | VRP             | In (Pump pick-up) | opto IN4  | 0–12 V square wave |
| 12  | G-B         | VRT             | In (Pump pick-up) | opto IN5  | 0–12 V square wave |
| 13  | LG          | THA             | In (Intake temp) | ADC ch4    | NTC thermistor |
| 14  | G-R         | THW             | In (Coolant temp) | ADC ch5   | NTC thermistor |
| 15  | —           | TE2             | Bi-dir diag      | spare IO   | Blink code when shorted to E1 |
| 16  | —           | VF1             | Out (rich/lean)  | ADC ch6    | 0-5 V mixture voltage |
| 17  | —           | TE1             | Bi-dir diag      | spare IO   | |
| 18  | —           | TT              | Out (timing)     | opto OUT?  | Used with timing light |
| 19  | B-R         | BATT S          | In (+12 sense)   | ADC ch7    | Battery for OBD logging |
| 20  | L-B         | STA             | In (start)       | DI ch1     | High during crank |
| 21  | W-R         | NSW             | In (neutral SW)  | DI ch2     | Auto trans only |
| 22  | Y-W         | SPV             | Out (spill valve)| monitor    | Injector pump PWM |
| 23  | L-G         | TAC             | Out (timing adv) | monitor    | |
| 24  | W-B         | ACT             | Out (advance sol)| monitor    | |
| 25  | Y-K         | S-REL           | Out (starter RL) | —          | |
| 26  | W-B         | SP1             | In (speed)       | DI ch3     | 5 V square |
| 27  | Br-R        | STP             | In (stop lamp)   | DI ch4     | 12 V |
| 28  | Br-R        | IMI             | Bi-dir (immob.)  | —          | |
| 29  | Br-R        | IMO             | Bi-dir (immob.)  | —          | |
| 30  | —           | EGR             | Out (EGR VSV)    | monitor    | |
| 31  | —           | H-REL           | Out (heater RL)  | monitor    | |
| 32  | —           | ISCV            | Out (idle valve) | monitor    | |
| 33  | —           | NE              | In (engine speed)| opto IN1   | 5 V square (>2 kHz at redline) |
| 34  | —           | G- / G+         | In (pump TDC)    | opto IN2/3 | Dual-phase pulses |

> **Colours** per Toyota legend: Br = Brown, B-R = Brown-Red, B-Y = Black-Yellow, P-K = Pink, LG = Light-Green, G-R = Green-Red, L-B = Light-Blue, W-R = White-Red, Y-W = Yellow-White, W-B = White-Black.

---

## Board-tap key

* **ADC chX** – Routed to ADS131M08 differential channel X via 1 MΩ/100 kΩ divider.
* **opto INx** – 5-24 V square-wave sensed via HCPL-0630 opto isolator.
* **monitor** – Only observed (high-impedance > 1 MΩ), not used in control loop.
* **—** – Passed straight through, not tapped on Intercept board.

---

### Diagnostic connector cross-reference

The under-dash *CHECK CONNECTOR* (21-pin OP3) provides easy access to
E1, TE1, TE2, VF1 and BATT.  Use those for quick bench testing if you
cannot insert the Intercept board.

---

## Revision history

| Date | By | Notes |
|------|----|-------|
| 2025-11-12 | Cursor AI | Initial draft from EWD pages 206-208 |

