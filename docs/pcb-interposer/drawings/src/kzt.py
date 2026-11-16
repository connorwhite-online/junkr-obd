"""Drawings KZT-301 (assembly) and KZT-302 (tails + taps) for the rev C tap board.

    python3 docs/pcb-interposer/drawings/src/kzt.py   (needs KiCad 7 pcbnew + matplotlib)
"""
import os, sys, csv
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", ".."))
OUT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(ROOT, "kicad", "interposer"))
import gen_board as G
import sheet
sheet.TOTAL = 2; sheet.DATE = "2026-11-13"
from sheet import *
from matplotlib.patches import Rectangle, Circle, Polygon

names = {}
for r in csv.DictReader(open(os.path.join(ROOT, "hardware", "ecu_interposer", "pinout_kzj95_at.csv"))):
    names[(r["connector"], int(r["pin"]))] = (r["name"] or "-", r["function"])
TAPS = {(p, n): (rb, g) for p, n, rb, g in G.TAPS}

# ------------------------------------------------------------------ sheet 1
sh = Sheet("KZT-301", "TAP BOARD rev C: ASSEMBLY", "E5 + E6 + E8 through one PCB, DevKitC on board",
           scale="1.5:1", material="FR-4 1.6, 2 oz Cu", process="PCB: 2 layer, 1.6 mm, 2 oz Cu,\nHASL, green / white silk.\nTray + lid: PETG FDM or\nSLS PA12, 1.4 wall.\nGerbers: kicad/interposer/gerbers", rev="C", sheet=1)
ax = sh.ax; K = 1.5
X0, Y0 = 40, 150
P = lambda x, y: (X0 + (x + 153.0) * K, Y0 + (21.0 - y) * K)
def poly(pts, lw=0.8, fc="none", ls="-", z=3):
    ax.add_patch(Polygon([P(*p) for p in pts], closed=True, fill=fc != "none", fc=fc, ec=INK, lw=lw, ls=ls, zorder=z))
def rect(x0, y0, x1, y1, **kw): poly([(x0, y0), (x1, y0), (x1, y1), (x0, y1)], **kw)
def hdim(x1, x2, y, txt):
    ax.annotate("", (x1, y), (x2, y), arrowprops=dict(arrowstyle="<|-|>", lw=THIN, color=INK, shrinkA=0, shrinkB=0, mutation_scale=5))
    ax.text((x1 + x2) / 2, y + 0.8, txt, ha="center", va="bottom", fontsize=6, bbox=dict(fc="white", ec="none", pad=0.1))
def vdim(x, y1, y2, txt):
    ax.annotate("", (x, y1), (x, y2), arrowprops=dict(arrowstyle="<|-|>", lw=THIN, color=INK, shrinkA=0, shrinkB=0, mutation_scale=5))
    ax.text(x - 0.8, (y1 + y2) / 2, txt, ha="right", va="center", fontsize=6, rotation=90, bbox=dict(fc="white", ec="none", pad=0.1))
def lead(p, q, txt, right=True):
    ax.plot([p[0], q[0]], [p[1], q[1]], lw=THIN, color=INK, zorder=6); ax.plot(*p, "o", ms=1.6, color=INK, zorder=6)
    ax.plot([q[0], q[0] + (5 if right else -5)], [q[1], q[1]], lw=THIN, color=INK, zorder=6)
    ax.text(q[0] + (6 if right else -6), q[1], txt, ha="left" if right else "right", va="center", fontsize=5.8, zorder=6, linespacing=1.3)

sh.label(X0, Y0 + 63 * K + 30, "PLAN, TOP SIDE  (1.5:1)   harness edge up, ECU edge down")
poly(G.OUTLINE, lw=1.2, fc="#fbfbfb")
# divider area on the back
rect(-116.5, 8.0, -38.0, 20.4, lw=0.4, ls="--")
ax.text(*P(-77, 20.0), "17 dividers on the BACK (KZT-302)", fontsize=5.2, ha="center", va="bottom", zorder=4)
# header body
rect(-117.8, -25.5, 0.0, -7.0, lw=0.9, fc="#e6e6e6")
for plug, xc in (("E8", -94.2), ("E6", -62.4), ("E5", -27.1)):
    ax.text(*P(xc, -17), plug, fontsize=8, weight="bold", ha="center", va="center", zorder=5)
for x, y in G.PEGS:
    ax.add_patch(Circle(P(x, y), G.PEG_D / 2 * K, fill=False, lw=0.5, zorder=5))
for plug, d in G.PLUGS.items():
    n = len(d["cols"])
    for i, (x, t) in enumerate(d["cols"]):
        xr, _ = G.route_x(plug, i)
        for (px, py) in ((x, 0.0), (x, -4.5), (x, G.TAIL_Y[0]), (xr, G.TAIL_Y[1])):
            ax.add_patch(Circle(P(px, py), (0.9 if t == 1 else 1.1) * K / 1.6, fc=INK, ec=INK, lw=0.1, zorder=5))
