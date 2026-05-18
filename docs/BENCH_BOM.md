# Bench Testing BOM

What you need to actually run through [`BENCH_TESTING.md`](BENCH_TESTING.md).
Split into:

1. **Bring-your-own from the product BOM** — parts you're buying anyway
   for the final build that get used during bench bring-up too.
2. **Bench-only essentials** — tools and consumables you need to do the
   bring-up but won't end up in the car.
3. **Optional debugging gear** — nice-to-haves that pay for themselves
   the first time something doesn't work the way the firmware claims.

US suppliers throughout. Search URLs over direct product pages so they
stay valid.

---

## 1. From the product BOM (already on your shopping list)

These come from [`BOM.md`](BOM.md). Listed here so you know which stage
needs them on hand.

| Item | First used in stage | Notes |
| ---- | ------------------- | ----- |
| Adafruit Qualia ESP32-S3 | 2 | Cabin MCU. |
| ESP32-C3 SuperMini | 3 | Engine-bay MCU. Buy 2 — easier to debug if you fry one. |
| Adafruit MAX31855 breakout (PID 269) | 3 | Thermocouple amp. |
| Adafruit ADS1115 breakout (PID 1085) | 3 | 16-bit ADC. |
| TJA1051 / SN65HVD230 modules × 2 | 4 | CAN transceivers, one per MCU. |
| K-type thermocouple probe | 3 (loop test), 5 (real) | Or just a short loop of copper wire for Stage 3. |
| 10 kΩ NTC + a real automotive coolant sensor | 5 | Bench-only NTC for Stage 3 can be the resistor pair below. |
| MAP sensor (0.2–4.7 V output, 0–29 psi) | 5 | Pin down the part in Stage 6. |
| 120 Ω 1 % resistors × 2 | 4 | CAN bus termination, one each end. |
| 10 kΩ 1 % resistors × ~5 | 3 | Divider sims + final NTC divider. |

---

## 2. Bench-only essentials

