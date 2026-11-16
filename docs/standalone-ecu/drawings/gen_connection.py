"""Connection drawings: how JUNKR sits between the truck's harness and the stock ECU.

    python3 gen_connection.py

Writes conn_1_location.png, conn_2_shadow.png, conn_3_signals.png and
conn_4_E5.png, conn_5_E6.png, conn_6_E8.png (A3 landscape, 150 dpi).

Every pin's behaviour comes from the board itself: the script imports
kicad/standalone_ecu/tools/circuit.py and looks at what each harness net
connects to (relay, tap, open-drain driver, power path). Pin names and wire
colours are from the wiring manual EWD269F: connector faces p.210, engine
control pp.206-209, ECT pp.220-222, ECU location (RHD) p.27.
"""
import os
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Circle, FancyArrowPatch, FancyBboxPatch, Polygon, Rectangle

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "kicad", "standalone_ecu", "tools"))
import circuit as CIR  # noqa: E402

INK, MUTE, LINE = "#1b1f24", "#6a737d", "#d0d7de"
ACC = "#0b62d6"
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 9})

# ---------------------------------------------------------------- categories
CAT = {
    "SW":     ("#c2410c", "#fff1e8", "Switched by a JUNKR relay",
               "Output pin. Relay NC = stock ECU (shadow), NO = JUNKR (takeover / swap)."),
    "SENSE":  ("#0b62d6", "#e8f1ff", "Passes straight through, JUNKR listens",
               "Sensor or switch. Same copper to both ECUs; JUNKR taps it at 47k-100k."),
    "SHARED": ("#7c3aed", "#f3edff", "Shared output, JUNKR idle in shadow",
               "Lamp, tach or diagnostic line. Open-drain wired-OR: either ECU can pull it."),
    "POWER":  ("#1a7f37", "#e7f6ec", "Power, feeds both",
               "+B, IGSW, BATT, M-REL, VC. Diodes keep the two ECUs from back-feeding."),
    "GND":    ("#1b1f24", "#eceff1", "Ground, common", "E01, E02, E1, E2 tie to both ECUs."),
    "NC":     ("#9aa4ae", "#ffffff", "Empty cavity", "Not used on the KZJ95 A/T (EWD269F)."),
}
GROUP_OF = {p: g for g, lst in CIR.TAKEOVER.items() for p in lst}

# Wire colours checked on EWD269F pp.207, 221, 220 where the interposer CSV had blanks.
WIRE_FIX = {("E5", 1): "P", ("E5", 2): "P-B", ("E5", 15): "P-L", ("E5", 20): "G-B", ("E5", 21): "G-W",
            ("E5", 23): "Y", ("E5", 24): "G-B", ("E6", 13): "Y"}
TOYOTA = {"B": "#111111", "W": "#f5f5f5", "R": "#d62828", "G": "#2a9d3c", "L": "#1d4ed8", "Y": "#f2c500",
          "BR": "#7a4a1d", "GR": "#8d949c", "LG": "#9be08a", "O": "#f08a24", "P": "#f29ec8", "V": "#7e3fb8",
          "SB": "#7cc7f0"}


def load_wires():
    wires = {k: (r.get("wire") or "").strip() for k, r in CIR.PO.items()}
    wires.update(WIRE_FIX)
    return wires


WIRES = load_wires()
NTC = {"THA", "THW", "THF", "THO", "VRP", "VRT"}


def classify(plug, pin):
    """(category, shadow text, swap text) from the board netlist."""
    name = CIR.PO[(plug, pin)]["name"]
    if not name:
        return "NC", "-", "-"
    if (plug, pin) in GROUP_OF:
        g = GROUP_OF[(plug, pin)]
        return "SW", "relay NC: stock ECU drives it", "relay NO: JUNKR drives it (%s group)" % g
    net = CIR.harness_net(plug, pin)
    if net == "GND":
        return "GND", "common ground", "common ground"
    parts = {r: v for r, v in ((r, next(p.value for p in CIR.c.parts if p.ref == r)) for r, _ in CIR.c.nets[net])}
    vals = set(parts.values())
    if name == "E2":
        return "GND", "sensor ground, common", "sensor ground"
    if name == "VC":
        return "POWER", "stock ECU's 5 V; JUNKR's own VC off", "JUNKR supplies 5 V"
    if name == "+B":
        return "POWER", "feeds both (JUNKR via diode)", "feeds JUNKR"
    if name == "IGSW":
        return "POWER", "wakes both (JUNKR via diode)", "wakes JUNKR"
    if name == "M-REL":
        return "POWER", "either ECU holds the ECD relay", "JUNKR holds the ECD relay"
    if name == "BATT":
        return "POWER", "keep-alive, JUNKR reads it", "keep-alive, read"
    if any(v.startswith("DRV8803") for v in vals):
        return "SHARED", "stock ECU drives; JUNKR off", "JUNKR drives"
    if name in NTC:
        return "SENSE", "stock ECU reads; JUNKR listens", "JUNKR reads (own 2k49 pull-up)"
    return "SENSE", "stock ECU reads; JUNKR listens", "JUNKR reads"


FUNC = {}
for (plug, pin), row in CIR.PO.items():
    FUNC[(plug, pin)] = row.get("function", "") if isinstance(row, dict) else ""

ROWS = {"E5": [list(range(1, 14)), list(range(14, 27))],
        "E6": [list(range(1, 9)), list(range(9, 17))],
        "E8": [list(range(1, 12)), list(range(12, 23))]}
PARTNO = {"E5": "TE 174516-6 housing", "E6": "TE 174514-6 housing", "E8": "TE 174515-6 housing"}


