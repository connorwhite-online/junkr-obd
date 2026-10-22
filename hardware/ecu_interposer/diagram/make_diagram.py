"""Build the C26 connector sheet: exploded view + cross-section analysis.

Needs OpenSCAD and xvfb-run (for the 3D render), numpy, matplotlib, Pillow.
    python3 make_diagram.py            -> connector_diagram.png / .pdf
"""

import re
import subprocess
import tempfile
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import PathPatch
from matplotlib.path import Path as MPath
from PIL import Image

HERE = Path(__file__).resolve().parent
CAD = HERE.parent / "cad"
SCAD = CAD / "diagram.scad"

# ---- geometry (mirrors cad/sections.scad + params.scad for section C26) ----
L = 42.44                     # C26 cavity length
CAV_H, SLOT_TOP, DEPTH, WALL = 11.82, 15.02, 14.50, 1.48
ROW_040, ROW_070 = (7.05, 1.85), (8.05, 2.85)
PROUD = {1: 7.09, 2: 7.99}
COLS = [(2.37 + 3.5 * i, 2) for i in range(3)] + [(12.47 + 3.0 * i, 1) for i in range(8)] + \
       [(36.57 + 3.5 * i, 2) for i in range(2)]
CLR, SLIDE, KEY = 0.15, 0.25, 0.20
BODY = {1: (2.00, 2.20), 2: (2.85, 2.80)}          # receptacle body w, h  [MEASURE]
LANCE = {1: 0.45, 2: 0.55}
FACE_WALL, PLUG_GAP, INSERT = 1.0, 0.2, DEPTH - 0.2
SHROUD_FLOOR, CH_WALL, FLANGE_T, CH_LEN = 9.0, 1.8, 2.0, 26.0
PLUG_LEN = INSERT + 6.0
Z_SH = PLUG_GAP + PLUG_LEN + (2 * CH_WALL + 2 * FLANGE_T + CH_LEN) - 2 * CH_WALL
Z_FL = Z_SH + SHROUD_FLOOR
Z_OEM = Z_FL + PLUG_GAP
E = 30.0
OFF = dict(hdr=-E, plug=0, rec=E, tab=2.2 * E, sh=3.2 * E, oem=4.4 * E, orec=5.4 * E)

COL = dict(header="#5d6d7e", header_pins="#d4ac0d", plug="#2e86c1", receptacles="#ba7a45",
           tabs="#d4ac0d", shroud="#e67e22", oem_plug="#aab7b8", oem_receptacles="#ba7a45",
           chamber="#e5e8e8", wires="#922b21")
INK = "#1b2631"
DIM = "#c0392b"

CAM = ["--camera=112,0,8,62,0,55,470", "--imgsize=2200,1100", "--projection=o", "--colorscheme=Tomorrow"]


def scad(out, *defs, xvfb=False):
    cmd = (["xvfb-run", "-a"] if xvfb else []) + ["openscad", "-q", "-o", str(out)]
    for d in defs:
        cmd += ["-D", d]
    subprocess.run(cmd + [str(SCAD)], check=True, cwd=CAD, capture_output=True)


def svg_paths(f):
    """OpenSCAD 2D SVG -> list of (N,2) rings, y flipped back to +up."""
    txt = Path(f).read_text()
    rings = []
    for d in re.findall(r'd="([^"]+)"', txt):
        for sub in re.findall(r"M([^z]+)z", d):
            pts = np.array([[float(a), float(b)] for a, b in re.findall(r"(-?[\d.e-]+),(-?[\d.e-]+)", sub)])
            if len(pts) > 2:
                pts[:, 1] *= -1
                rings.append(pts)
    return rings


def draw_rings(ax, rings, color, alpha=1.0, lw=0.6, z=1):
    if not rings:
        return
    verts, codes = [], []
    for r in rings:
        verts += list(r) + [r[0]]
        codes += [MPath.MOVETO] + [MPath.LINETO] * (len(r) - 1) + [MPath.CLOSEPOLY]
    ax.add_patch(PathPatch(MPath(verts, codes), fc=color, ec=INK, lw=lw, alpha=alpha, zorder=z))


