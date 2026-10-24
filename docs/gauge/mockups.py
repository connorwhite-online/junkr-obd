"""Render the round-gauge screen mockups (480 x 480) as SVG.

    python3 docs/gauge/mockups.py

Writes one SVG per screen next to this file. The numbers are made-up
example readings; the layout is what matters.
"""
import math
import os

W = 480
C = W / 2
OUT = os.path.dirname(os.path.abspath(__file__))

BG = "#000"
FG = "#f2f2f2"
DIM = "#8a8a8a"
TRACK = "#262626"
BOOST = "#22c7e8"
EGT = "#ff8a1f"
WARN = "#ffb000"
RED = "#ff3b30"
OK = "#34c759"
FONT = "font-family='Inter, Helvetica, Arial, sans-serif'"


def pt(r, a):
    """Point at radius r, angle a degrees clockwise from 12 o'clock."""
    t = math.radians(a)
    return C + r * math.sin(t), C - r * math.cos(t)


def arc(r, a1, a2, color, width, cap="round"):
    """Arc from a1 to a2 (clockwise if a2 > a1, otherwise counter-clockwise)."""
    x1, y1 = pt(r, a1)
    x2, y2 = pt(r, a2)
    large = 1 if abs(a2 - a1) > 180 else 0
    sweep = 1 if a2 > a1 else 0
    return (f"<path d='M{x1:.1f},{y1:.1f} A{r},{r} 0 {large} {sweep} {x2:.1f},{y2:.1f}' "
            f"fill='none' stroke='{color}' stroke-width='{width}' stroke-linecap='{cap}'/>")


def tick(r1, r2, a, color, width=3):
    x1, y1 = pt(r1, a)
    x2, y2 = pt(r2, a)
    return f"<line x1='{x1:.1f}' y1='{y1:.1f}' x2='{x2:.1f}' y2='{y2:.1f}' stroke='{color}' stroke-width='{width}'/>"


def text(x, y, s, size, color=FG, weight=400, anchor="middle"):
    return (f"<text x='{x:.1f}' y='{y:.1f}' {FONT} font-size='{size}' font-weight='{weight}' "
            f"fill='{color}' text-anchor='{anchor}'>{s}</text>")


def pill(x, y, w, h, label, color, filled=False):
    fill = color if filled else "none"
    tc = BG if filled else color
    return (f"<rect x='{x - w / 2:.1f}' y='{y - h / 2:.1f}' width='{w}' height='{h}' rx='{h / 2}' "
            f"fill='{fill}' stroke='{color}' stroke-width='2'/>"
            + text(x, y + h * 0.18, label, h * 0.5, tc, 700))


def screen(name, body, page=None):
    dots = ""
    if page is not None:
        n, i = 4, page
        for k in range(n):
            x = C - (n - 1) * 9 + k * 18
            dots += f"<circle cx='{x}' cy='{W - 22}' r='4' fill='{FG if k == i else '#444'}'/>"
    svg = (f"<svg xmlns='http://www.w3.org/2000/svg' width='{W}' height='{W}' viewBox='0 0 {W} {W}'>"
           f"<rect width='{W}' height='{W}' fill='#d9d9d9'/>"
           f"<circle cx='{C}' cy='{C}' r='{C}' fill='{BG}'/>"
           + body + dots + "</svg>")
    with open(os.path.join(OUT, f"{name}.svg"), "w") as f:
        f.write(svg)


def split_gauge(r, lo_a, hi_a, frac, color, width=22):
    """One half of the split ring. lo_a is the empty end, hi_a the full end."""
    a = lo_a + (hi_a - lo_a) * max(0.0, min(1.0, frac))
    return arc(r, lo_a, hi_a, TRACK, width) + arc(r, lo_a, a, color, width)


# ---- 1. Drive ---------------------------------------------------------------
R = 212
b = ""
# Boost on the left, filling upward: 215 deg (bottom-left) -> 335 deg (top-left).
boost_max, boost = 25.0, 12.6
b += split_gauge(R, 215, 335, boost / boost_max, BOOST)
cut = 215 + 120 * 15.0 / boost_max          # ECU fuel cut ~15 psi
b += tick(R - 18, R + 18, cut, RED, 4)
seen = 215 + 120 * 13.8 / boost_max         # what the ECU is being told
b += tick(R - 14, R + 14, seen, FG, 2)
# EGT on the right, filling upward: 145 deg -> 25 deg.
egt_max, egt = 1600.0, 1080
b += split_gauge(R, 145, 25, egt / egt_max, EGT)
lim = 145 - 120 * 1250 / egt_max
b += tick(R - 18, R + 18, lim, RED, 4)
b += text(C - 118, 140, "BOOST", 16, BOOST, 700)
b += text(C + 118, 140, "EGT", 16, EGT, 700)
b += pill(C, 92, 132, 30, "TUNE +6%", OK)
b += text(C, 220, "12.6", 104, FG, 700)
b += text(C, 252, "psi  ·  ECU sees 13.8", 20, DIM)
b += text(C, 322, "1080", 60, EGT, 700)
b += text(C, 348, "°F pre-turbo", 18, DIM)
b += text(C - 70, 392, "COOL 192°", 19, FG, 600)
b += text(C + 70, 392, "ATF 168°", 19, FG, 600)
b += text(C, 420, "13.9 V", 17, DIM)
screen("01_drive", b, 0)

