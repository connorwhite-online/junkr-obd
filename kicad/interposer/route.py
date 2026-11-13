#!/usr/bin/env python3
"""Route the tap nets of the interposer board and pour TAP_GND.

A small two-layer grid maze router (A*, 8-way, vias allowed). The 64
pass-throughs are already drawn (locked) by gen_board.py; this only adds
the 34 short tap / ADC connections, then the ground pours, and writes
interposer.kicad_pcb.

    python3 gen_board.py && python3 route.py
"""
import heapq
import json
import math
import os

import numpy as np
import pcbnew as P

import gen_board as G

HERE = os.path.dirname(os.path.abspath(__file__))
STEP = 0.1                     # grid, mm
CLR = 0.22                     # copper clearance used while routing (DRC rule is 0.2)
TW = 0.25                      # tap track width
VIA_D, VIA_DRILL = 0.6, 0.3
VIA_COST = 30.0
LAYERS = (P.F_Cu, P.B_Cu)

x0, y0, x1, y1 = G.BOARD
NX = int(round((x1 - x0) / STEP)) + 1
NY = int(round((y1 - y0) / STEP)) + 1
gx = lambda x: int(round((x - x0) / STEP))
gy = lambda y: int(round((y - y0) / STEP))
wx = lambda i: x0 + i * STEP
wy = lambda j: y0 + j * STEP

FREE, BLOCK = 0, -1


def mm(v):
    return P.ToMM(v)


def fr(pt):
    return mm(pt.x) - G.OX, mm(pt.y) - G.OY


class Grid:
    def __init__(self):
        # owner net code per cell, per layer, for track centres (inflate = CLR + TW/2)
        self.trk = np.zeros((2, NX, NY), dtype=np.int32)
        # same for via centres (inflate = CLR + VIA_D/2)
        self.via = np.zeros((2, NX, NY), dtype=np.int32)
        X = x0 + np.arange(NX) * STEP
        Y = y0 + np.arange(NY) * STEP
        self.X, self.Y = np.meshgrid(X, Y, indexing="ij")
        e = 0.3 + TW / 2
        edge = (self.X < x0 + e) | (self.X > x1 - e) | (self.Y < y0 + e) | (self.Y > y1 - e)
        ev = (self.X < x0 + 0.3 + VIA_D / 2) | (self.X > x1 - 0.3 - VIA_D / 2) | \
             (self.Y < y0 + 0.3 + VIA_D / 2) | (self.Y > y1 - 0.3 - VIA_D / 2)
        cx0, cy0, cx1, cy1 = G.CUT          # the notch where the harness plugs go
        for arr, e_ in ((self.trk, 0.3 + TW / 2), (self.via, 0.3 + VIA_D / 2)):
            m = (self.X > cx0 - e_) & (self.Y < cy1 + e_)
            arr[:, m] = BLOCK
        self.trk[:, edge] = BLOCK
        self.via[:, ev] = BLOCK

    def _mark(self, arr, layer, mask, code):
        a = arr[layer]
        cur = a[mask]
        new = np.where((cur == FREE) | (cur == code), code, BLOCK)
        a[mask] = new

    def shape(self, layer, kind, cx, cy, sx, sy, code, extra=0.0):
        """kind 'circ' (sx = diameter) or 'rect' (sx, sy full size)."""
        for arr, inf in ((self.trk, CLR + TW / 2), (self.via, CLR + VIA_D / 2)):
            inf += extra
            if kind == "circ":
                r = sx / 2 + inf
                m = (self.X - cx) ** 2 + (self.Y - cy) ** 2 <= r * r
            else:
                # rounded-rect Minkowski sum
                dx = np.maximum(np.abs(self.X - cx) - sx / 2, 0)
                dy = np.maximum(np.abs(self.Y - cy) - sy / 2, 0)
                m = dx * dx + dy * dy <= inf * inf
            self._mark(arr, layer, m, code)

    def segment(self, layer, ax, ay, bx, by, w, code):
        for arr, inf in ((self.trk, CLR + TW / 2), (self.via, CLR + VIA_D / 2)):
            r = w / 2 + inf
            vx, vy = bx - ax, by - ay
            L2 = vx * vx + vy * vy or 1e-9
            t = np.clip(((self.X - ax) * vx + (self.Y - ay) * vy) / L2, 0, 1)
            px, py = ax + t * vx, ay + t * vy
            m = (self.X - px) ** 2 + (self.Y - py) ** 2 <= r * r
            self._mark(arr, layer, m, code)


