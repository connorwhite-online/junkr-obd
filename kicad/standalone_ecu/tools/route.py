#!/usr/bin/env python3
"""Route junkr_ecu_unrouted.kicad_pcb on a 0.1 mm grid (A* in router_core.c).

Layers: F.Cu (0), In1.Cu = solid GND plane (1), In2.Cu (2), B.Cu (3).
Every SMD GND pad gets a short escape to a via into the plane; all other
nets are routed as trees on F, In2 and B. Then GND zones are poured on all
layers, zones are filled and KiCad's DRC runs.

    /usr/bin/python3 route.py
"""
import ctypes
import math
import os
import subprocess
import sys
import time

import numpy as np
import pcbnew as P

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..")
G = 0.1                                   # grid, mm
LAYERS = [P.F_Cu, P.In1_Cu, P.In2_Cu, P.B_Cu]
PLANE = 1

CLASSES = {  # name: (track width, clearance)
    "Default": tuple(float(v) for v in os.environ.get("ROUTE_DEFAULT", "0.2,0.15").split(",")),
    "Rail": (0.25, 0.15),
    "Power": (0.5, 0.2),
    "HighCurrent": (1.5, 0.25),
}
NETCLASS = {}
for n in ("3V3", "3V3A", "3V3_RF", "VREF", "VC", "VPU"):
    NETCLASS[n] = "Rail"
for n in ("PB", "VIN", "VIN_RAW", "V5", "V5_OR", "VRLY", "BUCK_SW", "VBUS", "IGSW", "AUX_12V",
          "O_SVR", "H_SVR", "S_SVR", "MREL", "MREL_SRC", "H_S1", "S_S1", "O_S1", "H_S2", "S_S2",
          "O_S2", "H_SL", "S_SL", "O_SL", "H_SREL", "S_SREL", "O_SREL", "H_EGR", "S_EGR", "O_EGR",
          "H_STH1", "S_STH1", "O_STH1", "H_STH2", "S_STH2", "O_STH2", "BATT"):
    NETCLASS[n] = "Power"
for n in ("H_SPV", "S_SPV", "O_SPV", "SPV_SHUNT", "SPV_REC_N", "H_TCV", "S_TCV", "O_TCV", "TCV_SHUNT"):
    NETCLASS[n] = "HighCurrent"
RAIL_PRI = 1 if os.environ.get("ROUTE_RAILFIRST") else 3
VIA = {"Default": (0.6, 0.3), "Rail": (0.6, 0.3), "Power": (0.8, 0.4), "HighCurrent": (1.0, 0.5)}

lib = ctypes.CDLL(os.environ.get("ROUTER_SO", "/tmp/claude-0/work/router_core2.so"))
I32P = np.ctypeslib.ndpointer(dtype=np.int32, flags="C_CONTIGUOUS")
I16P = np.ctypeslib.ndpointer(dtype=np.int16, flags="C_CONTIGUOUS")
U8P = np.ctypeslib.ndpointer(dtype=np.uint8, flags="C_CONTIGUOUS")
F32P = np.ctypeslib.ndpointer(dtype=np.float32, flags="C_CONTIGUOUS")
lib.astar.restype = ctypes.c_int
lib.astar.argtypes = [I32P, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
                      I32P, ctypes.c_int, U8P,
                      I16P, I16P, ctypes.c_int, I16P, I16P, ctypes.c_int,
                      ctypes.c_float, F32P, U8P, ctypes.c_int, ctypes.c_int,
                      ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int,
                      ctypes.c_int, ctypes.c_int, I32P, ctypes.c_int]
lib.set_rip.restype = None
lib.set_rip.argtypes = [U8P, ctypes.c_int, ctypes.c_float]
lib.set_hist.restype = None
lib.set_hist.argtypes = [F32P]
lib.stamp.restype = None
lib.stamp.argtypes = [I32P, ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int, I32P, ctypes.c_int,
                      I16P, I16P, ctypes.c_int, ctypes.c_int, ctypes.c_int]


