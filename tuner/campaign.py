"""Iterate the learn loop against the simulator and score every round.

    python -m tuner campaign --rounds 20

Each round: drive N random cycles in random environments with the *learning*
table (dither on), feed the logs to the learner, adopt its *best* table, and
evaluate that table on a fixed test suite against stock.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np

from . import channels
from .channels import Capabilities
from .learner import Config, Learner
from .sim import Env, Piggyback, Segment, drive, random_cycle
from .tables import TableSet

TRAIN_ENVS = [Env(0, 30), Env(0, 5), Env(1500, 20), Env(2500, 0), Env(800, 40)]
EVAL_ENVS = {"sea level 25C": Env(0, 25), "1800 m 10C": Env(1800, 10), "hot 40C": Env(300, 40)}

WOT = [Segment("idle", dur=2), Segment("cruise", 40, 40, 15), Segment("wot", 40, 100)]
HILL = [Segment("idle", dur=2), Segment("cruise", 60, 60, 10), Segment("wot", 60, 90, grade=6)]
CRUISE = [Segment("idle", dur=2), Segment("cruise", 90, 90, 40), Segment("cruise", 90, 90, 90)]


@dataclass
class Score:
    wot_s: float
    hill_s: float
    l100: float
    min_lambda: float
    max_egt: float
    cuts: int


def evaluate(tables: TableSet, caps: Capabilities, env: Env) -> Score:
    rng = np.random.default_rng(0)
    pb = Piggyback(tables, caps, learning=False)
    w = drive(WOT, env, pb, rng, noise=False)
    h = drive(HILL, env, pb, rng, noise=False)
    c = drive(CRUISE, env, pb, rng, noise=False)
    # fuel economy over the steady part only
    log = c.log
    m = log["t"] > 60
    l100 = float(np.sum(log["fuel_lph"][m]) / np.sum(log["vss"][m]) * 100) if "fuel_lph" in log else float("nan")
    lam = [r.log.get("lambda") for r in (w, h, c)]
    egt = [r.log.get("egt") for r in (w, h, c)]
    from .sim import engine, stock_ecu  # noqa: F401  (truth check below uses logs only)
    return Score(w.wot_times[-1], h.wot_times[-1], l100,
                 float(min(x.min() for x in lam)) if lam[0] is not None else float("nan"),
                 float(max(x.max() for x in egt)) if egt[0] is not None else float("nan"),
                 w.cut_events + h.cut_events + c.cut_events)


def run(rounds=15, drives_per_round=4, modules=None, cfg: Config | None = None, seed=1, verbose=True):
    names = [c.name for c in channels.CORE]
    for m in (modules if modules is not None else channels.MODULES):
        names += [c.name for c in channels.MODULES[m]]
    caps = Capabilities.from_channels(names)
    lr = Learner(caps, cfg or Config())
    rng = np.random.default_rng(seed)
    learn_tbl = TableSet()
    full = Capabilities.from_channels(channels.ALL)      # evaluation always measures everything
    stock = {k: evaluate(TableSet(), full, e) for k, e in EVAL_ENVS.items()}
    history = []
    violations = 0
    samples = 0
    if verbose:
        print(caps.describe())
    for r in range(rounds):
        for _ in range(drives_per_round):
            env = TRAIN_ENVS[rng.integers(len(TRAIN_ENVS))]
            res = drive(random_cycle(rng, 6), env, Piggyback(learn_tbl, caps, learning=True), rng)
            # hidden-truth safety audit of the *training* drives (what the truck would have done)
            truth = drive_truth_audit(res)
            violations += truth
            samples += len(res.log["t"])
            lr.observe(res.log)
        best, info = lr.propose()
        lr.end_round(best)
        learn_tbl = best
        scores = {k: evaluate(best, full, e) for k, e in EVAL_ENVS.items()}
        history.append((info, scores))
        if verbose:
            parts = []
            for k, s in scores.items():
                s0 = stock[k]
                parts.append(f"{k}: WOT {100*(s0.wot_s/s.wot_s-1):+5.1f}% hill {100*(s0.hill_s/s.hill_s-1):+5.1f}% "
                             f"econ {100*(s0.l100/s.l100-1):+5.1f}% λmin {s.min_lambda:.2f} EGT {s.max_egt:.0f} cut {s.cuts}")
            print(f"round {r+1:2d} moved {info['moved']:4d} backoff {info['backed_off']:3d} | " + " | ".join(parts))
    return lr, stock, history, violations / max(samples, 1)


def drive_truth_audit(res, lambda_hard=1.12, egt_hard=730.0) -> int:
    """Count samples where the real engine went past hard limits during learning."""
    log = res.log
    bad = 0
    if "lambda" in log:
        bad += int(np.sum(log["lambda"] < lambda_hard))
    if "egt" in log:
        bad += int(np.sum(log["egt"] > egt_hard))
    return bad + res.cut_events