def load_obstacles(b, grid):
    for pad in b.GetPads():
        cx, cy = fr(pad.GetPosition())
        sx, sy = mm(pad.GetSize().x), mm(pad.GetSize().y)
        code = pad.GetNetCode() or BLOCK
        kind = "circ" if pad.GetShape() == P.PAD_SHAPE_CIRCLE else "rect"
        if pad.GetAttribute() == P.PAD_ATTRIB_NPTH:
            for li in (0, 1):
                grid.shape(li, "circ", cx, cy, sx, sx, BLOCK, 0.05)
            continue
        for li, L in enumerate(LAYERS):
            if pad.IsOnLayer(L):
                grid.shape(li, kind, cx, cy, sx, sy, code)
        if pad.GetAttribute() == P.PAD_ATTRIB_PTH:      # the hole blocks vias everywhere
            d = mm(pad.GetDrillSize().x)
            for li in (0, 1):
                m = (grid.X - cx) ** 2 + (grid.Y - cy) ** 2 <= (d / 2 + 0.25 + VIA_D / 2) ** 2
                grid._mark(grid.via, li, m, BLOCK)
    for t in b.GetTracks():
        if t.GetClass() == "PCB_VIA":
            vx, vy = fr(t.GetPosition())
            for li in (0, 1):
                grid.shape(li, "circ", vx, vy, mm(t.GetWidth()), 0, t.GetNetCode())
            continue
        if t.GetClass() != "PCB_TRACK":
            continue
        li = 0 if t.GetLayer() == P.F_Cu else 1
        ax, ay = fr(t.GetStart())
        bx, by = fr(t.GetEnd())
        grid.segment(li, ax, ay, bx, by, mm(t.GetWidth()), t.GetNetCode())


def astar(grid, code, starts, goals):
    """starts/goals: sets of (layer, i, j). Returns list of (layer, i, j)."""
    goal_set = set(goals)
    gi = [g[1] for g in goals]
    gj = [g[2] for g in goals]
    gcx, gcy = sum(gi) / len(gi), sum(gj) / len(gj)
    h = lambda i, j: math.hypot(i - gcx, j - gcy) * 0.95
    dist = {}
    prev = {}
    pq = []
    for s in starts:
        dist[s] = 0.0
        heapq.heappush(pq, (h(s[1], s[2]), 0.0, s))
    nbr = [(1, 0, 1.0), (-1, 0, 1.0), (0, 1, 1.0), (0, -1, 1.0),
           (1, 1, 1.4142), (1, -1, 1.4142), (-1, 1, 1.4142), (-1, -1, 1.4142)]
    trk, via = grid.trk, grid.via
    while pq:
        f, d, node = heapq.heappop(pq)
        if d > dist.get(node, 1e18):
            continue
        if node in goal_set:
            path = [node]
            while node in prev:
                node = prev[node]
                path.append(node)
            return path[::-1]
        L, i, j = node
        for di, dj, c in nbr:
            ni, nj = i + di, j + dj
            if not (0 <= ni < NX and 0 <= nj < NY):
                continue
            o = trk[L, ni, nj]
            if o != FREE and o != code:
                continue
            nd = d + c
            nn = (L, ni, nj)
            if nd < dist.get(nn, 1e18):
                dist[nn] = nd
                prev[nn] = node
                heapq.heappush(pq, (nd + h(ni, nj), nd, nn))
        # via
        oL = 1 - L
        if all(via[k, i, j] in (FREE, code) for k in (0, 1)) and trk[oL, i, j] in (FREE, code):
            nd = d + VIA_COST
            nn = (oL, i, j)
            if nd < dist.get(nn, 1e18):
                dist[nn] = nd
                prev[nn] = node
                heapq.heappush(pq, (nd + h(i, j), nd, nn))
    return None


def simplify(path):
    """Collapse a grid path into (layer, [points]) runs plus via points."""
    runs, vias = [], []
    cur = [path[0]]
    for a in path[1:]:
        if a[0] != cur[-1][0]:
            vias.append((a[1], a[2]))
            runs.append(cur)
            cur = [a]
        else:
            cur.append(a)
    runs.append(cur)
    out = []
    sgn = lambda v: (v > 0) - (v < 0)
    for run in runs:
        pts = [run[0]]
        for k in range(1, len(run) - 1):
            d1 = (sgn(run[k][1] - run[k - 1][1]), sgn(run[k][2] - run[k - 1][2]))
            d2 = (sgn(run[k + 1][1] - run[k][1]), sgn(run[k + 1][2] - run[k][2]))
            if d1 != d2:
                pts.append(run[k])
        if len(run) > 1:
            pts.append(run[-1])
        out.append((run[0][0], [(wx(p[1]), wy(p[2])) for p in pts]))
    return out, [(wx(i), wy(j)) for i, j in vias]


