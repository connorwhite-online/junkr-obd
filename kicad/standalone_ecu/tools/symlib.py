"""Load KiCad symbols (system libraries + our own) and describe their pins.

Symbols that `extends` another are flattened, because a schematic's
lib_symbols section needs complete symbols.
"""
import copy
import os

from sexpr import Q, parse, find, findall, dump

SYS = "/usr/share/kicad/symbols"
HERE = os.path.dirname(os.path.abspath(__file__))
OWN = os.path.join(HERE, "..", "lib", "junkr_ecu.kicad_sym")

_cache = {}


def _lib(path):
    if path not in _cache:
        tree = parse(open(path).read())[0]
        _cache[path] = {str(s[1]): s for s in findall(tree, "symbol")}
    return _cache[path]


def _libpath(lib):
    if lib == "junkr_ecu":
        return OWN
    return os.path.join(SYS, lib + ".kicad_sym")


def _rename_units(sym, old, new):
    for sub in findall(sym, "symbol"):
        nm = str(sub[1])
        if nm.startswith(old + "_"):
            sub[1] = Q(new + nm[len(old):])


def load(lib_id):
    """Return a flattened copy of the symbol, named lib_id."""
    lib, name = lib_id.split(":", 1)
    syms = _lib(_libpath(lib))
    sym = copy.deepcopy(syms[name])
    ext = find(sym, "extends")
    if ext:
        parent = load(lib + ":" + str(ext[1]))
        pname = str(ext[1])
        out = [e for e in parent if not (isinstance(e, list) and e and e[0] == "property")]
        out[1] = Q(name)
        props = findall(sym, "property")
        # keep parent props the child does not override
        child_keys = {str(p[1]) for p in props}
        pprops = [p for p in findall(parent, "property") if str(p[1]) not in child_keys]
        idx = 2
        while idx < len(out) and not (isinstance(out[idx], list) and out[idx][0] == "symbol"):
            idx += 1
        out = out[:idx] + props + pprops + out[idx:]
        _rename_units(out, pname.split(":")[-1], name)
        sym = out
    sym[1] = Q(lib_id)
    return sym


class Pin:
    __slots__ = ("number", "name", "unit", "x", "y", "angle", "length", "etype", "hidden")

    def __repr__(self):
        return "Pin(%s %s u%d)" % (self.number, self.name, self.unit)


def pins(sym):
    """All pins of a flattened symbol. unit 0 = shared by all units."""
    name = str(sym[1]).split(":", 1)[1]
    out = []
    for sub in findall(sym, "symbol"):
        nm = str(sub[1])
        parts = nm[len(name) + 1:].split("_")
        unit = int(parts[0])
        for p in findall(sub, "pin"):
            pin = Pin()
            pin.etype = str(p[1])
            at = find(p, "at")
            pin.x, pin.y = float(at[1]), float(at[2])
            pin.angle = int(float(at[3])) if len(at) > 3 else 0
            pin.length = float(find(p, "length")[1])
            pin.name = str(find(p, "name")[1])
            pin.number = str(find(p, "number")[1])
            pin.unit = unit
            pin.hidden = "hide" in p
            out.append(pin)
    return out


def units(sym):
    us = {p.unit for p in pins(sym)}
    us.discard(0)
    return sorted(us) or [1]