# ---------------------------------------------------------------- sheet frame
def sheet(title, sub, num, total=6):
    fig = plt.figure(figsize=(16.54, 11.69), dpi=150)
    ax = fig.add_axes([0, 0, 1, 1])
    ax.set_xlim(0, 420); ax.set_ylim(0, 297); ax.axis("off")
    ax.add_patch(Rectangle((8, 8), 404, 281, fill=False, lw=1.3, ec=INK))
    ax.add_patch(Rectangle((272, 8), 140, 30, fill=False, lw=1.1, ec=INK))
    ax.plot([272, 412], [28, 28], color=INK, lw=0.6); ax.plot([272, 412], [18, 18], color=INK, lw=0.6)
    ax.text(276, 33, "JUNKR ECU rev A  ·  KZJ95 1KZ-TE / A343F  ·  RHD", fontsize=8, weight="bold", va="center")
    ax.text(276, 23, title, fontsize=9.5, weight="bold", va="center", color=ACC)
    ax.text(276, 13, "Ref EWD269F pp.27, 206-210, 220-222      Sheet %d of %d" % (num, total), fontsize=7.5, va="center")
    ax.text(16, 280, title, fontsize=18, weight="bold", va="top")
    ax.text(16, 270, sub, fontsize=10, va="top", color=MUTE)
    return fig, ax


def save(fig, name):
    fig.savefig(os.path.join(HERE, name))
    plt.close(fig)
    print("wrote", name)


def rbox(ax, x, y, w, h, fc="#f6f8fa", ec=INK, lw=1.3, r=2.5, ls="-", z=1):
    ax.add_patch(FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0,rounding_size=%s" % r,
                                fc=fc, ec=ec, lw=lw, ls=ls, zorder=z))


def arrow(ax, a, b, color=INK, lw=1.6, style="-|>", ls="-", z=3):
    ax.add_patch(FancyArrowPatch(a, b, arrowstyle=style, mutation_scale=12, color=color, lw=lw, ls=ls, zorder=z,
                                 shrinkA=0, shrinkB=0))


def callout(ax, x, y, n, color=ACC):
    ax.add_patch(Circle((x, y), 3.0, fc=color, ec="none", zorder=6))
    ax.text(x, y, str(n), ha="center", va="center", color="white", fontsize=8, weight="bold", zorder=7)


def legend(ax, x, y, cats=("SW", "SENSE", "SHARED", "POWER", "GND", "NC"), w=120, dy=11):
    for i, k in enumerate(cats):
        ec, fc, head, body = CAT[k]
        yy = y - i * dy
        ax.add_patch(Rectangle((x, yy - 3.2), 7, 6.4, fc=fc, ec=ec, lw=1.6))
        ax.text(x + 10, yy + 1.2, head, fontsize=8.5, weight="bold", color=ec if k != "NC" else MUTE, va="center")
        ax.text(x + 10, yy - 3.2, body, fontsize=7, color=INK, va="center")


def face(ax, x, y, plug, cell=9.0, h=10.0, label=True, show_names=True, title=None):
    """Harness plug, mating face, numbered as EWD269F p.210 (female: 1 at top left)."""
    rows = ROWS[plug]
    n = len(rows[0])
    w = n * cell + 6
    H = 2 * h + 9
    rbox(ax, x, y, w, H, fc="#fbfbfb", ec=INK, lw=1.4, r=1.5)
    # latch on top, centre (as drawn on p.210)
    ax.add_patch(Rectangle((x + w / 2 - 9, y + H), 18, 3.2, fc="#fbfbfb", ec=INK, lw=1.2))
    ax.add_patch(Rectangle((x + w / 2 - 4, y + H + 0.8), 8, 1.6, fc=INK, ec=INK))
    for r, row in enumerate(rows):
        for i, pin in enumerate(row):
            cat = classify(plug, pin)[0]
            ec, fc = CAT[cat][0], CAT[cat][1]
            cx, cy = x + 3 + i * cell, y + 4 + (1 - r) * (h + 1)
            ax.add_patch(Rectangle((cx + 0.4, cy), cell - 0.8, h, fc=fc, ec=ec, lw=1.2 if cat != "NC" else 0.6,
                                   ls="-" if cat != "NC" else (0, (2, 1.5))))
            ax.text(cx + cell / 2, cy + h - 2.6, str(pin), ha="center", va="center", fontsize=6.3, color=MUTE)
            if show_names:
                nm = CIR.PO[(plug, pin)]["name"] or ""
                fs = 6.3 if len(nm) <= 4 else 5.2
                ax.text(cx + cell / 2, cy + 3.0, nm, ha="center", va="center", fontsize=fs, weight="bold",
                        color=ec if cat != "NC" else MUTE)
    if label:
        ax.text(x, y + H + 6, title or plug, fontsize=11, weight="bold", va="bottom")
    return w, H


def swatch(ax, x, y, code, w=10, h=4.2):
    if not code:
        ax.text(x + w / 2, y, "?", ha="center", va="center", fontsize=7, color=MUTE)
        return
    parts = code.split("-")
    base = TOYOTA.get(parts[0], "#cccccc")
    ax.add_patch(Rectangle((x, y - h / 2), w, h, fc=base, ec=INK, lw=0.5))
    if len(parts) > 1:
        ax.add_patch(Rectangle((x, y - 0.7), w, 1.4, fc=TOYOTA.get(parts[1], "#cccccc"), ec="none"))


