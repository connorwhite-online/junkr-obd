# ECU-Intercept Board – Schematic Stub

This text file mirrors the first-pass block diagram before we commit the actual KiCad `.kicad_sch` binary.

## Top-level blocks

1. **Toyota harness connector (J1) – 34-pin male**
2. **Toyota ECU connector (J2) – 34-pin female**
3. **High-impedance tap/buffer**
   * TLV9002 dual op-amp ×4 (8 channels)
   * 1 MΩ / 100 kΩ divider front-end, RC 1 k / 100 pF anti-alias
4. **Σ-Δ ADC** – TI ADS131M08 (8-ch, 24-bit, SPI)
5. **Digital isolator** – TI ISO7842 (4-ch, 100 Mbps)
6. **Power**
   * Automotive TVS SMBJ24A on +12 V
   * LM2841YMF (buck 5 V) → TPS563201 (3.3 V) for local logic
7. **SPI header to Board B** – 6-pin JST-PA (MISO, MOSI, CLK, CS, DRDY, GND)
8. **Test points & harness continuity LEDs**

## Net list sketch (key signals)

| Net | From | To |
|-----|------|----|
| `VC_SENSE` | J1-6 | Divider-A0 → Op-amp-A0 → ADC-CH0 |
| `VTA` | J1-9 | Divider-A1 → Op-amp-A1 → ADC-CH1 |
| `PIM` | J1-10 | Divider-A2 → Op-amp-A2 → ADC-CH2 |
| `THA` | J1-13 | Divider-A3 → Op-amp-A3 → ADC-CH3 |
| `THW` | J1-14 | Divider-A4 → Op-amp-A4 → ADC-CH4 |
| `VF1` | J1-16 | Divider-A5 → Op-amp-A5 → ADC-CH5 |
| `BATT_S` | J1-19 | Divider-A6 → Op-amp-A6 → ADC-CH6 |
| `RESERVED` | — | ADC-CH7 (future) |

Digital:
* `SPI_MISO`, `SPI_MOSI`, `SPI_CLK`, `SPI_CS`, `DRDY` leave isolator to J3.

Ground strategy:
* Sensor ground (E2) star-linked to op-amp reference and ADC REF-.
* Chassis ground isolated except shield tab.

---

### Next actions
1. Capture this net-list in KiCad 7 schematic.
2. Import Toyota connector footprints (Tyco 2-0963363-1 & 2-0963364-1) into local library.
3. Run ERC and add design rule notes.