| Item | Approx. $ | Link |
| ---- | --------- | ---- |
| **Solderless breadboard, full-size (~830 tie points)** | $5–10 | [Amazon US](https://www.amazon.com/s?k=full+size+breadboard+830) · [SparkFun](https://www.sparkfun.com/search/results?term=full+size+breadboard) |
| **Half-size breadboard** (for the second MCU during Stage 4) | $3–5 | [Amazon US](https://www.amazon.com/s?k=half+size+breadboard) |
| **Jumper wire kit** — M-M, M-F, F-F, mixed lengths | $8–12 | [Amazon US](https://www.amazon.com/s?k=breadboard+jumper+wire+kit+mixed) · [Adafruit](https://www.adafruit.com/?q=jumper+wire+kit) |
| **Alligator clip test leads** — set of 10, mixed color | $7–10 | [Amazon US](https://www.amazon.com/s?k=alligator+clip+test+leads) · [SparkFun](https://www.sparkfun.com/search/results?term=alligator+clip) |
| **USB-C cables × 2** — data-capable (not charge-only) | $8–15 | [Amazon US](https://www.amazon.com/s?k=usb-c+data+cable+3ft) |
| **Multimeter** — any decent autoranging one | $25–60 | [Amazon US](https://www.amazon.com/s?k=autoranging+multimeter) · Astro AI, Klein, Fluke 101+ |
| **10 kΩ linear trim pot** (or any 5–25 kΩ pot) | $1–3 | [Digi-Key](https://www.digikey.com/en/products/result?keywords=10k+linear+trim+pot+breadboard) · [Adafruit](https://www.adafruit.com/?q=trim+pot+10k) |
| **Twisted-pair wire, 22–24 AWG, ~2 m** | $5–10 | [Amazon US](https://www.amazon.com/s?k=22awg+twisted+pair+stranded) — or twist your own from two pieces of solid-core. |
| **Hand vacuum / pressure pump** (Mityvac MV4525 or equivalent) | $40–80 | [Amazon US](https://www.amazon.com/s?k=mityvac+MV4525) · [Summit Racing](https://www.summitracing.com/search?searchTerm=hand+vacuum+pump) |
| **Soldering iron** with adjustable temp 200–450 °C | $25–80 | [Amazon US](https://www.amazon.com/s?k=adjustable+soldering+iron+pinecil) · Pinecil V2 (~$30) is excellent. |
| **Solder + flux** | $10–15 | [Amazon US](https://www.amazon.com/s?k=63%2F37+solder+0.6mm) |
| **Thermometer** — anything that reads water + air temp accurately | $10–20 | [Amazon US](https://www.amazon.com/s?k=digital+kitchen+thermometer) |
| **Mug / glass container** for the warm-water NTC bath | — | The kitchen. |

**Approximate bench-only total: $150–300** depending on what you already own.

---

## 3. Optional debugging gear (worth it if you don't have it)

| Item | Approx. $ | Why it pays off | Link |
| ---- | --------- | --------------- | ---- |
| **Logic analyzer** (Saleae clone, 8-channel, 24 MHz) | $12–20 | Lets you see SPI / I²C bytes on the wire — the difference between "it doesn't work" and "ah, MISO is stuck low because CS is floating". Use [Saleae Logic 2](https://www.saleae.com/pages/downloads) (free with clones in legacy mode) or sigrok-cli. | [Amazon US](https://www.amazon.com/s?k=saleae+logic+analyzer+24mhz) |
| **USB-to-CAN sniffer** (CANable v2, Innomaker USB2CAN, Macchina M2) | $30–60 | Three-way debug on the CAN bus — capture frames between your two MCUs to confirm IDs, payloads, and bit timing. Pairs with [SavvyCAN](https://savvycan.com/) (free GUI). | [Amazon US](https://www.amazon.com/s?k=canable+v2) · [Innomaker](https://www.amazon.com/s?k=innomaker+usb2can) |
| **Bench power supply** — variable 0–30 V, 3 A | $50–120 | Necessary for Stage 5 (MAP needs 5 V regulated) and for confirming the engine-bay shield's 12 V input survives 9–16 V cranking simulation. | [Amazon US](https://www.amazon.com/s?k=variable+bench+power+supply+30v+3a) |
| **Oscilloscope** — entry-level 50–100 MHz | $200–400 | Only really needed if you have to debug analog noise (MAP signal, NTC line). A logic analyzer covers digital. | [Siglent SDS1052X-E search](https://www.amazon.com/s?k=siglent+sds1052x-e) · [Rigol DS1054Z search](https://www.amazon.com/s?k=rigol+ds1054z) |
| **ESD wrist strap** | $5–10 | The ESP32-S3 and TJA1051 don't love ESD. Cheap insurance. | [Amazon US](https://www.amazon.com/s?k=esd+wrist+strap) |
| **Helping hands / PCB vise** | $15–40 | Frees both your hands when soldering jumpers to a breakout. | [Amazon US](https://www.amazon.com/s?k=pcb+vise+third+hand) · [SparkFun](https://www.sparkfun.com/search/results?term=helping+hands) |

---

## What you absolutely cannot skip

If your bench is bare and you're starting from zero, prioritize this
list in order — each unblocks the next stage:

1. Breadboard + jumper kit
2. Two USB-C cables
3. Multimeter
4. Soldering iron + solder (for putting headers on the breakouts)
5. 10 kΩ pot
6. Twisted pair wire + alligator clips
7. (Stage 5 only) hand vacuum/pressure pump

Logic analyzer and CAN sniffer become useful when something goes wrong
in Stage 3 or 4 — don't preemptively buy them, but bookmark them for if
the bring-up gets stuck.

---

## Cross-references

- Sensor + connector parts: [`BOM.md`](BOM.md)
- Stage-by-stage walkthrough: [`BENCH_TESTING.md`](BENCH_TESTING.md)
- PCB design once bench is green: [`../kicad/README.md`](../kicad/README.md)
