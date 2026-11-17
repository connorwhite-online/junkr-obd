#!/usr/bin/env python3
"""Read JUNKR ECU logs (.jlog) and turn them into CSV or Parquet.

The format is in docs/standalone-ecu/LOGGING.md and shared/ecu_log.h. The
log describes its own channels, so this script needs no update when the
firmware adds one.

    python3 tools/junkrlog.py info  drive_00012.jlog
    python3 tools/junkrlog.py csv   drive_00012.jlog            # -> drive_00012/{samples,injections,events}.csv
    python3 tools/junkrlog.py parquet drive_00012.jlog          # needs pandas + pyarrow
    python3 tools/junkrlog.py fetch http://junkr.local logs/    # download every drive from the truck

As a library:

    import junkrlog
    log = junkrlog.read("drive_00012.jlog")
    log.samples      # list of dicts: t_s, rpm, q, adv, ...
    log.to_pandas()  # {"samples": DataFrame, "injections": ..., "events": ...}

Standard library only; pandas and pyarrow are optional.
"""
from __future__ import annotations

import argparse
import csv
import json
import os
import struct
import sys
import urllib.request
from dataclasses import dataclass, field

SYNC = b"\xa5\x5a"
T_META, T_SCHEMA, T_SAMPLE, T_INJ, T_EVENT, T_FILE = 1, 2, 3, 4, 5, 6
WIRE = {"h": "<h", "H": "<H", "i": "<i", "I": "<I"}

EVENTS = {
    1: "boot", 2: "mode", 3: "dtc", 4: "takeover_denied", 5: "sync_lost",
    6: "dropped", 7: "arm", 8: "link", 9: "overload",
}
FLAGS = [
    "synced", "rpm_valid", "inject", "takeover", "arm_req", "arm_sw", "supervisor_ok", "arm",
    "idl", "brake", "starter", "od_sw", "s1", "s2", "sl", "svr", "timing_open_loop", "tdc_valid",
    "standalone", "timing_fault", "egt_limit", "wb_valid", "wb_learn",
]
DTCS = ["ne", "timing", "pedal", "overspeed", "ect", "map", "vbatt", "supervisor", "egt", "wideband"]
MODES = {0: "shadow", 1: "takeover"}


def crc16(data: bytes, crc: int = 0xFFFF) -> int:
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def frames(buf: bytes):
    """Yield (type, payload) for every frame with a good CRC; skips junk."""
    i, n = 0, len(buf)
    bad = 0
    while True:
        j = buf.find(SYNC, i)
        if j < 0 or j + 7 > n:
            break
        typ = buf[j + 2]
        ln = buf[j + 3] | (buf[j + 4] << 8)
        end = j + 5 + ln + 2
        if ln > 1024 or end > n:
            i = j + 1
            bad += ln <= 1024 and end <= n
            continue
        got = buf[end - 2] | (buf[end - 1] << 8)
        if crc16(buf[j + 2:end - 2]) != got:
            bad += 1
            i = j + 1
            continue
        yield typ, buf[j + 5:end - 2]
        i = end
    frames.bad = bad


@dataclass
class Channel:
    name: str
    unit: str
    type: str
    scale: float


@dataclass
class Log:
    meta: dict = field(default_factory=dict)
    file: dict = field(default_factory=dict)
    channels: list = field(default_factory=list)
    samples: list = field(default_factory=list)
    injections: list = field(default_factory=list)
    events: list = field(default_factory=list)
    bad_frames: int = 0

    @property
    def duration_s(self) -> float:
        return self.samples[-1]["t_s"] - self.samples[0]["t_s"] if len(self.samples) > 1 else 0.0

    def to_pandas(self):
        import pandas as pd
        return {
            "samples": pd.DataFrame(self.samples),
            "injections": pd.DataFrame(self.injections),
            "events": pd.DataFrame(self.events),
        }


class _Clock:
    """Turns the STM32's wrapping 32-bit microsecond counter into seconds."""

    def __init__(self):
        self.last = None
        self.base = 0

    def __call__(self, t_us: int) -> float:
        if self.last is not None and t_us < self.last and self.last - t_us > 1 << 31:
            self.base += 1 << 32
        self.last = t_us
        return (self.base + t_us) / 1e6


