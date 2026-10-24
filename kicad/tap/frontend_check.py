"""Check the tap board front ends against the real input ranges.

Every tapped line already has R_SPLICE (47 kΩ) in series at the splice in
the interposer chamber, so it is part of every divider here. Run:

    python3 kicad/tap/frontend_check.py

It prints the level each front end sees at the corners of its input range
and exits non-zero if any check fails. Values match schematic.md.
"""
import sys

R_SPLICE = 47e3        # at the splice, 1 %
VREF = 3.0             # REF3030 into both MCP3208s (VDD 3.3 V)
LVC_VTP_MAX = 2.0      # 74LVC14 positive-going threshold, worst case at VCC 3.6 V
LVC_VTN_MIN = 0.8      # negative-going threshold, worst case at VCC 2.7 V
LVC_VIN_MAX = 5.5      # LVC inputs are 5.5 V tolerant, also with VCC = 0 (Ioff)
TOL = 0.01             # resistor tolerance

fails = []

def check(name, ok, detail):
    print(f"  {'ok ' if ok else 'FAIL'}  {name}: {detail}")
    if not ok:
        fails.append(name)

def div(vin, rtop, rbot):
    return vin * rbot / (rtop + rbot)

def worst(vin, rtop, rbot):
    lo = div(vin, rtop * (1 + TOL), rbot * (1 - TOL))
    hi = div(vin, rtop * (1 - TOL), rbot * (1 + TOL))
    return lo, hi

print("ANA: R_SPLICE + 100k series, 10M to GND, MCP6004 follower (5 V), 3.3k/4.7k to the ADC")
R_SER, R_PD = 100e3, 10e6
R_OT, R_OB = 3.3e3, 4.7e3
load = R_SPLICE + R_SER + R_PD
check("sensor load", load > 10e6, f"{load / 1e6:.2f} MΩ on the ECU line (THW pull-up ~2 kΩ → {2e3 / load * 100:.3f} % error)")
for vin in (0.0, 5.0, 5.1):
    lo, hi = worst(vin, R_SPLICE + R_SER, R_PD)
    a_lo, a_hi = div(lo, R_OT * 1.01, R_OB * 0.99), div(hi, R_OT * 0.99, R_OB * 1.01)
    check(f"ADC at {vin:.1f} V in", a_hi < VREF, f"{a_lo:.3f}–{a_hi:.3f} V (ref {VREF} V), {a_hi / VREF * 4095:.0f} counts max")
fc = 1 / (2 * 3.14159 * (R_OT * R_OB / (R_OT + R_OB)) * 100e-9)
check("anti-alias corner", 500 < fc < 2000, f"{fc:.0f} Hz with 100 nF at the ADC pin (sampled at 1 kHz/ch per firmware plan)")
clamp_i = (16.0 - 5.3) / (R_SPLICE + R_SER)
check("op-amp input clamp current at 16 V fault", clamp_i < 1e-3, f"{clamp_i * 1e6:.0f} µA")

print("V-MON (+B): R_SPLICE + 56k top, 22k bottom, 100 nF")
for vin in (9.0, 14.4, 16.0):
    lo, hi = worst(vin, R_SPLICE + 56e3, 22e3)
    check(f"ADC at {vin:.1f} V", hi < VREF, f"{lo:.3f}–{hi:.3f} V")
print("  note: BATT is not tapped. It is live with the key off and would back-feed an unpowered board.")

print("DIG / CAP / CAP-HV: R_SPLICE + 10k series, 1M to GND, BZX84C5V1 to GND, 22 pF, 74LVC14")
R_S2, R_PD2 = 10e3, 1e6
top = R_SPLICE + R_S2
for vin, want in ((0.0, "low"), (0.8, "low"), (4.5, "high"), (5.0, "high"), (14.4, "high")):
    lo, hi = worst(vin, top, R_PD2)
    v_lo, v_hi = min(lo, 5.1), min(hi, 5.1)        # zener clamps above ~5 V
    if want == "low":
        check(f"{vin:.1f} V in reads low", v_hi < LVC_VTN_MIN, f"{v_lo:.2f}–{v_hi:.2f} V")
    else:
        check(f"{vin:.1f} V in reads high", v_lo > LVC_VTP_MAX and v_hi <= LVC_VIN_MAX, f"{v_lo:.2f}–{v_hi:.2f} V")
ecu_load = top + R_PD2
check("load on the ECU line", ecu_load > 1e6, f"{ecu_load / 1e6:.2f} MΩ (a 100 kΩ ECU pull-up loses {100e3 / ecu_load * 100:.1f} %)")
spike = (70.0 - 5.4) / top
check("zener current on a 70 V SPV/TCV flyback", spike < 5e-3, f"{spike * 1e3:.2f} mA")
always_live = (14.4 - 5.1) / top
check("STP (live with key off) into the zener", always_live < 0.5e-3, f"{always_live * 1e3:.2f} mA to GND, nothing into the 3V3 rail (LVC Ioff)")
tau = top * 22e-12
check("edge delay", tau < 2e-6, f"RC {tau * 1e6:.2f} µs (1° crank at 4000 rpm = 41.7 µs; constant, calibrated out)")

print()
if fails:
    print(f"{len(fails)} check(s) failed: {', '.join(fails)}")
    sys.exit(1)
print("all front-end checks pass")