# ---------------------------------------------------------------- sheet 1: where
def sheet_location():
    fig, ax = sheet("Where it goes: behind the glovebox, passenger side",
                    "RHD truck, seen from the passenger seat. The stock ECU is behind the glovebox on the left (EWD269F p.27). "
                    "JUNKR goes right next to it.", 1)
    # --- dash, front view
    ox, oy = 18, 120
    ax.add_patch(Polygon([(ox, oy), (ox + 250, oy), (ox + 250, oy + 92), (ox + 228, oy + 120), (ox + 12, oy + 120), (ox, oy + 98)],
                         closed=True, fc="#f6f8fa", ec=INK, lw=1.4))
    ax.text(ox + 4, oy + 124, "Dashboard, front view (cab interior)", fontsize=10, weight="bold")
    # windscreen line
    ax.plot([ox + 14, ox + 226], [oy + 116, oy + 116], color=LINE, lw=1)
    # steering wheel (RHD: driver on the right)
    ax.add_patch(Circle((ox + 205, oy + 52), 30, fill=False, ec=INK, lw=2.2))
    ax.add_patch(Circle((ox + 205, oy + 52), 7, fc="#eceff1", ec=INK, lw=1.2))
    for dx, dy in ((-28, 0), (28, 0), (0, -28)):
        ax.plot([ox + 205, ox + 205 + dx * 0.78], [oy + 52, oy + 52 + dy * 0.78], color=INK, lw=2)
    ax.text(ox + 205, oy + 12, "DRIVER (right)", ha="center", fontsize=8.5, color=MUTE)
    # centre stack
    rbox(ax, ox + 112, oy + 18, 46, 80, fc="#eceff1")
    ax.text(ox + 135, oy + 92, "centre stack", ha="center", fontsize=7.5, color=MUTE)
    for k in range(3):
        rbox(ax, ox + 118, oy + 70 - k * 15, 34, 10, fc="white", r=1)
    # clock hole (gauge) note
    ax.add_patch(Circle((ox + 135, oy + 28), 4.5, fc="white", ec=INK, lw=1))
    # glovebox (removed) on the passenger side
    gx, gy, gw, gh = ox + 18, oy + 28, 78, 42
    rbox(ax, gx, gy, gw, gh, fc="none", ec=INK, lw=1.3, ls=(0, (5, 3)))
    ax.text(gx + gw / 2, gy - 5, "glovebox (remove)", ha="center", fontsize=8.5, color=MUTE)
    ax.text(ox + 30, oy + 12, "PASSENGER (left)", ha="center", fontsize=8.5, color=MUTE)
    # stock ECU and JUNKR behind it (hidden lines)
    ex, ey = gx + 8, gy + 18
    rbox(ax, ex, ey, 34, 18, fc="#ffffff", ec=INK, lw=1.4, r=1)
    ax.text(ex + 17, ey + 12, "stock ECU", ha="center", fontsize=7.5, weight="bold")
    ax.text(ex + 17, ey + 6, "89661-xxxxx", ha="center", fontsize=6.5, color=MUTE)
    jx, jy = gx + 46, gy + 6
    rbox(ax, jx, jy, 28, 20, fc="#e8f1ff", ec=ACC, lw=1.6, r=1)
    ax.text(jx + 14, jy + 13, "JUNKR", ha="center", fontsize=8, weight="bold", color=ACC)
    ax.text(jx + 14, jy + 7, "tray", ha="center", fontsize=6.5, color=ACC)
    callout(ax, ex - 4, ey + 20, 1)
    callout(ax, jx + 31, jy + 23, 2)
    # passenger side relay box reference (as on p.27, far left)
    rbox(ax, ox + 4, oy + 50, 9, 14, fc="white", r=0.8)
    ax.text(ox + 8.5, oy + 46, "R/B", ha="center", fontsize=6.5, color=MUTE)

    # --- view from below with glovebox out (matches the plug photo)
    bx0, by0 = 282, 150
    ax.text(bx0 - 6, by0 + 96, "Looking up behind the glovebox (glovebox out)", fontsize=10, weight="bold")
    ax.text(bx0 - 6, by0 + 90, "as in your photo: wire side of the plugs", fontsize=8.5, color=MUTE)
    rbox(ax, bx0, by0 + 40, 104, 30, fc="#eef0f2", ec=INK, lw=1.4, r=1)
    ax.text(bx0 + 52, by0 + 64, "STOCK ECU (end face)", ha="center", fontsize=8.5, weight="bold")
    plugs = [("E5", 26, 38), ("E6", 16, 24), ("E8", 22, 32)]
    px = bx0 + 4
    for nme, n, w in plugs:
        cat_col = {"E5": "#c2410c", "E6": "#0b62d6", "E8": "#1a7f37"}[nme]
        rbox(ax, px, by0 + 43, w, 14, fc="white", ec=cat_col, lw=1.6, r=1)
        ax.text(px + w / 2, by0 + 52, nme, ha="center", fontsize=9, weight="bold", color=cat_col)
        ax.text(px + w / 2, by0 + 46.5, "%d pin" % n, ha="center", fontsize=6.5, color=MUTE)
        # wires leaving downward/left
        for k in range(5):
            ax.plot([px + 4 + k * (w - 8) / 4, px + 4 + k * (w - 8) / 4 - 6], [by0 + 43, by0 + 30], color=MUTE, lw=0.7)
        px += w + 2
    # corrugated sleeve to the right of E8
    for k in range(6):
        ax.add_patch(Rectangle((bx0 + 104 + k * 3.2, by0 + 44), 3, 12, fc="#333", ec="#555", lw=0.4))
    ax.text(bx0 + 114, by0 + 38, "sleeve", fontsize=7, color=MUTE, ha="center")
    ax.annotate("", xy=(bx0 + 2, by0 + 22), xytext=(bx0 + 102, by0 + 22),
                arrowprops=dict(arrowstyle="<->", color=MUTE, lw=0.8))
    ax.text(bx0 + 52, by0 + 16, "left = E5 (widest), middle = E6, right = E8", ha="center", fontsize=8, color=INK)
    ax.text(bx0 + 52, by0 + 9, "small white plug beside them is NOT an ECU plug", ha="center", fontsize=7.5, color=MUTE)

    # --- notes
    nx, ny = 18, 104
    ax.text(nx, ny, "NOTES", fontsize=10, weight="bold")
    notes = [
        (1, "Stock ECU stays in its factory bracket in shadow mode. Its three plugs come off and go into JUNKR instead; "
            "JUNKR's three tails plug back into it."),
        (2, "JUNKR tray: 206 x 146 x ~40 mm with four M5 ears (two at each end). Mount it beside or below the stock ECU "
            "so the 150 mm tails reach without strain. [VERIFY space on the truck before fixing.]"),
        (3, "The tray's harness opening faces the plugs' wire bundle; the tails leave through three slots in the lid. "
            "Keep the RUN switch lead (J11) and USB-C reachable with the glovebox shut."),
        (4, "Disconnect the battery negative before unplugging anything. Toyota plugs: press the centre latch, rock gently, "
            "never pull by the wires."),
    ]
    for i, (n, t) in enumerate(notes):
        callout(ax, nx + 3, ny - 9 - i * 12, n)
        ax.text(nx + 9, ny - 9 - i * 12, t, fontsize=8.3, va="center", wrap=True)
    save(fig, "conn_1_location.png")


