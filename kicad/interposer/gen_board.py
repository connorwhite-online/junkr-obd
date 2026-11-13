#!/usr/bin/env python3
"""Generate the PCB interposer board (KiCad 7, pcbnew API).

    python3 gen_board.py            -> interposer_unrouted.kicad_pcb + interposer.dsn
    (freerouting routes the 12 tap nets, see route.sh)
    python3 gen_board.py --finish   -> imports interposer.ses, adds the
                                       ground pours, writes interposer.kicad_pcb

The board sits between the ECU and the three OEM harness plugs:

  harness plugs ==> [ TE 176122-6 header ]==board==[ OEM-style female plugs ] ==> ECU
                       (the real ECU header)         (short tails soldered in)

Every one of the 64 pins passes straight through on a short trace on
both copper layers. Twelve E6/E8 pins also feed a 100k / bottom-resistor
divider (on the back) and go to a 2x7 header for the ESP32-S3.

Coordinates are those of the 176122-6 footprint (mm, KiCad Y down):
pin rows at y = 0 (upper cavity row) and y = -4.5 (lower), header mouth
at y = -25.5 (the harness edge). +y points at the ECU.
"""
import math
import os
import sys

import pcbnew as P

HERE = os.path.dirname(os.path.abspath(__file__))
OX, OY = 150.0, 100.0          # footprint frame -> sheet


def V(x, y):
    return P.VECTOR2I(P.FromMM(x + OX), P.FromMM(y + OY))


# ---------------------------------------------------------------------------
# Geometry
# ---------------------------------------------------------------------------
# Header pad X per column, from the measured 176122-6 footprint. Each
# plug lists its columns in Toyota pin order (manual column 1 first).
# Type 2 = 070 (1.8 mm tab), 1 = 040 (1.0 mm tab).
PLUGS = {
    "E8": dict(pad0=0, cols=[(-109.548, 2), (-106.050, 2), (-102.949, 1), (-99.949, 1),
                             (-96.949, 1), (-93.950, 1), (-90.950, 1), (-87.950, 1),
                             (-84.950, 1), (-81.951, 1), (-78.849, 2)]),
    "E6": dict(pad0=22, cols=[(-72.901 + 3.0 * i, 1) for i in range(8)]),
    "E5": dict(pad0=38, cols=[(-45.954, 2), (-42.454, 2), (-39.350, 1), (-36.350, 1),
                              (-33.350, 1), (-30.350, 1), (-27.351, 1), (-24.351, 1),
                              (-21.351, 1), (-18.352, 1), (-15.250, 2), (-11.750, 2),
                              (-8.250, 2)]),
}
ROW_Y = (0.0, -4.50088)        # header rows: upper, lower
TAIL_Y = (3.0, 5.8)            # ECU-side tail holes: upper, lower
HDR_PAD, HDR_DRILL = 1.8, 1.1
TAIL = {1: (1.9, 1.2), 2: (2.3, 1.5)}     # pad, drill for 040 / 070 tails
PEGS = [(-114.600, -10.201), (-75.750, -10.201), (-49.050, -10.201), (-3.200, -10.201)]
PEG_D = 3.5                    # [VERIFY] header board-lock pegs

# Board: a 129.5 x 46 main strip under the header, plus a "flag" at the -X end
# that carries the DevKitC and reaches past the harness edge (no plugs there).
BOARD = (-153.0, -42.0, 5.5, 21.0)        # bounding box x0, y0, x1, y1 (ECU edge at y1)
CUT = (-124.0, -42.0, 5.5, -25.0)         # not board: where the harness plugs go
OUTLINE = [(-153.0, -42.0), (-124.0, -42.0), (-124.0, -25.0), (5.5, -25.0),
           (5.5, 21.0), (-153.0, 21.0)]
CLEAR = 0.25
MOUNT = [(-121.5, -22.5), (-121.5, 18.5), (3.0, -22.5), (3.0, 17.5), (-114.0, 3.0)]   # M2; the last one also holds the lid
MOUNT_D = 2.2

# Pin names (E6 / E8 / E5), from hardware/ecu_interposer/pinout_kzj95_at.csv
def load_pinout():
    import csv
    names = {}
    with open(os.path.join(HERE, "..", "..", "hardware", "ecu_interposer", "pinout_kzj95_at.csv")) as f:
        for r in csv.DictReader(f):
            nm = r["name"].replace("/", "").replace("+", "P").replace("-", "N") or "NC"
            names[(r["connector"], int(r["pin"]))] = nm
    return names

