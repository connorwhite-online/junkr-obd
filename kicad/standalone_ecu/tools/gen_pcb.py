#!/usr/bin/env python3
"""Build the unrouted board from circuit.py + placement.py (KiCad 7 pcbnew).

    /usr/bin/python3 gen_pcb.py   ->  ../junkr_ecu_unrouted.kicad_pcb

route.py then routes it. Footprints come from the stock KiCad libraries,
except the ECU header and the tails (lib/junkr_ecu.pretty).
"""
import math
import os
import sys

import pcbnew as P

import circuit as CIR
import placement as PL
from gen_sch import uid

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..")
FPLIB = "/usr/share/kicad/footprints"


def mm(v):
    return P.FromMM(v)


def V(x, y):
    return P.VECTOR2I(mm(x), mm(y))


_fpcache = {}


def load_fp(fpid):
    lib, name = fpid.split(":")
    path = os.path.join(OUT, "lib", "junkr_ecu.pretty") if lib == "junkr_ecu" else os.path.join(FPLIB, lib + ".pretty")
    fp = P.FootprintLoad(path, name)
    if fp is None:
        raise SystemExit("missing footprint " + fpid)
    fp.SetFPID(P.LIB_ID(lib, name))
    return fp


MARGIN = float(os.environ.get("PLACE_MARGIN", "0.9"))   # routing room around each part


def courtyard(fp, margin=None):
    margin = MARGIN if margin is None else margin
    fp.BuildCourtyardCaches()
    cy = fp.GetCourtyard(P.F_CrtYd)
    if cy.OutlineCount():
        bb = cy.BBox()
    else:
        bb = fp.GetBoundingBox(False, False)
    x0, y0 = P.ToMM(bb.GetX()), P.ToMM(bb.GetY())
    return (x0 - margin, y0 - margin, x0 + P.ToMM(bb.GetWidth()) + margin, y0 + P.ToMM(bb.GetHeight()) + margin)


class Occ:
    """Coarse occupancy of courtyards (0.25 mm cells) for placement."""

    def __init__(self, w, h, g=0.25):
        import numpy as np
        self.g = g
        self.a = np.zeros((int(h / g) + 2, int(w / g) + 2), dtype=bool)

    def _idx(self, r):
        g = self.g
        return (max(0, int(math.floor(r[1] / g))), max(0, int(math.floor(r[0] / g))),
                int(math.ceil(r[3] / g)), int(math.ceil(r[2] / g)))

    def free(self, r, region=None):
        if r[0] < 0.5 or r[1] < 0.5 or r[2] > PL.W - 0.5 or r[3] > PL.H - 0.5:
            return False
        if region and (r[0] < region[0] or r[1] < region[1] or r[2] > region[2] or r[3] > region[3]):
            return False
        y0, x0, y1, x1 = self._idx(r)
        return not self.a[y0:y1, x0:x1].any()

    def take(self, r):
        y0, x0, y1, x1 = self._idx(r)
        self.a[y0:y1, x0:x1] = True


POWERISH = {"GND", "3V3", "3V3A", "3V3_RF", "V5", "V5_OR", "VIN", "PB", "VREF", "VC", "VRLY", "VBUS"}


def build():
    board = P.BOARD()
    board.SetCopperLayerCount(6)
    ds = board.GetDesignSettings()
    ds.SetCopperLayerCount(6)
    nets = {}

    def net(name):
        if name not in nets:
            ni = P.NETINFO_ITEM(board, name)
            board.Add(ni)
            nets[name] = ni
        return nets[name]

    for n in sorted(CIR.c.nets):
        net(n)

    fps = {}
    sheet_uuid = {}
    for p in CIR.c.parts:
        fp = load_fp(p.fp)
        fp.SetReference(p.ref)
        fp.SetValue(p.value)
        for k, v in p.fields.items():
            fp.SetProperty(k, str(v))
        fp.SetPath(P.KIID_PATH("/%s/%s" % (uid("sheet", p.sheet), uid("sym", p.ref, 1))))
        conns = CIR.c.conns[p.ref]
        for pad in fp.Pads():
            n = pad.GetNumber()
            if n in conns:
                pad.SetNet(net(conns[n]))
        fps[p.ref] = fp
    return board, fps, nets


