"""Install drawings for the standalone ECU.

    python3 gen_drawings.py   ->  install_shadow.png, install_swap.png, harness_pinout.png

Pin names follow Connor's wiring manual EWD269F (KZJ95, p.210 connector
list, pp.206-207 ECD system), as tabulated in docs/ECU_INTERPOSER_RESEARCH.md.
"""
import os
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Rectangle, FancyArrowPatch

HERE = os.path.dirname(os.path.abspath(__file__))
INK, MUTE, ACC, OK, WARN = "#1b1f24", "#6a737d", "#0b62d6", "#1a7f37", "#b35900"
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 9})


def sheet(title, sub, num):
    fig = plt.figure(figsize=(16.5, 11.7), dpi=110)   # A3 landscape
    ax = fig.add_axes([0, 0, 1, 1]); ax.set_xlim(0, 420); ax.set_ylim(0, 297); ax.axis("off")
    ax.add_patch(Rectangle((8, 8), 404, 281, fill=False, lw=1.4, ec=INK))
    # title block
    ax.add_patch(Rectangle((262, 8), 150, 34, fill=False, lw=1.2, ec=INK))
    for y in (19, 30):
        ax.plot([262, 412], [y, y], color=INK, lw=0.6)
    ax.text(266, 35, "JUNKR STANDALONE ECU rev A, KZJ95 1KZ-TE / A343F", fontsize=9, weight="bold", va="center")
    ax.text(266, 24.5, title, fontsize=10.5, weight="bold", va="center", color=ACC)
    ax.text(266, 13.5, "Ref: EWD269F p.210 (connectors), pp.206-207 (ECD)   Sheet %d of 3" % num, fontsize=8, va="center")
    ax.text(16, 279, title, fontsize=17, weight="bold", va="top")
    ax.text(16, 269, sub, fontsize=10, va="top", color=MUTE)
    return fig, ax


