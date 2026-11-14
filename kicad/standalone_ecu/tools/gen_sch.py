#!/usr/bin/env python3
"""Write the KiCad 7 schematic (root + one sheet per block) from circuit.py.

Every pin gets a net label at its end, so the drawing is "label style": each
block is a column of parts with their nets written next to the pins. The
netlist KiCad reads back from these files must equal circuit.py's (see
check_netlist.py).
"""
import collections
import os
import uuid

import circuit as CIR
import symlib
from sexpr import Q, dump

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
PROJECT = "junkr_ecu"
NS = uuid.UUID("6f1c5a8e-2b7d-4c1e-9a55-1d2f0c9b7e11")

SHEETS = [  # file stem, title
    ("connectors", "ECU header and stock-ECU tails"),
    ("takeover", "Takeover relays (shadow mode)"),
    ("power", "Power"),
    ("mcu", "STM32H743 engine CPU"),
    ("analog", "Analog inputs"),
    ("digital_in", "VR, edge and switch inputs"),
    ("outputs", "Low-side outputs, M-REL, SVR"),
    ("injection", "Spill valve and timing valve drivers"),
    ("safety", "Safety supervisor and ARM chain"),
    ("comms", "ESP32-S3, USB, microSD, AUX"),
]
PAPER = {"A4": (297, 210), "A3": (420, 297), "A2": (594, 420), "A1": (841, 594)}
POWER_NETS = {"GND", "3V3", "3V3A", "3V3_RF", "V5", "V5_OR", "VIN", "PB", "VBUS", "VREF", "VC", "VRLY"}


def uid(*parts):
    return Q(str(uuid.uuid5(NS, "/".join(str(p) for p in parts))))


def eff(size=1.27, justify=None, hide=False):
    e = ["effects", ["font", ["size", size, size]]]
    if justify:
        e.append(["justify"] + justify)
    if hide:
        e.append("hide")
    return e


def snap(v, g=2.54):
    return round(round(v / g) * g, 4)


ROOT_UUID = uid("root")


def net_sheets():
    ns = collections.defaultdict(set)
    sheet_of = {p.ref: p.sheet for p in CIR.c.parts}
    for net, lst in CIR.c.nets.items():
        for ref, _ in lst:
            ns[net].add(sheet_of[ref])
    return ns


def label_len(text):
    return len(text) * 1.15 + 4.5


