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


def courtyard(fp, margin=0.3):
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
    board.SetCopperLayerCount(4)
    ds = board.GetDesignSettings()
    ds.SetCopperLayerCount(4)
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
        r = courtyard(fps[ref], 0)
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
    return placed


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
    path = os.path.join(OUT, "junkr_ecu_unrouted.kicad_pcb")
    board.Save(path)
    print("saved", path, len(fps), "footprints")


if __name__ == "__main__":
    main()
