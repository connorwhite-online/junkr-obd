import numpy as np, matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle, Polygon, FancyArrowPatch, Circle
render = shade = rot = load = None   # 3D line-art helpers not needed for these sheets
plt.rcParams["font.family"] = "DejaVu Sans"
INK = "#111111"; THIN = 0.35; MED = 0.6
FRONT = np.array([[-1, 0, 0], [0, 1, 0], [0, 0, -1]], float)
TOP   = np.array([[-1, 0, 0], [0, 0, 1], [0, 1, 0]], float)
RIGHT = np.array([[0, 0, 1], [0, 1, 0], [-1, 0, 0]], float)
BACK  = np.array([[1, 0, 0], [0, 1, 0], [0, 0, 1]], float)
BOTTOM = np.array([[-1, 0, 0], [0, 0, -1], [0, -1, 0]], float)
DATE = "2026-10-23"; TOTAL = 10

class Sheet:
    def __init__(self, num, title, subtitle="", scale="AS SHOWN", material="SEE BOM", process="", rev="A", sheet=1):
        self.fig = plt.figure(figsize=(420 / 25.4, 297 / 25.4))
        ax = self.ax = self.fig.add_axes([0, 0, 1, 1]); ax.set_xlim(0, 420); ax.set_ylim(0, 297); ax.axis("off")
        ax.add_patch(Rectangle((10, 10), 400, 277, fill=False, lw=1.2, ec=INK))
        ax.add_patch(Rectangle((5, 5), 410, 287, fill=False, lw=0.3, ec=INK))
        # zone marks
        for i in range(8):
            x = 10 + 50 * i + 25
            for y in (7.5, 289.5): ax.text(x, y, str(i + 1), ha="center", va="center", fontsize=6, color=INK)
        for j, L in enumerate("ABCDEF"):
            y = 287 - (277 / 6) * j - 277 / 12
            for x in (7.5, 412.5): ax.text(x, y, L, ha="center", va="center", fontsize=6, color=INK)
        # title block 180 x 40 at bottom right
        x0, y0, w, h = 230, 10, 180, 42
        ax.add_patch(Rectangle((x0, y0), w, h, fill=True, fc="white", lw=1.0, ec=INK, zorder=5))
        def cell(x, y, cw, ch, label, val, fs=8, bold=False):
            ax.add_patch(Rectangle((x, y), cw, ch, fill=False, lw=0.5, ec=INK, zorder=6))
            ax.text(x + 1.2, y + ch - 1.2, label, fontsize=4.6, va="top", color="#444", zorder=7)
            ax.text(x + 1.5, y + 1.6, val, fontsize=fs, va="bottom", weight="bold" if bold else "normal", color=INK, zorder=7)
        cell(x0, y0 + 28, 180, 14, "PROJECT", "KZJ95 1KZ-TE  ECU INTERPOSER + TAP  (piggyback, zero-cut)", 8.5, True)
        cell(x0, y0 + 14, 180, 14, "TITLE", title, 10, True)
        if subtitle: ax.text(x0 + 178, y0 + 15.5, subtitle, fontsize=6, ha="right", va="bottom", color="#333", zorder=7)
        cell(x0, y0, 34, 14, "DWG NO.", num, 9, True)
        cell(x0 + 34, y0, 12, 14, "REV", rev, 9, True)
        cell(x0 + 46, y0, 22, 14, "SCALE", scale, 7)
        cell(x0 + 68, y0, 44, 14, "MATERIAL", material, 6.2)
        cell(x0 + 112, y0, 34, 14, "DRAWN / DATE", "Claude  " + DATE, 6.2)
        cell(x0 + 146, y0, 34, 14, "SHEET", f"{sheet} OF {TOTAL}", 7)
        # projection + general notes block
        ax.add_patch(Rectangle((x0 - 62, y0), 62, 20, fill=True, fc="white", lw=0.8, ec=INK, zorder=5))
        ax.text(x0 - 61, y0 + 18.8, "UNITS mm   GENERAL TOL ±0.2\nUNLESS NOTED.  BREAK SHARP EDGES.\nDIMS FROM params.scad / sections.scad",
                fontsize=4.6, va="top", zorder=7)
        self.third_angle(x0 - 12, y0 + 5.5)
        if process:
            ax.add_patch(Rectangle((x0 - 62, y0 + 20), 62, 22, fill=True, fc="white", lw=0.8, ec=INK, zorder=5))
            ax.text(x0 - 61, y0 + 40.8, "PROCESS", fontsize=4.6, va="top", color="#444", zorder=7)
            ax.text(x0 - 61, y0 + 37, process, fontsize=5.2, va="top", zorder=7, linespacing=1.35)

    def third_angle(self, cx, cy):
        ax = self.ax
        ax.add_patch(Polygon([(cx - 9, cy - 2.5), (cx - 9, cy + 2.5), (cx - 3, cy + 4), (cx - 3, cy - 4)], closed=True, fill=False, lw=0.5, zorder=7))
        ax.add_patch(Circle((cx + 4, cy), 4, fill=False, lw=0.5, zorder=7)); ax.add_patch(Circle((cx + 4, cy), 1.8, fill=False, lw=0.5, zorder=7))
        ax.plot([cx - 11, cx + 9.5], [cy, cy], lw=0.3, ls=(0, (6, 1.5, 1, 1.5)), color=INK, zorder=7)

    def note(self, x, y, text, fs=6.5, **kw):
        self.ax.text(x, y, text, fontsize=fs, va="top", color=INK, linespacing=1.4, **kw)

    def label(self, x, y, text, fs=7.5, **kw):
        self.ax.text(x, y, text, fontsize=fs, weight="bold", color=INK, **kw)

    def save(self, path, dpi=200):
        self.fig.savefig(path, dpi=dpi, facecolor="white"); plt.close(self.fig)

