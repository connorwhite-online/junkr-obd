"""Tiny S-expression reader/writer for KiCad files.

Atoms are kept as Python strings. Quoted strings are wrapped in Q so they
round-trip with their quotes.
"""
import re


class Q(str):
    """A quoted string atom."""


_TOK = re.compile(r'\s*(?:(\()|(\))|"((?:[^"\\]|\\.)*)"|([^\s()"]+))', re.S)


def parse(text):
    stack = [[]]
    pos = 0
    n = len(text)
    while pos < n:
        m = _TOK.match(text, pos)
        if not m:
            if text[pos:].strip() == "":
                break
            raise ValueError("bad sexpr at %d: %r" % (pos, text[pos:pos + 40]))
        pos = m.end()
        if m.group(1):
            stack.append([])
        elif m.group(2):
            item = stack.pop()
            stack[-1].append(item)
        elif m.group(3) is not None:
            stack[-1].append(Q(m.group(3).replace('\\"', '"').replace('\\\\', '\\')))
        else:
            stack[-1].append(m.group(4))
    return stack[0]


def _atom(a):
    if isinstance(a, Q):
        return '"' + a.replace('\\', '\\\\').replace('"', '\\"') + '"'
    if isinstance(a, float):
        s = ("%.4f" % a).rstrip("0").rstrip(".")
        return s if s not in ("-0", "") else "0"
    return str(a)


def dump(x, indent=0):
    """Serialise a nested list. Short lists stay on one line."""
    if not isinstance(x, list):
        return _atom(x)
    flat = "(" + " ".join(dump(e, 0) if not isinstance(e, list) else _flat(e) for e in x) + ")"
    if len(flat) < 100 and flat.count("(") < 6:
        return flat
    pad = "  " * (indent + 1)
    head = []
    rest = []
    for e in x:
        if isinstance(e, list) and (rest or head):
            rest.append(e)
        elif rest:
            rest.append(e)
        else:
            head.append(e)
    s = "(" + " ".join(_atom(h) for h in head)
    for e in rest:
        s += "\n" + pad + dump(e, indent + 1)
    return s + "\n" + "  " * indent + ")"


def _flat(x):
    return "(" + " ".join(_flat(e) if isinstance(e, list) else _atom(e) for e in x) + ")"


def find(x, key):
    """First child list whose head is key."""
    for e in x:
        if isinstance(e, list) and e and e[0] == key:
            return e
    return None


def findall(x, key):
    return [e for e in x if isinstance(e, list) and e and e[0] == key]