def parse(buf: bytes, decode_bits: bool = True) -> Log:
    log = Log()
    fmt = None
    clock = _Clock()
    for typ, p in frames(buf):
        if typ == T_SCHEMA:
            count = p[1]
            k = 2
            chans = []
            for _ in range(count):
                t = chr(p[k]); k += 1
                (scale,) = struct.unpack_from("<f", p, k); k += 4
                e = p.index(0, k); name = p[k:e].decode(); k = e + 1
                e = p.index(0, k); unit = p[k:e].decode(); k = e + 1
                chans.append(Channel(name, unit, t, scale))
            if [c.name for c in chans] != [c.name for c in log.channels]:
                log.channels = chans
                fmt = "<II" + "".join(WIRE[c.type][1] for c in chans)
        elif typ == T_SAMPLE and fmt:
            if len(p) != struct.calcsize(fmt):
                continue
            vals = struct.unpack(fmt, p)
            row = {"t_s": round(clock(vals[0]), 6), "seq": vals[1]}
            for c, v in zip(log.channels, vals[2:]):
                row[c.name] = v / c.scale if c.scale != 1 else v
            if decode_bits:
                if "flags" in row:
                    for b, nm in enumerate(FLAGS):
                        row["f_" + nm] = int(row["flags"]) >> b & 1
                if "mode" in row:
                    row["mode_name"] = MODES.get(int(row["mode"]), str(row["mode"]))
            log.samples.append(row)
        elif typ == T_INJ and len(p) >= 21:
            t, per, seg, q, close, opn, _r = struct.unpack_from("<IIIHhhH", p)
            log.injections.append({
                "t_s": round(clock(t), 6), "segment": seg, "cylinder_slot": seg % 4,
                "period_us": per, "rpm": round(30e6 / per, 1) if per else 0.0,
                "q": q / 100, "spv_close": close / 100, "spv_open": opn / 100, "inject": p[20],
            })
        elif typ == T_EVENT and len(p) >= 13:
            t, code, a, b = struct.unpack_from("<IBII", p)
            ev = {"t_s": round(clock(t), 6), "code": code, "event": EVENTS.get(code, str(code)), "a": a, "b": b}
            if code == 3:
                ev["detail"] = ",".join(n for i, n in enumerate(DTCS) if a >> i & 1) or "clear"
            elif code == 2:
                ev["detail"] = f"{MODES.get(b, b)} -> {MODES.get(a, a)}"
            log.events.append(ev)
        elif typ == T_META:
            for line in p.decode(errors="replace").splitlines():
                if "=" in line:
                    k2, v = line.split("=", 1)
                    log.meta[k2] = v
        elif typ == T_FILE:
            try:
                log.file.update(json.loads(p.decode()))
            except ValueError:
                pass
    log.bad_frames = getattr(frames, "bad", 0)
    return log


def read(path: str, **kw) -> Log:
    with open(path, "rb") as f:
        return parse(f.read(), **kw)


def write_csv(log: Log, outdir: str):
    os.makedirs(outdir, exist_ok=True)
    for name, rows in (("samples", log.samples), ("injections", log.injections), ("events", log.events)):
        path = os.path.join(outdir, name + ".csv")
        with open(path, "w", newline="") as f:
            if rows:
                keys = list(rows[0].keys())
                for r in rows:
                    for k in r:
                        if k not in keys:
                            keys.append(k)
                w = csv.DictWriter(f, fieldnames=keys, lineterminator="\n")
                w.writeheader()
                w.writerows(rows)
        print(path, len(rows), "rows")
    units = {c.name: c.unit for c in log.channels}
    with open(os.path.join(outdir, "channels.json"), "w") as f:
        json.dump({"meta": log.meta, "file": log.file, "units": units}, f, indent=1)


def info(log: Log) -> str:
    out = []
    out.append(f"firmware {log.meta.get('fw', '?')}, calibration {log.meta.get('cal_crc', '?')}")
    if log.file:
        out.append("file: " + ", ".join(f"{k}={v}" for k, v in log.file.items()))
    out.append(f"{len(log.samples)} samples over {log.duration_s:.1f} s, "
               f"{len(log.injections)} injections, {len(log.events)} events, {log.bad_frames} bad frames")
    out.append(f"{len(log.channels)} channels: " + ", ".join(f"{c.name} [{c.unit}]" for c in log.channels))
    if log.samples:
        rpm = [s.get("rpm", 0) for s in log.samples]
        out.append(f"rpm max {max(rpm):.0f}")
    for e in log.events[:40]:
        out.append(f"  {e['t_s']:10.3f}  {e['event']:<16} {e.get('detail', '')} a={e['a']} b={e['b']}")
    return "\n".join(out)


def fetch(base: str, outdir: str):
    """Download every drive the ECU lists that isn't already in outdir."""
    os.makedirs(outdir, exist_ok=True)
    base = base.rstrip("/")
    with urllib.request.urlopen(base + "/api/drives", timeout=10) as r:
        drives = json.load(r)["drives"]
    for d in drives:
        path = os.path.join(outdir, d["name"])
        if os.path.exists(path) and os.path.getsize(path) == d["size"]:
            continue
        print("fetching", d["name"], d["size"], "bytes")
        urllib.request.urlretrieve(f"{base}/drives/{d['name']}", path)


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    for c in ("info", "csv", "parquet"):
        s = sub.add_parser(c)
        s.add_argument("file")
        if c != "info":
            s.add_argument("outdir", nargs="?")
    s = sub.add_parser("fetch")
    s.add_argument("url", help="e.g. http://junkr.local or http://192.168.4.1")
    s.add_argument("outdir", nargs="?", default="logs")
    a = ap.parse_args(argv)
    if a.cmd == "fetch":
        fetch(a.url, a.outdir)
        return
    log = read(a.file)
    if a.cmd == "info":
        print(info(log))
    elif a.cmd == "csv":
        write_csv(log, a.outdir or os.path.splitext(a.file)[0])
    else:
        outdir = a.outdir or os.path.splitext(a.file)[0]
        os.makedirs(outdir, exist_ok=True)
        for name, df in log.to_pandas().items():
            df.to_parquet(os.path.join(outdir, name + ".parquet"))
            print(os.path.join(outdir, name + ".parquet"), len(df), "rows")


if __name__ == "__main__":
    sys.exit(main())
