"""Log channels and plug-in sensor modules.

The interposer always provides the CORE channels (tapped off the ECU plug).
Everything else comes from optional sensor modules (wideband, baro, EGT, ...)
that hang off the CAN bus. The learner looks at which modules are present
and scales how much authority it is allowed to take (see `Capabilities`).
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Channel:
    name: str
    unit: str
    lo: float          # plausible range; samples outside are dropped
    hi: float
    doc: str


# Tapped from the ECU connector (always present once the interposer is in).
CORE = [
    Channel("t", "s", 0, 1e7, "time since key-on"),
    Channel("rpm", "rpm", 0, 5500, "engine speed (TDC)"),
    Channel("va", "%", 0, 100, "accelerator position (VA)"),
    Channel("pim", "kPa", 20, 300, "manifold absolute pressure, real (PIM tap, ECU side of nothing)"),
    Channel("pim_seen", "kPa", 20, 300, "PIM as presented to the ECU after the piggyback"),
    Channel("thw", "C", -40, 130, "coolant temp (THW)"),
    Channel("thw_seen", "C", -40, 130, "THW as presented to the ECU"),
    Channel("tha", "C", -40, 120, "intake air temp (THA)"),
    Channel("thf", "C", -40, 120, "fuel temp (THF)"),
    Channel("spv_deg", "deg", 0, 60, "spill-valve closed duration in pump degrees (fuel quantity proxy)"),
    Channel("tcv", "%", 0, 100, "timing control valve duty (commanded timing)"),
    Channel("timing", "deg", -10, 30, "actual injection timing from NE/TDC phase, deg BTDC"),
    Channel("vss", "km/h", 0, 220, "vehicle speed (SP1)"),
]

# Optional plug-in sensor modules. Key = module id, value = channels it adds.
MODULES: dict[str, list[Channel]] = {
    "wideband": [Channel("lambda", "-", 0.5, 2.0, "exhaust lambda (LSU 4.9 via CAN wideband)")],
    "baro": [Channel("baro", "kPa", 50, 110, "ambient pressure")],
    "egt": [Channel("egt", "C", -20, 1100, "pre-turbo exhaust gas temp")],
    "iat2": [Channel("iat2", "C", -40, 150, "post-intercooler charge temp")],
    "ambient": [Channel("ambient", "C", -40, 60, "ambient air temp")],
    "gps": [
        Channel("gps_speed", "km/h", 0, 250, "GPS ground speed"),
        Channel("alt", "m", -500, 6000, "altitude"),
        Channel("grade", "%", -30, 30, "road grade"),
    ],
    "imu": [Channel("accel", "m/s2", -15, 15, "longitudinal acceleration")],
    "fuel_flow": [Channel("fuel_lph", "L/h", 0, 60, "measured fuel flow")],
}

ALL = {c.name: c for c in CORE} | {c.name: c for chs in MODULES.values() for c in chs}


@dataclass(frozen=True)
class Capabilities:
    """What the learner may do, given which modules are fitted.

    The rule: the learner may only push a quantity as far as it can *measure*
    the consequence. No wideband and no EGT means no added fuel at all.
    """

    modules: frozenset[str]

    @classmethod
    def from_channels(cls, names) -> "Capabilities":
        names = set(names)
        mods = {m for m, chs in MODULES.items() if all(c.name in names for c in chs)}
        return cls(frozenset(mods))

    def has(self, module: str) -> bool:
        return module in self.modules

    @property
    def max_fuel_gain(self) -> float:
        """Max fraction of extra smoke-limited fuel the piggyback may unlock."""
        if self.has("wideband") and self.has("egt"):
            return 0.20
        if self.has("wideband") or self.has("egt"):
            return 0.08
        return 0.0

    @property
    def can_add_fuel(self) -> bool:
        return self.max_fuel_gain > 0

    @property
    def performance_signal(self) -> str:
        """Best available measure of 'more power'."""
        if self.has("imu") and self.has("gps"):
            return "accel_grade_corrected"
        if self.has("imu"):
            return "accel"
        return "fuel_proxy"

    @property
    def economy_signal(self) -> str:
        return "fuel_flow" if self.has("fuel_flow") else "spv_proxy"

    def describe(self) -> str:
        mods = ", ".join(sorted(self.modules)) or "none"
        return (f"modules: {mods} | max extra fuel: {self.max_fuel_gain:.0%} | "
                f"perf signal: {self.performance_signal} | econ signal: {self.economy_signal}")