# ---------------------------------------------------------------- sheet 2: shadow-mode connection
def junkr_tray(ax, x, y, w=150, h=92):
    """Plan view of the tray (scaled ~0.73 of 206 x 146), header on the left edge."""
    rbox(ax, x, y, w, h, fc="#eef4ff", ec=ACC, lw=1.8, r=3)
    ax.text(x + w / 2 + 8, y + h - 8, "JUNKR ECU", ha="center", fontsize=13, weight="bold", color=ACC)
    ax.text(x + w / 2 + 8, y + h - 15, "tray + lid, 206 x 146 mm (plan view, lid on)", ha="center", fontsize=7.5, color=MUTE)
    return x, y, w, h


def sheet_shadow():
    fig, ax = sheet("Shadow mode: harness into JUNKR, JUNKR back into the stock ECU",
                    "No cutting or splicing. Three factory plugs out of the stock ECU, into JUNKR's J1. "
                    "Three 150 mm tails from JUNKR back into the stock ECU.", 2)
    # Before / after strip
    sy0 = 241
    ax.text(16, sy0 + 17, "BEFORE", fontsize=9, weight="bold", color=MUTE)
    rbox(ax, 16, sy0, 42, 12, fc="#f6f8fa"); ax.text(37, sy0 + 6, "truck harness", ha="center", va="center", fontsize=8)
    arrow(ax, (58, sy0 + 6), (78, sy0 + 6), color=MUTE)
    rbox(ax, 78, sy0, 36, 12, fc="#f6f8fa"); ax.text(96, sy0 + 6, "stock ECU", ha="center", va="center", fontsize=8)
    ax.text(130, sy0 + 17, "AFTER (shadow)", fontsize=9, weight="bold", color=ACC)
    rbox(ax, 130, sy0, 42, 12, fc="#f6f8fa"); ax.text(151, sy0 + 6, "truck harness", ha="center", va="center", fontsize=8)
    arrow(ax, (172, sy0 + 6), (190, sy0 + 6), color=INK)
    ax.text(181, sy0 + 9, "J1", ha="center", fontsize=7, color=ACC)
    rbox(ax, 190, sy0, 32, 12, fc="#eef4ff", ec=ACC)
    ax.text(206, sy0 + 6, "JUNKR", ha="center", va="center", fontsize=8, weight="bold", color=ACC)
    arrow(ax, (222, sy0 + 6), (242, sy0 + 6), color="#1a7f37")
    ax.text(232, sy0 + 9, "tails", ha="center", fontsize=7, color="#1a7f37")
    rbox(ax, 242, sy0, 36, 12, fc="#f6f8fa"); ax.text(260, sy0 + 6, "stock ECU", ha="center", va="center", fontsize=8)

    # JUNKR tray, plan view at 0.73 scale. J1 runs along the left edge; board x
    # (along the header) maps to drawing y, so E5 is at the top as on the stock ECU drawing.
    tx, ty, tw, th = junkr_tray(ax, 112, 80, 150, 145)
    S = 150 / 206.0
    def yb(bx):
        return ty + 14 + (bx - 41) * S
    seg = {"E8": (41.1, 83.6), "E6": (83.9, 110.2), "E5": (110.9, 159.3)}   # header sections (board x)
    slot = {"E8": (47.3, 83.6), "E6": (83.9, 110.2), "E5": (110.9, 154.2)}  # lid slots over J4, J3, J2
    col = {"E5": "#c2410c", "E6": "#0b62d6", "E8": "#1a7f37"}
    ax.add_patch(Rectangle((tx - 3, yb(41.1)), 8, yb(159.3) - yb(41.1), fc=ACC, ec=ACC))
    for p, (a0, a1) in seg.items():
        ax.plot([tx - 3, tx + 5], [yb(a0), yb(a0)], color="white", lw=1.2)
        ax.text(tx + 1, (yb(a0) + yb(a1)) / 2, p, rotation=90, ha="center", va="center", fontsize=7, color="white", weight="bold")
    ax.text(tx + 8, ty + th - 24, "J1  TE 176122-6", fontsize=8.5, weight="bold", color=ACC)
    ax.text(tx + 8, ty + th - 29, "same 64-way header as the stock ECU", fontsize=7, color=ACC, va="top")
    # legend inside, top right
    ix = tx + 70
    rows = [("SW", "10 outputs via relays (NC = stock)"), ("SENSE", "inputs: same copper, JUNKR listens"),
            ("SHARED", "lamps / tach: wired-OR, JUNKR idle"), ("POWER", "+B, IGSW, M-REL: diode-ORed")]
    ax.text(ix, ty + th - 27, "inside, per pin (sheets 3-6):", fontsize=7.3, color=MUTE)
    for i, (k, t) in enumerate(rows):
        ec, fc = CAT[k][0], CAT[k][1]
        ax.add_patch(Rectangle((ix, ty + th - 37 - i * 7.5), 5, 4.5, fc=fc, ec=ec, lw=1.2))
        ax.text(ix + 7, ty + th - 34.7 - i * 7.5, t, fontsize=6.9, va="center")

    # harness plugs on the left, lined up with their header sections
    hx = 16
    ax.text(hx, 220, "TRUCK HARNESS", fontsize=9.5, weight="bold")
    ax.text(hx, 215, "unchanged, nothing cut", fontsize=7.5, color=MUTE)
    plug_c = {p: (yb(a0) + yb(a1)) / 2 for p, (a0, a1) in seg.items()}
    for p, yc in plug_c.items():
        for k in range(6):
            ax.plot([hx, hx + 26], [yc - 5.5 + k * 2.2, yc - 5.5 + k * 2.2], color=MUTE, lw=0.8)
        rbox(ax, hx + 26, yc - 8.5, 30, 17, fc="white", ec=INK, lw=1.5, r=1.2)
        ax.text(hx + 41, yc + 2.5, p, ha="center", fontsize=11, weight="bold")
        ax.text(hx + 41, yc - 4, "stock plug", ha="center", fontsize=6.5, color=MUTE)
        arrow(ax, (hx + 56, yc), (tx - 3, yc), color=INK, lw=1.8)
        ax.text((hx + 56 + tx) / 2, yc + 3, "plugs in", ha="center", fontsize=6.5, color=MUTE)

    # lid slots and tails (they leave the lid and run over it to the stock ECU)
    sx_, sw_ = tx + 27 * S, 8.5 * S
    for p, (a0, a1) in slot.items():
        rbox(ax, sx_, yb(a0), sw_, yb(a1) - yb(a0), fc="#1b1f24", ec="#1b1f24", lw=0.8, r=1.5, z=4)
    ax.text(sx_ - 1, yb(159.3) + 3, "lid slots for the tails\n(J2 E5, J3 E6, J4 E8)", fontsize=6.6, color="#1a7f37", va="bottom")
    sx, sy = 330, 80
    rbox(ax, sx, sy, 72, 145, fc="#f6f8fa", ec=INK, lw=1.5)
    ax.text(sx + 36, sy + 137, "STOCK ECU", ha="center", fontsize=11, weight="bold")
    ax.text(sx + 36, sy + 130, "stays in its bracket", ha="center", fontsize=7.5, color=MUTE)
    for p, yc in plug_c.items():
        y0 = yc
        xs = sx_ + sw_ / 2
        ax.plot([xs, xs + 6, 300], [y0, y0, y0], color="#1a7f37", lw=4.5, solid_capstyle="round", alpha=0.35, zorder=5)
        ax.plot([xs, 300], [y0, y0], color="#1a7f37", lw=1.2, zorder=5)
        ax.add_patch(Rectangle((sx - 2, yc - 9.5), 8, 19, fc="#d0d7de", ec=INK, lw=1))
        ax.text(sx + 9, yc, "%s socket" % p, fontsize=7.5, va="center")
        rbox(ax, 300, yc - 8.5, 28, 17, fc="white", ec="#1a7f37", lw=1.6, r=1.2)
        ax.text(314, yc + 2.5, p, ha="center", fontsize=10, weight="bold", color="#1a7f37")
        ax.text(314, yc - 4, "new plug", ha="center", fontsize=6.5, color="#1a7f37")
        ax.text(314, yc - 11, PARTNO[p] + "\n+ 173716-1 terminals", ha="center", fontsize=5.8, color=MUTE, va="top")
        arrow(ax, (328, y0), (sx - 2, y0), color="#1a7f37", lw=1.6)
    ax.text(281, 232, "TAILS J2-J4", ha="center", fontsize=9, weight="bold", color="#1a7f37")
    ax.text(281, 227, "150 mm, soldered to the board", ha="center", fontsize=7, color="#1a7f37")
    ya = yb(41.1) - 6
    ax.annotate("", xy=(sx_ + sw_ / 2, ya), xytext=(300, ya), arrowprops=dict(arrowstyle="<->", color="#1a7f37", lw=0.8))
    ax.text(250, ya + 2.5, "tail length 150 mm (slot to plug), not to scale", ha="center", fontsize=7, color="#1a7f37")
    # J11 RUN switch and USB (USB-C is on the tray's left wall in reality; drawn below for clarity)
    ax.add_patch(Rectangle((tx + 60, ty - 20), 30, 12, fc="white", ec="#c2410c", lw=1.4))
    ax.text(tx + 75, ty - 12, "RUN switch", ha="center", fontsize=8, weight="bold", color="#c2410c")
    ax.text(tx + 75, ty - 17.5, "OFF in shadow", ha="center", fontsize=6.5, color="#c2410c")
    ax.plot([tx + 75, tx + 75], [ty - 8, ty], color="#c2410c", lw=1.4)
    ax.text(tx + 77, ty - 5, "J11", fontsize=6.5, color="#c2410c")
    ax.add_patch(Rectangle((tx + 96, ty - 20), 34, 12, fc="white", ec=MUTE, lw=1))
    ax.text(tx + 113, ty - 14, "USB-C / Wi-Fi", ha="center", fontsize=7, color=MUTE)
    ax.plot([tx + 113, tx + 113], [ty - 8, ty], color=MUTE, lw=1, ls="--")

    # Steps
    ax.text(16, 64, "STEPS", fontsize=10, weight="bold")
    steps = ["Battery negative off. Glovebox out (sheet 1).",
             "Unplug E5, E6, E8 from the stock ECU (press each latch).",
             "Plug them into JUNKR J1. Each plug only fits its own slot, same order as on the stock ECU.",
             "Plug JUNKR's three tails into the stock ECU. Again each fits only its own socket.",
             "RUN switch OFF. Battery on, key on: the truck starts and drives exactly as before; JUNKR logs and learns."]
    for i, s in enumerate(steps):
        callout(ax, 20, 55 - i * 8.6, i + 1)
        ax.text(26, 55 - i * 8.6, s, fontsize=8.3, va="center")
    ax.text(272, 64, "WHY THE STOCK ECU CAN'T TELL", fontsize=9, weight="bold")
    ax.text(272, 58, "Every pin reaches the stock ECU on its own copper. Inputs are only\ntapped at 47k or more. "
            "The ten outputs pass through a relay contact that\nrests closed to the stock ECU (NC). With RUN off the coils are unpowered.",
            fontsize=7.6, va="top", linespacing=1.45)
    save(fig, "conn_2_shadow.png")