# DevKitC
yb, yt = G.DK_PIN1_Y - 21 * 2.54 - 7.0, G.DK_PIN1_Y + 8.0
rect(G.DK_X - 12.7, yb, G.DK_X + 12.7, yt, lw=0.9, fc="#eef2f8", z=4)
rect(G.DK_X - 9, yt - 1, G.DK_X + 9, yt - 18, lw=0.5, fc="white", z=4)
ax.text(*P(G.DK_X, yt - 11), "ESP32-S3\nWROOM-1", fontsize=5, ha="center", va="center", zorder=5)
for dx in (-6.5, 6.5): rect(G.DK_X + dx - 4.5, yb + 7.5, G.DK_X + dx + 4.5, yb - 0.5, lw=0.5, fc="white", z=4)
ax.text(*P(G.DK_X, -12), "U1  ESP32-S3-DevKitC-1", fontsize=5.5, ha="center", rotation=90, va="center", zorder=5)
for pad in range(1, 45):
    ax.add_patch(Circle(P(*G.dk_xy(pad)), 0.55 * K, fc=INK, ec=INK, lw=0.1, zorder=5))
for x, y in G.MOUNT:
    ax.add_patch(Circle(P(x, y), 1.1 * K, fill=False, lw=0.6, zorder=5))
    ax.plot([P(x, y)[0] - 2.5, P(x, y)[0] + 2.5], [P(x, y)[1]] * 2, lw=0.25, color=INK, zorder=5)
    ax.plot([P(x, y)[0]] * 2, [P(x, y)[1] - 2.5, P(x, y)[1] + 2.5], lw=0.25, color=INK, zorder=5)
# dims
hdim(P(-153, 0)[0], P(5.5, 0)[0], Y0 + 63 * K + 8, "158.5")
hdim(P(-124, 0)[0], P(5.5, 0)[0], P(0, -25)[1] + 14, "129.5")
hdim(P(G.DK_X - 12.7, 0)[0], P(G.DK_X + 12.7, 0)[0], Y0 - 7, "25.4")
vdim(X0 - 8, P(0, 21)[1], P(0, -42)[1], "63")
vdim(P(5.5, 0)[0] + 10, P(0, 21)[1], P(0, -25)[1], "46")
vdim(P(5.5, 0)[0] + 10, P(0, -25)[1], P(0, -42)[1], "17")
hdim(P(-117.8, 0)[0], P(0, 0)[0], P(0, -25.5)[1] + 4, "117.8  header 176122-6")
lead(P(G.DK_X, yb), (X0 + 52, Y0 + 63 * K + 18), "USB-C (power + flash), beside the harness plugs")
lead(P(G.DK_X, yt), (X0 + 60, Y0 - 16), "antenna: 1 mm past the ECU edge, no copper under it")
lead(P(-60, 4.4), (P(5.5, 0)[0] + 24, Y0 + 30), "J2 / J3 / J4 tail pads:\nupper row = upper cavity row,\nsquare pad = pin 1")
lead(P(-27, -10), (P(5.5, 0)[0] + 24, Y0 + 72), "J1 TE 176122-6, used whole:\nE8 (22) + E6 (16) + E5 (26)")
lead(P(3.0, 17.5), (P(5.5, 0)[0] + 24, Y0 + 8), "H1-H5  M2 self-tap to tray\n(H5 next to E8 also holds lid)")

# side view (section through E8): Y horizontal, Z up
SX, SY = 40, 62
sh.label(SX, SY + 36, "SIDE VIEW  (1:1)   ECU left, harness right")
S = lambda y, z: (SX + 70 + (y + 0) * 1.0, SY + z * 1.0)   # 3D frame Y3 (harness +), Z
def srect(y0, z0, y1, z1, fc="none", lw=0.8, ls="-"):
    a, b = S(y0, z0); c, d = S(y1, z1)
    ax.add_patch(Rectangle((a, b), c - a, d - b, fill=fc != "none", fc=fc, ec=INK, lw=lw, ls=ls, zorder=3))
