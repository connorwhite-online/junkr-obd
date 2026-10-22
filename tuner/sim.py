"""A deliberately simple 1KZ-TE + stock ECU + truck simulator.

Purpose: let the learning loop be developed and regression-tested without
the truck. The model is calibrated to published 1KZ-TE numbers (≈96 kW,
≈343 Nm, ~10 psi stock boost, ECU boost cut ≈15 psi) but it is NOT a
substitute for real logs. Constants marked TRUTH are hidden from the
learner: it only ever sees the logged channels.
"""

from __future__ import annotations

import math
from dataclasses import dataclass, field

import numpy as np

from .channels import Capabilities
from .tables import ACTUATORS, TableSet

# ---------------------------------------------------------------- constants
VD = 2.982e-3                 # m^3
VE = 0.88
R_AIR = 287.0
FUEL_DENS = 830.0             # kg/m^3
LHV = 43.0e6                  # J/kg
AFR_ST = 14.5
STOCK_SMOKE_LAMBDA = 1.32     # TRUTH: stock ECU smoke-limit target
BOOST_CUT_KPA_ABS = 204.0     # TRUTH: ECU cuts fuel above ~15 psi gauge (sea-level referenced)
WASTEGATE_KPA = 70.0          # stock ~10 psi

MASS = 2250.0
CDA = 1.25
CRR = 0.013
WHEEL_R = 0.39
FINAL = 4.10
GEARS = [4.31, 2.33, 1.44, 1.00, 0.84]
DRIVELINE_EFF = 0.88
DT = 0.1


def baro_at(alt_m: float) -> float:
    return 101.3 * math.exp(-alt_m / 8400.0)


def fmax(rpm: float) -> float:
    """Full-pedal fuel demand, mm^3/stroke (TRUTH)."""
    return 80.0 - 14.0 * ((rpm - 2000.0) / 2200.0) ** 2


def air_mg(map_kpa: float, iat_c: float) -> float:
    return map_kpa * 1000.0 * (VD / 4) * VE / (R_AIR * (iat_c + 273.15)) * 1e6


def mm3_to_mg(mm3: float) -> float:
    return mm3 * 1e-9 * FUEL_DENS * 1e6


def mg_to_mm3(mg: float) -> float:
    return mg / (1e-9 * FUEL_DENS * 1e6)


# ---------------------------------------------------------------- stock ECU
@dataclass
class EcuOut:
    fuel: float          # mm^3/stroke
    timing: float        # deg BTDC
    cut: bool


def stock_ecu(rpm, va, pim_seen, tha, thw_seen, thf) -> EcuOut:
    demand = va / 100.0 * fmax(rpm)
    if rpm < 850:                                  # idle governor
        demand = max(demand, 9.0 + 0.02 * (850 - rpm))
    smoke = mg_to_mm3(air_mg(pim_seen, tha) / (AFR_ST * STOCK_SMOKE_LAMBDA))
    fuel = min(demand, smoke)
    if rpm > 4300:                                 # over-speed governor
        fuel *= max(0.0, 1 - (rpm - 4300) / 300)
    if thw_seen < 60:
        fuel *= 1 + 0.004 * (60 - thw_seen)
    fuel *= 1 + 0.0015 * (thf - 40)
    cut = pim_seen > BOOST_CUT_KPA_ABS
    if cut:
        fuel = min(fuel, 10.0)
    timing = 1.5 + 0.0028 * rpm + 0.06 * fuel
    if thw_seen < 80:
        timing += 0.12 * (80 - thw_seen)
    return EcuOut(fuel, timing, cut)


# ---------------------------------------------------------------- engine
def optimal_timing(rpm, fuel):
    """TRUTH: stock is retarded ~3 deg at light load (noise/NOx), near-optimal at full load."""
    part = max(0.0, 1 - fuel / 55.0)
    return 1.5 + 0.0028 * rpm + 0.06 * fuel + 0.5 + 3.0 * part


def engine(rpm, fuel_mm3, timing, map_kpa, iat):
    fuel_mg = mm3_to_mg(fuel_mm3)
    air = air_mg(map_kpa, iat)
    lam = air / max(fuel_mg * AFR_ST, 1e-6)
    eta = 0.41 - 0.025 * ((rpm - 2000) / 2000) ** 2
    eta *= 1 - 0.004 * (timing - optimal_timing(rpm, fuel_mm3)) ** 2
    eta *= 1 - 1.5 * max(0.0, 1.2 - lam) ** 2
    torque = 4 * fuel_mg * 1e-6 * LHV * eta / (4 * math.pi) - (22 + 0.006 * rpm)
    egt = iat + 150 + 600 / lam + 6 * (optimal_timing(rpm, fuel_mm3) - timing)
    return torque, lam, egt