def dim_h(ax, x0, x1, y, text, ext_from=None, above=True, fs=9, color=DIM):
    ax.annotate("", (x0, y), (x1, y), arrowprops=dict(arrowstyle="<|-|>", color=color, lw=0.9,
                                                      shrinkA=0, shrinkB=0, mutation_scale=7))
    if ext_from is not None:
        for x in (x0, x1):
            ax.plot([x, x], [ext_from, y], color=color, lw=0.5, ls=(0, (2, 2)))
    ax.text((x0 + x1) / 2, y + (0.35 if above else -0.35), text, ha="center",
            va="bottom" if above else "top", fontsize=fs, color=color)


def dim_v(ax, x, y0, y1, text, ext_from=None, right=True, fs=9, color=DIM):
    ax.annotate("", (x, y0), (x, y1), arrowprops=dict(arrowstyle="<|-|>", color=color, lw=0.9,
                                                      shrinkA=0, shrinkB=0, mutation_scale=7))
    if ext_from is not None:
        for y in (y0, y1):
            ax.plot([ext_from, x], [y, y], color=color, lw=0.5, ls=(0, (2, 2)))
    ax.text(x + (0.4 if right else -0.4), (y0 + y1) / 2, text, ha="left" if right else "right",
            va="center", fontsize=fs, color=color)


def webs():
    """Thinnest walls in the female plug, from the same rules as interposer.scad."""
    out = {}
    for (xa, ta), (xb, tb) in zip(COLS, COLS[1:]):
        w = xb - xa - (BODY[ta][0] + BODY[tb][0]) / 2 - 2 * CLR
        key = {(2, 2): "070 | 070", (1, 1): "040 | 040"}.get((ta, tb), "070 | 040")
        out[key] = min(out.get(key, 9), w)
    rows = {t: (r[0] - r[1]) - (BODY[t][1] + 2 * CLR) for t, r in ((1, ROW_040), (2, ROW_070))}
    floor = {t: r[1] - (BODY[t][1] / 2 + CLR) - SLIDE - LANCE[t] for t, r in ((1, ROW_040), (2, ROW_070))}
    top = {t: (CAV_H - SLIDE) - (r[0] + BODY[t][1] / 2 + CLR + LANCE[t]) for t, r in ((1, ROW_040), (2, ROW_070))}
    return out, rows, floor, top


