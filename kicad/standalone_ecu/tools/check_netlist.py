#!/usr/bin/env python3
"""Compare the netlist KiCad reads from the schematic with circuit.py.

    kicad-cli sch export netlist --format kicadsexpr -o /tmp/ecu.net junkr_ecu.kicad_sch
    python3 check_netlist.py /tmp/ecu.net

Nets are compared as sets of (ref, pin). KiCad prefixes sheet-local labels
with the sheet path, so names are not compared, only the groupings.
"""
import sys

import circuit as CIR
from sexpr import parse, find, findall


def kicad_groups(path):
    tree = parse(open(path).read())[0]
    groups = {}
    for n in findall(find(tree, "nets"), "net"):
        name = str(find(n, "name")[1])
        nodes = frozenset((str(find(x, "ref")[1]), str(find(x, "pin")[1])) for x in findall(n, "node"))
        groups[nodes] = name
    return groups


def main(path):
    kg = kicad_groups(path)
    mine = {}
    for net, lst in CIR.c.nets.items():
        mine[frozenset(lst)] = net
    k_by_pin = {}
    for g, name in kg.items():
        for rp in g:
            k_by_pin[rp] = (name, g)
    bad = 0
    for g, net in mine.items():
        if g in kg:
            continue
        bad += 1
        rp = next(iter(g))
        kn = k_by_pin.get(rp)
        print("MISMATCH %s: mine %d pins, kicad %s %d pins" % (net, len(g), kn[0] if kn else None,
                                                                len(kn[1]) if kn else 0))
        if kn:
            print("   only mine:", sorted(g - kn[1])[:6], " only kicad:", sorted(kn[1] - g)[:6])
    # pins kicad connected that circuit leaves unconnected
    extra = [g for g in kg if len(g) > 1 and g not in mine]
    print("%d nets in circuit, %d in kicad, %d mismatched, %d extra kicad groups" % (
        len(mine), len(kg), bad, len(extra)))
    return bad


if __name__ == "__main__":
    sys.exit(1 if main(sys.argv[1]) else 0)
