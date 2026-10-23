"""Correction tables: one 4-D grid per actuator, over (baro, intake temp, rpm, pedal).

The firmware does the same multilinear lookup (src/tap/piggyback.h), so a
table learned here behaves identically on the truck.
"""

from __future__ import annotations

import json
from dataclasses import dataclass, field

import numpy as np

# Axes. Baro covers sea level to ~3000 m; temp covers winter to desert.
AXES = {
    "baro": np.array([70.0, 85.0, 101.0]),          # kPa
    "iat": np.array([0.0, 25.0, 45.0]),             # C (intake air, THA)
    "rpm": np.array([800, 1200, 1600, 2000, 2400, 2800, 3200, 3600, 4000, 4400], float),
    "va": np.array([0, 15, 30, 45, 60, 75, 90, 100], float),  # pedal %
}
DIMS = ("baro", "iat", "rpm", "va")
SHAPE = tuple(len(AXES[d]) for d in DIMS)


@dataclass(frozen=True)
class Actuator:
    """One thing the piggyback can bend, with hard limits that no table may exceed."""

    name: str
    unit: str
    lo: float
    hi: float
    step: float          # max change per learning round, per cell
    probe: float         # dither amplitude used while learning
    doc: str


ACTUATORS = {
    # Added to the real PIM before the ECU sees it. +ve lets the smoke limiter
    # allow more fuel; -ve keeps a raised (mechanical) boost under the ECU's
    # ~15 psi boost cut. The firmware also clamps pim_seen below the cut.
    "pim_off": Actuator("pim_off", "kPa", -30.0, 25.0, 3.0, 2.0,
                        "offset added to PIM as seen by the ECU"),
    # Subtracted coolant temp (warm engine only). The ECU's timing map carries
    # a cold-advance term; a small negative offset borrows some of it.
    "thw_off": Actuator("thw_off", "C", -15.0, 0.0, 3.0, 3.0,
                        "offset added to THW as seen by the ECU (warm engine only)"),
}


# Safety tables learned alongside the actuators. Not optimised for score;
# they bound what the actuators may do in real time.
#   pim_rcap: max ratio pim_seen / pim. Learned from the wideband: when the ECU
#   is smoke-limited, real lambda = stock lambda * pim / pim_seen, so this cap
#   is (pessimistic stock lambda) / lambda_min. The firmware enforces it every
#   sample, which covers turbo-lag transients that a slow table cannot.
LIMITS = {"pim_rcap": (1.0, 1.20, 1.03)}   # lo, hi, default (small exploration allowance)


def _locate(axis: np.ndarray, x: np.ndarray):
    x = np.clip(x, axis[0], axis[-1])
    i = np.clip(np.searchsorted(axis, x, side="right") - 1, 0, len(axis) - 2)
    f = (x - axis[i]) / (axis[i + 1] - axis[i])
    return i, f


def corner_weights(q: dict[str, np.ndarray]):
    """Yield (flat_index, weight) for the 16 grid corners around each query point."""
    locs = [_locate(AXES[d], np.asarray(q[d], float)) for d in DIMS]
    for corner in range(1 << len(DIMS)):
        idx = []
        w = 1.0
        for k, (i, f) in enumerate(locs):
            bit = (corner >> k) & 1
            idx.append(i + bit)
            w = w * (f if bit else (1.0 - f))
        yield np.ravel_multi_index(tuple(idx), SHAPE), w


def lookup(grid: np.ndarray, q: dict[str, np.ndarray]) -> np.ndarray:
    flat = grid.ravel()
    out = 0.0
    for fi, w in corner_weights(q):
        out = out + flat[fi] * w
    return np.asarray(out, float)


@dataclass
class TableSet:
    grids: dict[str, np.ndarray] = field(default_factory=lambda: (
        {a: np.zeros(SHAPE) for a in ACTUATORS} | {k: np.full(SHAPE, v[2]) for k, v in LIMITS.items()}))

    def lookup(self, name: str, q: dict[str, np.ndarray]) -> np.ndarray:
        return lookup(self.grids[name], q)

    def clamp(self, limits: dict[str, tuple[float, float]] | None = None) -> "TableSet":
        for a, g in self.grids.items():
            lo, hi = (ACTUATORS[a].lo, ACTUATORS[a].hi) if a in ACTUATORS else LIMITS[a][:2]
            if limits and a in limits:
                lo, hi = max(lo, limits[a][0]), min(hi, limits[a][1])
            np.clip(g, lo, hi, out=g)
        return self

    def copy(self) -> "TableSet":
        return TableSet({a: g.copy() for a, g in self.grids.items()})

    # --- serialisation -------------------------------------------------
    def to_json(self) -> str:
        return json.dumps({
            "axes": {d: AXES[d].tolist() for d in DIMS},
            "dims": list(DIMS),
            "tables": {a: np.round(g, 3).tolist() for a, g in self.grids.items()},
        })

    @classmethod
    def from_json(cls, s: str) -> "TableSet":
        d = json.loads(s)
        if d["dims"] != list(DIMS) or any(d["axes"][k] != AXES[k].tolist() for k in DIMS):
            raise ValueError("table axes do not match this version of the tuner")
        return cls({a: np.array(v, float) for a, v in d["tables"].items()})

    def to_c_header(self) -> str:
        """Tables as a C++ header for src/tap/piggyback.h (pb::Axes, pb::Tables)."""
        def arr(a):
            vals = [repr(round(float(x), 4)) + "f" for x in np.ravel(a)]
            rows = [", ".join(vals[i:i + 10]) for i in range(0, len(vals), 10)]
            return "{" + ",\n        ".join(rows) + "}"
        lines = [
            "// Generated by `python -m tuner export`. Do not edit by hand.",
            "#pragma once",
            '#include "piggyback.h"',
            "",
            "namespace pb {",
            "static const Axes AXES = {",
            *[f"    {arr(AXES[d])},  // {d}" for d in DIMS],
            "};",
            "static const Tables TABLES = {",
            *[f"    {arr(self.grids[k])},  // {k}" for k in ("pim_off", "thw_off", "pim_rcap")],
            "};",
            "}  // namespace pb",
            "",
        ]
        return "\n".join(lines)