# ---------------------------------------------------------------- environment / cycles
@dataclass
class Env:
    alt: float = 0.0
    ambient: float = 20.0

    @property
    def baro(self):
        return baro_at(self.alt)


@dataclass
class Segment:
    kind: str            # "cruise" | "wot" | "idle"
    v0: float = 0.0      # km/h
    v1: float = 0.0
    dur: float = 0.0     # s (cruise/idle)
    grade: float = 0.0   # %


def random_cycle(rng: np.random.Generator, minutes=8.0) -> list[Segment]:
    segs = [Segment("idle", dur=2.0)]
    t = 0.0
    v = 0.0
    while t < minutes * 60:
        r = rng.random()
        if r < 0.35:
            v1 = float(rng.uniform(v + 15, 110)) if v < 95 else 110.0
            segs.append(Segment("wot", v, v1, grade=float(rng.uniform(-2, 6))))
            t += 15
            v = v1
        else:
            v = float(rng.choice([40, 60, 80, 90, 100]))
            dur = float(rng.uniform(25, 60))
            segs.append(Segment("cruise", v, v, dur, float(rng.uniform(-3, 7))))
            t += dur
    return segs


# ---------------------------------------------------------------- piggyback (mirrors firmware)
@dataclass
class Piggyback:
    tables: TableSet
    caps: Capabilities
    learning: bool = False
    periods: dict = field(default_factory=lambda: {"pim_off": 2.0, "thw_off": 24.0})
    margin_kpa: float = 6.0

    def apply(self, t, rpm, va, pim, tha, thw, baro_est):
        q = {"baro": baro_est, "iat": tha, "rpm": rpm, "va": va}
        # each actuator gets its own square-wave dither so their effects are separable
        s, age = {}, {}
        u, tbl = {}, {}
        for a, act in ACTUATORS.items():
            per = self.periods[a]
            s[a] = 1.0 if (t % per) < per / 2 else -1.0
            age[a] = t % (per / 2) if self.learning else 999.0
            val = tbl[a] = min(max(float(self.tables.lookup(a, q)), act.lo), act.hi)
            if self.learning:
                val += s[a] * act.probe
            u[a] = min(max(val, act.lo), act.hi)
        rcap = float(self.tables.lookup("pim_rcap", q)) if self.caps.can_add_fuel else 1.0

        def cap_pim(off):
            # authority: never unlock more fuel than the fitted sensors can police
            off = min(off, self.caps.max_fuel_gain * pim)
            if off > 0:
                off = min(off, pim * (max(rcap, 1.0) - 1.0))
            return min(pim + off, BOOST_CUT_KPA_ABS - self.margin_kpa) - pim

        u["pim_off"] = cap_pim(u["pim_off"])
        tbl["pim_off"] = cap_pim(tbl["pim_off"])   # so u - tbl is exactly the dither that got through
        pim_seen = pim + u["pim_off"]
        if thw < 75:                      # never touch warm-up behaviour
            u["thw_off"] = 0.0
            tbl["thw_off"] = 0.0
        thw_seen = thw + u["thw_off"]
        return pim_seen, thw_seen, u, tbl, s, age


# ---------------------------------------------------------------- drive
@dataclass
class DriveResult:
    log: dict[str, np.ndarray]
    wot_times: list[float] = field(default_factory=list)   # seconds per WOT segment
    fuel_l: float = 0.0
    dist_km: float = 0.0
    cut_events: int = 0