class View:
    """A rendered projection placed on a sheet. (cx, cy) = paper position of the model point `anchor`."""
    def __init__(self, sh, meshes, R, scale, cx, cy, anchor=None, ppm=None, shaded=True, crease=28, title=None, lw=2, tgap=14, bounds=None):
        self.sh, self.R, self.k = sh, np.asarray(R, float), scale
        ppm = ppm or max(8, int(scale * 7.9))
        e, ext, fg, nb = render(meshes, self.R, ppm, crease=crease, lw=lw, bounds=bounds)
        img = (shade(nb, fg) if shaded else np.ones_like(e)) * (1 - e)
        allv = np.vstack([m.vertices for m in meshes])
        a = (allv.min(0) + allv.max(0)) / 2 if anchor is None else np.asarray(anchor, float)
        av = self.R @ a
        self.ox, self.oy = cx - av[0] * scale, cy - av[1] * scale
        x0, x1, y0, y1 = ext
        rgba = np.dstack([img, img, img, np.where((img < 0.999), 1.0, 0.0)])
        sh.ax.imshow(rgba, extent=[self.ox + x0 * scale, self.ox + x1 * scale, self.oy + y0 * scale, self.oy + y1 * scale],
                     interpolation="lanczos", zorder=2)
        self.bb = (self.ox + x0 * scale, self.ox + x1 * scale, self.oy + y0 * scale, self.oy + y1 * scale)
        if title:
            sh.ax.text((self.bb[0] + self.bb[1]) / 2, self.bb[2] - tgap, title, ha="center", va="top", fontsize=7.5, weight="bold")

    def P(self, p):
        """model 3D point -> paper"""
        v = self.R @ np.asarray(p, float)
        return np.array([self.ox + v[0] * self.k, self.oy + v[1] * self.k])

    def dim(self, p1, p2, off, text=None, horiz=None, fs=6.2):
        """linear dimension between model points p1,p2; off = paper mm offset perpendicular (signed)."""
        a, b = self.P(p1), self.P(p2)
        if horiz is None: horiz = abs(b[0] - a[0]) >= abs(b[1] - a[1])
        ax = self.sh.ax
        if horiz:
            y = (max(a[1], b[1]) if off > 0 else min(a[1], b[1])) + off
            for p in (a, b): ax.plot([p[0], p[0]], [p[1] + np.sign(off) * 0.8, y + np.sign(off) * 1.2], lw=THIN, color=INK, zorder=4)
            ax.annotate("", (a[0], y), (b[0], y), arrowprops=dict(arrowstyle="<|-|>", lw=THIN, color=INK, shrinkA=0, shrinkB=0, mutation_scale=5), zorder=4)
            val = abs(b[0] - a[0]) / self.k
            ax.text((a[0] + b[0]) / 2, y + 0.6, text or f"{val:.2f}".rstrip("0").rstrip("."), ha="center", va="bottom", fontsize=fs, zorder=4,
                    bbox=dict(fc="white", ec="none", pad=0.1))
        else:
            x = (max(a[0], b[0]) if off > 0 else min(a[0], b[0])) + off
            for p in (a, b): ax.plot([p[0] + np.sign(off) * 0.8, x + np.sign(off) * 1.2], [p[1], p[1]], lw=THIN, color=INK, zorder=4)
            ax.annotate("", (x, a[1]), (x, b[1]), arrowprops=dict(arrowstyle="<|-|>", lw=THIN, color=INK, shrinkA=0, shrinkB=0, mutation_scale=5), zorder=4)
            val = abs(b[1] - a[1]) / self.k
            ax.text(x - 0.6, (a[1] + b[1]) / 2, text or f"{val:.2f}".rstrip("0").rstrip("."), ha="center", va="bottom", rotation=90, fontsize=fs, zorder=4,
                    bbox=dict(fc="white", ec="none", pad=0.1))

    def leader(self, p, dx, dy, text, fs=6, balloon=False):
        a = self.P(p); b = a + np.array([dx, dy]); ax = self.sh.ax
        ax.plot([a[0], b[0]], [a[1], b[1]], lw=THIN, color=INK, zorder=4)
        ax.plot(*a, marker="o", ms=1.6, color=INK, zorder=4)
        if balloon:
            ax.add_patch(Circle(b, 3.6, fc="white", ec=INK, lw=0.6, zorder=5))
            ax.text(*b, text, ha="center", va="center", fontsize=7.5, weight="bold", zorder=6)
        else:
            ax.plot([b[0], b[0] + (6 if dx >= 0 else -6)], [b[1], b[1]], lw=THIN, color=INK, zorder=4)
            ax.text(b[0] + (7 if dx >= 0 else -7), b[1], text, ha="left" if dx >= 0 else "right", va="center", fontsize=fs, zorder=4, linespacing=1.3)

    def cl(self, p1, p2, ext=3):
        a, b = self.P(p1), self.P(p2); d = (b - a) / (np.linalg.norm(b - a) + 1e-9)
        a, b = a - d * ext, b + d * ext
        self.sh.ax.plot([a[0], b[0]], [a[1], b[1]], lw=0.3, color=INK, ls=(0, (10, 2, 2, 2)), zorder=4)

    def cutline(self, p1, p2, name, ext=4, arrow=(0, 1)):
        a, b = self.P(p1), self.P(p2); d = (b - a) / np.linalg.norm(b - a)
        a, b = a - d * ext, b + d * ext
        ax = self.sh.ax
        ax.plot([a[0], b[0]], [a[1], b[1]], lw=0.5, color=INK, ls=(0, (12, 2, 2, 2)), zorder=4)
        for p in (a, b):
            ax.plot([p[0], p[0] - d[0] * 4], [p[1], p[1] - d[1] * 4] if p is a else [p[1], p[1] + d[1] * 4], lw=1.4, color=INK, zorder=4)
            q = p + np.array(arrow) * 6
            ax.annotate("", q, p, arrowprops=dict(arrowstyle="-|>", lw=0.8, color=INK, mutation_scale=8), zorder=4)
            ax.text(q[0] + arrow[0] * 2.5, q[1] + arrow[1] * 2.5, name, ha="center", va="center", fontsize=9, weight="bold", zorder=4)

