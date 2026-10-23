"""Cross-check src/tap/piggyback.h against the Python simulator's Piggyback.

Builds a tiny host program with g++ against tables exported from Python and
compares outputs on random inputs. Skipped when g++ is not installed.
"""

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

import numpy as np

from tuner.channels import Capabilities
from tuner.sim import BOOST_CUT_KPA_ABS, Piggyback
from tuner.tables import ACTUATORS, AXES, LIMITS, SHAPE, TableSet

REPO = Path(__file__).resolve().parents[2]

MAIN = r"""
#include "tables_gen.h"
#include <cstdio>
int main() {
    pb::Limits L;
    L.max_fuel_gain = %(gain)ff;
    L.boost_cut_kpa = %(cut)ff;
    float t, rpm, va, pim, tha, thw, baro; int learning;
    while (scanf("%%d %%f %%f %%f %%f %%f %%f %%f", &learning, &t, &rpm, &va, &pim, &tha, &thw, &baro) == 8) {
        pb::Output o = pb::apply(pb::AXES, pb::TABLES, L, learning, t, rpm, va, pim, tha, thw, baro);
        printf("%%.5f %%.5f %%.5f %%.5f\n", o.pim_seen, o.thw_seen, o.pim_tbl, o.thw_tbl);
    }
}
"""


@unittest.skipUnless(shutil.which("g++"), "g++ not installed")
class FirmwareMatchesPython(unittest.TestCase):
    def check(self, caps: Capabilities, seed: int):
        rng = np.random.default_rng(seed)
        ts = TableSet()
        ts.grids["pim_off"] = rng.uniform(ACTUATORS["pim_off"].lo - 5, ACTUATORS["pim_off"].hi + 5, SHAPE)
        ts.grids["thw_off"] = rng.uniform(ACTUATORS["thw_off"].lo - 5, 3, SHAPE)
        ts.grids["pim_rcap"] = rng.uniform(0.95, LIMITS["pim_rcap"][1], SHAPE)
        ts.grids = {k: np.round(g, 4) for k, g in ts.grids.items()}   # what the header stores

        with tempfile.TemporaryDirectory() as d:
            d = Path(d)
            (d / "tables_gen.h").write_text(ts.to_c_header())
            (d / "main.cpp").write_text(MAIN % {"gain": caps.max_fuel_gain, "cut": BOOST_CUT_KPA_ABS})
            subprocess.run(["g++", "-std=c++17", "-O1", "-Wall", "-Werror", f"-I{REPO / 'src/tap'}", f"-I{d}",
                            "-o", str(d / "pb"), str(d / "main.cpp")], check=True)
            rows = []
            for _ in range(400):
                rows.append((int(rng.integers(0, 2)), rng.uniform(0, 100), rng.uniform(600, 4600),
                             rng.uniform(-5, 105), rng.uniform(95, 230), rng.uniform(-10, 50),
                             rng.uniform(60, 100), rng.uniform(65, 104)))
            stdin = "\n".join(" ".join(f"{x:.4f}" if isinstance(x, float) else str(x) for x in r) for r in rows)
            out = subprocess.run([str(d / "pb")], input=stdin, capture_output=True, text=True, check=True).stdout

        got = np.array([[float(x) for x in line.split()] for line in out.strip().splitlines()])
        self.assertEqual(len(got), len(rows))
        for r, g in zip(rows, got):
            learning, t, rpm, va, pim, tha, thw, baro = (round(x, 4) if isinstance(x, float) else x for x in r)
            p = Piggyback(ts, caps, learning=bool(learning))
            pim_seen, thw_seen, u, tbl, *_ = p.apply(t, rpm, va, pim, tha, thw, baro)
            want = [pim_seen, thw_seen, tbl["pim_off"], tbl["thw_off"]]
            np.testing.assert_allclose(g, want, atol=2e-3, err_msg=f"inputs {r}")

    def test_full_sensors(self):
        self.check(Capabilities(frozenset({"wideband", "egt"})), 0)

    def test_one_sensor(self):
        self.check(Capabilities(frozenset({"egt"})), 1)

    def test_no_sensors(self):
        self.check(Capabilities(frozenset()), 2)


if __name__ == "__main__":
    unittest.main()