srect(-21, -3.2, 25, 1.6, fc="#f2f2f2", lw=0.6)                 # tray floor/walls (approx)
srect(-21, 0, 25, 1.6, fc="#cfe3cf", lw=0.9)                     # board
srect(7, 1.6, 25.5, 21.0, fc="#e6e6e6", lw=0.9)                  # header body
ax.text(*S(16, 11), "176122-6", fontsize=5.5, ha="center", va="center", rotation=90, zorder=5)
srect(25.5, 4.1, 70, 19.5, fc="white", lw=0.7, ls="--")          # harness plug
ax.text(*S(48, 12), "harness plug\n(unchanged)", fontsize=5.5, ha="center", va="center", zorder=5)
srect(-8.5, 1.6, 7, 21.0, fc="#fafafa", lw=0.5, ls=":")          # lid space
ax.text(*S(-1, 17), "lid", fontsize=5.5, ha="center", zorder=5)
ax.plot([S(-1, 4)[0], S(-21, 6)[0], S(-60, 8)[0]], [S(-1, 4)[1], S(-21, 6)[1], S(-60, 8)[1]], lw=1.2, color="#b5452f", zorder=4)
ax.text(*S(-40, 10), "150 mm tails to the ECU-side plugs", fontsize=5.5, ha="center", zorder=5)
hdim(S(-21, 0)[0], S(25.5, 0)[0], SY - 9, "46.5 board + header")
vdim(S(-21, 0)[0] - 4, S(0, -3.2)[1], S(0, 21)[1], "24.2")

sh.note(262, 122,
"HOW IT FITS\n"
"1  The board sits where the ECU's header is now.\n"
"    The stiff harness plugs into J1 and doesn't move.\n"
"2  Three new TE plugs on 150 mm tails go into the ECU.\n"
"    The ECU slides back about 5 cm on its bracket.\n"
"3  USB-C from a switched car charger powers U1.\n\n"
"All 64 pins pass straight through on copper on both\n"
"layers. Unpowered, the truck runs exactly stock.\n"
"TAP_GND (ESP32 ground) never touches an ECU ground.\n\n"
"[VERIFY] before ordering\n"
"- DevKitC row spacing 22.86 (0.9 in): measure yours.\n"
"- 176122-6 pad numbering: meter check on a header.\n"
"- Plug keying: read the numbers on the truck's plugs.\n\n"
"DRC 0 errors / 0 unconnected (KiCad 7).", fs=6.2)
sh.save(os.path.join(OUT, "KZT-301.png"))

# ------------------------------------------------------------------ sheet 2
sh = Sheet("KZT-302", "TAP BOARD rev C: TAILS + TAPS", "wire every ECU-side plug pin N to tail pad N",
           scale="NTS", material="SEE BOM", process="Tails: 150 mm each.\n040 pins: 22 AWG + 175061-1.\n070 pins: 18 AWG + 173631-1.\nCrimp, tug-test, load from\nthe wire side, then solder.", rev="C", sheet=2)
ax = sh.ax
cols = [("E8", "J2  E8  22-pin  (TE 175110-6)", 18), ("E6", "J3  E6  16-pin  (TE 175109-6)", 148), ("E5", "J4  E5  26-pin  (TE 175111-x)", 278)]
for plug, title, x in cols:
    d = G.PLUGS[plug]; n = len(d["cols"])
    y = 280
    ax.text(x, y, title, fontsize=7.5, weight="bold", va="top"); y -= 7
    hdr = ["pin", "row", "signal", "wire", "tap"]
    xs = [x, x + 9, x + 19, x + 41, x + 62]
    for h, xx in zip(hdr, xs): ax.text(xx, y, h, fontsize=5.6, weight="bold", va="top")
    ax.plot([x, x + 122], [y - 3.2, y - 3.2], lw=0.4, color=INK); y -= 4.6
    for pin in range(1, 2 * n + 1):
        i = (pin - 1) % n; t = d["cols"][i][1]
        nm = names.get((plug, pin), ("-", ""))[0]
        tap = TAPS.get((plug, pin))
        tp = f"100k / {tap[0]} -> GPIO{tap[1]}" if tap else ""
        row = "upper" if pin <= n else "lower"
        vals = [str(pin), row, nm, "18 AWG" if t == 2 else "22 AWG", tp]
        for v, xx in zip(vals, xs):
            ax.text(xx, y, v, fontsize=5.4, va="top", weight="bold" if tap else "normal")
        y -= 4.15
        if pin == n: ax.plot([x, x + 122], [y + 1.0, y + 1.0], lw=0.25, color="#888", ls="--")
sh.note(18, 52,
"Bold rows are tapped: a 100k top resistor from the pin to the GPIO node, and the listed bottom resistor\n"
"from the node to TAP_GND, both 0805 on the back of the board next to the plug they serve.\n"
"Signals: PIM boost, THW/THA/THF/THO temps, VA pedal, VC 5 V ref, E2 sensor gnd, +B battery, W check-engine,\n"
"TAC rpm, SP1 speed, S1/S2 shift solenoids (gear), SL lock-up, L4 low range, TFN transfer neutral.\n"
"Never tapped: SPV, TCV, NE, TDC, grounds. 070 (18 AWG) pins: E8 cols 1, 2, 11; E5 cols 1, 2, 11, 12, 13.", fs=6.0)
sh.save(os.path.join(OUT, "KZT-302.png"))
print("ok")