# Taps, rev C (ESP32-S3-DevKitC-1 on the board): (plug, pin, bottom R, GPIO).
# The 12 stage 1 channels keep the GPIO map of docs/BUILD_STAGES.md (ADC1,
# GPIO1-12). E5 adds five 0/12 V on-off inputs: A/T solenoids SL S1 S2 and
# the transfer L4 / neutral switches, on GPIO13-17.
TAPS = [("E6", 2, "82k", 1), ("E6", 4, "82k", 2), ("E8", 5, "27k", 3), ("E6", 3, "82k", 4),
        ("E6", 5, "82k", 5), ("E6", 13, "82k", 6), ("E6", 11, "82k", 7), ("E6", 1, "82k", 8),
        ("E8", 12, "18k", 9), ("E6", 9, "82k", 10), ("E8", 18, "27k", 11), ("E8", 9, "150k", 12),
        ("E5", 1, "27k", 13), ("E5", 2, "27k", 14), ("E5", 15, "27k", 15), ("E5", 7, "27k", 16),
        ("E5", 8, "27k", 17)]

# ESP32-S3-DevKitC-1 headers, pin 1 at the antenna end. J1 = pads 1..22,
# J3 = pads 23..44 (Espressif user guide, v1.1).
DK_J1 = ["3V3", "3V3", "RST", 4, 5, 6, 7, 15, 16, 17, 18, 8, 3, 46, 9, 10, 11, 12, 13, 14, "5V", "GND"]
DK_J3 = ["GND", 43, 44, 1, 2, 42, 41, 40, 39, 38, 37, 36, 35, 0, 45, 48, 47, 21, 20, 19, "GND", "GND"]
DK_X = -138.5                  # centre between the rows
DK_ROW = 22.86                 # [VERIFY] row spacing (0.9 in) against the real board
DK_PIN1_Y = 14.0               # pin 1 (antenna end, toward the ECU); USB end at -Y
GPIO_PIN = {}
for _k, _g in enumerate(DK_J1 + DK_J3):
    if isinstance(_g, int):
        GPIO_PIN[_g] = _k + 1


def dk_xy(pad):
    """Pad position: J1 on the +X row (toward the dividers), J3 on the -X row."""
    k = (pad - 1) % 22
    x = DK_X + DK_ROW / 2 if pad <= 22 else DK_X - DK_ROW / 2
    return x, DK_PIN1_Y - k * 2.54


def pin_xy(plug, pin):
    """Return (column index, row, column x) of a Toyota pin."""
    cols = PLUGS[plug]["cols"]
    n = len(cols)
    row = 0 if pin <= n else 1
    i = (pin - 1) % n
    return i, row, cols[i][0]


def route_x(plug, i):
    """X of the lower-row route: centre of the free gap to the next column."""
    cols = PLUGS[plug]["cols"]
    x, t = cols[i]
    if i + 1 < len(cols):
        xn, tn = cols[i + 1]
    else:
        xn, tn = x + 3.4, 1
    # gaps in the header row and in the tail row; take the tighter
    r_h = HDR_PAD / 2
    r1, r2 = TAIL[t][0] / 2, TAIL[tn][0] / 2
    lo, hi = max(x + r_h, x + r1), min(xn - r_h, xn - r2)
    xr = (lo + hi) / 2
    w = min(1.2, (hi - lo) - 2 * CLEAR)
    return xr, round(w, 2)


# ---------------------------------------------------------------------------
# Board helpers
# ---------------------------------------------------------------------------
board = None
nets = {}
CONNS = []


def net(name):
    if name not in nets:
        n = P.NETINFO_ITEM(board, name)
        board.Add(n)
        nets[name] = n
    return nets[name]


def text(s, x, y, size=1.0, layer=P.F_SilkS, angle=0, parent=None, mirror=False):
    t = P.PCB_TEXT(parent or board)
    t.SetText(s)
    t.SetPosition(V(x, y))
    t.SetLayer(layer)
    t.SetTextSize(P.VECTOR2I(P.FromMM(size), P.FromMM(size)))
    t.SetTextThickness(P.FromMM(size * 0.15))
    if angle:
        t.SetTextAngleDegrees(angle)
    if mirror:
        t.SetMirrored(True)
    board.Add(t)


