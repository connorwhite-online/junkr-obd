"""Our own symbols: parts KiCad does not ship, plus the ECU header and tails.

Pinouts come from the datasheets named next to each part.
"""
import csv
import os

from symlib import make_symbol, write_lib

HERE = os.path.dirname(os.path.abspath(__file__))
PINOUT = os.path.join(HERE, "..", "..", "..", "hardware", "ecu_interposer", "pinout_kzj95_at.csv")

# Header pad = PAD0[plug] + Toyota pin (same numbering as the rev C tap board).
PAD0 = {"E8": 0, "E6": 22, "E5": 38}
NPINS = {"E5": 26, "E6": 16, "E8": 22}


def pinout():
    rows = {}
    with open(PINOUT) as f:
        for r in csv.DictReader(f):
            rows[(r["connector"], int(r["pin"]))] = r
    return rows


def pin_label(r):
    return r["name"] or "nc"


def build():
    po = pinout()
    syms = []

    # Analog Devices MAX9924-MAX9927 datasheet, Pin Description (MAX9926 column)
    syms.append(make_symbol(
        "MAX9926", "U", "MAX9926UAEE+", "Package_SO:QSOP-16_3.9x4.9mm_P0.635mm",
        [[(16, "IN1+", "L", "input"), (15, "IN1-", "L", "input"), (9, "IN2+", "L", "input"),
          (10, "IN2-", "L", "input"), (3, "BIAS1", "L", "passive"), (6, "BIAS2", "L", "passive"),
          (2, "EXT1", "L", "input"), (7, "EXT2", "L", "input"),
          (14, "VCC", "R", "power_in"), (1, "INT_THRS1", "R", "input"), (8, "INT_THRS2", "R", "input"),
          (13, "ZERO_EN", "R", "input"), (4, "COUT1", "R", "open_collector"),
          (5, "COUT2", "R", "open_collector"), (12, "DIRN", "R", "open_collector"),
          (11, "GND", "R", "power_in")]],
        desc="Dual differential VR sensor interface, adaptive threshold"))

    # TI DRV8803 datasheet SLVSAW5D, Figure 5-1 (DW, SOIC-20)
    syms.append(make_symbol(
        "DRV8803DW", "U", "DRV8803DW", "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm",
        [[(18, "IN1", "L", "input"), (17, "IN2", "L", "input"), (13, "IN3", "L", "input"),
          (12, "IN4", "L", "input"), (10, "~{ENBL}", "L", "input"), (11, "RESET", "L", "input"),
          (20, "~{FAULT}", "L", "open_collector"), (19, "NC", "L", "no_connect"),
          (1, "VM", "R", "power_in"), (2, "VCLAMP", "R", "passive"), (3, "OUT1", "R", "open_collector"),
          (4, "OUT2", "R", "open_collector"), (8, "OUT3", "R", "open_collector"),
          (9, "OUT4", "R", "open_collector"), (5, "GND", "R", "power_in"), (6, "GND", "R", "passive"),
          (7, "GND", "R", "passive"), (14, "GND", "R", "passive"), (15, "GND", "R", "passive"),
          (16, "GND", "R", "passive")]],
        desc="Quad protected low-side driver, 60 V, clamp diodes to VCLAMP"))

    # TI TPS7B4253-Q1 datasheet, Figure 4-1 (DDA HSOIC-8)
    syms.append(make_symbol(
        "TPS7B4253DDA", "U", "TPS7B4253QDDARQ1",
        "Package_SO:HSOP-8-1EP_3.9x4.9mm_P1.27mm_EP2.41x3.1mm",
        [[(8, "IN", "L", "power_in"), (5, "ADJ", "L", "input"), (2, "NC", "L", "no_connect"),
          (7, "NC", "L", "no_connect"),
          (1, "OUT", "R", "power_out"), (4, "FB", "R", "input"), (3, "GND", "R", "power_in"),
          (6, "GND", "R", "passive"), (9, "EP", "R", "passive")]],
        desc="40 V 300 mA voltage-tracking LDO, sensor supply, short-to-battery proof"))

    # TI UCC27517A datasheet, DBV SOT-23-5
    syms.append(make_symbol(
        "UCC27517A", "U", "UCC27517ADBVR", "Package_TO_SOT_SMD:SOT-23-5",
        [[(3, "IN+", "L", "input"), (4, "IN-", "L", "input"), (2, "GND", "L", "power_in"),
          (1, "VDD", "R", "power_in"), (5, "OUT", "R", "output")]],
        desc="4 A single low-side gate driver"))

    # The stock ECU header, TE 176122-6, as three units (one per harness plug)
    units = []
    for plug in ("E8", "E6", "E5"):
        n = NPINS[plug]
        lst = []
        for pin in range(1, n + 1):
            r = po[(plug, pin)]
            side = "L" if pin <= n // 2 else "R"
            lst.append((PAD0[plug] + pin, "%s-%d %s" % (plug, pin, pin_label(r)), side, "passive"))
        units.append(lst)
    syms.append(make_symbol(
        "TE_176122-6", "J", "TE 176122-6", "junkr_ecu:TE_176122-6_RA", units, width=30.48,
        desc="Stock 1KZ-TE ECU header (64-way hybrid). Unit A = E8 22P, B = E6 16P, C = E5 26P"))

    # Tails to the stock ECU (shadow kit): one symbol per plug, pad = Toyota pin
    for plug in ("E5", "E6", "E8"):
        n = NPINS[plug]
        lst = []
        for pin in range(1, n + 1):
            r = po[(plug, pin)]
            side = "L" if pin <= n // 2 else "R"
            lst.append((pin, "%d %s" % (pin, pin_label(r)), side, "passive"))
        syms.append(make_symbol(
            "TAILS_" + plug, "J", "Tails %s (to stock ECU)" % plug, "junkr_ecu:Tails_" + plug, [lst],
            width=25.4, desc="Solder pads for 150 mm tails to a new %s plug for the stock ECU" % plug))
    write_lib(syms)


if __name__ == "__main__":
    build()
    print("wrote junkr_ecu.kicad_sym")
