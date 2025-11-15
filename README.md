# JNKR Gauge / ECU-Intercept System  **(1KZ-TE Land Cruiser)**

> **2025 architecture refresh – ESP32-S3 + three custom PCBs + round RGB display**
>
> *Old Arduino-Mega / Nextion material has moved to [`docs/legacy/`](docs/legacy/).*

The project now consists of:

1. **ECU-Intercept Board** – inline with Toyota 34-pin ECU harness, taps analogue signals via ADS131M08 and streams isolated SPI.
2. **Dash I/O Board** – carries the Adafruit *Qualia ESP32-S3* (#5800), drives a 2.1" 480 × 480 RGB-666 TFT, receives data from the intercept board, speaks RS-485 to the engine bay.
3. **Engine-bay Sensor Node** – sealed module that measures EGT (MAX31856), wideband AFR (LSU4.9), and baro/thermo; sends `SensorFrame` packets over RS-485.

Key documentation:

* [`docs/BOARD_ARCHITECTURE.md`](docs/BOARD_ARCHITECTURE.md) – block diagram & connectors
* [`docs/ECU_PINOUT_1KZTE.md`](docs/ECU_PINOUT_1KZTE.md) – 34-pin ECU cheat-sheet
* [`docs/RS485_PROTOCOL.md`](docs/RS485_PROTOCOL.md) – packet spec between boards
* [`docs/CUSTOM_PCB_DESIGN.md`](docs/CUSTOM_PCB_DESIGN.md) – PCB details for all three boards
* [`docs/WIRING.md`](docs/WIRING.md) – updated harness guidance

Firmware examples:

* `examples/ecu_analog_intercept_v2.ino` – **Dash Master** (ESP32-S3) – SPI ingest, RS-485 parse, TFT output
* `examples/engine_node_v0.1.ino` – **Engine-bay Node** (ESP32-C3) – sends dummy `SensorFrame`

Hardware source lives in `hardware/` – KiCad 7 projects (intercept-board, dash-io-board, engine-bay-node).

---

## Quick Start (developers)

```bash
# clone repo
$ git clone https://github.com/connorwhite/junkr-obd.git && cd junkr-obd

# open docs
$ open docs/BOARD_ARCHITECTURE.md

# compile dash firmware
$ arduino-cli compile --fqbn esp32:esp32:adafruit_qs3 examples/ecu_analog_intercept_v2.ino
```

---

## Legacy material

The previous single-board Arduino Mega + Nextion implementation is still available for reference but is **no longer maintained**.  Find it under:

```
docs/legacy/
examples/legacy/
```

---

## Licence

MIT – © 2025 JNKR Project

---

## ⚠️ Disclaimer

**IMPORTANT**: This system is for monitoring purposes only. It does not modify any engine control signals.

- Use at your own risk
- Improper installation can damage sensors or vehicle systems
- Always verify readings against known good gauges
- Do not rely solely on this system for critical safety decisions
- Author assumes no liability for damage or injury

---

## 💡 Credits

- Inspired by the [jnkr-ecu](https://github.com/connorwhite-online/jnkr-ecu) project
- Based on Arduino Mega 2560 platform
- Nextion HMI display for visualization
- Thanks to the automotive Arduino community

---

## 📞 Support

For questions, issues, or support:
- Open an [Issue](https://github.com/connorwhite-online/jnkr-gauge/issues)
- Check the [Wiki](https://github.com/connorwhite-online/jnkr-gauge/wiki)
- Join discussions in Issues

---

**Built with ❤️ for the turbodiesel community**

**Version 1.0.0** | Last Updated: 2025-11-07