"""Watch -> learn -> propose: the safe, constrained learning loop.

Method: safe extremum seeking on a grid
---------------------------------------
While learning, the firmware adds a small square-wave *dither* to each
actuator, each with its own period so their effects can be told apart. Inside
one drive, every grid node (baro x intake temp x rpm x pedal) then sees the
same conditions with the actuator a little up and a little down. That is a
paired A/B test that between-drive differences (weather, route, load) can't
contaminate.

Per node and per response (objective, EGT) we keep Bayesian least-squares
statistics of

    y = level + g_p * d_pim + g_t * d_thw + covariates(rpm, pedal, ambient, baro)

where d_* is the dither offset actually applied (applied value - table value).
`g` is the local gradient at the current table point.

After each round, each node takes one step in the direction of its gradient,
but only if:
  * the improvement is confident (lower bound > 0, `z_improve` sigmas),
  * the pessimistic EGT prediction stays under `egt_max`,
  * extra fuel stays inside the lambda ratio cap learned from the wideband
    (an explicit model of how the ECU's smoke limiter responds to PIM), and
  * the fitted sensor modules allow that much authority at all.
Old evidence decays (`forget`), so the gradient stays local as the table moves.
"""

from __future__ import annotations

from dataclasses import dataclass, field

import numpy as np

from .channels import Capabilities
from .tables import ACTUATORS, AXES, LIMITS, SHAPE, TableSet, corner_weights

ACT = list(ACTUATORS)                 # ["pim_off", "thw_off"]
NF = 7                                # features: 1, d_pim, d_thw, drpm, dva, d_ambient, dbaro
RESP = ("obj", "egt")


@dataclass
class Config:
    lambda_min: float = 1.20          # smoke / soot limit
    egt_max: float = 700.0            # C, pre-turbo, sustained
    k_safe: float = 2.0               # sigmas of pessimism on constraints
    z_improve: float = 1.5            # sigmas of confidence before a node moves
    forget: float = 0.6               # per-round decay of old evidence (keeps gradients local)
    min_weight: float = 20.0          # samples (corner-weighted) before a node may move
    # ignore samples this soon after a dither flip. Fuel answers PIM within a
    # pump stroke, so the PIM dither can be fast (which averages out turbo
    # spool and other trends inside a pull); coolant-temp effects are slower.
    settle_pim_s: float = 0.4
    settle_thw_s: float = 3.0
    env_pool: float = 0.25            # how much neighbouring baro/temp bins share objective evidence
    env_pool_safety: float = 0.05     # ...and EGT evidence (kept local: heat and altitude matter)
    reg: float = 0.002                # preference for small corrections (objective units per actuator unit)
    mode: float = 0.0                 # -1 economy ... 0 balanced ... +1 power
    mass: float = 2250.0
    cda: float = 1.25
    crr: float = 0.013


PRIOR = np.diag([1e-4, 1e-1, 1e-1, 1e-2, 1e-2, 1e-2, 1e-2])


@dataclass
class Stats:
    A: np.ndarray = field(default_factory=lambda: np.zeros((*SHAPE, NF, NF)))
    b: np.ndarray = field(default_factory=lambda: np.zeros((*SHAPE, NF)))
    yy: np.ndarray = field(default_factory=lambda: np.zeros(SHAPE))
    w: np.ndarray = field(default_factory=lambda: np.zeros(SHAPE))

    def decay(self, f):
        self.A *= f
        self.b *= f
        self.yy *= f
        self.w *= f

    def pooled(self, g):
        def pool(x):
            tot = x.sum(axis=(0, 1), keepdims=True)
            return x + g * (tot - x)
        return Stats(pool(self.A), pool(self.b), pool(self.yy), pool(self.w))

    def posterior(self):
        A = self.A + PRIOR
        Ainv = np.linalg.inv(A)
        beta = np.einsum("...ij,...j->...i", Ainv, self.b)
        dof = np.maximum(self.w - NF, 1.0)
        sse = np.maximum(self.yy - np.einsum("...i,...i->...", beta, self.b), 0)
        sigma2 = sse / dof + 1e-6
        return beta, Ainv, sigma2