def line(x0, y0, x1, y1, layer, w=0.15, parent=None):
    s = P.PCB_SHAPE(parent or board)
    s.SetShape(P.SHAPE_T_SEGMENT)
    s.SetStart(V(x0, y0))
    s.SetEnd(V(x1, y1))
    s.SetLayer(layer)
    s.SetWidth(P.FromMM(w))
    (parent or board).Add(s)


def fp_line(fp, x0, y0, x1, y1, layer, w=0.15):
    s = P.FP_SHAPE(fp)
    s.SetShape(P.SHAPE_T_SEGMENT)
    s.SetStart(V(x0, y0))
    s.SetEnd(V(x1, y1))
    s.SetLocalCoord()
    s.SetLayer(layer)
    s.SetWidth(P.FromMM(w))
    fp.Add(s)


def rect(fp, x0, y0, x1, y1, layer, w=0.15):
    for a, b, c, d in ((x0, y0, x1, y0), (x1, y0, x1, y1), (x1, y1, x0, y1), (x0, y1, x0, y0)):
        fp_line(fp, a, b, c, d, layer, w)


def new_fp(ref, value, x, y, name):
    fp = P.FOOTPRINT(board)
    fp.SetReference(ref)
    fp.SetValue(value)
    fp.SetFPID(P.LIB_ID("junkr_interposer", name))
    fp.SetPosition(V(x, y))
    fp.Reference().SetPosition(V(x, y))
    fp.Value().SetPosition(V(x, y))
    fp.Value().SetVisible(False)
    return fp


def pth(fp, num, x, y, pad, drill, netname=None, shape=P.PAD_SHAPE_CIRCLE):
    p = P.PAD(fp)
    p.SetNumber(str(num))
    p.SetShape(shape)
    p.SetAttribute(P.PAD_ATTRIB_PTH)
    p.SetLayerSet(P.PAD.PTHMask())
    p.SetSize(P.VECTOR2I(P.FromMM(pad), P.FromMM(pad)))
    p.SetDrillSize(P.VECTOR2I(P.FromMM(drill), P.FromMM(drill)))
    p.SetPosition(V(x, y))
    p.SetLocalCoord()
    if netname:
        p.SetNet(net(netname))
    fp.Add(p)
    return p


def npth(fp, x, y, d):
    p = P.PAD(fp)
    p.SetNumber("")
    p.SetShape(P.PAD_SHAPE_CIRCLE)
    p.SetAttribute(P.PAD_ATTRIB_NPTH)
    p.SetLayerSet(P.PAD.UnplatedHoleMask())
    p.SetSize(P.VECTOR2I(P.FromMM(d), P.FromMM(d)))
    p.SetDrillSize(P.VECTOR2I(P.FromMM(d), P.FromMM(d)))
    p.SetPosition(V(x, y))
    p.SetLocalCoord()
    fp.Add(p)


def smd(fp, num, x, y, w, h, netname=None):
    p = P.PAD(fp)
    p.SetNumber(str(num))
    p.SetShape(P.PAD_SHAPE_ROUNDRECT)
    p.SetRoundRectRadiusRatio(0.25)
    p.SetAttribute(P.PAD_ATTRIB_SMD)
    p.SetLayerSet(P.PAD.SMDMask())
    p.SetSize(P.VECTOR2I(P.FromMM(w), P.FromMM(h)))
    p.SetPosition(V(x, y))
    p.SetLocalCoord()
    if netname:
        p.SetNet(net(netname))
    fp.Add(p)
    return p


def via(x, y, netname, d=0.6, drill=0.3):
    v = P.PCB_VIA(board)
    v.SetPosition(V(x, y))
    v.SetWidth(P.FromMM(d))
    v.SetDrill(P.FromMM(drill))
    v.SetNet(net(netname))
    board.Add(v)


def track(x0, y0, x1, y1, w, netname, layer, locked=True):
    t = P.PCB_TRACK(board)
    t.SetStart(V(x0, y0))
    t.SetEnd(V(x1, y1))
    t.SetWidth(P.FromMM(w))
    t.SetLayer(layer)
    t.SetNet(net(netname))
    t.SetLocked(locked)
    board.Add(t)