# ------------------------------------------------------------------ build
def main():
    tmp = Path(tempfile.mkdtemp())
    subprocess.run(["xvfb-run", "-a", "openscad", "-q", "-o", str(tmp / "ex.png"), *CAM, str(SCAD)],
                   check=True, cwd=CAD, capture_output=True)
    # projection calibration: render known points, fit the (affine) ortho map
    P = np.array([[-30, -10, -40], [30, -10, -40], [-30, 25, 200], [30, 25, 60], [0, 0, 120], [20, -5, 250]], float)
    uv = []
    for i, p in enumerate(P):
        f = tmp / f"m{i}.png"
        subprocess.run(["xvfb-run", "-a", "openscad", "-q", "-o", str(f), *CAM, "-D", 'view="markers"',
                        "-D", f"MARKERS=[[{p[0]},{p[1]},{p[2]}]]", str(SCAD)], check=True, cwd=CAD, capture_output=True)
        a = np.asarray(Image.open(f).convert("RGB")).astype(int)
        ys, xs = np.nonzero(np.abs(a - a[0, 0]).sum(2) > 30)
        uv.append([xs.mean(), ys.mean()])
    M = np.linalg.lstsq(np.c_[P, np.ones(len(P))], np.array(uv), rcond=None)[0]
    proj = lambda x, y, z: np.array([x, y, z, 1.0]) @ M

    comps_x = ["header", "plug", "receptacles", "header_pins"]
    comps_l = ["chamber", "header", "plug", "receptacles", "wires", "tabs", "shroud", "oem_plug",
               "oem_receptacles", "header_pins"]
    xsec = {}
    for c in comps_x:
        f = tmp / f"x_{c}.svg"
        scad(f, 'view="cut"', 'cut="x"', "cut_at=4", f'comp="{c}"')
        xsec[c] = svg_paths(f)
    lcols = [(COLS[0][0], "070 column (end of C26)"), (COLS[4][0], "040 column")]
    lsec = []
    for x, _ in lcols:
        d = {}
        for c in comps_l:
            f = tmp / f"l_{c}.svg"
            scad(f, 'view="cut"', 'cut="l"', f"cut_at={x - L / 2}", 'E=0', f'comp="{c}"')
            d[c] = svg_paths(f)
        lsec.append(d)

    # -------------------------------------------------------------- sheet
    fig = plt.figure(figsize=(22, 27), dpi=110, facecolor="white")
    fig.text(0.04, 0.975, "1KZ-TE ECU interposer: C26 section, exploded view and cross-section analysis",
             fontsize=22, weight="bold", color=INK)
    fig.text(0.04, 0.962, "Header TE 176122-6 (geometry measured from TE's 3D model). Terminal bodies and the "
             "harness latch are estimates [MEASURE]. All dimensions in mm.", fontsize=12, color="#566573")

    # ---- 1. exploded view
    ax = fig.add_axes([0.02, 0.62, 0.96, 0.32])
    img = np.asarray(Image.open(tmp / "ex.png").convert("RGB")).copy()
    bg = np.all(np.abs(img.astype(int) - img[0, 0].astype(int)).sum(-1, keepdims=True) < 6, axis=-1)
    img[bg] = 255                                   # white background to match the sheet
    ax.imshow(img)
    ax.set_xlim(250, 1950)
    ax.set_ylim(1020, 70)
    ax.axis("off")
    ax.set_title("1  Exploded view (mating axis left to right)", loc="left", fontsize=15, weight="bold",
                 color=INK, x=0.02)
    calls = [   # (anchor in scene coords, text, label position in image px, label side)
        ((-8, 16.5, OFF["hdr"] - 4), "ECU header (on the ECU board)\nTE 176122-6, C26 cavity", (420, 330), "l"),
        ((-6, 6.0, OFF["hdr"] + 2), "Header pins: 040 tabs 0.98 x 0.62\n070 tabs 1.78 x 0.62", (560, 560), "l"),
        ((-14, 2.0, OFF["plug"] + 8), "female_plug (printed)\nkey ridges ride in the header slots", (720, 700), "l"),
        ((-21, 3.0, OFF["rec"] + 8), "Receptacles\n040: TE 175061-1, 070: TE 173631-1", (880, 830), "l"),
        ((0, 8.0, 65), "Pigtails (~40 mm), one per pin\ntaps spliced here, inside the chamber", (980, 190), "r"),
        ((-21, 2.0, OFF["tab"] + Z_FL - 8), "Tabs\n040: TE 173682-1, 070: TE 173645-1", (1060, 960), "l"),
        ((12, 16.5, OFF["sh"] + Z_SH + 18), "male_shroud (printed)\ncopy of the 176122-6 cavity", (1240, 300), "r"),
        ((8, 15.0, OFF["oem"] + Z_OEM + 10), "OEM harness plug\nTE 175111-x (26P) [VERIFY]", (1460, 430), "r"),
        ((21, 8.0, OFF["orec"] + Z_OEM + 14), "OEM receptacles and harness\n(unchanged, nothing cut)", (1700, 640), "r"),
    ]
    for i, (p, text, (tx, ty), side) in enumerate(calls, 1):
        u, v = proj(*p)
        ax.annotate("", (u, v), (tx, ty), arrowprops=dict(arrowstyle="-", color=INK, lw=0.8))
        ax.plot(u, v, "o", ms=4, color=INK)
        ax.text(tx, ty, f" {i} ", fontsize=11, weight="bold", color="white", ha="center", va="center",
                bbox=dict(boxstyle="circle,pad=0.25", fc=INK, ec="none"))
        ax.text(tx + (22 if side == "r" else -22), ty, text, fontsize=10.5, color=INK,
                ha="left" if side == "r" else "right", va="center")
    fig.text(0.98, 0.615, "The two-piece chamber that clamps both flanges is hidden here; it is shown in section 3.",
             fontsize=10, color="#566573", ha="right")

    # ---- 2. transverse section
    ax = fig.add_axes([0.04, 0.34, 0.60, 0.26])
    ax.set_title("2  Section A-A: interposer plug seated in the ECU header, cut 4 mm above the header floor",
                 loc="left", fontsize=15, weight="bold", color=INK, pad=10)
    for c, a in (("header", 1), ("plug", 1), ("receptacles", 1), ("header_pins", 1)):
        draw_rings(ax, xsec[c], COL[c], a, z={"header": 1, "plug": 2, "receptacles": 3, "header_pins": 4}[c])
    xc = lambda x: x - L / 2
    # cavity outline + overall
    dim_h(ax, -L / 2, L / 2, -4.3, f"cavity {L:.2f}", ext_from=-0.2, above=False)
    xs = [xc(x) for x, _ in COLS]
    dim_h(ax, xs[0], xs[1], -2.0, "3.5", ext_from=ROW_070[1] - 1.6, above=False)
    dim_h(ax, xs[2], xs[3], -2.0, "3.1", ext_from=ROW_070[1] - 1.6, above=False)
    dim_h(ax, xs[3], xs[4], -2.0, "3.0", ext_from=ROW_040[1] - 1.4, above=False)
    ax.text(xs[7], -2.4, "040 pitch 3.0 x 8 columns", fontsize=9, color=DIM, ha="center", va="top")
    dim_v(ax, L / 2 + WALL + 1.6, ROW_070[1], ROW_070[0], "rows 5.2", ext_from=xs[-1])
    dim_v(ax, L / 2 + WALL + 1.6, 0, ROW_070[1], "2.85", ext_from=L / 2)
    dim_v(ax, -L / 2 - WALL - 1.6, 0, CAV_H, "11.82", ext_from=-L / 2, right=False)
    dim_v(ax, -L / 2 - WALL - 4.6, 0, SLOT_TOP, "15.02\n(slots)", ext_from=-L / 2, right=False)
    dim_v(ax, xs[4] + 1.5, ROW_040[1], ROW_040[0], "5.2", fs=8)
    # call out thinnest web
    wx = (xs[1] + xs[2]) / 2
    ax.annotate("thinnest web 0.35\n(070 | 070 receptacles)", (wx, ROW_070[0]), (wx + 3, 19.3),
                fontsize=10, color=DIM, ha="center",
                arrowprops=dict(arrowstyle="->", color=DIM, lw=0.9))
    ax.annotate("key ridge in header slot\n0.20 clearance per side", (xc(17.7), 13.4), (xc(17.7) + 6, 19.3),
                fontsize=10, color=INK, ha="center", arrowprops=dict(arrowstyle="->", color=INK, lw=0.9))
    ax.annotate("polarising rail\nin plug groove", (L / 2 - 0.4, 5.6), (L / 2 + 6.5, 13.5),
                fontsize=10, color=INK, ha="center", arrowprops=dict(arrowstyle="->", color=INK, lw=0.9))
    ax.annotate("sliding fit\n0.25 per side", (-L / 2 + 0.12, 9.0), (-L / 2 - 5, 19.3), fontsize=10,
                color=INK, ha="center", arrowprops=dict(arrowstyle="->", color=INK, lw=0.9))
    ax.set_xlim(-L / 2 - 10, L / 2 + 9)
    ax.set_ylim(-6.5, 21.5)
    ax.set_aspect("equal")
    ax.axis("off")
    # legend
    for i, (c, name) in enumerate((("header", "ECU header"), ("plug", "female_plug (printed)"),
                                   ("receptacles", "receptacle (envelope)"), ("header_pins", "header tab"))):
        ax.add_patch(plt.Rectangle((-L / 2 - 9.5 + i * 11.5, -6.2), 1.2, 0.9, fc=COL[c], ec=INK, lw=0.5))
        ax.text(-L / 2 - 8 + i * 11.5, -5.75, name, fontsize=9, va="center", color=INK)

    # ---- analysis table
    w, rows, floor, top = webs()
    ax = fig.add_axes([0.66, 0.34, 0.32, 0.26])
    ax.axis("off")
    ax.set_title("Analysis", loc="left", fontsize=15, weight="bold", color=INK, pad=10)
    eng = {t: PROUD[t] - (PLUG_GAP + FACE_WALL) for t in (1, 2)}
    lines = [
        ("Fit in the header", None),
        ("Plug envelope vs cavity", f"{L - 2 * SLIDE:.2f} x {CAV_H - 2 * SLIDE:.2f} in {L:.2f} x {CAV_H:.2f}  (0.25/side)"),
        ("Key ridges vs slots", "0.20/side, unique pattern per section"),
        ("Insertion depth", f"{INSERT:.1f} of {DEPTH:.1f} (stops {PLUG_GAP:.1f} short of the floor)"),
        ("Pin engagement", None),
        ("040 tab into receptacle", f"{eng[1]:.2f}  (tab {PROUD[1]:.2f} proud - gap 0.2 - face wall 1.0)"),
        ("070 tab into receptacle", f"{eng[2]:.2f}  (tab {PROUD[2]:.2f} proud)"),
        ("Shroud side", "identical by construction (same tab heights)"),
        ("Walls in the printed plug", None),
        ("Between columns", "  ".join(f"{k}: {v:.2f}" for k, v in w.items())),
        ("Between rows", f"040: {rows[1]:.2f}   070: {rows[2]:.2f}"),
        ("Under lower row (incl. lance)", f"040: {floor[1]:+.2f}   070: {floor[2]:+.2f}"),
        ("Over upper row (incl. lance)", f"040: {top[1]:.2f}   070: {top[2]:.2f}"),
        ("Verdict", None),
        ("", "Fits and engages as designed. Walls under 0.6 mm need resin or MJF."),
        ("", "Lower 040 lance reliefs break 0.1 mm through the underside: they"),
        ("", "become open windows, which is harmless but visible."),
        ("", "Not checked yet: latch retention and terminal body sizes [MEASURE]."),
    ]
    y = 0.97
    for k, v in lines:
        if v is None:
            y -= 0.012
            ax.text(0, y, k, fontsize=12, weight="bold", color=INK, transform=ax.transAxes, va="top")
        else:
            ax.text(0.0, y, k, fontsize=10.5, color="#566573", transform=ax.transAxes, va="top")
            ax.text(0.40 if k else 0.0, y, v, fontsize=10.5, color=INK, transform=ax.transAxes, va="top")
        y -= 0.052

    # ---- 3. lengthwise sections
    for j, ((x, label), d) in enumerate(zip(lcols, lsec)):
        ax = fig.add_axes([0.04, 0.17 - j * 0.15, 0.94, 0.14])
        if j == 0:
            ax.set_title("3  Section B-B: lengthwise through one pin column, assembled (ECU on the left, "
                         "harness on the right)", loc="left", fontsize=15, weight="bold", color=INK, pad=24)
        order = ["chamber", "header", "plug", "shroud", "oem_plug", "receptacles", "oem_receptacles",
                 "wires", "tabs", "header_pins"]
        for z, c in enumerate(order):
            draw_rings(ax, d[c], COL[c], 0.55 if c in ("chamber", "oem_plug") else 1.0, z=z + 1)
        t = 2 if j == 0 else 1
        ry = (ROW_070 if t == 2 else ROW_040)[0]
        top_y = SLOT_TOP + WALL + 2.2
        ax.text(-17, top_y + 5.5, label, fontsize=12, weight="bold", color=INK)
        # ECU side engagement
        dim_h(ax, 0, PROUD[t], top_y + 1.2, f"tab {PROUD[t]:.2f}", ext_from=ry)
        dim_h(ax, PLUG_GAP + FACE_WALL, PROUD[t], top_y + 4.0, f"engaged {PROUD[t] - 1.2:.2f}",
              ext_from=ry)
        dim_h(ax, 0, INSERT + PLUG_GAP, -4.2, f"insert {INSERT:.1f} / depth {DEPTH:.1f}", ext_from=0, above=False)
        dim_h(ax, PLUG_GAP + PLUG_LEN, Z_SH, -4.2, "chamber: pigtails + tap splices", ext_from=0, above=False)
        dim_h(ax, Z_SH, Z_FL, -4.2, "floor 9.0", ext_from=0, above=False)
        dim_h(ax, Z_FL, Z_FL + PROUD[t], top_y + 1.2, f"tab {PROUD[t]:.2f}", ext_from=ry)
        ax.text(-12, -1.0, "ECU\nboard", fontsize=9, color="#566573", ha="right", va="center")
        ax.text(Z_OEM + PLUG_LEN + 8, ry - 2.6, "to OEM\nharness", fontsize=9, color="#566573", va="center")
        ax.set_xlim(-18, Z_OEM + PLUG_LEN + 30)
        ax.set_ylim(-8, top_y + 7.5)
        ax.set_aspect("equal")
        ax.axis("off")

    fig.text(0.04, 0.012, "Generated by hardware/ecu_interposer/diagram/make_diagram.py from cad/diagram.scad. "
             "Sections are true scale.", fontsize=10, color="#566573")
    out = HERE / "connector_diagram"
    fig.savefig(out.with_suffix(".png"), dpi=110)
    fig.savefig(out.with_suffix(".pdf"))
    print("wrote", out.with_suffix(".png"), out.with_suffix(".pdf"))


if __name__ == "__main__":
    main()