def place(sh, path, x, y, w=None, h=None, anchor="nw"):
    from PIL import Image
    im = np.asarray(Image.open(path).convert("RGBA")) / 255.0
    ih, iw = im.shape[:2]
    if w is None: w = h * iw / ih
    if h is None: h = w * ih / iw
    x0 = x if "w" in anchor else x - w
    y1 = y if "n" in anchor else y + h
    sh.ax.imshow(im, extent=[x0, x0 + w, y1 - h, y1], interpolation="lanczos", zorder=3)
    return x0, y1 - h, w, h

def block(sh, x, y, w, h, title, body="", fs=6.5, bold=True, dash=False, fc="white"):
    sh.ax.add_patch(Rectangle((x, y), w, h, fill=True, fc=fc, ec=INK, lw=0.8, ls="--" if dash else "-", zorder=3))
    sh.ax.text(x + w / 2, y + h - 2.2, title, ha="center", va="top", fontsize=fs + 0.5, weight="bold" if bold else "normal", zorder=4)
    if body: sh.ax.text(x + w / 2, y + h - 6.5, body, ha="center", va="top", fontsize=fs - 0.6, zorder=4, linespacing=1.3)
    return (x, y, w, h)

def arrow(sh, p, q, text="", both=False, lw=0.8, fs=5.6, ls="-", off=(0, 1.2), ha="center"):
    sh.ax.annotate("", q, p, arrowprops=dict(arrowstyle="<|-|>" if both else "-|>", lw=lw, color=INK, ls=ls, mutation_scale=7, shrinkA=0, shrinkB=0), zorder=4)
    if text: sh.ax.text((p[0] + q[0]) / 2 + off[0], (p[1] + q[1]) / 2 + off[1], text, ha=ha, va="bottom", fontsize=fs, zorder=5,
                        bbox=dict(fc="white", ec="none", pad=0.3))

def line(sh, pts, lw=0.8, ls="-"):
    pts = np.array(pts); sh.ax.plot(pts[:, 0], pts[:, 1], lw=lw, ls=ls, color=INK, zorder=4)
