#!/usr/bin/env python3
"""Fabrication outputs from the routed board: Gerbers, drills, BOM, CPL.

    /usr/bin/python3 gen_fab.py   ->  ../fab/{gerbers.zip, bom.csv, cpl.csv}

The BOM groups parts by value + footprint + MPN and is shaped for JLCPCB /
PCBWay assembly (Comment, Designator, Footprint, MPN). Through-hole parts
(relays, header, tails) are listed too but marked for hand soldering.
"""
import collections
import csv
import os
import subprocess
import zipfile

import pcbnew as P

import circuit as CIR

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
FAB = os.path.join(ROOT, "fab")
BOARD = os.path.join(ROOT, "junkr_ecu.kicad_pcb")

HAND = ("Relay_THT", "junkr_ecu", "Connector_JST", "Connector_PinHeader", "Connector_Molex", "MountingHole")


def gerbers():
    gdir = os.path.join(FAB, "gerbers")
    os.makedirs(gdir, exist_ok=True)
    layers = "F.Cu,In1.Cu,In2.Cu,In3.Cu,In4.Cu,B.Cu,F.Paste,B.Paste,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts"
    subprocess.check_call(["kicad-cli", "pcb", "export", "gerbers", "-o", gdir + "/", "-l", layers,
                           "--subtract-soldermask", BOARD], stdout=subprocess.DEVNULL)
    subprocess.check_call(["kicad-cli", "pcb", "export", "drill", "-o", gdir + "/", "--format", "excellon",
                           "--excellon-separate-th", "--generate-map", "--map-format", "gerberx2", BOARD],
                          stdout=subprocess.DEVNULL)
    zpath = os.path.join(FAB, "junkr_ecu_gerbers.zip")
    with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
        for f in sorted(os.listdir(gdir)):
            z.write(os.path.join(gdir, f), f)
    return zpath


def bom_cpl():
    board = P.LoadBoard(BOARD)
    parts = {p.ref: p for p in CIR.c.parts}
    groups = collections.OrderedDict()
    cpl = []
    for fp in sorted(board.GetFootprints(), key=lambda f: f.GetReference()):
        ref = fp.GetReference()
        if ref not in parts:
            continue
        p = parts[ref]
        lib = p.fp.split(":")[0]
        mpn = p.fields.get("MPN", "")
        note = CIR.MPN.get(p.value, ("", ""))[1]
        key = (p.value, p.fp, mpn)
        g = groups.setdefault(key, {"refs": [], "hand": lib in HAND or lib.startswith("Connector") and "SMD" not in p.fp,
                                    "note": note, "sheet": p.sheet})
        g["refs"].append(ref)
        pos = fp.GetPosition()
        side = "bottom" if fp.IsFlipped() else "top"
        cpl.append((ref, P.ToMM(pos.x), -P.ToMM(pos.y), side, fp.GetOrientationDegrees()))
    os.makedirs(FAB, exist_ok=True)
    with open(os.path.join(FAB, "bom.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Comment", "Designator", "Footprint", "MPN", "Qty", "Assembly", "Note"])
        for (value, fpn, mpn), g in groups.items():
            w.writerow([value, ",".join(g["refs"]), fpn.split(":")[1], mpn, len(g["refs"]),
                        "hand solder" if g["hand"] else "SMT", g["note"]])
    with open(os.path.join(FAB, "cpl.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
        for ref, x, y, side, rot in cpl:
            w.writerow([ref, "%.3fmm" % x, "%.3fmm" % y, side, "%.1f" % rot])
    return groups


def main():
    z = gerbers()
    groups = bom_cpl()
    n = sum(len(g["refs"]) for g in groups.values())
    print("gerbers:", z)
    print("bom: %d lines, %d parts" % (len(groups), n))


if __name__ == "__main__":
    main()
