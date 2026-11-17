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
import shutil
import subprocess
import zipfile

import pcbnew as P

import circuit as CIR

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
FAB = os.path.join(ROOT, "fab")
BOARD = os.environ.get("FAB_BOARD", os.path.join(ROOT, "junkr_ecu.kicad_pcb"))

HAND = ("Relay_THT", "junkr_ecu", "Connector_JST", "Connector_PinHeader", "Connector_Molex", "MountingHole")


def gerbers():
    gdir = os.path.join(FAB, "gerbers")
    shutil.rmtree(gdir, ignore_errors=True)
    os.makedirs(gdir, exist_ok=True)
    layers = "F.Cu,In1.Cu,In2.Cu,In3.Cu,In4.Cu,B.Cu,F.Paste,B.Paste,F.SilkS,B.SilkS,F.Mask,B.Mask,Edge.Cuts"
    subprocess.check_call(["kicad-cli", "pcb", "export", "gerbers", "-o", gdir + "/", "-l", layers,
                           "--subtract-soldermask", BOARD], stdout=subprocess.DEVNULL)
    subprocess.check_call(["kicad-cli", "pcb", "export", "drill", "-o", gdir + "/", "--format", "excellon",
                           "--excellon-separate-th", "--generate-map", "--map-format", "gerberx2", BOARD],
                          stdout=subprocess.DEVNULL)
    zpath = os.path.join(FAB, "junkr_ecu_gerbers.zip")
    with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
        # Layers and drills only, at the top level: JLC's parser chokes on extras.
        for f in sorted(os.listdir(gdir)):
            if f.endswith(".gbrjob") or "drl_map" in f:
                continue
            z.write(os.path.join(gdir, f), f)
    return zpath


def sourcing():
    """(Comment, Footprint) -> row of sourcing.csv: the exact MPN, LCSC number and buy link."""
    with open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "sourcing.csv")) as f:
        return {(r["Comment"], r["Footprint"]): r for r in csv.DictReader(f)}


TAILS = ("J2", "J3", "J4")   # hand-made shadow-mode pigtails, never fab assembled

# JLCPCB upload variants. "jlcpcb" is the board as designed. "jlcpcb_lean" is the
# cheaper build: same values, ratings and packages on cheaper preferred parts,
# all status LEDs red (a basic part), and the SWD/UPDI fallback headers left
# off (flashing is over USB-C; fit them by hand if a chip is ever bricked).
VARIANTS = {
    "jlcpcb": {"swap": {}, "dnp": TAILS},
    "jlcpcb_lean": {
        "swap": {
            ("SS310", "D_SMA"): ("SS310", "C7420364"),
            ("BAT54S", "SOT-23"): ("BAT54S", "C7420333"),
            ("BZT52C12", "D_SOD-123"): ("BZT52C12", "C19077410"),
            ("BSS123", "SOT-23"): ("BSS123", "C7420338"),
            ("green", "LED_0603_1608Metric"): ("KT-0603R (red)", "C2286"),
            ("yellow", "LED_0603_1608Metric"): ("KT-0603R (red)", "C2286"),
            ("blue", "LED_0603_1608Metric"): ("KT-0603R (red)", "C2286"),
        },
        "dnp": TAILS + ("J10", "J12"),
    },
}


# Body centre relative to the footprint origin (KiCad axes, unrotated), for
# footprints whose origin is not the part centre JLC's models use.
CPL_OFFSET = {"Relay_DPDT_Omron_G5V-2": (3.81, 8.9)}


def bom_cpl():
    SRC = sourcing()
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
        off = CPL_OFFSET.get(str(fp.GetFPID().GetLibItemName()))
        if off:   # footprint origin is pin 1; JLC places by body centre
            import math
            a = math.radians(fp.GetOrientationDegrees())
            dx, dy = off
            pos = P.VECTOR2I(pos.x + P.FromMM(dx * math.cos(a) + dy * math.sin(a)),
                             pos.y + P.FromMM(-dx * math.sin(a) + dy * math.cos(a)))
        side = "bottom" if fp.IsFlipped() else "top"
        cpl.append((ref, P.ToMM(pos.x), -P.ToMM(pos.y), side, fp.GetOrientationDegrees()))
    os.makedirs(FAB, exist_ok=True)
    with open(os.path.join(FAB, "bom.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["Comment", "Designator", "Footprint", "MPN", "LCSC", "Qty", "Assembly", "Buy link", "Note"])
        for (value, fpn, mpn), g in groups.items():
            src = SRC.get((value, fpn.split(":")[1]), {})
            w.writerow([value, ",".join(g["refs"]), fpn.split(":")[1], src.get("MPN") or mpn, src.get("LCSC", ""),
                        len(g["refs"]), "hand solder" if g["hand"] else "SMT", src.get("Buy link", ""), g["note"]])
    # JLCPCB turnkey assembly: their BOM and CPL formats, one folder per variant
    for name, var in VARIANTS.items():
        jdir = os.path.join(FAB, name)
        os.makedirs(jdir, exist_ok=True)
        rows = collections.OrderedDict()   # merge lines that end up on the same LCSC part
        for (value, fpn, mpn), g in groups.items():
            refs = [r for r in g["refs"] if r not in var["dnp"]]
            if not refs:
                continue
            fps = fpn.split(":")[1]
            src = SRC.get((value, fps), {})
            part, lcsc = var["swap"].get((value, fps), (src.get("MPN") or mpn or value, src.get("LCSC", "")))
            row = rows.setdefault((lcsc or part, fps), [part, [], fps, lcsc])
            row[1] += refs
        with open(os.path.join(jdir, "bom_jlcpcb.csv"), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["Comment", "Designator", "Footprint", "LCSC Part #"])
            for part, refs, fps, lcsc in rows.values():
                w.writerow([part, ",".join(refs), fps, lcsc])
        with open(os.path.join(jdir, "cpl_jlcpcb.csv"), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
            for ref, x, y, side, rot in cpl:
                if ref not in var["dnp"]:
                    w.writerow([ref, "%.3fmm" % x, "%.3fmm" % y, side.capitalize(), "%.1f" % rot])
    return groups


def main():
    z = gerbers()
    groups = bom_cpl()
    n = sum(len(g["refs"]) for g in groups.values())
    print("gerbers:", z)
    print("bom: %d lines, %d parts" % (len(groups), n))


if __name__ == "__main__":
    main()
