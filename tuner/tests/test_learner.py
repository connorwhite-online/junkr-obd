"""Short end-to-end runs against the simulator (a few seconds each)."""

import tempfile
import unittest
from pathlib import Path

import numpy as np

from tuner import campaign
from tuner.learner import Learner
from tuner.tables import ACTUATORS, LIMITS


class CampaignSmokeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.full = campaign.run(rounds=2, drives_per_round=2, verbose=False)
        cls.bare = campaign.run(rounds=2, drives_per_round=2, modules=[], verbose=False)

    def test_no_hard_limit_violations(self):
        for lr, _, _, viol in (self.full, self.bare):
            self.assertEqual(viol, 0.0)

    def test_tables_stay_inside_hard_limits(self):
        for lr, *_ in (self.full, self.bare):
            for a, g in lr.tables.grids.items():
                lo, hi = (ACTUATORS[a].lo, ACTUATORS[a].hi) if a in ACTUATORS else LIMITS[a][:2]
                self.assertTrue(np.all((g >= lo - 1e-9) & (g <= hi + 1e-9)), a)

    def test_eval_never_cuts_fuel(self):
        for _, _, history, _ in (self.full, self.bare):
            for _, scores in history:
                for s in scores.values():
                    self.assertEqual(s.cuts, 0)

    def test_save_load_round_trip(self):
        lr = self.full[0]
        with tempfile.TemporaryDirectory() as d:
            p = Path(d) / "state.npz"
            lr.save(p)
            back = Learner.load(p)
        self.assertEqual(back.rounds, lr.rounds)
        self.assertEqual(back.caps, lr.caps)
        for k, g in lr.tables.grids.items():
            np.testing.assert_array_equal(back.tables.grids[k], g)


if __name__ == "__main__":
    unittest.main()
