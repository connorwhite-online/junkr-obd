# Build Stages and Parts

Three stages, each with a parts list you can click through. Adafruit and
Pololu prices were checked 2026-10-26; the rest are rough.

| Stage | You get | Parts |
| --- | --- | --- |
| 1. Tap + web app | Everything the ECU sees, on your phone, plus codes and baro | ~$250–450 |
| 2. Round gauge | Boost, temps, rpm and alerts on the dash | ~$90–140 |
| 3. Engine-bay box | EGT, real boost, outside air, extra temps | ~$270–420 |

```
            engine bay  ┆  cabin
 ┌────────────────────┐ ┆    ┌──────────────┐          ┌────────────────────┐
 │ Engine-bay node (3)│ ┆    │  Stock ECU   │          │ Phone web app  (1) │
 └─────────┬──────────┘ ┆    └──────┬───────┘          └─────────▲──────────┘
           │            ┆      interposer                        ┆ Wi-Fi
           │            ┆    ┌──────┴───────┐                    ┆
           └────CAN─────┼────┤ Tap box  (1) ├┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┘
                        ┆    └──────┬───────┘
                        ┆           └──CAN──── Round gauge (2), clock hole
```

## Stage 1: Tap the ECU

Read-only. The interposer sits between the ECU and its three plugs; the
tap box logs and serves the phone app.

![The three interposer sections](build/interposers.png)

| One interposer, exploded | Tap box (66 × 46 × 18 mm) |
| --- | --- |
| ![E5 interposer exploded](build/interposer_exploded.png) | ![Tap box exploded](build/tapbox.png) |

STLs: [`hardware/ecu_interposer/stl/`](../hardware/ecu_interposer/stl/).

### Buy

| Part | Qty | ~$ | Link |
| --- | --- | --- | --- |
| Interposer prints, MJF PA12 (upload the STLs) | 1 set | 50–100 | [JLC3DP](https://jlc3dp.com/) |
| TE 040 receptacle 175061-1 | 60 | 15–30 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=175061-1) |
| TE 040 tab 173682-1 | 60 | 15–30 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=173682-1) |
| TE 070 receptacle 173631-1 | 20 | 8–15 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=173631-1) |
| TE 070 tab 173645-1 | 20 | 8–15 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=173645-1) |
| 47 kΩ 0603 resistors (splices) | 60 | 3 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=RC0603FR-0747KL) |
| 0.5 A PTC fuse 1812L050/30MR | 2 | 2 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=1812L050%2F30MR) |
| Tap board PCB, 4-layer, assembled | 5 | 40–80 | [JLCPCB](https://jlcpcb.com/) |
| Tap board parts (whole list) | 1 | 60–90 | [kicad/tap/bom.csv](../kicad/tap/bom.csv) |
| 5 V 1 A buck, Pololu D24V10F5 | 1 | 12.95 | [Pololu 2831](https://www.pololu.com/product/2831) |
| **BME280 baro/temp sensor**, STEMMA QT | 1 | 14.95 | [Adafruit 2652](https://www.adafruit.com/product/2652) |
| STEMMA QT cable, 200 mm | 1 | 1.25 | [Adafruit 4401](https://www.adafruit.com/product/4401) |
| Code-reader relay TQ2-5V | 1 | 4 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=TQ2-5V) |
| High-endurance microSD, 32 GB | 1 | 10–15 | [Amazon](https://www.amazon.com/s?k=high+endurance+microsd+32gb) |
| Crimper for TE open-barrel terminals | 1 | 30–50 | [Amazon](https://www.amazon.com/s?k=engineer+PA-09+crimper) |

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

## Decide before the tap board gets laid out

- [ ] Code-reader relay on the tap board? Recommended.
- [ ] BME280 socket on the tap board? Recommended.
- [ ] Intercooler or not? Sets where the turbo-outlet sensor goes.