# ---------------------------------------------------------------------------
def build():
    global board
    board = P.NewBoard(os.path.join(HERE, "interposer_unrouted.kicad_pcb"))
    names = load_pinout()

    ds = board.GetDesignSettings()
    ds.m_TrackMinWidth = P.FromMM(0.2)
    ds.m_ViasMinSize = P.FromMM(0.6)
    ds.m_MinThroughDrill = P.FromMM(0.3)
    ds.m_CopperEdgeClearance = P.FromMM(0.3)
    nc = ds.m_NetSettings.m_DefaultNetClass
    nc.SetClearance(P.FromMM(0.2))
    nc.SetTrackWidth(P.FromMM(0.25))
    nc.SetViaDiameter(P.FromMM(0.6))
    nc.SetViaDrill(P.FromMM(0.3))

    # board outline
    for (a, b), (c, d) in zip(OUTLINE, OUTLINE[1:] + OUTLINE[:1]):
        line(a, b, c, d, P.Edge_Cuts, 0.1)

    def netname(plug, pin):
        return f"{plug}_{pin:02d}_{names.get((plug, pin), 'NC')}"

    # J1: the real ECU header, harness side ---------------------------------
    j1 = new_fp("J1", "TE 176122-6", 0, 0, "TE_176122-6_RA")
    for plug, d in PLUGS.items():
        n = len(d["cols"])
        for i, (x, t) in enumerate(d["cols"]):
            for r in (0, 1):
                pin = i + 1 + r * n
                pth(j1, d["pad0"] + pin, x, ROW_Y[r], HDR_PAD, HDR_DRILL, netname(plug, pin))
    for x, y in PEGS:
        npth(j1, x, y, PEG_D)
    rect(j1, -117.8, -25.5, 0.0, -7.0, P.F_Fab)
    rect(j1, -117.8, -25.5, 0.0, -7.0, P.F_CrtYd, 0.05)
    fp_line(j1, -117.8, -25.5, 0.0, -25.5, P.F_SilkS, 0.2)
    j1.Reference().SetPosition(V(-118.8, -2))
    board.Add(j1)

    # H1..H4: M2 holes that clamp the board between tray and lid
    for k, (x, y) in enumerate(MOUNT):
        h = new_fp(f"H{k + 1}", "M2", x, y, "MountingHole_2.2mm")
        npth(h, x, y, MOUNT_D)
        h.Reference().SetVisible(False)
        board.Add(h)

    # J2..J4: ECU-side tails (OEM plug housing wires soldered in) -----------
    refs = {"E8": "J2", "E6": "J3", "E5": "J4"}
    for plug, d in PLUGS.items():
        cols = d["cols"]
        n = len(cols)
        fp = new_fp(refs[plug], f"{plug} tails ({2 * n}P)", cols[0][0], TAIL_Y[0], f"Tails_{plug}")
        for i, (x, t) in enumerate(cols):
            pad, drill = TAIL[t]
            xr, w = route_x(plug, i)
            pu, pl = i + 1, i + 1 + n
            pth(fp, pu, x, TAIL_Y[0], pad, drill, netname(plug, pu),
                P.PAD_SHAPE_RECT if pu == 1 else P.PAD_SHAPE_CIRCLE)
            pth(fp, pl, xr, TAIL_Y[1], pad, drill, netname(plug, pl))
            # pass-through copper, both layers, locked
            wu = 1.0 if t == 1 else 1.4
            for L in (P.F_Cu, P.B_Cu):
                track(x, ROW_Y[0], x, TAIL_Y[0], wu, netname(plug, pu), L)
                track(x, ROW_Y[1], xr, ROW_Y[1] + 2.25, w, netname(plug, pl), L)
                track(xr, ROW_Y[1] + 2.25, xr, TAIL_Y[1], w, netname(plug, pl), L)
        xa, xb = cols[0][0] - 1.6, cols[-1][0] + 2.0
        fp.Reference().SetPosition(V((xa + xb) / 2, 8.0))
        fp.Reference().SetVisible(False)
        board.Add(fp)
        # silk: plug name and pin-1 marks, harness and ECU sides
        text(plug, (xa + xb) / 2, -21.5, 1.6)
        text(plug, (xa + xb) / 2, 8.6, 1.2)
        text("1", cols[0][0] - 1.6, 3.0, 0.8)
        text(str(n + 1), cols[0][0] - 0.4, -6.7, 0.8)
        fp_line(fp, xa, 7.4, xb, 7.4, P.F_SilkS, 0.15)

    # Divider cells on the back, under the E8/E6 plugs ----------------------
    # TAP_GND is the ESP32's ground: it never touches the ECU's grounds.
    cell_x0, pitch = -114.0, 4.6
    taps_sorted = sorted(TAPS, key=lambda t: pin_xy(t[0], t[1])[2]
                         + (route_x(t[0], pin_xy(t[0], t[1])[0])[0] - pin_xy(t[0], t[1])[2]
                            if pin_xy(t[0], t[1])[1] else 0))
    CONNS.clear()
    for k, (plug, pin, rb, gpio) in enumerate(taps_sorted):
        cx = cell_x0 + k * pitch
        sig = netname(plug, pin)
        adc = f"ADC_GPIO{gpio:02d}"
        for ref, val, cy, n1, n2 in ((f"R{gpio}", "100k", 10.0, sig, adc),
                                     (f"R{gpio + 50}", rb, 14.5, adc, "TAP_GND")):
            r = new_fp(ref, val, cx, cy, "R_0805")
            # Flip() below mirrors Y, so pad 1 ends up at cy - 0.95
            smd(r, 1, cx, cy + 0.95, 1.0, 1.3, n1)
            smd(r, 2, cx, cy - 0.95, 1.0, 1.3, n2)
            rect(r, cx - 0.9, cy - 1.75, cx + 0.9, cy + 1.75, P.F_CrtYd, 0.05)
            r.Reference().SetVisible(False)
            r.Flip(V(cx, cy), False)
            board.Add(r)
        # optional 10 nF filter cap (DNP by default), between the two resistors
        c = new_fp(f"C{gpio}", "10n DNP", cx + 2.0, 12.25, "C_0603")
        smd(c, 1, cx + 1.2, 12.25, 0.9, 0.95, adc)
        smd(c, 2, cx + 2.8, 12.25, 0.9, 0.95, "TAP_GND")
        c.Reference().SetVisible(False)
        c.Flip(V(cx + 2.0, 12.25), False)
        board.Add(c)
        # ADC node: R_top pad 2 -> R_bot pad 1, and the cap
        track(cx, 10.95, cx, 13.55, 0.3, adc, P.B_Cu)
        track(cx, 12.25, cx + 1.2, 12.25, 0.3, adc, P.B_Cu)
        # TAP_GND stitching vias at the bottom resistor and the cap
        via(cx, 16.7, "TAP_GND")
        track(cx, 15.45, cx, 16.7, 0.4, "TAP_GND", P.B_Cu)
        via(cx + 2.8, 13.45, "TAP_GND")
        track(cx + 2.8, 12.25, cx + 2.8, 13.45, 0.4, "TAP_GND", P.B_Cu)
        for dy, s_ in ((17.7, f"{plug}-{pin}"), (18.7, names.get((plug, pin))), (19.7, rb)):
            text(s_, cx + 0.3, dy, 0.8, P.B_SilkS, mirror=True)
        CONNS.append((sig, ("tail", plug, pin), (f"R{gpio}", 1)))
        CONNS.append((adc, (f"R{gpio + 50}", 1), ("U1", GPIO_PIN[gpio])))

    # one routed TAP_GND link from the DevKitC to the divider ground, so the
    # pours can't end up as islands
    g0 = taps_sorted[0][3]
    CONNS.append(("TAP_GND", ("U1", 22), (f"R{g0 + 50}", 2)))

    # U1: ESP32-S3-DevKitC-1, soldered on its two 1x22 pin headers ----------
    # Rows run along Y in the flag at the -X end. The antenna end (pin 1)
    # points at the ECU edge with no copper pour under it; the USB end
    # overhangs the harness-side edge so a cable plugs straight in.
    u1 = new_fp("U1", "ESP32-S3-DevKitC-1 N16R8", DK_X, DK_PIN1_Y - 10.5 * 2.54, "ESP32S3_DevKitC1_THT")
    tapped = {t[3]: t for t in TAPS}
    for pad, g in enumerate(DK_J1 + DK_J3, start=1):
        x, y = dk_xy(pad)
        nn = "TAP_GND" if g == "GND" else (f"ADC_GPIO{g:02d}" if g in tapped else None)
        pth(u1, pad, x, y, 1.7, 1.0, nn, P.PAD_SHAPE_RECT if pad in (1, 23) else P.PAD_SHAPE_CIRCLE)
        if g in tapped:
            lab = names.get((tapped[g][0], tapped[g][1]))
        elif isinstance(g, int):
            lab = None
        else:
            lab = g
        if lab:
            text(lab, x + (-3.0 if pad <= 22 else 3.0), y, 0.8)
    yb, yt = DK_PIN1_Y - 21 * 2.54 - 7.0, DK_PIN1_Y + 8.0
    rect(u1, DK_X - 12.7, yb, DK_X + 12.7, yt, P.F_Fab, 0.1)
    rect(u1, DK_X - 12.7, BOARD[1] + 0.6, DK_X + 12.7, BOARD[3] - 0.6, P.F_SilkS, 0.15)
    rect(u1, DK_X - 13.0, yb - 0.3, DK_X + 13.0, yt + 0.3, P.F_CrtYd, 0.05)
    fp_line(u1, DK_X - 12.7, DK_PIN1_Y + 2.0, DK_X + 12.7, DK_PIN1_Y + 2.0, P.F_SilkS, 0.15)
    u1.Reference().SetPosition(V(DK_X, -12.0))
    # J3 pin 1 (GND) sits in the antenna keep-out: run it out to the pour
    gx_, gy_ = dk_xy(23)
    track(gx_, gy_, gx_ - 1.9, gy_ - 1.9, 0.5, "TAP_GND", P.F_Cu)
    track(gx_ - 1.9, gy_ - 1.9, gx_ - 1.9, gy_ - 3.2, 0.5, "TAP_GND", P.F_Cu)
    board.Add(u1)
    text("ANTENNA", DK_X, DK_PIN1_Y + 4.5, 1.0)
    text("USB", DK_X, -40.5, 1.0)

    # silk legends
    text("HARNESS SIDE: OEM plugs click in here", -60, -23.6, 1.0)
    text("ECU SIDE", -20, 19.6, 1.0)
    text("junkr-obd  KZJ95 1KZ-TE PCB interposer  rev C", -60, -21.0, 1.0, P.B_SilkS, mirror=True)
    text("TAP_GND = ESP32 ground only", -60, -18.5, 0.8, P.B_SilkS, mirror=True)

    board.Save(os.path.join(HERE, "interposer_unrouted.kicad_pcb"))
    import json
    json.dump(dict(conns=CONNS, refs=refs), open(os.path.join(HERE, "conns.json"), "w"), indent=1)
    print("wrote interposer_unrouted.kicad_pcb + conns.json")


