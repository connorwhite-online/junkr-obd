import unittest

import numpy as np

from tuner.channels import Capabilities
from tuner.sim import BOOST_CUT_KPA_ABS, Piggyback
from tuner.tables import ACTUATORS, TableSet

ALL = Capabilities(frozenset({"wideband", "egt"}))
NONE = Capabilities(frozenset())


def pb(caps, pim=0.0, thw=0.0, rcap=None, learning=False):
    ts = TableSet()
    ts.grids["pim_off"][...] = pim
    ts.grids["thw_off"][...] = thw
    if rcap is not None:
        ts.grids["pim_rcap"][...] = rcap
    return Piggyback(ts, caps, learning=learning)


def run(p, pim=170.0, thw=88.0, t=0.5):
    return p.apply(t, 2400, 100, pim, 30.0, thw, 101.0)


class SafetyTest(unittest.TestCase):
    def test_no_sensors_never_adds_fuel(self):
        pim_seen, *_ = run(pb(NONE, pim=25, learning=True))
        self.assertLessEqual(pim_seen, 170.0)

    def test_no_sensors_may_still_lower_pim(self):
        pim_seen, *_ = run(pb(NONE, pim=-10))
        self.assertAlmostEqual(pim_seen, 160.0)

    def test_ratio_cap(self):
        pim_seen, *_ = run(pb(ALL, pim=25, rcap=1.05))
        self.assertAlmostEqual(pim_seen, 170.0 * 1.05, places=6)

    def test_authority_cap(self):
        pim_seen, *_ = run(pb(Capabilities(frozenset({"egt"})), pim=25, rcap=1.2), pim=120)
        self.assertAlmostEqual(pim_seen, 120 * 1.08, places=6)

    def test_boost_cut_margin_wins(self):
        p = pb(ALL, pim=25, rcap=1.2)
        for pim in (150, 190, 200, 230):
            pim_seen, *_ = run(p, pim=pim)
            self.assertLessEqual(pim_seen, BOOST_CUT_KPA_ABS - p.margin_kpa + 1e-9)

    def test_thw_untouched_when_cold(self):
        _, thw_seen, u, *_ = run(pb(ALL, thw=-10, learning=True), thw=60)
        self.assertEqual(thw_seen, 60)
        self.assertEqual(u["thw_off"], 0)

    def test_thw_offset_when_warm(self):
        _, thw_seen, *_ = run(pb(ALL, thw=-10), thw=88)
        self.assertAlmostEqual(thw_seen, 78)

    def test_dither_is_the_only_difference_between_u_and_tbl(self):
        p = pb(ALL, pim=-10, thw=-6, learning=True)
        for t in np.arange(0, 30, 0.37):
            _, _, u, tbl, s, _ = run(p, pim=150, t=t)
            for a, act in ACTUATORS.items():
                self.assertAlmostEqual(u[a] - tbl[a], s[a] * act.probe, places=9)

    def test_dither_off_when_not_learning(self):
        p = pb(ALL, pim=-4)
        for t in (0.1, 1.1, 13.0):
            _, _, u, tbl, *_ = run(p, t=t)
            self.assertEqual(u, tbl)


if __name__ == "__main__":
    unittest.main()
