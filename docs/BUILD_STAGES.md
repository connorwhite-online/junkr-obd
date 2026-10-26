# Build Stages and Parts

Three stages, each with a parts list you can click through. Adafruit and
Pololu prices and stock were checked 2026-10-26; the rest are rough.

| Stage | You get | Parts |
| --- | --- | --- |
| 1. Tap + web app | Boost, temps, rpm, speed, volts, check-engine light and baro on your phone | ~$150 |
| 2. Round gauge | Boost, temps, rpm and alerts on the dash | ~$90–140 |
| 3. Engine-bay box | EGT, real boost, outside air, extra temps | ~$270–420 |

```
            engine bay  ┆  cabin
 ┌────────────────────┐ ┆    ┌──────────────┐          ┌────────────────────┐
 │ Engine-bay node (3)│ ┆    │  Stock ECU   │          │ Phone web app  (1) │
 └─────────┬──────────┘ ┆    └──────┬───────┘          └─────────▲──────────┘
           │            ┆   12 back-probes                       ┆ Wi-Fi
           │            ┆    ┌──────┴───────┐                    ┆
           └────CAN─────┼────┤ Tap box  (1) ├┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘
                        ┆    └──────┬───────┘
                        ┆           └──CAN──── Round gauge (2), clock hole
```

## Stage 1: Tap the ECU (simple version)

An ESP32 Feather plus two off-the-shelf ADC boards, wired to 12 pins on
the back of two ECU plugs (E6 and E8). Everything is in stock at Adafruit
or Amazon, about $150, and there's no custom PCB.

**Why it isn't just software:** the 1996 ECU has no data port and sends
nothing, so there's nothing to plug into. You listen to the sensor wires
directly. Those carry 5 V and 12 V, and the ESP32's pins die above 3.3 V,
so each wire gets a resistor divider or an optocoupler. That and wiring is
the whole hardware job; everything else is software.

```
E6 plug, 8 wires ──► dividers (68k/100k) ──► ADS1115 ×2 ──I²C──┐
E8 plug, 4 wires ──► PC817 optocouplers ───────────────────────┤
fuse box (add-a-fuse) ──► 12→5 V buck ──USB-C─────────────────►├─ ESP32-S3 Feather
                                                               │  + SD logger, BME280, CAN
                                                               └─ Wi-Fi ─► phone web app
```

**You get:** boost, coolant, intake, fuel and ATF temps, pedal, rpm,
speed, battery volts, the check-engine light, baro and cabin temp.
**Not yet:** injection timing and fuel quantity. Those come off the pump
sensors and spill valve on E5, which need the full custom tap board, so
that becomes a later upgrade.

### The 12 taps

No cutting: back-probe pins slide in behind the wire at the back of the plug.

| Pin | Wire | Goes to |
| --- | --- | --- |
| E6-2 | PIM (boost) | ADS #1 |
| E6-4 | THW (coolant) | ADS #1 |
| E6-3 | THA (intake air) | ADS #1 |
| E6-5 | THF (fuel temp) | ADS #1 |
| E6-13 | THO (ATF temp) | ADS #2 |
| E6-11 | VA (pedal) | ADS #2 |
| E6-1 | VC (5 V reference) | ADS #2 |
| E6-9 | E2 (sensor ground) | ADC ground |
| E8-12 | +B (battery volts) | ADS #2, through a 100k/22k divider |
| E8-18 | TAC (rpm) | optocoupler |
| E8-9 | SP1 (speed) | optocoupler |
| E8-5 | W (check-engine light) | optocoupler |

Power comes from an add-a-fuse in the fuse box, not from the ECU.

### Buy