def place(board, fps):
    occ = Occ(PL.W, PL.H)
    placed = {}

    def put(ref, x, y, rot):
        fp = fps[ref]
        fp.SetOrientationDegrees(rot)
        fp.SetPosition(V(x, y))
        r = courtyard(fp)
        placed[ref] = r
        occ.take(r)

    for ref, (x, y, rot) in list(PL.ANCHORS.items()) + list(PL.FIXED.items()):
        put(ref, x, y, rot)
    # keep the ESP32 antenna keep-out free of parts
    occ.take((0, 0, 44, 14))
    # mounting holes (M3) in the corners
    for (x, y) in PL.HOLES:
        occ.take((x - 4, y - 4, x + 4, y + 4))

    refs_sheet = {p.ref: p.sheet for p in CIR.c.parts}
    # floating parts: biggest first, then in circuit order
    floating = [p.ref for p in CIR.c.parts if p.ref not in placed]

    def area(ref):
        r = courtyard(fps[ref], 0.0)
        return (r[2] - r[0]) * (r[3] - r[1])

    floating.sort(key=lambda r: -area(r))
    local = {}

    def rect_at(ref, x, y, rot):
        if (ref, rot) not in local:
            fp = fps[ref]
            fp.SetOrientationDegrees(rot)
            fp.SetPosition(V(0, 0))
            local[(ref, rot)] = courtyard(fp)
        r = local[(ref, rot)]
        return (r[0] + x, r[1] + y, r[2] + x, r[3] + y)
    # net -> placed pad positions
    def pad_pos(ref):
        fp = fps[ref]
        return [(pad.GetNetname(), P.ToMM(pad.GetPosition().x), P.ToMM(pad.GetPosition().y)) for pad in fp.Pads()]

    netpos = {}
    for ref in placed:
        for n, x, y in pad_pos(ref):
            if n and n not in POWERISH:
                netpos.setdefault(n, []).append((x, y))

    # second pass ordering: parts with more placed neighbours first
    pending = list(floating)
    while pending:
        def score(ref):
            nn = [n for n in CIR.c.conns[ref].values() if n not in POWERISH and n in netpos]
            return -len(nn)
        pending.sort(key=lambda r: (score(r), -area(r)))
        ref = pending.pop(0)
        fp = fps[ref]
        region = PL.REGION.get(refs_sheet[ref])
        pts = []
        for n in CIR.c.conns[ref].values():
            if n in netpos and n not in POWERISH:
                pts += netpos[n]
        if not pts:
            # decoupling caps etc.: go near the IC sharing the rail on this sheet
            rails = [n for n in CIR.c.conns[ref].values() if n != "GND"]
            for other, r in placed.items():
                if refs_sheet.get(other) == refs_sheet[ref] and other[0] == "U":
                    if any(n in CIR.c.conns[other].values() for n in rails):
                        pts.append(((r[0] + r[2]) / 2, (r[1] + r[3]) / 2))
        if pts:
            tx = sum(p[0] for p in pts) / len(pts)
            ty = sum(p[1] for p in pts) / len(pts)
        else:
            tx, ty = ((region[0] + region[2]) / 2, (region[1] + region[3]) / 2) if region else (PL.W / 2, PL.H / 2)
        if region:
            tx = min(max(tx, region[0] + 2), region[2] - 2)
            ty = min(max(ty, region[1] + 2), region[3] - 2)
        best = None
        for reg in (region, None):
            for radius in [x * 0.5 for x in range(0, 160)]:
                n = max(1, int(2 * math.pi * radius / 0.75))
                for k in range(n):
                    a = 2 * math.pi * k / n
                    x, y = tx + radius * math.cos(a), ty + radius * math.sin(a)
                    for rot in (0, 90):
                        r = rect_at(ref, round(x * 4) / 4, round(y * 4) / 4, rot)
                        if occ.free(r, reg):
                            best = (round(x * 4) / 4, round(y * 4) / 4, rot)
                            break
                    if best:
                        break
                if best:
                    break
            if best:
                break
        if not best:
            raise SystemExit("no room for " + ref)
        put(ref, *best)
        for n, x, y in pad_pos(ref):
            if n and n not in POWERISH:
                netpos.setdefault(n, []).append((x, y))
    for ref, (x, y, rot) in getattr(PL, "MOVES", {}).items():
        fps[ref].SetOrientationDegrees(rot)
        fps[ref].SetPosition(V(x, y))
    snap_decouplers(fps, "U100", ("3V3", "VCAP"))
    return placed


