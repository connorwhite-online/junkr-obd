# Cabin Shield — Schematic / Netlist

Tiny breakout that pairs with the Qualia ESP32-S3. Hosts the CAN
transceiver, the 120 Ω termination jumper, the Deutsch harness plug,
and a 4-wire ribbon to the Qualia (3.3 V, GND, TWAI_TX, TWAI_RX). The
Qualia is powered separately over USB-C; the harness `+12V` line lands
on a no-connect pad here so it's still terminated mechanically.

## Block summary

```
        Qualia ESP32-S3 GPIO header (via J1, 4-pin ribbon)
            +3V3   GND   TWAI_TX   TWAI_RX
              │     │       │         │
              └─────┴───────┼─────────┘
                            │
                       U1 TJA1051 ── CAN_H / CAN_L ──► J2 (DT06-4S harness plug)
                                    │
                                  120 Ω term (R1) gated by JP1

        J2 also carries +12V (no-connect on this board, harness side
        feeds the engine-bay shield only).
```

## Designators

| Ref | Part | Notes |
| --- | ---- | ----- |
| J1 | 1×4 pin header / JST-XH or Stemma-like | Qualia connection (3.3V, GND, TWAI_TX, TWAI_RX) |
| J2 | Deutsch DT06-4S female plug | Harness out |
| U1 | TJA1051T/3 breakout (or HW-194 module) | Same module family as the engine-bay shield's U6 |
| R1 | 120 Ω 1 % 0805 | CAN termination |
| JP1 | 1×2 pin header + shunt | Term enable jumper |
| C1 | 100 nF 0805 X7R | U1 decoupling |
| C2 | 10 µF 1206 X5R | Local bulk on the 3.3 V line into U1 |
| H1, H2 | M3 mounting holes, non-plated | Two corners |

## Net-by-net wiring

| Net | From | To |
| --- | ---- | -- |
| `+3V3` | J1.1 | U1 VCC, C1+, C2+ |
| `GND` | J1.2 | U1 GND, U1 S (silent), C1−, C2− |
| `CAN_TX` | J1.3 | U1 TXD |
| `CAN_RX` | J1.4 | U1 RXD |
| `CAN_H` | U1 CANH | J2.3, JP1.1 |
| `CAN_L` | U1 CANL | J2.4, R1.2 |
| (term path) | JP1.2 | R1.1 |
| `+12V_NC` | J2.1 | **No-connect** (label as NC; do not run to anything) |
| `GND` (harness) | J2.2 | tie to local `GND` |

> Pulling +12V across to the cabin enclosure is intentional: leaves the
> option open to power the Qualia from the harness later if you want to
> ditch USB-C. For revision 1 it's just terminated to a pad and
> ignored.

## Qualia connection (J1, 4-pin)

| Pin | Signal | Qualia GPIO |
| --- | ------ | ----------- |
| 1 | +3V3 | 3V3 (any of the Qualia's 3.3 V outputs) |
| 2 | GND | GND |
| 3 | TWAI_TX | GPIO5 |
| 4 | TWAI_RX | GPIO6 |

Use a 4-conductor 28 AWG silicone ribbon (≤ 30 cm) terminated in
whatever connector matches the Qualia's exposed I/O header. JST-SH /
JST-PH / 0.1" header all work — pick the one that mates with the Qualia
variant you have.

## Connector pinout — J2 (DT06-4S, harness plug)

Same as the engine-bay J1 (this is the mating end of the same harness):

| Pin | Signal |
| --- | ------ |
| 1 | +12 V (no-connect on cabin board) |
| 2 | GND |
| 3 | CAN_H |
| 4 | CAN_L |