| Part | Qty | ~$ | Link |
| --- | --- | --- | --- |
| ESP32-S3 Feather | 1 | 17.50 | [Adafruit 5477](https://www.adafruit.com/product/5477) |
| Adalogger FeatherWing (SD card + clock) | 1 | 8.95 | [Adafruit 2922](https://www.adafruit.com/product/2922) |
| FeatherWing Doubler (solder area for the dividers) | 1 | 7.50 | [Adafruit 2890](https://www.adafruit.com/product/2890) |
| ADS1115 ADC | 2 | 29.90 | [Adafruit 1085](https://www.adafruit.com/product/1085) |
| BME280 baro/temp | 1 | 14.95 | [Adafruit 2652](https://www.adafruit.com/product/2652) |
| STEMMA QT cable, 200 mm | 3 | 3.75 | [Adafruit 4401](https://www.adafruit.com/product/4401) |
| CAN Pal transceiver (used in stage 2) | 1 | 3.95 | [Adafruit 5708](https://www.adafruit.com/product/5708) |
| PC817 optocouplers | 1 pack | 7 | [Amazon](https://www.amazon.com/s?k=pc817+optocoupler+dip) |
| 1 % resistor kit | 1 | 10 | [Amazon](https://www.amazon.com/s?k=1%25+metal+film+resistor+kit) |
| Back-probe pins | 1 kit | 12 | [Amazon](https://www.amazon.com/s?k=back+probe+pins+kit) |
| 12 V → 5 V USB-C buck | 1 | 10 | [Amazon](https://www.amazon.com/s?k=12v+to+5v+usb+c+buck+converter+hardwire) |
| Add-a-fuse + inline fuse | 1 | 8 | [Amazon](https://www.amazon.com/s?k=add+a+fuse+mini+blade) |
| 22 AWG wire + heat-shrink | 1 | 10 | [Amazon](https://www.amazon.com/s?k=22+awg+stranded+wire+kit) |
| High-endurance microSD, 32 GB | 1 | 10 | [Amazon](https://www.amazon.com/s?k=high+endurance+microsd+32gb) |

### Later upgrade: interposers + custom tap board

Adds injection timing and fuel quantity (E5) and replaces the back-probes
with a permanent fit. Parts list: [`kicad/tap/bom.csv`](../kicad/tap/bom.csv).

![The three interposer sections](build/interposers.png)

| One interposer, exploded | Tap box (66 × 46 × 18 mm) |
| --- | --- |
| ![E5 interposer exploded](build/interposer_exploded.png) | ![Tap box exploded](build/tapbox.png) |

### Where the baro sensor goes

- Pressure reads true baro anywhere in the cabin, so position only matters
  for the temperature reading.
- Hang the BME280 on its 200 mm cable below the glovebox, in footwell air.
  Keep it off the tap box (its electronics run warm) and out of heater
  ducts and sun.
- It reads cabin temp. True outside temp comes in stage 3, behind the
  grille.

### Codes

Try it today: key on, engine off, bridge **TE1–E1** in the check connector
under the hood, count the check-engine light flashes. The relay above adds
a "Read codes" button to the app (engine-off only).

## Stage 2: Round gauge

The firmware is already written. These are its real screens:

![Gauge pages rendered from the firmware](gauge/firmware/contact.png)

### Buy

| Part | Qty | ~$ | Link |
| --- | --- | --- | --- |
| Qualia ESP32-S3 | 1 | 19.95 | [Adafruit 5800](https://www.adafruit.com/product/5800) |
| 2.1" round display, no touch (touch version $39.95, [5792](https://www.adafruit.com/product/5792)) | 1 | 24.95 | [Adafruit 5806](https://www.adafruit.com/product/5806) |
| CAN transceiver module (SN65HVD230) | 2 | 8 | [Amazon](https://www.amazon.com/s?k=sn65hvd230+can+module) |
| 12 V → USB-C 5 V converter, hardwired | 1 | 10–15 | [Amazon](https://www.amazon.com/s?k=12v+to+usb+c+step+down+converter+hardwire) |
| Deutsch DT 4-pin connector kit | 1 | 15–20 | [Amazon](https://www.amazon.com/s?k=deutsch+dt+4+pin+connector+kit) |
| Twisted-pair wire, 20 AWG, ~3 m | 1 | 10 | [Amazon](https://www.amazon.com/s?k=20awg+twisted+pair+automotive+wire) |
| Momentary button (if the clock buttons are gone) | 1 | 3 | [Amazon](https://www.amazon.com/s?k=12mm+momentary+push+button) |
| USB-CAN sniffer for bench debugging (optional) | 1 | 30–60 | [Amazon](https://www.amazon.com/s?k=canable+v2) |

The bezel gets printed once the clock hole is measured.

## Stage 3: Engine-bay sensor box

| Sensor | Where |
| --- | --- |
| EGT | Pre-turbo, in the exhaust manifold collector |
| 3-bar MAP | Firewall, teed into the stock PIM hose |
| Outside air temp | Behind the grille, out of radiator airflow |
| Radiator outlet temp | Lower radiator hose |
| Turbo outlet air temp | Pipe from the turbo to the intake |
| Oil temp | Sump drain plug |
| Wideband (later) | Downpipe |

Everything wires to one IP65 box on the firewall
([`kicad/engine_bay/`](../kicad/engine_bay/)). Coolant and intake temps
already come from the ECU.

### Buy

| Part | Qty | ~$ | Link |
| --- | --- | --- | --- |
| ESP32-C3 SuperMini (one spare) | 2 | 8 | [Amazon](https://www.amazon.com/s?k=esp32-c3+supermini) |
| MAX31855 thermocouple amp | 1 | 14.95 | [Adafruit 269](https://www.adafruit.com/product/269) |
| ADS1115 ADC | 2 | 29.90 | [Adafruit 1085](https://www.adafruit.com/product/1085) |
| 5 V 500 mA buck D24V5F5 (+ 3.3 V twin D24V5F3) | 1 + 1 | 18 | [Pololu 2843](https://www.pololu.com/product/2843) · [2842](https://www.pololu.com/product/2842) |
| IP65 box ~80×60×30 mm + cable glands | 1 | 15–25 | [Amazon](https://www.amazon.com/s?k=ip65+abs+junction+box+small+cable+gland) |
| **EGT probe**, K-type ungrounded, 1/8" NPT, open tip | 1 | 25–60 | [Amazon](https://www.amazon.com/s?k=egt+probe+k+type+ungrounded+1%2F8+npt) |
| K-type extension lead, braided, 1.5 m | 1 | 10–15 | [Amazon](https://www.amazon.com/s?k=k+type+thermocouple+extension+braided) |
| 1/8"-27 NPT tap + 11/32" drill | 1 | 15 | [Amazon](https://www.amazon.com/s?k=1%2F8-27+npt+tap+11%2F32+drill) |
| **GM 3-bar MAP** 12569240 + pigtail | 1 | 25–45 | [Amazon](https://www.amazon.com/s?k=gm+3+bar+map+sensor+12569240+pigtail) |
| 1/8" barbed tee + 3 ft vacuum hose | 1 | 5 | [Amazon](https://www.amazon.com/s?k=1%2F8+barbed+tee+vacuum+hose) |
| Air temp sensors (outside air + turbo outlet), GM open-element | 2 | 20–30 | [Amazon](https://www.amazon.com/s?k=gm+open+element+intake+air+temperature+sensor) |
| Radiator hose temp adapter + sender | 1 | 20–35 | [Amazon](https://www.amazon.com/s?k=radiator+hose+temperature+sensor+adapter) |
| Oil temp sender + drain-plug adapter (measure the thread first) | 1 | 20–40 | [Amazon](https://www.amazon.com/s?k=oil+temperature+sender+drain+plug+adapter) |
| Wideband O2, later: Bosch LSU 4.9 + CAN controller | 1 | 200–300 | [Amazon](https://www.amazon.com/s?k=14point7+spartan+3+lsu+4.9) |

## Decide

- [ ] Start with the simple stage 1 and leave the custom tap board for later? Recommended.
- [ ] Read codes with a paperclip for now, and add a code-reader relay later? Recommended.
- [ ] Intercooler or not? Sets where the turbo-outlet sensor goes.