def snap_decouplers(fps, ic, nets):
    """Each supply pin of the MCU gets its own decoupling cap, moved to the
    nearest free spot to that pin. The placer works by sheet and region and had
    left them 40 mm away, which is useless for decoupling and floods the board
    with supply tracks. Everything else stays where it is."""
    import numpy as np
    G = 0.25
    sheet = {p.ref: p.sheet for p in CIR.c.parts}
    u = fps[ic]

    def grid(skip):
        occ = np.zeros((int(PL.H / G) + 2, int(PL.W / G) + 2), dtype=bool)
        for ref, fp in fps.items():
            if ref in skip:
                continue
            x0, y0, x1, y1 = courtyard(fp, 0.25)
            occ[max(0, int(y0 / G)):int(y1 / G) + 1, max(0, int(x0 / G)):int(x1 / G) + 1] = True
        for (hx, hy) in PL.HOLES:
            occ[int((hy - 4) / G):int((hy + 4) / G), max(0, int((hx - 4) / G)):int((hx + 4) / G)] = True
        return occ

    pins = [p for p in u.Pads() if p.GetNetname() in nets]
    caps = {}
    for ref, fp in fps.items():
        ns = sorted(p.GetNetname() for p in fp.Pads())
        if ref.startswith("C") and sheet.get(ref) == sheet[ic] and len(ns) == 2 and "GND" in ns:
            n = [x for x in ns if x != "GND"][0]
            if n in nets:
                caps.setdefault(n, []).append(ref)
    for n in caps:   # small caps first: 100n at the pins
        caps[n].sort(key=lambda r: (fps[r].GetValue() != "100n", r))
    moved = []
    for pin in pins:
        pool = caps.get(pin.GetNetname(), [])
        if not pool:
            continue
        ref = pool.pop(0)
        fp = fps[ref]
        occ = grid({ref})
        px, py = P.ToMM(pin.GetPosition().x), P.ToMM(pin.GetPosition().y)
        best = None
        for rot in (0, 90):
            fp.SetOrientationDegrees(rot)
            fp.SetPosition(V(0, 0))
            cx0, cy0, cx1, cy1 = courtyard(fp, 0.25)
            for r10 in range(0, 80):
                rad = r10 * 0.25
                n = max(1, int(2 * math.pi * rad / 0.25))
                for k in range(n):
                    a = 2 * math.pi * k / n
                    x, y = round((px + rad * math.cos(a)) * 4) / 4, round((py + rad * math.sin(a)) * 4) / 4
                    x0, y0, x1, y1 = cx0 + x, cy0 + y, cx1 + x, cy1 + y
                    if x0 < 1 or y0 < 1 or x1 > PL.W - 1 or y1 > PL.H - 1:
                        continue
                    if not occ[int(y0 / G):int(y1 / G) + 1, int(x0 / G):int(x1 / G) + 1].any():
                        d = math.hypot(x - px, y - py)
                        if best is None or d < best[0]:
                            best = (d, x, y, rot)
                        break
                if best and best[0] <= rad:
                    break
        if best and best[0] < 12:
            fp.SetOrientationDegrees(best[3])
            fp.SetPosition(V(best[1], best[2]))
            moved.append("%s->%s.%s %.1fmm" % (ref, ic, pin.GetNumber(), best[0]))
        else:
            pool.insert(0, ref)
    print("decouplers:", ", ".join(moved))


def outline(board):
    pts = [(0, 0), (PL.W, 0), (PL.W, PL.H), (0, PL.H)]
    for i in range(4):
        s = P.PCB_SHAPE(board)
        s.SetShape(P.SHAPE_T_SEGMENT)
        s.SetStart(V(*pts[i]))
        s.SetEnd(V(*pts[(i + 1) % 4]))
        s.SetLayer(P.Edge_Cuts)
        s.SetWidth(mm(0.1))
        board.Add(s)
    for (x, y) in PL.HOLES:
        fp = load_fp("MountingHole:MountingHole_3.2mm_M3")
        fp.SetReference("H%d" % (len([1 for f in board.GetFootprints() if f.GetReference().startswith("H")]) + 1))
        fp.SetPosition(V(x, y))
        board.Add(fp)


def silk(board):
    def text(s, x, y, size=1.5, layer=P.F_SilkS):
        t = P.PCB_TEXT(board)
        t.SetText(s)
        t.SetPosition(V(x, y))
        t.SetLayer(layer)
        t.SetTextSize(P.VECTOR2I(mm(size), mm(size)))
        t.SetTextThickness(mm(size * 0.15))
        board.Add(t)
    text("JUNKR ECU rev A  -  1KZ-TE ECD-V3 standalone", 100, 136.5, 1.6)
    text("HARNESS PLUGS", 100, 33.6, 1.0)


def main():
    board, fps, nets = build()
    for ref, fp in fps.items():
        board.Add(fp)
    place(board, fps)
    outline(board)
    silk(board)
    path = os.environ.get("PLACE_OUT", os.path.join(OUT, "junkr_ecu_unrouted.kicad_pcb"))
    board.Save(path)
    print("saved", path, len(fps), "footprints")


if __name__ == "__main__":
    main()