def bbox(sym, unit):
    """Bounding box (lib coords, y up) of graphics + pin ends for a unit."""
    name = str(sym[1]).split(":", 1)[1]
    xs, ys = [], []
    for sub in findall(sym, "symbol"):
        u = int(str(sub[1])[len(name) + 1:].split("_")[0])
        if u not in (0, unit):
            continue
        for g in sub:
            if not isinstance(g, list):
                continue
            if g[0] == "rectangle":
                s, e = find(g, "start"), find(g, "end")
                xs += [float(s[1]), float(e[1])]
                ys += [float(s[2]), float(e[2])]
            elif g[0] == "polyline":
                for xy in findall(find(g, "pts"), "xy"):
                    xs.append(float(xy[1]))
                    ys.append(float(xy[2]))
            elif g[0] == "circle":
                c = find(g, "center")
                r = float(find(g, "radius")[1])
                xs += [float(c[1]) - r, float(c[1]) + r]
                ys += [float(c[2]) - r, float(c[2]) + r]
            elif g[0] == "pin":
                at = find(g, "at")
                xs.append(float(at[1]))
                ys.append(float(at[2]))
    if not xs:
        return (-2.54, -2.54, 2.54, 2.54)
    return (min(xs), min(ys), max(xs), max(ys))


def prop(sym, key):
    for p in findall(sym, "property"):
        if str(p[1]) == key:
            return str(p[2])
    return ""


# ---------------------------------------------------------------------------
# Building our own symbols
# ---------------------------------------------------------------------------
def _eff(size=1.27, hide=False, justify=None):
    e = ["effects", ["font", ["size", size, size]]]
    if justify:
        e.append(["justify"] + justify)
    if hide:
        e.append("hide")
    return e


def make_symbol(name, ref, value, footprint, unit_pins, desc="", datasheet="~", width=None):
    """unit_pins: list (one per unit) of lists of (number, name, side, etype).

    side is L or R. Pins are spaced 2.54 mm. Returns the sexpr list.
    """
    sym = ["symbol", Q(name), ["in_bom", "yes"], ["on_board", "yes"]]
    if len(unit_pins) > 1:
        sym.insert(2, ["pin_names", ["offset", 1.016]])
    sub_graphics = []
    sub_pins = []
    maxh = 0
    for u, plist in enumerate(unit_pins, start=1):
        left = [p for p in plist if p[2] == "L"]
        right = [p for p in plist if p[2] == "R"]
        rows = max(len(left), len(right), 1)
        h = rows * 2.54
        maxh = max(maxh, h)
        nl = max([len(p[1]) for p in left] + [0])
        nr = max([len(p[1]) for p in right] + [0])
        w = width or max(10.16, round((nl + nr) * 1.0 / 2.54 + 2) * 2.54)
        top = h / 2
        top = round(top / 1.27) * 1.27
        g = ["symbol", Q("%s_%d_1" % (name, u)),
             ["rectangle", ["start", -w / 2, top + 1.27], ["end", w / 2, top - h - 1.27 + 2.54],
              ["stroke", ["width", 0.254], ["type", "default"]], ["fill", ["type", "background"]]]]
        for side, lst in (("L", left), ("R", right)):
            for i, (num, pname, _s, et) in enumerate(lst):
                y = top - i * 2.54
                x = -w / 2 - 5.08 if side == "L" else w / 2 + 5.08
                ang = 0 if side == "L" else 180
                g.append(["pin", et, "line", ["at", round(x, 3), round(y, 3), ang], ["length", 5.08],
                          ["name", Q(pname), _eff()], ["number", Q(str(num)), _eff()]])
        sub_pins.append(g)
    top = maxh / 2 + 3.81
    sym += [["property", Q("Reference"), Q(ref), ["at", 0, round(top, 2), 0], _eff()],
            ["property", Q("Value"), Q(value), ["at", 0, round(-top, 2), 0], _eff()],
            ["property", Q("Footprint"), Q(footprint), ["at", 0, 0, 0], _eff(hide=True)],
            ["property", Q("Datasheet"), Q(datasheet), ["at", 0, 0, 0], _eff(hide=True)],
            ["property", Q("ki_description"), Q(desc), ["at", 0, 0, 0], _eff(hide=True)]]
    sym += sub_pins
    return sym


def write_lib(symbols, path=OWN):
    tree = ["kicad_symbol_lib", ["version", 20220914], ["generator", "junkr_gen"]] + symbols
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write(dump(tree) + "\n")
    _cache.pop(path, None)