# ------------------------------------------------------------------ signals
def derive(log: dict[str, np.ndarray], caps: Capabilities, cfg: Config):
    """Per-sample objective + constraint signals, and a validity mask."""
    rpm, va = log["rpm"], log["va"]
    n = len(rpm)
    valid = (rpm > 900) & (log["vss"] > 5)
    for k, settle in (("dither_age", cfg.settle_pim_s), ("dither_age_thw", cfg.settle_thw_s)):
        if k in log:
            valid &= log[k] >= settle

    # power estimate at the wheels, if we can see acceleration
    if caps.performance_signal.startswith("accel"):
        v = log["vss"] / 3.6
        grade = log["grade"] / 100 if "grade" in log else np.zeros(n)
        f = cfg.mass * (log["accel"] + 9.81 * grade) + 0.5 * 1.2 * cfg.cda * v * v + cfg.crr * cfg.mass * 9.81
        p_kw = f * v / 1000
        omega = np.maximum(rpm, 1) * 2 * np.pi / 60
        perf = p_kw * 1000 / omega / 100            # ~engine torque / 100 Nm
    else:
        p_kw = None
        perf = log["spv_deg"] / 10                   # more fuel ~ more torque (inside limits)

    if caps.economy_signal == "fuel_flow" and p_kw is not None:
        econ = p_kw / np.maximum(log["fuel_lph"], 0.3)            # kWh at wheels per litre
    elif caps.economy_signal == "fuel_flow":
        econ = -log["fuel_lph"] / np.maximum(log["vss"], 5) * 10  # -L/100km / 10
    else:
        econ = -log["spv_deg"] * rpm / np.maximum(log["vss"], 5) / 1000

    wp = 1 / (1 + np.exp(-(va - 65 + 10 * cfg.mode) / 6))
    obj = wp * perf + (1 - wp) * econ
    out = {"obj": obj}
    if caps.has("egt"):
        out["egt"] = log["egt"] / 100.0
    valid &= np.isfinite(obj)
    return out, valid


def baro_of(log) -> np.ndarray:
    """Baro from the module, else from PIM at key-on (engine stopped = ambient)."""
    if "baro" in log:
        return log["baro"]
    off = log["rpm"] < 50
    est = float(np.median(log["pim"][off])) if off.any() else 101.3
    return np.full(len(log["rpm"]), est)