# ---------------------------------------------------------------- sheet 3: signal paths
def ecu_block(ax, x, y, label, col=INK, fc="#f6f8fa", w=34, h=22):
    rbox(ax, x, y, w, h, fc=fc, ec=col, lw=1.4, r=1.5)
    ax.text(x + w / 2, y + h / 2, label, ha="center", va="center", fontsize=8, weight="bold", color=col)


def sheet_signals():
    fig, ax = sheet("What happens to each kind of pin",
                    "Five circuits cover all 59 used pins (the other 5 cavities are empty). Left: truck harness (J1). Right: stock ECU (tails). "
                    "Middle: what's on the JUNKR board.", 3)
    panels = [
        ("SW", "1  Outputs JUNKR can take over (10 pins)",
         "SPV E5-11, TCV E5-12, SVR E8-13, S1 E5-2, S2 E5-15, SL E5-1, EGR E5-24, S/TH1 E5-10, S/TH2 E5-9, S-REL E8-2"),
        ("SENSE", "2  Sensors and switches (30 pins)",
         "NE+/-, TDC+/-, SP2+/-, SP1, VA, IDL, PIM, THW, THA, THF, THO, VRP, VRT, STA, STP, NSW, L, 2, L4, TFN, P, OD2, SNW, IMI, HSW, TE1, TE2"),
        ("SHARED", "3  Lamps, tach, diagnostics (10 pins)", "W, G-IND, OIL-W, SNWL, TAC, VF, TT, IMO, A/C, ACT"),
        ("POWER", "4  Power (5 pins)", "+B E8-12, IGSW E8-14, M-REL E8-3, VC E6-1, BATT E8-1 (keep-alive, read only)"),
        ("GND", "5  Grounds (4 pins)", "E01 E5-13, E02 E5-26, E1 E5-14, E2 E6-9"),
    ]
    W, H = 196, 62
    pos = [(14, 196), (214, 196), (14, 128), (214, 128), (14, 60)]
    for (k, title, pins), (x, y) in zip(panels, pos):
        ec, fc = CAT[k][0], CAT[k][1]
        rbox(ax, x, y, W, H, fc="white", ec=ec, lw=1.5, r=2)
        ax.text(x + 4, y + H - 5, title, fontsize=10, weight="bold", color=ec, va="center")
        ax.text(x + 4, y + 5, pins, fontsize=6.6, color=MUTE, va="center")
        ly = y + 29
        # harness on the left, stock ECU on the right
        ax.text(x + 6, ly + 12, "harness", fontsize=7, color=MUTE)
        ax.add_patch(Rectangle((x + 6, ly - 2), 8, 4, fc=INK))
        ecu_block(ax, x + W - 40, ly - 11, "stock\nECU")
        rbox(ax, x + 50, y + 10, 92, 41, fc=fc, ec=ec, lw=1, r=1.5, ls=(0, (4, 2)))
        ax.text(x + 96, y + 48, "JUNKR board", ha="center", fontsize=7, color=ec)
        if k == "SW":
            # relay: COM from harness, NC to stock, NO to JUNKR driver
            cx = x + 90
            ax.plot([x + 14, cx - 8], [ly, ly], color=INK, lw=1.6)
            ax.add_patch(Circle((cx - 8, ly), 1.2, fc=INK))
            ax.plot([cx - 8, cx + 8], [ly, ly + 7], color=INK, lw=1.8)          # blade resting on NC (top)
            ax.add_patch(Circle((cx + 9, ly + 7), 1.2, fc="white", ec=INK))
            ax.add_patch(Circle((cx + 9, ly - 7), 1.2, fc="white", ec=INK))
            ax.text(cx + 9, ly + 11, "NC", ha="center", fontsize=6.5)
            ax.text(cx + 9, ly - 12, "NO", ha="center", fontsize=6.5)
            ax.plot([cx + 10, x + W - 40], [ly + 7, ly + 7], color=INK, lw=1.6)
            ax.plot([cx + 10, cx + 30], [ly - 7, ly - 7], color=ec, lw=1.6)
            ecu_block(ax, cx + 30, ly - 13, "driver", col=ec, fc="white", w=18, h=12)
            ax.text(cx - 26, ly + 12, "relay coil powered\nonly when ARMed", fontsize=6.5, color=ec, ha="center")
            ax.plot([cx - 30, cx - 30], [ly, ly - 12], color=ec, lw=0.8)
            ax.text(cx - 30, ly - 15, "tap (listens)", fontsize=6.5, color=ec, ha="center")
        elif k == "SENSE":
            ax.plot([x + 14, x + W - 40], [ly, ly], color=INK, lw=1.6)
            tx = x + 90
            ax.add_patch(Circle((tx, ly), 1.2, fc=INK))
            ax.plot([tx, tx], [ly, ly - 6], color=ec, lw=1)
            ax.add_patch(Rectangle((tx - 2, ly - 14), 4, 8, fc="white", ec=ec, lw=1))
            ax.text(tx + 4, ly - 10, "47k-100k", fontsize=6.5, color=ec, va="center")
            ax.plot([tx, tx], [ly - 14, ly - 16], color=ec, lw=1)
            ax.text(tx + 4, ly - 16.5, "to JUNKR input", fontsize=6.5, color=ec, va="center")
            ax.text(tx - 34, ly + 4, "same copper, straight through", fontsize=6.5, color=INK)
            ax.text(tx + 6, ly + 8, "NTC pull-ups and VC\nswitch in only with\nno stock ECU (swap)", fontsize=6, color=MUTE, va="top")
        elif k == "SHARED":
            ax.plot([x + 14, x + W - 40], [ly, ly], color=INK, lw=1.6)
            tx = x + 90
            ax.add_patch(Circle((tx, ly), 1.2, fc=INK))
            ax.plot([tx, tx], [ly, ly - 6], color=ec, lw=1)
            ecu_block(ax, tx - 12, ly - 17, "open drain\n(off)", col=ec, fc="white", w=24, h=11)
            ax.text(tx + 15, ly - 13, "pulls low only\nin takeover / swap", fontsize=6.5, color=ec, va="center")
            ax.text(tx - 34, ly + 4, "lamp / tach line, wired-OR", fontsize=6.5, color=INK)
        elif k == "POWER":
            ax.plot([x + 14, x + W - 40], [ly, ly], color=INK, lw=1.6)
            tx = x + 90
            ax.add_patch(Circle((tx, ly), 1.2, fc=INK))
            ax.plot([tx, tx], [ly, ly - 6], color=ec, lw=1)
            ax.add_patch(Polygon([(tx - 3, ly - 6), (tx + 3, ly - 6), (tx, ly - 11)], closed=True, fc=ec, ec=ec))
            ax.plot([tx - 3, tx + 3], [ly - 11, ly - 11], color=ec, lw=1.2)
            ax.plot([tx, tx], [ly - 11, ly - 16], color=ec, lw=1)
            ax.text(tx - 3, ly - 16, "JUNKR supply", fontsize=6.5, color=ec, ha="right", va="center")
            ax.text(tx + 6, ly - 8, "diode: JUNKR can draw,\nnever back-feeds", fontsize=6.3, color=ec, va="center")
            ax.text(tx - 34, ly + 4, "+B / IGSW / M-REL", fontsize=6.5, color=INK)
        else:
            ax.plot([x + 14, x + W - 40], [ly, ly], color=INK, lw=1.6)
            tx = x + 90
            ax.add_patch(Circle((tx, ly), 1.2, fc=INK))
            ax.plot([tx, tx], [ly, ly - 9], color=INK, lw=1)
            for i, ww in enumerate((8, 5, 2)):
                ax.plot([tx - ww / 2, tx + ww / 2], [ly - 9 - i * 1.6, ly - 9 - i * 1.6], color=INK, lw=1.2)
            ax.text(tx + 6, ly - 8, "JUNKR ground (E2 via a 0R link\nnear the analog front end)", fontsize=6.3, va="center")
    # RUN switch note box
    rbox(ax, 214, 60, 196, 62, fc="#fff8f3", ec="#c2410c", lw=1.2, r=2)
    ax.text(218, 116, "When does a relay move?", fontsize=10, weight="bold", color="#c2410c")
    ax.text(218, 109, "The relay coils are fed from VRLY, which is +B switched by ARM:\n\n"
                     "    ARM = ARM_REQ (STM32)  AND  SAFE_OK (ATtiny watchdog)  AND  RUN switch\n\n"
                     "Shadow mode: RUN off, so ARM is low, VRLY is dead and every relay\n"
                     "rests on NC. The stock ECU drives everything. Any fault, a watchdog\n"
                     "trip, or RUN off drops the relays back to the stock ECU instantly.",
            fontsize=7.8, va="top", linespacing=1.35)
    save(fig, "conn_3_signals.png")