# ---- 2. Temps (towing / climbing) --------------------------------------------
b = text(C, 72, "TEMPS", 18, DIM, 700)
rows = [
    ("COOLANT", 212, "°F", 120, 250, 221, 230, FG),
    ("ATF", 196, "°F", 100, 280, 230, 260, FG),
    ("EGT", 1180, "°F", 200, 1600, 1100, 1250, EGT),
    ("INTAKE", 118, "°F", 0, 200, 140, 160, FG),
    ("FUEL", 104, "°F", 0, 200, 150, 170, FG),
]
y = 112
for label, v, unit, lo, hi, warn, danger, color in rows:
    frac = (v - lo) / (hi - lo)
    col = RED if v >= danger else WARN if v >= warn else color
    x0, x1 = 92, 388
    b += text(x0, y, label, 15, DIM, 700, "start")
    b += text(x1, y + 2, f"{v}{unit}", 26, col, 700, "end")
    b += f"<rect x='{x0}' y='{y + 10}' width='{x1 - x0}' height='8' rx='4' fill='{TRACK}'/>"
    b += f"<rect x='{x0}' y='{y + 10}' width='{(x1 - x0) * frac:.1f}' height='8' rx='4' fill='{col}'/>"
    wx = x0 + (x1 - x0) * (warn - lo) / (hi - lo)
    b += f"<rect x='{wx:.1f}' y='{y + 6}' width='2' height='16' fill='{WARN}'/>"
    y += 54
# 10-minute coolant trend
pts = []
for k in range(60):
    t = k / 59
    v = 196 + 14 * t + 3 * math.sin(k / 4)
    pts.append(f"{120 + 240 * t:.1f},{432 - (v - 190) * 2.2:.1f}")
b += f"<polyline points='{' '.join(pts)}' fill='none' stroke='{FG}' stroke-width='2'/>"
b += text(C, 450, "coolant · last 10 min", 13, DIM)
screen("02_temps", b, 1)

# ---- 3. Engine ----------------------------------------------------------------
b = ""
rpm_max, rpm = 4500, 2350
b += arc(R, 225, 495, TRACK, 22)
b += arc(R, 225, 225 + 270 * rpm / rpm_max, "#7d7dff", 22)
red0 = 225 + 270 * 4000 / rpm_max
b += arc(R, red0, 495, RED, 6, "butt")
for k in range(1, 5):
    a = 225 + 270 * k * 1000 / rpm_max
    b += text(*pt(R - 40, a), str(k), 18, DIM, 600)
b += text(C, 150, "RPM", 16, DIM, 700)
b += text(C, 240, "2350", 92, FG, 700)
grid = [("TIMING", "8.5°", "BTDC"), ("FUEL", "21.4°", "SPV"),
        ("PEDAL", "62%", "VA"), ("TCV", "48%", "duty")]
for i, (k, v, s) in enumerate(grid):
    x = C - 72 + 144 * (i % 2)
    yy = 300 + 70 * (i // 2)
    b += text(x, yy, k, 14, DIM, 700)
    b += text(x, yy + 32, v, 32, FG, 700)
b += text(C, 444, "4 → 3 lock-up ON", 15, DIM)
screen("03_engine", b, 2)

# ---- 4. Tune ------------------------------------------------------------------
b = text(C, 72, "PIGGYBACK", 18, DIM, 700)
b += pill(C, 108, 150, 36, "ARMED", OK, True)
rows = [("BOOST", "12.6", "13.8", "psi"), ("PEDAL", "62", "70", "%"), ("COOLANT", "189", "189", "°F")]
y = 172
for label, real, seen_, unit in rows:
    changed = real != seen_
    b += text(92, y, label, 14, DIM, 700, "start")
    b += text(92, y + 36, real, 34, FG, 700, "start")
    b += text(C + 6, y + 30, "→", 26, DIM)
    b += text(388, y + 36, f"{seen_}", 34, WARN if changed else FG, 700, "end")
    b += text(388, y, f"ECU sees · {unit}", 13, DIM, 400, "end")
    y += 72
b += text(C, 402, "authority 20%  ·  used 6%", 16, FG, 600)
b += text(C, 428, "learning: OFF  ·  wideband λ 1.34", 14, DIM)
screen("04_tune", b, 3)

# ---- 5. Alert overlay -----------------------------------------------------------
b = arc(226, 0.1, 359.9, RED, 14)
b += text(C, 150, "EGT HIGH", 34, RED, 800)
b += text(C, 262, "1290", 120, FG, 800)
b += text(C, 300, "°F  ·  limit 1250", 22, DIM)
b += text(C, 354, "LIFT / DOWNSHIFT", 26, FG, 700)
b += text(C, 386, "tune pulled to stock", 18, DIM)
screen("05_alert", b)

print("wrote SVGs to", OUT)