def pad_cells(grid, pad, code):
    cx, cy = fr(pad.GetPosition())
    cells = set()
    for li, L in enumerate(LAYERS):
        if pad.IsOnLayer(L):
            i, j = gx(cx), gy(cy)
            for di in range(-2, 3):
                for dj in range(-2, 3):
                    cells.add((li, i + di, j + dj))
    return cells, (cx, cy)


def main():
    b = P.LoadBoard(os.path.join(HERE, "interposer_unrouted.kicad_pcb"))
    meta = json.load(open(os.path.join(HERE, "conns.json")))
    refs = meta["refs"]
    grid = Grid()
    load_obstacles(b, grid)

    def find_pad(spec):
        if spec[0] == "tail":
            fp = b.FindFootprintByReference(refs[spec[1]])
            num = str(spec[2])
        else:
            fp = b.FindFootprintByReference(spec[0])
            num = str(spec[1])
        for p in fp.Pads():
            if p.GetNumber() == num:
                return p
        raise KeyError(spec)

    G.board = b
    failed = []
    for netname, a, c in meta["conns"]:
        ni = b.FindNet(netname)
        code = ni.GetNetCode()
        pa, pc = find_pad(a), find_pad(c)
        sa, ca = pad_cells(grid, pa, code)
        sc, cc = pad_cells(grid, pc, code)
        path = astar(grid, code, sa, sc)
        if not path:
            failed.append(netname)
            print("FAILED", netname)
            continue
        runs, vias = simplify(path)
        for k, (li, pts) in enumerate(runs):
            if k == 0:
                pts = [ca] + pts
            if k == len(runs) - 1:
                pts = pts + [cc]
            for (ax, ay), (bx, by) in zip(pts, pts[1:]):
                if abs(ax - bx) < 1e-6 and abs(ay - by) < 1e-6:
                    continue
                t = P.PCB_TRACK(b)
                t.SetStart(G.V(ax, ay))
                t.SetEnd(G.V(bx, by))
                t.SetWidth(P.FromMM(TW))
                t.SetLayer(LAYERS[li])
                t.SetNet(ni)
                b.Add(t)
                grid.segment(li, ax, ay, bx, by, TW, code)
        for vx, vy in vias:
            v = P.PCB_VIA(b)
            v.SetPosition(G.V(vx, vy))
            v.SetWidth(P.FromMM(VIA_D))
            v.SetDrill(P.FromMM(VIA_DRILL))
            v.SetNet(ni)
            b.Add(v)
            for li in (0, 1):
                grid.shape(li, "circ", vx, vy, VIA_D, VIA_D, code)
        print(f"routed {netname}: {len(path)} cells, {len(vias)} vias")

    # TAP_GND pours under the plugs (ECU side), both layers
    # Under the ECU-side plugs, plus the DevKitC flag, except under its antenna
    ya = G.DK_PIN1_Y - 1.5
    poly = [(x0 + 0.4, y0 + 0.4), (-124.4, y0 + 0.4), (-124.4, -24.6), (-118.5, -24.6),
            (-118.5, 7.4), (x1 - 0.4, 7.4), (x1 - 0.4, y1 - 0.4), (-124.4, y1 - 0.4),
            (-124.4, ya), (x0 + 0.4, ya)]
    for L in LAYERS:
        G.add_zone(b, "TAP_GND", L, poly)
    P.ZONE_FILLER(b).Fill(b.Zones())
    b.Save(os.path.join(HERE, "interposer.kicad_pcb"))
    # footprint library, so the board opens without missing-library warnings
    lib = os.path.join(HERE, "junkr_interposer.pretty")
    io = P.IO_MGR.PluginFind(P.IO_MGR.KICAD_SEXP)
    if not os.path.isdir(lib):
        io.FootprintLibCreate(lib)
    seen = set()
    for fp in b.GetFootprints():
        name = fp.GetFPID().GetLibItemName().wx_str()
        if name in seen:
            continue
        seen.add(name)
        f = P.FOOTPRINT(fp)
        if f.IsFlipped():
            f.Flip(f.GetPosition(), False)
        f.SetPosition(P.VECTOR2I(0, 0))
        f.SetOrientationDegrees(0)
        f.SetReference("REF**")
        io.FootprintSave(lib, f)
    with open(os.path.join(HERE, "fp-lib-table"), "w") as t:
        t.write('(fp_lib_table\n  (lib (name "junkr_interposer")(type "KiCad")'
                '(uri "${KIPRJMOD}/junkr_interposer.pretty")(options "")(descr "PCB interposer parts"))\n)\n')
    print("wrote interposer.kicad_pcb", "failed:", failed)


if __name__ == "__main__":
    main()