# ---------------------------------------------------------------- sheets 4-6: one per plug
DESC = {
    "SL": "A/T lock-up solenoid", "S1": "A/T shift solenoid No.1", "S2": "A/T shift solenoid No.2",
    "SP2+": "A/T speed sensor No.2 (+)", "SP2-": "A/T speed sensor No.2 (-)",
    "NE+": "pump speed sensor (+)", "NE-": "pump speed sensor (-)", "TDC+": "crank TDC sensor (+)",
    "TDC-": "crank TDC sensor (-)", "L4": "transfer L4 switch", "TFN": "transfer neutral switch",
    "S/TH1": "intake constrictor VSV 1", "S/TH2": "intake constrictor VSV 2", "SPV": "spill valve (fuel quantity)",
    "TCV": "timing control valve", "E01": "power ground", "E02": "power ground", "E1": "signal ground",
    "L": "shift lever L", "2": "shift lever 2", "SNWL": "2nd-start lamp", "SNW": "2nd-start switch",
    "EGR": "EGR VSV", "VC": "5 V sensor supply", "PIM": "turbo pressure sensor", "THA": "intake air temp",
    "THW": "coolant temp", "THF": "fuel temp", "VRP": "pump correction resistor 1", "TE2": "diag terminal TE2",
    "VF": "diag output VF", "E2": "sensor ground", "VA": "accelerator position", "IDL": "idle switch",
    "THO": "A/T fluid temp", "VRT": "pump correction resistor 2", "TE1": "diag terminal TE1", "TT": "A/T diag terminal",
    "BATT": "battery keep-alive", "S-REL": "sub glow relay", "M-REL": "ECD main relay control", "STP": "stop lamp switch",
    "W": "check engine lamp", "OIL-W": "A/T oil temp lamp", "G-IND": "glow indicator", "IMO": "immobiliser out",
    "SP1": "vehicle speed", "A/C": "A/C clutch signal", "STA": "starter signal", "+B": "ECU power (ECD relay)",
    "SVR": "spill valve relay", "IGSW": "ignition switch", "P": "pattern select (PWR)", "OD2": "overdrive switch",
    "IMI": "immobiliser in", "TAC": "tachometer out", "ACT": "A/C cut", "HSW": "idle-up switch", "NSW": "neutral start switch",
}


