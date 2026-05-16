# Bill of Materials — Simple Gauge (Qualia ESP32-S3)

Three-parameter automotive gauge: **EGT**, **coolant temp**, **boost/MAP**.

All suppliers below are US-based (Adafruit, Digi-Key, Mouser, SparkFun, Amazon US,
RockAuto). Links use supplier search rather than direct product pages so they
stay valid as SKUs move — pick the in-stock variant that matches the spec.

---

## Cabin compute + display

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| Adafruit Qualia ESP32-S3 (TFT or IPS variant) | ESP32-S3, on-board 480×480 display, USB-C | 1 | $40–55 | [adafruit.com](https://www.adafruit.com/?q=qualia+esp32-s3) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=adafruit+qualia+esp32-s3) · [Mouser](https://www.mouser.com/c/?q=adafruit%20qualia%20esp32-s3) |

> Note: `src/main.cpp:50` currently calls `tft.init(320, 240)` against an ST7789
> driver — confirm this matches the panel variant you actually buy. The
> `project.xml` declares 480×480.

## Engine-bay MCU

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| ESP32-C3 SuperMini (or ESP32-S3 Mini) | RISC-V or Xtensa, built-in TWAI/CAN controller, small footprint, USB-C for flashing | 1 | $3–8 | [Amazon US (C3 SuperMini)](https://www.amazon.com/s?k=esp32-c3+supermini) · [Adafruit (ESP32-S3 Mini)](https://www.adafruit.com/?q=esp32-s3+mini) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=esp32-c3+mini) |

> Industrial-temp parts (`-I` / 105 °C ambient) are worth the small premium
> if the board mounts inside the engine bay rather than behind the firewall.

## CAN link

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| NXP TJA1051T/3 CAN transceiver (3.3 V I/O) | Or SN65HVD230 / MCP2562FD — high-speed CAN, 3.3 V logic | 2 | $1–3 ea | [Digi-Key](https://www.digikey.com/en/products/result?keywords=TJA1051T%2F3) · [Mouser](https://www.mouser.com/c/?q=TJA1051T%2F3) |
| CAN transceiver breakout (alt., for prototyping) | SN65HVD230 module or similar | 2 | $4–8 ea | [Amazon US](https://www.amazon.com/s?k=sn65hvd230+can+module) · [SparkFun](https://www.sparkfun.com/search/results?term=can+transceiver) |
| 120 Ω, 1 %, 1/4 W resistor | bus termination, one at each end | 2 | <$1 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=120+ohm+1%25+resistor) |

---

## Sensors

### EGT (exhaust gas temperature)

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| K-type thermocouple probe | M8×1.25 or 1/8" NPT, ≥1000 °C, Inconel sheath preferred | 1 | $15–60 | [Amazon US](https://www.amazon.com/s?k=k-type+thermocouple+egt+probe+1%2F8+npt) · [Summit Racing](https://www.summitracing.com/search?searchTerm=k-type+egt+probe) |
| Adafruit MAX31855 thermocouple amp breakout | SPI, cold-junction comp, 3.3 V | 1 | $15 | [adafruit.com](https://www.adafruit.com/?q=max31855) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=adafruit+max31855) · [Mouser](https://www.mouser.com/c/?q=adafruit%20max31855) |
| Weld-in EGT bung | matches probe thread | 1 | $5–10 | [Summit Racing](https://www.summitracing.com/search?searchTerm=egt+weld+bung) · [Amazon US](https://www.amazon.com/s?k=egt+weld+bung) |

### Coolant temperature

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| 10 kΩ NTC thermistor, β = 3950 K | for bench testing | 1 | $1–4 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=NTC+10k+3950) · [Mouser](https://www.mouser.com/c/?q=NTC%2010k%203950) |
| OEM-grade automotive coolant sensor (alt.) | NTC, threaded for your block — verify resistance curve and update `sensors.cpp` constants | 1 | $15–25 | [RockAuto](https://www.rockauto.com/en/catalog/) · [AutoZone](https://www.autozone.com/search?searchText=coolant+temperature+sensor) |
| 10 kΩ resistor, 1 %, 1/4 W | series resistor for divider (must match `SERIES_RESISTOR` in `sensors.cpp:10`) | 1 | <$1 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=10k+1%25+resistor+axial) |

### Boost / MAP

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| Pressure sensor, 0.2–4.7 V output, ~0–30 psi (gauge or absolute) | matches `V_ZERO = 0.2`, `V_MAX = 4.7` in `sensors.cpp:27–28` — e.g. NXP **MPX4250AP** or a 3-wire automotive MAP sensor | 1 | $15–40 | [Digi-Key (MPX4250AP)](https://www.digikey.com/en/products/result?keywords=MPX4250AP) · [Mouser](https://www.mouser.com/c/?q=MPX4250AP) · [RockAuto (GM 3-bar MAP)](https://www.rockauto.com/en/catalog/) |

> ⚠️ The README lists **MPX2200GP**, but that part outputs 0–40 mV (unamplified
> bridge) — it will *not* drive the ADS1115 to the 4.7 V the firmware expects.
> Either swap to an amplified part (MPX4250AP / MPX5700 / GM 3-bar) **or** add a
> bridge-amplifier circuit + new calibration constants. Recommended fix: buy an
> amplified MAP sensor and update `README.md:59`.

### Shared ADC

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| Adafruit ADS1115 16-bit ADC breakout | I²C, ±4.096 V range | 1 | $15 | [adafruit.com](https://www.adafruit.com/?q=ads1115) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=adafruit+ads1115) · [SparkFun](https://www.sparkfun.com/search/results?term=ads1115) |

---

## Power

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| 12 V → 3.3 V buck regulator | ≥500 mA, automotive-rated input (handle 16 V cranking + load dump) | 1 | $5–15 | [Pololu](https://www.pololu.com/search?query=3.3v+step-down+regulator) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=automotive+12v+3.3v+buck) · [Mouser](https://www.mouser.com/c/?q=automotive%2012v%203.3v%20buck) |
| TVS diode, SMBJ24A or SMBJ30A | load-dump protection on 12 V input | 1 | <$1 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=SMBJ24A) · [Mouser](https://www.mouser.com/c/?q=SMBJ24A) |
| Reverse-polarity / input diode (SS34 or similar) | optional but recommended | 1 | <$1 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=SS34) |
| Add-a-fuse tap (mini/ATC) | switched 12 V tap into the fuse box | 1 | $5–10 | [Amazon US](https://www.amazon.com/s?k=add+a+fuse+mini+atc) |
| Inline 3–5 A blade fuse + holder | upstream of buck | 1 | $3–8 | [Amazon US](https://www.amazon.com/s?k=inline+blade+fuse+holder) |

---

## Engine-bay PCB passives

| Item | Spec | Qty | Approx. $ |
| --- | --- | --- | --- |
| 0.1 µF ceramic caps, 0805 or through-hole | decoupling for MAX31855, ADS1115, MCU rail | 4–6 | <$1 |
| 10 µF electrolytic / tantalum | bulk decoupling on 3.3 V | 2 | <$1 |
| 4.7 kΩ resistors | I²C pull-ups on SDA + SCL | 2 | <$1 |
| 100 nF cap across thermistor | analog noise filter on ADS1115 CH0 | 1 | <$1 |

Source any of the above from [Digi-Key](https://www.digikey.com/) or
[Mouser](https://www.mouser.com/).

---

## Harness / Connector (DT04-4P)

| Item | Spec | Qty | Approx. $ | Link |
| --- | --- | --- | --- | --- |
| Deutsch DT04-4P (receptacle, PCB-side, 4-pin) | TE Connectivity | 1 | $3–6 | [TE.com](https://www.te.com/en/search.html?q=DT04-4P) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=DT04-4P) · [Mouser](https://www.mouser.com/c/?q=DT04-4P) |
| Deutsch DT06-4S (plug, harness-side, 4-pin) | TE Connectivity | 1 | $3–6 | [TE.com](https://www.te.com/en/search.html?q=DT06-4S) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=DT06-4S) · [Mouser](https://www.mouser.com/c/?q=DT06-4S) |
| Deutsch DT-series solid contacts (size 16) + wedgelocks | match pin/socket per side | 1 set | $5–10 | [Ladd Industries](https://www.laddinc.com/) · [Digi-Key](https://www.digikey.com/en/products/result?keywords=deutsch+DT+size+16+contacts) |
| Deutsch crimp tool (size 16, open-barrel) | DTT-16-00 or equivalent | 1 | $30–80 | [Amazon US](https://www.amazon.com/s?k=deutsch+dt+crimp+tool+size+16) · [Ladd Industries](https://www.laddinc.com/) |
| Twisted-pair automotive wire, 20 AWG | CAN_H / CAN_L pair — CAN spec calls for twisted pair | ~10 ft | $8–15 | [Wirebarn](https://www.wirebarn.com/) · [Del City](https://www.delcity.net/) · [Amazon US](https://www.amazon.com/s?k=20awg+twisted+pair+automotive) |
| Automotive hookup wire, 18 AWG TXL | +12 V switched + GND legs | ~10 ft | $8–15 | [Wirebarn](https://www.wirebarn.com/) · [Amazon US](https://www.amazon.com/s?k=18awg+TXL+automotive+wire) |
| Convoluted loom + heat-shrink | bundle + strain relief | ~6 ft | $5–10 | [Amazon US](https://www.amazon.com/s?k=automotive+wire+loom+1%2F4+inch) |

---

## Enclosure (optional)

| Item | Spec | Link |
| --- | --- | --- |
| Engine-bay enclosure | IP65+, ABS or aluminum, ~80×60×30 mm | [Polycase](https://www.polycase.com/) · [Hammond Mfg](https://www.hammfg.com/electronics/small-case) · [Amazon US](https://www.amazon.com/s?k=ip65+abs+project+enclosure) |
| In-dash bezel for Qualia | 3D-printed or aftermarket gauge pod | [Amazon US](https://www.amazon.com/s?k=single+gauge+pod) |

---

## Known issues to fix before ordering

- **Firmware split is not done yet.** Current `src/sensors.cpp` reads
  MAX31855 + ADS1115 directly on the cabin MCU; the new architecture
  requires moving that code into a separate `engine-bay/` PlatformIO env
  and reworking `src/` to be a TWAI listener. Follow-up PR.
- **`platformio.ini` does not list `Adafruit MAX31855`** even though
  `src/sensors.cpp:3` includes it. Add `adafruit/Adafruit MAX31855 library` to
  the engine-bay env's `lib_deps` when the split happens.
- **MAP sensor part vs. calibration mismatch:** README lists MPX2200GP
  (0–40 mV unamplified) but `src/sensors.cpp:27–28` expects a 0.2–4.7 V
  amplified output. Swap to MPX4250AP / GM 3-bar / similar before ordering.
- **Display init size** (`320×240` in `src/main.cpp:50`) does not match the
  declared 480×480 panel in `project.xml`.

---

## Rough total

| Bucket | Estimated cost |
| --- | --- |
| Cabin (Qualia) | $40–55 |
| Engine-bay MCU + CAN transceivers + termination | $10–25 |
| Breakouts (MAX31855, ADS1115) | $25–35 |
| Sensors (EGT probe + bung, thermistor, MAP) | $40–110 |
| Power + protection | $15–35 |
| Harness + Deutsch 4-pin + tooling | $50–125 (one-time tooling dominates) |
| Passives + enclosure | $15–40 |
| **Total** | **≈ $200–425** |