# ------------------------------------------------------------------ learner
@dataclass
class Learner:
    caps: Capabilities
    cfg: Config = field(default_factory=Config)
    stats: dict[str, Stats] = field(default_factory=lambda: {r: Stats() for r in RESP})
    tables: TableSet = field(default_factory=TableSet)          # current (exploit) table
    rounds: int = 0
    # ECU model: k = lambda * pim_seen / pim on smoke-limited samples (sum w, sum wk, sum wk^2)
    kstat: np.ndarray = field(default_factory=lambda: np.zeros((*SHAPE, 3)))
    # typical real PIM at each node (sum w, sum w*pim)
    pstat: np.ndarray = field(default_factory=lambda: np.zeros((*SHAPE, 2)))

    # ---- watch
    def observe(self, log: dict[str, np.ndarray]) -> int:
        sig, valid = derive(log, self.caps, self.cfg)
        idx = np.nonzero(valid)[0]
        if len(idx) == 0:
            return 0
        q = {"baro": baro_of(log)[idx], "iat": log["tha"][idx],
             "rpm": log["rpm"][idx], "va": log["va"][idx]}
        pim = log["pim"][idx]
        # exogenous temperature only: THA is pushed around by boost, which the
        # actuator changes, so it would soak up part of the effect we measure
        amb = log["ambient"][idx] if "ambient" in log else np.full(len(idx), AXES["iat"][1])
        dp = log["u_pim_off"][idx] - log["tbl_pim_off"][idx]
        dt = log["u_thw_off"][idx] - log["tbl_thw_off"][idx]
        flat = {r: (s.A.reshape(-1, NF, NF), s.b.reshape(-1, NF), s.yy.reshape(-1), s.w.reshape(-1))
                for r, s in self.stats.items()}
        lam = log["lambda"][idx] if "lambda" in log else None
        for fi, w in corner_weights(q):
            sub = np.unravel_index(fi, SHAPE)
            x = np.stack([np.ones_like(dp), dp, dt,
                          (q["rpm"] - AXES["rpm"][sub[2]]) / 1000,
                          (q["va"] - AXES["va"][sub[3]]) / 50,
                          (amb - AXES["iat"][sub[1]]) / 20,
                          (q["baro"] - AXES["baro"][sub[0]]) / 15], axis=1)
            xx = np.einsum("ni,nj->nij", x, x) * w[:, None, None]
            for r, y in sig.items():
                A, b, yy, ww = flat[r]
                yv = y[idx]
                np.add.at(A, fi, xx)
                np.add.at(b, fi, x * (w * yv)[:, None])
                np.add.at(yy, fi, w * yv * yv)
                np.add.at(ww, fi, w)
            np.add.at(self.pstat.reshape(-1, 2), fi, np.stack([w, w * pim], 1))
            if lam is not None:
                lim = (q["va"] >= 70) & (lam < 1.7)        # likely smoke-limited
                k = lam * log["pim_seen"][idx] / pim
                wl = w * lim
                np.add.at(self.kstat.reshape(-1, 3), fi, np.stack([wl, wl * k, wl * k * k], 1))
        return len(idx)

    # ---- ECU model
    def ratio_cap(self) -> np.ndarray:
        """Per-node max pim_seen/pim that keeps lambda >= lambda_min (pessimistic).

        When the ECU is smoke-limited its fuel scales with the PIM it sees, so
        real lambda = k * pim / pim_seen with k the stock lambda. The cap is then
        k_low / lambda_min, with k_low a pessimistic per-sample bound on k."""
        lo, hi, default = LIMITS["pim_rcap"]
        hi = min(hi, 1.0 + self.caps.max_fuel_gain)
        if not self.caps.can_add_fuel:
            return np.ones(SHAPE)
        if not self.caps.has("wideband"):
            return np.full(SHAPE, min(default, hi))
        w, sk, skk = self.kstat[..., 0], self.kstat[..., 1], self.kstat[..., 2]
        mean = sk / np.maximum(w, 1e-9)
        sd = np.sqrt(np.maximum(skk / np.maximum(w, 1e-9) - mean * mean, 0)) + 0.01
        learned = (mean - self.cfg.k_safe * sd) / self.cfg.lambda_min
        cap = np.where(w >= self.cfg.min_weight, learned, default)
        return np.clip(cap, lo, hi)

    # ---- propose
    def propose(self) -> tuple[TableSet, dict]:
        cfg = self.cfg
        rcap = self.ratio_cap()
        cur_p = self.tables.grids["pim_off"]
        cur_t = self.tables.grids["thw_off"]
        if self.stats["obj"].w.max() == 0:
            out = self.tables.copy()
            out.grids["pim_rcap"] = rcap
            return out, {"moved": 0, "backed_off": 0, "nodes_with_data": 0}

        so = self.stats["obj"].pooled(cfg.env_pool)
        bo, Ao, s2o = so.posterior()
        enough = so.w >= cfg.min_weight
        egt = None
        if "egt" in self.stats and self.stats["egt"].w.max() > 0:
            se = self.stats["egt"].pooled(cfg.env_pool_safety)
            egt = (*se.posterior(), se.w)

        pim_node = np.where(self.pstat[..., 0] > 0,
                            self.pstat[..., 1] / np.maximum(self.pstat[..., 0], 1e-9),
                            AXES["baro"][:, None, None, None] + np.zeros(SHAPE))
        u_cap = pim_node * (rcap - 1.0)
        sp, st = ACTUATORS["pim_off"].step, ACTUATORS["thw_off"].step

        best = np.zeros(SHAPE)             # score of staying put = 0
        new_p, new_t = cur_p.copy(), cur_t.copy()
        unsafe_now = np.zeros(SHAPE, bool)
        # single and double steps; a double step needs twice the confidence to win
        steps_p = (-2 * sp, -sp, 0.0, sp, 2 * sp)
        steps_t = (-2 * st, -st, 0.0, st, 2 * st)
        for dp, dt in [(a, b) for a in steps_p for b in steps_t]:
            up = np.clip(cur_p + dp, ACTUATORS["pim_off"].lo, ACTUATORS["pim_off"].hi)
            ut = np.clip(cur_t + dt, ACTUATORS["thw_off"].lo, ACTUATORS["thw_off"].hi)
            if not self.caps.can_add_fuel:
                up = np.minimum(up, 0.0)
            ddp, ddt = up - cur_p, ut - cur_t
            x = np.stack([np.zeros(SHAPE), ddp, ddt] + [np.zeros(SHAPE)] * 4, -1)
            gain = np.einsum("...i,...i->...", bo, x)
            gain -= cfg.reg * (np.abs(up) - np.abs(cur_p) + np.abs(ut) - np.abs(cur_t))
            sd = np.sqrt(s2o * np.einsum("...i,...ij,...j->...", x, Ao, x))
            score = gain - cfg.z_improve * sd

            # safety. Stock fuel (or less) is what Toyota validated, so limits
            # only bind on settings that unlock extra fuel.
            adds_fuel = up > 1e-9
            safe = ~adds_fuel | (up <= u_cap + 1e-9)
            if egt is not None:
                be, Ae, s2e, we = egt
                xl = x.copy()
                xl[..., 0] = 1.0                    # level + change
                m = np.einsum("...i,...i->...", be, xl)
                esd = np.sqrt(s2e * np.einsum("...i,...ij,...j->...", xl, Ae, xl))
                ok = m + cfg.k_safe * esd <= cfg.egt_max / 100
                safe &= ~adds_fuel | (ok & (we >= cfg.min_weight))
                if dp == 0 and dt == 0:
                    unsafe_now = adds_fuel & ~ok & (we >= cfg.min_weight)
            elif not self.caps.has("egt"):
                safe &= ~adds_fuel | self.caps.can_add_fuel

            cand = np.where(safe & enough, score, -np.inf)
            upd = cand > best
            best = np.where(upd, cand, best)
            new_p = np.where(upd, up, new_p)
            new_t = np.where(upd, ut, new_t)

        # current setting now looks too hot (or over the lambda cap): back off fuel
        over_cap = (cur_p > u_cap + 1e-9) & (cur_p > 0)
        back = unsafe_now | over_cap
        new_p = np.where(back, np.maximum(np.minimum(cur_p - sp, u_cap), 0.0), new_p)
        out = TableSet({"pim_off": new_p, "thw_off": new_t, "pim_rcap": rcap.copy()}).clamp()
        moved = int(np.sum((np.abs(out.grids["pim_off"] - cur_p) > 1e-9) |
                           (np.abs(out.grids["thw_off"] - cur_t) > 1e-9)))
        return out, {"moved": moved, "backed_off": int(back.sum()), "nodes_with_data": int(enough.sum())}

    def end_round(self, tables: TableSet):
        self.tables = tables
        for s in self.stats.values():
            s.decay(self.cfg.forget)
        self.rounds += 1

    # ---- persistence
    def save(self, path):
        arrs = {}
        for r, s in self.stats.items():
            arrs |= {f"{r}_A": s.A, f"{r}_b": s.b, f"{r}_yy": s.yy, f"{r}_w": s.w}
        for k, g in self.tables.grids.items():
            arrs[f"tbl_{k}"] = g
        np.savez_compressed(path, rounds=self.rounds, modules=np.array(sorted(self.caps.modules)),
                            kstat=self.kstat, pstat=self.pstat, **arrs)

    @classmethod
    def load(cls, path, cfg: Config | None = None) -> "Learner":
        d = np.load(path)
        caps = Capabilities(frozenset(d["modules"].tolist()))
        lr = cls(caps, cfg or Config())
        for r in RESP:
            lr.stats[r] = Stats(d[f"{r}_A"], d[f"{r}_b"], d[f"{r}_yy"], d[f"{r}_w"])
        for k in lr.tables.grids:
            lr.tables.grids[k] = d[f"tbl_{k}"]
        lr.kstat, lr.pstat = d["kstat"], d["pstat"]
        lr.rounds = int(d["rounds"])
        return lr
