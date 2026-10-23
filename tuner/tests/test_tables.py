import itertools
import unittest

import numpy as np

from tuner.tables import ACTUATORS, AXES, DIMS, LIMITS, SHAPE, TableSet, lookup


def q_at(idx):
    return {d: AXES[d][i] for d, i in zip(DIMS, idx)}


class LookupTest(unittest.TestCase):
    def setUp(self):
        self.g = np.random.default_rng(0).normal(size=SHAPE)

    def test_exact_at_nodes(self):
        rng = np.random.default_rng(1)
        for _ in range(50):
            idx = tuple(rng.integers(0, n) for n in SHAPE)
            self.assertAlmostEqual(float(lookup(self.g, q_at(idx))), self.g[idx], places=9)

    def test_linear_between_nodes(self):
        # a grid that is linear in every axis is reproduced exactly anywhere inside
        mesh = np.meshgrid(*[AXES[d] for d in DIMS], indexing="ij")
        lin = 0.5 * mesh[0] - 0.2 * mesh[1] + 0.01 * mesh[2] + 0.3 * mesh[3] + 7
        rng = np.random.default_rng(2)
        for _ in range(50):
            q = {d: rng.uniform(AXES[d][0], AXES[d][-1]) for d in DIMS}
            want = 0.5 * q["baro"] - 0.2 * q["iat"] + 0.01 * q["rpm"] + 0.3 * q["va"] + 7
            self.assertAlmostEqual(float(lookup(lin, q)), want, places=6)

    def test_clamps_outside_axes(self):
        lo = {d: AXES[d][0] - 1000 for d in DIMS}
        self.assertAlmostEqual(float(lookup(self.g, lo)), self.g[(0,) * 4])
        hi = {d: AXES[d][-1] + 1000 for d in DIMS}
        self.assertAlmostEqual(float(lookup(self.g, hi)), self.g[tuple(n - 1 for n in SHAPE)])

    def test_vectorised(self):
        q = {d: np.full(5, AXES[d][1]) for d in DIMS}
        out = lookup(self.g, q)
        self.assertEqual(out.shape, (5,))
        np.testing.assert_allclose(out, self.g[(1,) * 4])


class TableSetTest(unittest.TestCase):
    def test_defaults(self):
        ts = TableSet()
        for a in ACTUATORS:
            self.assertTrue(np.all(ts.grids[a] == 0))
        for k, (_, _, default) in LIMITS.items():
            self.assertTrue(np.all(ts.grids[k] == default))

    def test_clamp_to_hard_limits(self):
        ts = TableSet()
        for g in ts.grids.values():
            g[...] = 1e6
        ts.clamp()
        self.assertTrue(np.all(ts.grids["pim_off"] == ACTUATORS["pim_off"].hi))
        self.assertTrue(np.all(ts.grids["thw_off"] == ACTUATORS["thw_off"].hi))
        self.assertTrue(np.all(ts.grids["pim_rcap"] == LIMITS["pim_rcap"][1]))

    def test_json_round_trip(self):
        ts = TableSet()
        ts.grids["pim_off"][...] = np.random.default_rng(3).uniform(-5, 5, SHAPE)
        back = TableSet.from_json(ts.to_json())
        np.testing.assert_allclose(back.grids["pim_off"], ts.grids["pim_off"], atol=1e-3)

    def test_json_rejects_other_axes(self):
        s = TableSet().to_json().replace("4400.0", "4500.0")
        with self.assertRaises(ValueError):
            TableSet.from_json(s)

    def test_c_header_has_every_cell(self):
        h = TableSet().to_c_header()
        n_literals = h.count("f,") + h.count("f}")
        self.assertEqual(n_literals, 3 * int(np.prod(SHAPE)) + sum(SHAPE))


if __name__ == "__main__":
    unittest.main()