def disk(r):
    """Offsets of all cells within r cells."""
    n = int(math.ceil(r))
    dx, dy = [], []
    for y in range(-n, n + 1):
        for x in range(-n, n + 1):
            if x * x + y * y <= r * r + 1e-6:
                dx.append(x)
                dy.append(y)
    return np.array(dx, dtype=np.int16), np.array(dy, dtype=np.int16)


class Grid:
    def __init__(self, board):
        bb = board.GetBoardEdgesBoundingBox()
        self.x0, self.y0 = P.ToMM(bb.GetX()), P.ToMM(bb.GetY())
        self.Wmm, self.Hmm = P.ToMM(bb.GetWidth()), P.ToMM(bb.GetHeight())
        self.W, self.H, self.L = int(self.Wmm / G) + 1, int(self.Hmm / G) + 1, len(LAYERS)
        self.occ = np.zeros((self.L, self.H, self.W), dtype=np.int32)
        self.flat = self.occ.reshape(-1)

    def cell(self, x, y):
        return int(round((x - self.x0) / G)), int(round((y - self.y0) / G))

    def xy(self, cx, cy):
        return self.x0 + cx * G, self.y0 + cy * G

    def idx(self, l, cx, cy):
        return (l * self.H + cy) * self.W + cx


def pad_cells(grid, pad, grow=0.0):
    """Grid cells covered by the pad copper (+grow mm), as (cx, cy) arrays."""
    poly = pad.GetEffectivePolygon()
    bb = poly.BBox()
    x0, y0 = P.ToMM(bb.GetX()) - grow, P.ToMM(bb.GetY()) - grow
    x1, y1 = x0 + P.ToMM(bb.GetWidth()) + 2 * grow, y0 + P.ToMM(bb.GetHeight()) + 2 * grow
    cx0, cy0 = grid.cell(x0, y0)
    cx1, cy1 = grid.cell(x1, y1)
    xs, ys = [], []
    for cy in range(cy0 - 1, cy1 + 2):
        for cx in range(cx0 - 1, cx1 + 2):
            x, y = grid.xy(cx, cy)
            v = P.VECTOR2I(P.FromMM(x), P.FromMM(y))
            if grow <= 0:
                if poly.Contains(v):
                    xs.append(cx)
                    ys.append(cy)
            else:
                if poly.Contains(v) or poly.SquaredDistance(v) <= P.FromMM(grow) ** 2:
                    xs.append(cx)
                    ys.append(cy)
    return xs, ys


def pad_layers(pad):
    if pad.GetAttribute() in (P.PAD_ATTRIB_PTH, P.PAD_ATTRIB_NPTH):
        return [0, 1, 2, 3]
    return [0] if pad.IsOnLayer(P.F_Cu) else [3]