def drive(segs: list[Segment], env: Env, pb: Piggyback, rng: np.random.Generator,
          noise=True, wastegate=WASTEGATE_KPA) -> DriveResult:
    baro = env.baro
    v = segs[0].v0 / 3.6 if segs else 0.0
    boost = 0.0
    egt_f = env.ambient + 150
    thw, thf = 85.0, env.ambient + 20
    gear = 0
    t = 0.0
    rows = []
    res = DriveResult({})
    integ = 0.0
    baro_est = None
    pim_samples = []

    def nz(x, s):
        return x + (rng.normal(0, s) if noise else 0.0)

    for seg in segs:
        seg_t = 0.0
        target = seg.v1 if seg.kind == "wot" else seg.v0
        while True:
            if seg.kind in ("cruise", "idle") and seg_t >= seg.dur:
                break
            if seg.kind == "wot" and (v * 3.6 >= seg.v1 or seg_t > 60):
                res.wot_times.append(seg_t)
                break
            idle = seg.kind == "idle"
            # gear choice
            if not idle:
                for g in range(len(GEARS)):
                    r = v / WHEEL_R * 60 / (2 * math.pi) * GEARS[g] * FINAL
                    if r < (3800 if seg.kind == "wot" else 2300):
                        gear = g
                        break
            ratio = GEARS[gear] * FINAL
            rpm = 0.0 if idle else max(850.0, v / WHEEL_R * 60 / (2 * math.pi) * ratio)
            # driver
            if seg.kind == "wot":
                va = 100.0
            elif idle:
                va = 0.0
            else:
                err = target - v * 3.6
                integ = min(max(integ + err * DT * 0.8, 0), 60)
                va = min(max(8.0 + 4.0 * err + integ, 0.0), 100.0)
            iat = env.ambient + 0.35 * boost
            pim = baro + boost
            if idle:
                pim_samples.append(pim)
                pim_seen, thw_seen = pim, thw
                u = {a: 0.0 for a in ACTUATORS}
                tbl = {a: 0.0 for a in ACTUATORS}
                s = {a: 0.0 for a in ACTUATORS}
                age = {a: 0.0 for a in ACTUATORS}
                fuel, timing, cut, torque, lam, egt = 0, 0, False, 0, 2.0, env.ambient
            else:
                if baro_est is None:
                    baro_est = float(np.mean(pim_samples)) if pim_samples else 101.3
                pim_seen, thw_seen, u, tbl, s, age = pb.apply(t, rpm, va, pim, iat, thw, baro_est)
                ecu = stock_ecu(rpm, va, pim_seen, iat, thw_seen, thf)
                fuel, timing, cut = ecu.fuel, ecu.timing, ecu.cut
                torque, lam, egt = engine(rpm, fuel, timing, pim, iat)
                res.cut_events += int(cut)
                # vehicle
                grade = seg.grade / 100
                f_trac = torque * ratio * DRIVELINE_EFF / WHEEL_R
                f_res = 0.5 * 1.2 * CDA * v * v + CRR * MASS * 9.81 + MASS * 9.81 * grade
                acc = (f_trac - f_res) / MASS
                v = max(0.0, v + acc * DT)
                # turbo
                b_ss = min(wastegate, 75.0 * (fuel * rpm / (75 * 2300)) ** 1.4)
                boost += (b_ss - boost) * DT / 1.0
                fuel_lph = fuel * rpm * 1.2e-4
                res.fuel_l += fuel_lph * DT / 3600
                res.dist_km += v * DT / 1000
            egt_f += (egt - egt_f) * DT / 2.5
            rows.append((
                t, nz(rpm, 5) if rpm else 0.0, va, nz(pim, 0.5), nz(pim_seen, 0.5), thw, thw_seen,
                nz(iat, 0.3), thf, nz(fuel / 1.6, 0.2), 0.0, nz(timing, 0.15), v * 3.6,
                u["pim_off"], u["thw_off"], tbl["pim_off"], tbl["thw_off"], s["pim_off"], age["pim_off"], age["thw_off"],
                min(nz(lam, 0.01), 2.0), baro, nz(egt_f, 4), iat, env.ambient,
                v * 3.6, env.alt, seg.grade, nz(acc if not idle else 0.0, 0.05),
                nz(fuel * rpm * 1.2e-4 if not idle else 0.0, 0.05),
            ))
            t += DT
            seg_t += DT
    names = ["t", "rpm", "va", "pim", "pim_seen", "thw", "thw_seen", "tha", "thf", "spv_deg",
             "tcv", "timing", "vss", "u_pim_off", "u_thw_off", "tbl_pim_off", "tbl_thw_off", "dither", "dither_age",
             "dither_age_thw",
             "lambda", "baro", "egt", "iat2", "ambient", "gps_speed", "alt", "grade", "accel",
             "fuel_lph"]
    arr = np.array(rows, float)
    log = {n: arr[:, i] for i, n in enumerate(names)}
    # drop channels for modules that aren't fitted
    from .channels import MODULES
    for m, chs in MODULES.items():
        if not pb.caps.has(m):
            for c in chs:
                log.pop(c.name, None)
    res.log = log
    return res