def build_sheet(stem, title, page_no, nsheets):
    parts = [p for p in CIR.c.parts if p.sheet == stem]
    lib_ids = []
    items = []      # (part, unit, sym, pins of unit, bbox)
    for p in parts:
        sym = symlib.load(p.lib)
        if p.lib not in lib_ids:
            lib_ids.append(p.lib)
        allp = symlib.pins(sym)
        for u in symlib.units(sym):
            up = [x for x in allp if x.unit in (0, u)]
            items.append((p, u, sym, up, symlib.bbox(sym, u)))

    conns = CIR.c.conns
    # footprint of each item incl. labels: (w, h, ox, oy) where ox/oy = origin offset
    sized = []
    for p, u, sym, up, (x0, y0, x1, y1) in items:
        lm = rm = tm = bm = 3.0
        for pin in up:
            net = conns[p.ref].get(pin.number)
            if not net:
                continue
            L = label_len(net)
            if pin.angle == 0:
                lm = max(lm, L)
            elif pin.angle == 180:
                rm = max(rm, L)
            elif pin.angle == 90:
                bm = max(bm, L)
            else:
                tm = max(tm, L)
        w = (x1 - x0) + lm + rm + 4
        h = (y1 - y0) + tm + bm + 6
        sized.append((w, h, lm - x0 + 2, tm + y1 + 4))

    # paper: smallest that fits with shelf packing
    for paper in ("A3", "A2", "A1"):
        PW, PH = PAPER[paper]
        x, y, rowh = 15.0, 25.0, 0.0
        pos = []
        ok = True
        for (w, h, ox, oy) in sized:
            if x + w > PW - 15:
                x, y, rowh = 15.0, y + rowh, 0.0
            if y + h > PH - 40:
                ok = False
                break
            pos.append((snap(x + ox), snap(y + oy)))
            x += w
            rowh = max(rowh, h)
        if ok:
            break

    sheet_path = "/%s/%s" % (ROOT_UUID, uid("sheet", stem))
    ns = net_sheets()
    body = []
    done_pts = set()
    for (p, u, sym, up, bb), (sx, sy) in zip(items, pos):
        props = [
            ["property", Q("Reference"), Q(p.ref), ["at", sx, round(sy - bb[3] - 1.5, 3), 0], eff(justify=["left"])],
            ["property", Q("Value"), Q(p.value), ["at", sx, round(sy - bb[1] + 1.8, 3), 0], eff(justify=["left"])],
            ["property", Q("Footprint"), Q(p.fp), ["at", sx, sy, 0], eff(hide=True)],
            ["property", Q("Datasheet"), Q(symlib.prop(sym, "Datasheet") or "~"), ["at", sx, sy, 0], eff(hide=True)],
        ]
        for k, v in p.fields.items():
            props.append(["property", Q(k), Q(str(v)), ["at", sx, sy, 0], eff(hide=True)])
        s = ["symbol", ["lib_id", Q(p.lib)], ["at", sx, sy, 0], ["unit", u], ["in_bom", "yes"],
             ["on_board", "yes"], ["dnp", "no"], ["uuid", uid("sym", p.ref, u)]] + props
        for pin in symlib.pins(sym):
            if pin.unit in (0, u):
                s.append(["pin", Q(pin.number), ["uuid", uid("pin", p.ref, pin.number, u)]])
        s.append(["instances", ["project", Q(PROJECT), ["path", Q(sheet_path), ["reference", Q(p.ref)], ["unit", u]]]])
        body.append(s)
        # labels / no-connects at pin ends
        for pin in up:
            px, py = round(sx + pin.x, 3), round(sy - pin.y, 3)
            key = (px, py)
            if key in done_pts:
                continue
            net = conns[p.ref].get(pin.number)
            if pin.hidden and net is None:
                continue
            done_pts.add(key)
            if net is None:
                body.append(["no_connect", ["at", px, py], ["uuid", uid("nc", p.ref, pin.number, u)]])
                continue
            ang = {0: 180, 180: 0, 90: 270, 270: 90}[pin.angle]
            just = {0: ["left"], 180: ["right"], 90: ["right"], 270: ["left"]}[ang]
            glob = len(ns[net]) > 1 or net in POWER_NETS
            if glob:
                shape = "passive"
                body.append(["global_label", Q(net), ["shape", shape], ["at", px, py, ang],
                             ["fields_autoplaced"], eff(justify=just), ["uuid", uid("gl", p.ref, pin.number, u)],
                             ["property", Q("Intersheetrefs"), Q("${INTERSHEET_REFS}"), ["at", px, py, 0],
                              eff(hide=True)]])
            else:
                body.append(["label", Q(net), ["at", px, py, ang], ["fields_autoplaced"],
                             eff(justify=just + ["bottom"] if ang in (0, 180) else just),
                             ["uuid", uid("lb", p.ref, pin.number, u)]])
    libs = []
    for lid in lib_ids:
        libs.append(symlib.load(lid))
    tree = ["kicad_sch", ["version", 20230121], ["generator", "eeschema"], ["uuid", uid("sheetfile", stem)],
            ["paper", Q(paper)],
            ["title_block", ["title", Q("Junkr ECU: " + title)], ["date", Q("2026-11-13")], ["rev", Q("A")],
             ["company", Q("junkr-obd")],
             ["comment", 1, Q("Standalone ECU for the 1996 KZJ95 Prado 1KZ-TE (ECD-V3), automatic")],
             ["comment", 2, Q("Generated by kicad/standalone_ecu/tools/gen_sch.py from circuit.py. Edit circuit.py, not this file.")]],
            ["lib_symbols"] + libs] + body
    tree.append(["sheet_instances", ["path", Q("/"), ["page", Q(str(page_no))]]])
    with open(os.path.join(OUT, stem + ".kicad_sch"), "w") as f:
        f.write(dump(tree) + "\n")
    return paper, len(parts)