class Router:
    def __init__(self, board):
        self.board = board
        self.grid = Grid(board)
        self.netcode = {}
        self.pads = {}                    # net -> list of (pad, layers, cells)
        self.tracks = []                  # (net, layer, [(x,y)...], width)
        self.vias = []                    # (net, x, y, dia, drill)
        self.tree = {}                    # net -> list of flat source cells
        self.failed = []
        self.ripped = {}                  # net -> times ripped up
        self._rasterise()
        g = self.grid
        self.hard = np.zeros(g.L * g.H * g.W, dtype=np.uint8)
        for plist in self.pads.values():
            for _, _, cells in plist:
                self.hard[np.array(cells, dtype=np.int64)] = 1
        self.hist = np.zeros(g.L * g.H * g.W, dtype=np.float32)
        lib.set_hist(self.hist)

    def code(self, name):
        if name not in self.netcode:
            self.netcode[name] = len(self.netcode) + 1
        return self.netcode[name]

    def _rasterise(self):
        g = self.grid
        occ = g.occ
        # board edge keep-off 0.5 mm
        e = int(0.5 / G) + 1
        occ[:, :e, :] = -1
        occ[:, -e:, :] = -1
        occ[:, :, :e] = -1
        occ[:, :, -e:] = -1
        # footprint keepout zones (ESP32 antenna)
        for fp in self.board.GetFootprints():
            for z in fp.Zones():
                if z.GetIsRuleArea():
                    bb = z.GetBoundingBox()
                    cx0, cy0 = g.cell(P.ToMM(bb.GetX()), P.ToMM(bb.GetY()))
                    cx1, cy1 = g.cell(P.ToMM(bb.GetX() + bb.GetWidth()), P.ToMM(bb.GetY() + bb.GetHeight()))
                    occ[:, max(cy0, 0):max(cy1, 0) + 1, max(cx0, 0):max(cx1, 0) + 1] = -1
        for fp in self.board.GetFootprints():
            for pad in fp.Pads():
                name = pad.GetNetname()
                layers = pad_layers(pad)
                if pad.GetAttribute() == P.PAD_ATTRIB_NPTH or not name:
                    xs, ys = pad_cells(g, pad, grow=0.0)
                    if pad.GetAttribute() == P.PAD_ATTRIB_NPTH:
                        r = P.ToMM(pad.GetDrillSize().x) / 2
                        c = pad.GetPosition()
                        cx, cy = g.cell(P.ToMM(c.x), P.ToMM(c.y))
                        dx, dy = disk(r / G + 0.5)
                        xs, ys = list(cx + dx), list(cy + dy)
                    for l in layers:
                        for x, y in zip(xs, ys):
                            if 0 <= x < g.W and 0 <= y < g.H:
                                occ[l, y, x] = -1
                    continue
                code = self.code(name)
                xs, ys = pad_cells(g, pad)
                if not xs:
                    c = pad.GetPosition()
                    cx, cy = g.cell(P.ToMM(c.x), P.ToMM(c.y))
                    xs, ys = [cx], [cy]
                cells = []
                for l in layers:
                    if l == PLANE:
                        continue
                    for x, y in zip(xs, ys):
                        if occ[l, y, x] == 0:
                            occ[l, y, x] = code
                        cells.append(g.idx(l, x, y))
                self.pads.setdefault(name, []).append((pad, layers, cells))

    # ------------------------------------------------------------------
    def width(self, net):
        return CLASSES[NETCLASS.get(net, "Default")]

    def route_conn(self, net, src, tgt_cells, hint, width, clr, via, plane_target=False, window=12.0, cross=0.0):
        g = self.grid
        lib.set_rip(self.hard, self.code("GND"), cross)
        code = self.code(net)
        tgt = np.zeros(g.L * g.H * g.W, dtype=np.uint8)
        if plane_target:
            tgt.reshape(g.L, g.H, g.W)[PLANE] = 1
        for c in tgt_cells:
            tgt[c] = 1
        tdx, tdy = disk((width / 2 + clr + 0.05) / G)
        vdx, vdy = disk((via[0] / 2 + clr + 0.05) / G)
        layer_cost = np.array([1.0, 1.0, 1.15, 1.05], dtype=np.float32)
        layer_ok = np.array([1, 1 if plane_target else 0, 1, 1], dtype=np.uint8)
        srcs = np.array(src, dtype=np.int32)
        # window: bbox of src + tgt + margin
        pts = list(src) + list(tgt_cells)
        cx = [(c % (g.H * g.W)) % g.W for c in pts]
        cy = [(c % (g.H * g.W)) // g.W for c in pts]
        out = np.zeros(400000, dtype=np.int32)
        wins = (window, 40.0) if cross == 0 else (window, 60.0)
        for win in wins:
            m = int(win / G)
            bx0, by0 = max(0, min(cx) - m), max(0, min(cy) - m)
            bx1, by1 = min(g.W - 1, max(cx) + m), min(g.H - 1, max(cy) + m)
            n = lib.astar(g.flat, g.W, g.H, g.L, code, srcs, len(srcs), tgt, tdx, tdy, len(tdx),
                          vdx, vdy, len(vdx), 6.0 + via[0] * 4, layer_cost, layer_ok, PLANE,
                          self.code("GND"), bx0, by0, bx1, by1, hint[0], hint[1], out, len(out))
            if n:
                lib.set_rip(self.hard, 0, 0.0)
                return out[:n][::-1].copy()
        lib.set_rip(self.hard, 0, 0.0)
        return None

    def victims(self, net, path, width, clr, via):
        """Nets whose copper the path (with clearance) overlaps."""
        g = self.grid
        WH = g.W * g.H
        code = self.code(net)
        tdx, tdy = disk((width / 2 + clr + 0.05) / G)
        vdx, vdy = disk((via[0] / 2 + clr + 0.05) / G)
        hit = set()
        prev = None
        for c in path:
            l, rem = int(c // WH), int(c % WH)
            x, y = rem % g.W, rem // g.W
            layers = [l] if l != PLANE else []
            dx, dy = tdx, tdy
            if prev is not None and prev[0] != l:
                layers = [ll for ll in range(g.L) if ll != PLANE]
                dx, dy = vdx, vdy
            prev = (l, x, y)
            for ll in layers:
                xs = np.clip(x + dx, 0, g.W - 1)
                ys = np.clip(y + dy, 0, g.H - 1)
                o = g.occ[ll, ys, xs]
                for v in np.unique(o):
                    if v > 0 and v != code:
                        hit.add(int(v))
        names = {v: k for k, v in self.netcode.items()}
        return [names[v] for v in hit]

    def rip(self, net):
        g = self.grid
        code = self.code(net)
        for l in range(g.L):
            if l == PLANE:
                continue
            a = g.occ[l]
            a[a == code] = 0
        for _, layers, cells in self.pads[net]:
            c = np.array(cells, dtype=np.int64)
            sel = g.flat[c] == 0
            g.flat[c[sel]] = code
        self.tracks = [t for t in self.tracks if t[0] != net]
        self.vias = [v for v in self.vias if v[0] != net]
        self.failed = [f for f in self.failed if f[0] != net]
        self.tree[net] = []
        self.ripped[net] = self.ripped.get(net, 0) + 1

    def commit(self, net, path, width, via):
        """Turn a cell path into tracks + vias, stamp it into the grid."""
        g = self.grid
        code = self.code(net)
        WH = g.W * g.H
        cells = [(int(c // WH), int((c % WH) % g.W), int((c % WH) // g.W)) for c in path]
        sdx, sdy = disk(width / 2 / G)
        vdx, vdy = disk(via[0] / 2 / G)
        run = []
        for i, (l, x, y) in enumerate(cells):
            if run and run[-1][0] != l:
                # layer change: via at this xy
                if l != PLANE and run[-1][0] != PLANE:
                    pass
                self._flush(net, run, width)
                if not (l == PLANE or run[-1][0] == PLANE) or True:
                    vx, vy = g.xy(x, y)
                    self.vias.append((net, vx, vy, via[0], via[1]))
                    vc = np.array([g.idx(0, x, y)], dtype=np.int32)
                    lib.stamp(g.flat, g.W, g.H, g.L, code, vc, 1, vdx, vdy, len(vdx), 1, PLANE)
                    for ll in (0, 2, 3):
                        self.tree.setdefault(net, []).append(g.idx(ll, x, y))
                run = []
            run.append((l, x, y))
        self._flush(net, run, width)
        flat = np.array([g.idx(l, x, y) for l, x, y in cells if l != PLANE], dtype=np.int32)
        if len(flat):
            lib.stamp(g.flat, g.W, g.H, g.L, code, flat, len(flat), sdx, sdy, len(sdx), 0, PLANE)
        self.tree.setdefault(net, []).extend(int(c) for c in flat)

    def _flush(self, net, run, width):
        if len(run) < 2 or run[0][0] == PLANE:
            return
        g = self.grid
        pts = [run[0]]
        for i in range(1, len(run) - 1):
            a, b, c = pts[-1], run[i], run[i + 1]
            d1 = (b[1] - a[1], b[2] - a[2])
            d2 = (c[1] - b[1], c[2] - b[2])
            if d1[0] * d2[1] - d1[1] * d2[0] != 0:
                pts.append(b)
        pts.append(run[-1])
        self.tracks.append((net, run[0][0], [g.xy(p[1], p[2]) for p in pts], width))

    # ------------------------------------------------------------------
    def route_all(self):
        t0 = time.time()
        gnd = self.pads.get("GND", [])
        # 1) GND escapes: every SMD GND pad to the plane
        for pad, layers, cells in gnd:
            if PLANE in layers:
                continue
            w = min(0.4, max(0.2, min(P.ToMM(pad.GetSize().x), P.ToMM(pad.GetSize().y)) * 0.8))
            c = pad.GetPosition()
            hint = self.grid.cell(P.ToMM(c.x), P.ToMM(c.y))
            path = self.route_conn("GND", cells, [], hint, w, 0.15, (0.6, 0.3), plane_target=True, window=3.0)
            if path is None:
                self.failed.append(("GND", pad.GetParent().GetReference() + "." + pad.GetNumber()))
                continue
            self.commit("GND", path, w, (0.6, 0.3))
        print("GND escapes done %.1fs, %d vias" % (time.time() - t0, len(self.vias)))
        # 2) everything else, by class then size
        order = []
        for net, plist in self.pads.items():
            if net == "GND" or len(plist) < 2:
                continue
            xs = [P.ToMM(p.GetPosition().x) for p, _, _ in plist]
            ys = [P.ToMM(p.GetPosition().y) for p, _, _ in plist]
            span = (max(xs) - min(xs)) + (max(ys) - min(ys))
            pri = {"HighCurrent": 0, "Power": 1, "Rail": RAIL_PRI, "Default": 2}[NETCLASS.get(net, "Default")]
            order.append((pri, span, net))
        order.sort()
        for k, (_, _, net) in enumerate(order):
            self.route_net(net)
            if k % 40 == 0:
                print("  %d/%d nets, %.0fs, %d failed" % (k, len(order), time.time() - t0, len(self.failed)))
        # 3) rip-up and reroute: failed nets may run over other nets' tracks,
        #    which are torn up and requeued. Crossing gets dearer each round.
        queue = []
        for n, _ in self.failed:
            if n != "GND" and n not in queue:
                queue.append(n)
        it = 0
        limit = 2500
        budget = float(os.environ.get("ROUTE_BUDGET", "1e9"))
        while queue and it < limit and time.time() - t0 < budget:
            net = queue.pop(0)
            it += 1
            self.rip(net)
            # a net that keeps getting torn up stops tearing up others
            cross = 30.0 if self.ripped.get(net, 0) < 10 else 0.0
            for vn in self.route_net(net, cross=cross):
                if vn not in queue:
                    queue.append(vn)
            if any(f[0] == net for f in self.failed) and net not in queue and self.ripped[net] < 14:
                queue.append(net)
            if it % 25 == 0:
                print("  rip-up %d: queue %d, %d failed, %.0fs" % (it, len(queue), len(self.failed), time.time() - t0))
        for net in queue:   # out of iterations: route what can be routed without ripping
            self.rip(net)
            self.route_net(net)
        print("routing done %.0fs: %d tracks, %d vias, %d failed" % (
            time.time() - t0, len(self.tracks), len(self.vias), len(self.failed)))

    def route_net(self, net, cross=0.0):
        """Route a net. With cross > 0 a connection that cannot be made may run
        over other nets' tracks; those nets are ripped up and returned."""
        ripped = []
        plist = self.pads[net]
        w_cls, clr = self.width(net)
        via = VIA[NETCLASS.get(net, "Default")]
        # Prim order over pad centres
        pos = [(P.ToMM(p.GetPosition().x), P.ToMM(p.GetPosition().y)) for p, _, _ in plist]
        done = [0]
        todo = list(range(1, len(plist)))
        self.tree[net] = list(plist[0][2])
        while todo:
            best = min(((math.hypot(pos[i][0] - pos[j][0], pos[i][1] - pos[j][1]), j) for j in todo for i in done))
            j = best[1]
            pad = plist[j][0]
            padmin = min(P.ToMM(pad.GetSize().x), P.ToMM(pad.GetSize().y))
            w = w_cls
            if padmin < w:
                w = max(0.2, min(w, padmin))
            hint = self.grid.cell(*pos[j])
            path = self.route_conn(net, self.tree[net], plist[j][2], hint, w, clr, via)
            if path is None and w > 0.25:
                w = 0.25
                path = self.route_conn(net, self.tree[net], plist[j][2], hint, w, 0.15, (0.6, 0.3))
            if path is None and cross > 0:
                w = w if w <= 0.25 else 0.25
                v2 = via if w > 0.25 else (0.6, 0.3)
                path = self.route_conn(net, self.tree[net], plist[j][2], hint, w, clr if w > 0.25 else 0.15, v2,
                                       window=20.0, cross=cross)
                if path is not None:
                    vs = self.victims(net, path, w, clr if w > 0.25 else 0.15, v2)
                    if vs:
                        # make this corridor dearer for everyone next time
                        self.hist[np.array(path, dtype=np.int64)] += 1.5
                    for vn in vs:
                        self.rip(vn)
                        ripped.append(vn)
            if path is None:
                self.failed.append((net, pad.GetParent().GetReference() + "." + pad.GetNumber()))
            else:
                self.commit(net, path, w, via)
                self.tree[net].extend(plist[j][2])
            done.append(j)
            todo.remove(j)
        return ripped

    # ------------------------------------------------------------------
    def write(self, board):
        nets = {n.GetNetname(): n for n in board.GetNetsByName().values()} if False else None
        netmap = board.GetNetsByName()
        for net, layer, pts, width in self.tracks:
            ni = netmap[net]
            for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
                t = P.PCB_TRACK(board)
                t.SetStart(P.VECTOR2I(P.FromMM(x0), P.FromMM(y0)))
                t.SetEnd(P.VECTOR2I(P.FromMM(x1), P.FromMM(y1)))
                t.SetWidth(P.FromMM(width))
                t.SetLayer(LAYERS[layer])
                t.SetNet(ni)
                board.Add(t)
        seen = set()
        for net, x, y, dia, drill in self.vias:
            key = (round(x, 3), round(y, 3))
            if key in seen:
                continue
            seen.add(key)
            v = P.PCB_VIA(board)
            v.SetPosition(P.VECTOR2I(P.FromMM(x), P.FromMM(y)))
            v.SetWidth(P.FromMM(dia))
            v.SetDrill(P.FromMM(drill))
            v.SetNet(netmap[net])
            board.Add(v)


def add_zones(board):
    netmap = board.GetNetsByName()
    gnd = netmap["GND"]
    bb = board.GetBoardEdgesBoundingBox()
    x0, y0 = bb.GetX(), bb.GetY()
    x1, y1 = x0 + bb.GetWidth(), y0 + bb.GetHeight()
    for layer, prio in ((P.In1_Cu, 0), (P.F_Cu, 0), (P.B_Cu, 0), (P.In2_Cu, 0)):
        z = P.ZONE(board)
        z.SetLayer(layer)
        z.SetNet(gnd)
        z.SetAssignedPriority(prio)
        z.SetLocalClearance(P.FromMM(0.25))
        z.SetMinThickness(P.FromMM(0.2))
        z.SetThermalReliefGap(P.FromMM(0.3))
        z.SetThermalReliefSpokeWidth(P.FromMM(0.4))
        z.SetPadConnection(P.ZONE_CONNECTION_THERMAL)
        z.SetIslandRemovalMode(P.ISLAND_REMOVAL_MODE_ALWAYS)
        ol = z.Outline()
        ol.NewOutline()
        m = P.FromMM(0.3)
        for (x, y) in ((x0 + m, y0 + m), (x1 - m, y0 + m), (x1 - m, y1 - m), (x0 + m, y1 - m)):
            ol.Append(x, y)
        board.Add(z)
    filler = P.ZONE_FILLER(board)
    filler.Fill(board.Zones())


def main():
    src = os.path.join(OUT, "junkr_ecu_unrouted.kicad_pcb")
    board = P.LoadBoard(src)
    r = Router(board)
    print("grid %dx%dx%d, %d nets with pads" % (r.grid.W, r.grid.H, r.grid.L, len(r.pads)))
    r.route_all()
    r.write(board)
    add_zones(board)
    tag = os.environ.get("ROUTE_TAG", "")
    dst = os.path.join(OUT, "junkr_ecu%s.kicad_pcb" % tag)
    board.Save(dst)
    with open(os.path.join(OUT, "route_report%s.txt" % tag), "w") as f:
        f.write("failed connections: %d\n" % len(r.failed))
        for n, p in r.failed:
            f.write("%s %s\n" % (n, p))
    print("saved", dst)


if __name__ == "__main__":
    main()
