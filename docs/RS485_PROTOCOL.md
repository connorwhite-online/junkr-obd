# RS-485 Packet Protocol – Engine-bay Node ↔ Dash I/O

**Version 0.1 – 2025-11-12**

Designed for half-duplex 1 Mbps on a 4-wire harness (A, B, +12 V, GND).

---

## Electrical layer

| Parameter | Value |
|-----------|-------|
| Transceiver | ISO1050 or SN65HVD230 (3.3 V) |
| Baud | **1,000,000 bps** (8-N-1) |
| Direction control | GPIO `RS485_EN` – HIGH = TX, LOW = RX |
| Termination | 120 Ω at Board B and Board C |
| Fail-safe | 560 Ω pull-up on A, pull-down on B |

---

## Frame format

```
+------+------+------+------------+---------+-------+
| 0x55 | LEN  | MSG  |  PAYLOAD   | CRC16   | 0xAA |
+------+------+------+------------+---------+-------+
   1      1      1     LEN-3 bytes    2       1
```
* **0x55**  – Start-of-frame
* **LEN**   – Total bytes **including** SOF and EOF (min = 7)
* **MSG**   – Message ID (see table)
* **PAYLOAD** – Variable length
* **CRC16**  – CCITT-16 (poly 0x1021) over all bytes _except_ SOF & EOF
* **0xAA**  – End-of-frame

> Byte-stuffing not required because 0x55/0xAA cannot appear in CRC-checked
> positions without the checksum failing. If your implementation prefers
> COBS or SLIP, wrap this frame wholesale.

---

## Message IDs

| ID | Direction | Payload | Description |
|----|-----------|---------|-------------|
| 0x01 | C → B | `SensorFrame` | Engine-bay sensors (EGT, AFR, baro, PCB temp) |
| 0x02 | B → C | `CfgRequest`  | Ask node to send board info or change rate |
| 0x03 | C → B | `CfgReply`    | Current settings, firmware version |
| 0x10 | B → C | `Ping`        | Empty payload; node echoes as 0x11 |
| 0x11 | C → B | `Pong`        | Empty payload |

### `SensorFrame` (ID 0x01)

| Byte | Field | Type | Unit |
|------|-------|------|------|
| 0-1  | `seq` | uint16 | Frame counter |
| 2-3  | `egt` | int16  | °C (signed, ×1) |
| 4-5  | `afr` | uint16 | λ ×1000 (14.7 = 14700) |
| 6-7  | `baro`| uint16 | hPa |
| 8-9  | `pcb` | int16  | °C |

Total = 10 bytes payload → LEN = 7 + 10 = 17.

### `CfgRequest` (ID 0x02)

| Byte | Field | Type | Description |
|------|-------|------|-------------|
| 0    | `code`| uint8 | 0 = info, 1 = set rate |
| 1    | `rate`| uint8 | Frames/s (only valid when `code` = 1) |

### `CfgReply` (ID 0x03)

| Byte | Field | Type |
|------|-------|------|
| 0-1  | `fwver` | uint16 (e.g. 0x0100 = 1.0) |
| 2    | `rate`  | uint8 | Current frame rate |

---

## Example packet

Engine node sends 100 Hz sensor frame:

```
55 11 01 00 01 07 D0 39 30 27 10 00 5A 2B AA
|  |  |  |   |   |...payload........| CRC |EOF|
```

* `LEN` = 0x11 (17)
* `seq` = 1
* `egt` = 2000 → 200 °C
* `afr` = 0x3930 → 14736 → λ = 1.00
* `baro` = 0x2710 → 10000 hPa → 1000 mbar
* `pcb` = 0x005A → 90 °C

CRC16 = 0x2B5A (little-endian)

---

## Implementation notes

* TX turnaround delay: keep `RS485_EN` asserted for 2 byte-times after EOF.
* Node sends `SensorFrame` at configurable 10-500 Hz; default 100 Hz.
* Dash should poll with `Ping` every 2 s to detect link loss.

---

© 2025 Junkr Project – MIT Licence