def build_root():
    body = []
    for i, (stem, title) in enumerate(SHEETS):
        col, row = i % 5, i // 5
        x, y = 20 + col * 78, 40 + row * 60
        body.append(["sheet", ["at", x, y], ["size", 66, 40], ["fields_autoplaced"],
                     ["stroke", ["width", 0.1524], ["type", "solid"]], ["fill", ["color", 0, 0, 0, 0.0]],
                     ["uuid", uid("sheet", stem)],
                     ["property", Q("Sheetname"), Q(title), ["at", x, y - 0.7, 0], eff(justify=["left", "bottom"])],
                     ["property", Q("Sheetfile"), Q(stem + ".kicad_sch"), ["at", x, y + 40.6, 0],
                      eff(justify=["left", "top"])],
                     ["instances", ["project", Q(PROJECT), ["path", Q("/" + str(ROOT_UUID)), ["page", Q(str(i + 2))]]]]])
    notes = [
        "Junkr ECU rev A: plug-in standalone ECU for the 1KZ-TE (Denso ECD-V3 pump), KZJ95 Prado A/T.",
        "Plugs into the stock harness through the same TE 176122-6 header. Optional tails keep the stock ECU",
        "connected (shadow mode): inputs are shared high-impedance, outputs go through relays whose NC side",
        "is the stock ECU. ARM low (fault, watchdog, RUN switch off, power loss) = stock ECU drives again.",
        "Design notes: docs/standalone-ecu/DESIGN.md. Items marked [VERIFY] are unconfirmed.",
    ]
    for k, t in enumerate(notes):
        body.append(["text", Q(t), ["at", 20, 170 + k * 6, 0], eff(size=2.0, justify=["left", "bottom"]),
                     ["uuid", uid("note", k)]])
    tree = ["kicad_sch", ["version", 20230121], ["generator", "eeschema"], ["uuid", ROOT_UUID],
            ["paper", Q("A3")],
            ["title_block", ["title", Q("Junkr ECU rev A")], ["date", Q("2026-11-13")], ["rev", Q("A")],
             ["company", Q("junkr-obd")]],
            ["lib_symbols"]] + body + [["sheet_instances", ["path", Q("/"), ["page", Q("1")]]]]
    with open(os.path.join(OUT, PROJECT + ".kicad_sch"), "w") as f:
        f.write(dump(tree) + "\n")


def build_tables():
    with open(os.path.join(OUT, "sym-lib-table"), "w") as f:
        f.write('(sym_lib_table\n  (lib (name "junkr_ecu")(type "KiCad")(uri "${KIPRJMOD}/lib/junkr_ecu.kicad_sym")(options "")(descr "Junkr ECU parts"))\n)\n')
    with open(os.path.join(OUT, "fp-lib-table"), "w") as f:
        f.write('(fp_lib_table\n  (lib (name "junkr_ecu")(type "KiCad")(uri "${KIPRJMOD}/lib/junkr_ecu.pretty")(options "")(descr "Junkr ECU footprints"))\n)\n')


if __name__ == "__main__":
    issues = CIR.check()
    if issues:
        for i in issues:
            print("CHECK:", i)
        raise SystemExit(1)
    build_tables()
    build_root()
    for i, (stem, title) in enumerate(SHEETS):
        paper, n = build_sheet(stem, title, i + 2, len(SHEETS))
        print("%-12s %s %3d parts" % (stem, paper, n))