def sheet_plug(plug, num):
    count = {"E5": 26, "E6": 16, "E8": 22}[plug]
    fig, ax = sheet("%s (%d pin): every pin, shadow mode and full swap" % (plug, count),
                    "Pin numbering as EWD269F p.210: harness plug, looking into its open end, latch up (outline simplified). "
                    "From the wire side, as in your photo, left and right swap.", num)
    cell = {"E5": 13.0, "E6": 18.0, "E8": 15.0}[plug]
    w, H = face(ax, 16, 200, plug, cell=cell, h=15, title="%s harness plug, mating face" % plug)
    legend(ax, 16, 180, w=120)
    # table
    x0, y0 = 196, 258
    cols = [("Pin", 0), ("Name", 11), ("Wire", 27), ("Function", 43), ("Shadow mode", 92), ("Full swap", 152)]
    rh = 197 / max(count, 22)
    rh = min(rh, 8.6)
    ax.add_patch(Rectangle((x0 - 2, y0 - 3), 214, 7, fc="#eceff1", ec="none"))
    for t, dx in cols:
        ax.text(x0 + dx, y0 + 0.5, t, fontsize=8, weight="bold", va="center")
    for i in range(count):
        pin = i + 1
        y = y0 - (i + 1) * rh
        cat, sh, sw = classify(plug, pin)
        ec, fc = CAT[cat][0], CAT[cat][1]
        if i % 2 == 0:
            ax.add_patch(Rectangle((x0 - 2, y - rh / 2), 214, rh, fc="#fafbfc", ec="none"))
        ax.add_patch(Rectangle((x0 - 2, y - rh / 2 + 0.6), 1.6, rh - 1.2, fc=ec, ec="none"))
        name = CIR.PO[(plug, pin)]["name"] or "-"
        fs = 7.3 if rh > 7.5 else 6.6
        ax.text(x0 + 1, y, str(pin), fontsize=fs, va="center")
        ax.text(x0 + 11, y, name, fontsize=fs, va="center", weight="bold", color=ec if cat != "NC" else MUTE)
        code = WIRES.get((plug, pin), "")
        if cat != "NC":
            swatch(ax, x0 + 27, y, code, w=6)
            ax.text(x0 + 34.5, y, code, fontsize=fs - 0.6, va="center", color=MUTE)
        ax.text(x0 + 43, y, DESC.get(name, "not used"), fontsize=fs - 0.3, va="center")
        ax.text(x0 + 92, y, sh, fontsize=fs - 0.5, va="center", color=INK if cat != "NC" else MUTE)
        ax.text(x0 + 152, y, sw, fontsize=fs - 0.5, va="center", color=INK if cat != "NC" else MUTE)
    # counts
    from collections import Counter
    cnt = Counter(classify(plug, p)[0] for p in range(1, count + 1))
    ax.text(16, 110, "ON THIS PLUG", fontsize=9.5, weight="bold")
    for i, k in enumerate(("SW", "SENSE", "SHARED", "POWER", "GND", "NC")):
        ax.text(16, 102 - i * 6.5, "%2d  %s" % (cnt.get(k, 0), CAT[k][2]), fontsize=8.3, color=CAT[k][0] if k != "NC" else MUTE)
    ax.text(16, 58, "Wire colours: Toyota code, base colour then stripe\n(B black, W white, R red, G green, L blue, Y yellow,\n"
                    "BR brown, GR gray, LG light green, P pink, V violet,\nO orange). Check against your harness before probing.",
            fontsize=7.3, color=MUTE, va="top", linespacing=1.4)
    tail = {"E5": "J2", "E6": "J3", "E8": "J4"}[plug]
    ax.text(16, 30, "In shadow mode this plug goes into JUNKR J1, and JUNKR's tail %s\n(%s + 173716-1) goes into the stock ECU's %s socket."
            % (tail, PARTNO[plug], plug), fontsize=8, va="top", linespacing=1.4)
    save(fig, "conn_%d_%s.png" % (num, plug))


if __name__ == "__main__":
    sheet_location()
    sheet_shadow()
    sheet_signals()
    for n, p in ((4, "E5"), (5, "E6"), (6, "E8")):
        sheet_plug(p, n)