def box(ax, x, y, w, h, label, sub=None, fc="#f6f8fa", ec=INK, lw=1.4, fs=11):
    ax.add_patch(FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0,rounding_size=3", fc=fc, ec=ec, lw=lw))
    ax.text(x + w / 2, y + h - 6, label, ha="center", va="top", fontsize=fs, weight="bold")
    if sub:
        ax.text(x + w / 2, y + h - 13, sub, ha="center", va="top", fontsize=8, color=MUTE, linespacing=1.4)


def plug(ax, x, y, name, pins, part, side="R", color=INK):
    """A harness plug drawn as a small block with its pin count."""
    ax.add_patch(Rectangle((x, y), 26, 14, fc="white", ec=color, lw=1.6))
    ax.text(x + 13, y + 9.5, name, ha="center", va="center", fontsize=10.5, weight="bold", color=color)
    ax.text(x + 13, y + 3.5, "%d pin" % pins, ha="center", va="center", fontsize=7.5, color=MUTE)
    ax.text(x + 13, y - 3, part, ha="center", va="top", fontsize=7, color=MUTE)


def wire(ax, pts, color=INK, lw=2.2, ls="-", arrow=False):
    xs, ys = zip(*pts)
    ax.plot(xs, ys, color=color, lw=lw, ls=ls, solid_capstyle="round")
    if arrow:
        ax.add_patch(FancyArrowPatch(pts[-2], pts[-1], arrowstyle="-|>", mutation_scale=14, color=color, lw=lw))


def note(ax, x, y, n, text, w=None):
    ax.add_patch(plt.Circle((x, y), 3.2, fc=ACC, ec="none"))
    ax.text(x, y, str(n), ha="center", va="center", color="white", fontsize=8.5, weight="bold")
    ax.text(x + 6, y, text, va="center", fontsize=9, linespacing=1.45)


HARNESS = [("E5", 26), ("E6", 16), ("E8", 22)]
TAILS = {"E5": "174516-6", "E6": "174514-6", "E8": "174515-6"}


def harness_side(ax, x0=22):
    box(ax, x0, 100, 70, 150, "VEHICLE HARNESS", "behind the glovebox,\nleft side (RHD)")
    ys = [200, 160, 120]
    for (n, p), y in zip(HARNESS, ys):
        plug(ax, x0 + 40, y, n, p, "stock plug")
    return ys


def junkr(ax, x, y, w, h, shadow=True):
    box(ax, x, y, w, h, "JUNKR ECU", "STM32H743 + ESP32-S3 + ATtiny supervisor", fc="#eef4ff", ec=ACC, lw=2)
    # J1 header
    ax.add_patch(Rectangle((x - 4, y + 22), 10, h - 50, fc=ACC, ec=ACC))
    ax.text(x + 9, y + h - 30, "J1  TE 176122-6\n(same 64-way header\nas the stock ECU)", fontsize=8, va="top", color=ACC)
    # inside: input path and relays
    ix = x + 48
    ax.text(ix, y + h - 30, "Inputs (sensors, switches, NE/TDC)", fontsize=8.5, weight="bold", va="top")
    ax.text(ix, y + h - 37, "47k high-Z taps, never load the stock ECU" if shadow else
            "own pull-ups and 5 V VC switch on (no stock ECU found)", fontsize=8, color=MUTE, va="top")
    ax.text(ix, y + h - 50, "Outputs through Form-C relays", fontsize=8.5, weight="bold", va="top")
    groups = [("FUEL", "SPV E5-11, SVR E8-13"), ("TIME", "TCV E5-12"),
              ("TRANS", "S1 E5-2, S2 E5-15, SL E5-1"), ("AIR", "EGR E5-24, S/TH1-2, S-REL")]
    for i, (g, s) in enumerate(groups):
        gy = y + h - 64 - i * 13
        ax.add_patch(Rectangle((ix, gy - 4), 18, 8, fc="white", ec=INK, lw=1))
        ax.text(ix + 9, gy, g, ha="center", va="center", fontsize=7.5, weight="bold")
        ax.text(ix + 21, gy, s, va="center", fontsize=7.5)
        if shadow:
            ax.text(ix + 66, gy, "NC: stock ECU   NO: JUNKR", va="center", fontsize=7.5, color=OK if i == 0 else MUTE)
        else:
            ax.text(ix + 66, gy, "NO: JUNKR drives (RUN on)", va="center", fontsize=7.5, color=WARN if i == 0 else MUTE)
    ax.text(ix, y + 20, "Lamps, tach, VF, IMO: open drain, wired-OR", fontsize=8, color=MUTE)
    ax.text(ix, y + 13, "M-REL main relay: diode-ORed, either ECU holds it on", fontsize=8, color=MUTE)


def shadow():
    fig, ax = sheet("Install, shadow mode", "Stock ECU stays in the loop on factory pigtails. No cutting, no splicing: every joint is a factory TE plug.", 1)
    ys = harness_side(ax)
    junkr(ax, 140, 92, 160, 158)
    for (n, p), y in zip(HARNESS, ys):
        wire(ax, [(88, y + 7), (136, y + 7)], arrow=True)
    # tails out of the board to the stock ECU
    box(ax, 330, 100, 70, 150, "STOCK ECU", "89661-xxxxx\nunchanged, in its\nfactory mount", fc="#f6f8fa")
    for (n, p), y in zip(HARNESS, ys):
        plug(ax, 334, y, n, p, TAILS[n] + " + 173716-1", color=OK)
        wire(ax, [(300, y + 7), (334, y + 7)], color=OK, arrow=True)
    ax.text(304, 266, "Tails J2-J4", fontsize=9, weight="bold", color=OK)
    ax.text(304, 261, "pigtails soldered to the\nboard, ending in the\nstock ECU's own plugs", fontsize=7.5, color=OK, va="top")
    # RUN switch and USB
    ax.add_patch(Rectangle((215, 64), 34, 16, fc="white", ec=WARN, lw=1.6))
    ax.text(232, 75, "RUN switch", ha="center", fontsize=9, weight="bold", color=WARN)
    ax.text(232, 68, "dash, OFF", ha="center", fontsize=8, color=WARN)
    wire(ax, [(232, 80), (232, 92)], color=WARN, lw=1.6)
    ax.text(235, 85, "J11", fontsize=7.5, color=WARN)
    ax.add_patch(Rectangle((255, 64), 40, 16, fc="white", ec=MUTE, lw=1.2))
    ax.text(275, 72, "USB-C / Wi-Fi\nlaptop or phone", ha="center", va="center", fontsize=7.5, color=MUTE)
    wire(ax, [(275, 80), (275, 92)], color=MUTE, lw=1.2, ls="--")
    # notes
    ax.text(16, 84, "STEPS", fontsize=10, weight="bold")
    steps = ["Battery negative off. Remove the glovebox (EWD269F p.104 shows the ECU position).",
             "Unplug E5, E6, E8 from the stock ECU and plug them into JUNKR J1.",
             "Plug the three tails into the stock ECU. Each tail only fits its own socket.",
             "Fit the RUN switch to the dash, OFF. Battery on, key on: the truck must start and drive exactly as before."]
    for i, s in enumerate(steps):
        note(ax, 22, 74 - i * 9.5, i + 1, s)
    ax.text(302, 84, "RUN OFF: all relays rest on NC, the stock ECU drives,\nJUNKR only listens and learns.\n"
            "RUN ON (after the agreement gate): relays pull to NO.\nAny fault drops them, the stock ECU takes back over.",
            fontsize=8.5, va="top", linespacing=1.5)
    fig.savefig(os.path.join(HERE, "install_shadow.png")); plt.close(fig)


def swap():
    fig, ax = sheet("Install, full swap", "The stock ECU comes out. JUNKR takes the harness plugs directly: literally plug and play.", 2)
    ys = harness_side(ax)
    junkr(ax, 140, 92, 160, 158, shadow=False)
    for (n, p), y in zip(HARNESS, ys):
        wire(ax, [(88, y + 7), (136, y + 7)], arrow=True)
    box(ax, 330, 100, 70, 150, "STOCK ECU", "removed, keep it\nas a spare", fc="white", ec=MUTE)
    ax.plot([330, 400], [100, 250], color=MUTE, lw=1); ax.plot([330, 400], [250, 100], color=MUTE, lw=1)
    ax.text(304, 266, "Tails J2-J4", fontsize=9, weight="bold", color=MUTE)
    ax.text(304, 261, "not fitted, or left\ncoiled and capped", fontsize=7.5, color=MUTE, va="top")
    ax.add_patch(Rectangle((215, 64), 34, 16, fc="white", ec=WARN, lw=1.6))
    ax.text(232, 75, "RUN switch", ha="center", fontsize=9, weight="bold", color=WARN)
    ax.text(232, 68, "ON = engine runs", ha="center", fontsize=8, color=WARN)
    wire(ax, [(232, 80), (232, 92)], color=WARN, lw=1.6)
    ax.text(16, 84, "STEPS", fontsize=10, weight="bold")
    steps = ["Only after shadow mode and takeover testing pass (BENCH_TEST.md stages 5-7).",
             "Battery negative off. Unplug the tails from the stock ECU and remove it.",
             "E5, E6, E8 stay plugged into JUNKR J1, nothing else changes.",
             "RUN switch ON is now the engine's run permission: OFF stops fuel (SVR and SPV relays drop)."]
    for i, s in enumerate(steps):
        note(ax, 22, 74 - i * 9.5, i + 1, s)
    ax.text(302, 84, "No stock ECU on NC: a relay drop now means fuel off,\nso any fault stops the engine (the safe side for a diesel).\n"
            "JUNKR sees no VC from a stock ECU at boot and turns on\nits own 5 V reference and sensor pull-ups.",
            fontsize=8.5, va="top", linespacing=1.5)
    fig.savefig(os.path.join(HERE, "install_swap.png")); plt.close(fig)


E5 = ["SL", "S1", "SP2+", "NE+", "TDC+", "-", "L4", "TFN", "S/TH2", "S/TH1", "SPV", "TCV", "E01",
      "E1", "S2", "SP2-", "NE-", "TDC-", "-", "L", "2", "SNWL", "SNW", "EGR", "-", "E02"]
E6 = ["VC", "PIM", "THA", "THW", "THF", "VRP", "TE2", "VF", "E2", "-", "VA", "IDL", "THO", "VRT", "TE1", "TT"]
E8 = ["BATT", "S-REL", "M-REL", "STP", "W", "OIL-W", "G-IND", "IMO", "SP1", "A/C", "STA", "+B", "SVR",
      "IGSW", "P", "OD2", "IMI", "TAC", "ACT", "HSW", "-", "NSW"]
KEY = {"SPV": WARN, "TCV": WARN, "SVR": WARN, "SL": WARN, "S1": WARN, "S2": WARN, "EGR": WARN, "S/TH1": WARN, "S/TH2": WARN, "S-REL": WARN, "NE+": ACC, "NE-": ACC, "TDC+": ACC, "TDC-": ACC,
       "E01": INK, "E02": INK, "E1": INK, "E2": INK, "+B": OK, "BATT": OK, "IGSW": OK, "M-REL": OK,
       "VA": ACC, "PIM": ACC, "VRP": ACC, "VRT": ACC}


def pinout():
    fig, ax = sheet("Harness plugs and what JUNKR does with each pin",
                    "Pin numbers and names as in EWD269F p.210. Numbering only: check orientation against the numbers moulded on your plug.", 3)
    spots = {"E5": (22, 205), "E6": (22, 135), "E8": (150, 135)}
    for name, pins, part in (("E5", E5, "26 pin"), ("E6", E6, "16 pin"), ("E8", E8, "22 pin")):
        x, y0 = spots[name]
        half = len(pins) // 2
        cw = 13.5
        w = half * cw + 8
        ax.text(x, y0 + 47, "%s  (%s)" % (name, part), fontsize=12, weight="bold")
        ax.add_patch(FancyBboxPatch((x, y0), w, 40, boxstyle="round,pad=0,rounding_size=2", fc="#f6f8fa", ec=INK, lw=1.4))
        for row in range(2):
            for i in range(half):
                n = row * half + i
                cx, cy = x + 4 + i * cw, y0 + 24 - row * 20
                col = KEY.get(pins[n], MUTE if pins[n] == "-" else INK)
                ax.add_patch(Rectangle((cx, cy - 2), cw - 1.5, 13, fc="white", ec=col, lw=1.3 if col != MUTE else 0.6))
                ax.text(cx + (cw - 1.5) / 2, cy + 7.5, pins[n], ha="center", va="center", fontsize=6.6, weight="bold", color=col)
                ax.text(cx + (cw - 1.5) / 2, cy + 1.2, str(n + 1), ha="center", va="center", fontsize=6, color=MUTE)
    rows = [
        (WARN, "Driven by JUNKR through takeover relays", "SPV E5-11, TCV E5-12, SVR E8-13, S1 E5-2, S2 E5-15, SL E5-1, EGR E5-24, S/TH1 E5-10, S/TH2 E5-9, S-REL E8-2"),
        (ACC, "Read by JUNKR (high-Z, shared with the stock ECU)", "NE+/- E5-4/17, TDC+/- E5-5/18, SP2+/- E5-3/16, VA E6-11, PIM E6-2, THW E6-4, THA E6-3, THF E6-5, THO E6-13, VRP E6-6, VRT E6-14, SP1 E8-9"),
        (OK, "Power", "+B E8-12 (via ECD relay), BATT E8-1 (keep-alive), IGSW E8-14, M-REL E8-3 (diode-ORed)"),
        (INK, "Grounds", "E01 E5-13, E02 E5-26 (power), E1 E5-14 (signal), E2 E6-9 (sensor)"),
        (MUTE, "Switches and lamps (read, or open drain wired-OR)", "IDL, STP, STA, NSW, L4, TFN, L, 2, P, OD2, A/C, ACT, HSW, SNW; W, OIL-W, G-IND, SNWL, TAC, VF, IMO/IMI, TE1/TE2/TT"),
    ]
    ax.text(318, 252, "Every pin of all three plugs lands on J1\none to one, and in shadow mode on the\ntails one to one. The stock ECU sees its\nharness as before; only the relay-switched\noutputs pass through a relay contact\n(NC side) on the way.", fontsize=8.5, va="top", linespacing=1.5)
    y = 112
    ax.text(22, y + 9, "KEY", fontsize=10, weight="bold")
    for col, head, body in rows:
        ax.add_patch(Rectangle((22, y - 4), 7, 7, fc="white", ec=col, lw=1.8))
        ax.text(33, y, head, fontsize=9.5, weight="bold", color=col if col != MUTE else INK, va="center")
        ax.text(33, y - 6, body, fontsize=8, color=INK, va="center")
        y -= 14
    fig.savefig(os.path.join(HERE, "harness_pinout.png")); plt.close(fig)


if __name__ == "__main__":
    shadow(); swap(); pinout()
    print("wrote install_shadow.png, install_swap.png, harness_pinout.png")