def add_zone(b, netname, layer, poly):
    z = P.ZONE(b)
    z.SetLayer(layer)
    z.SetNetCode(b.FindNet(netname).GetNetCode())
    ol = z.Outline()
    ol.NewOutline()
    for x, y in poly:
        ol.Append(P.FromMM(x + OX), P.FromMM(y + OY))
    z.SetLocalClearance(P.FromMM(0.3))
    z.SetMinThickness(P.FromMM(0.25))
    z.SetPadConnection(P.ZONE_CONNECTION_THERMAL)
    b.Add(z)


def finish():
    b = P.LoadBoard(os.path.join(HERE, "interposer_unrouted.kicad_pcb"))
    ses = os.path.join(HERE, "interposer.ses")
    if os.path.exists(ses):
        P.ImportSpecctraSES(b, ses)
    # TAP_GND pours under the plugs (ECU side), both layers
    x0, _, x1, y1 = BOARD
    poly = [(x0 + 0.4, 7.2), (-50.0, 7.2), (-50.0, y1 - 0.4), (x0 + 0.4, y1 - 0.4)]
    add_zone(b, "TAP_GND", P.F_Cu, poly)
    add_zone(b, "TAP_GND", P.B_Cu, poly)
    P.ZONE_FILLER(b).Fill(b.Zones())
    b.Save(os.path.join(HERE, "interposer.kicad_pcb"))
    print("wrote interposer.kicad_pcb")


if __name__ == "__main__":
    if "--finish" in sys.argv:
        finish()
    else:
        build()
