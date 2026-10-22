import argparse

from . import campaign


def main():
    ap = argparse.ArgumentParser(prog="python -m tuner")
    sub = ap.add_subparsers(dest="cmd", required=True)
    c = sub.add_parser("campaign", help="iterate learning against the simulator")
    c.add_argument("--rounds", type=int, default=15)
    c.add_argument("--drives", type=int, default=4)
    c.add_argument("--modules", default="all", help="comma list, e.g. wideband,egt,imu,gps,fuel_flow; 'none' for core only")
    c.add_argument("--mode", type=float, default=0.0, help="-1 economy .. +1 power")
    c.add_argument("--save", help="write the learned state (.npz) here")
    e = sub.add_parser("export", help="write learned tables for the tap firmware")
    e.add_argument("state", nargs="?", help=".npz saved by `campaign --save`; omit for neutral tables")
    e.add_argument("--out", default="src/tap/tables_gen.h")
    e.add_argument("--json", help="also write the tables as JSON")
    a = ap.parse_args()
    if a.cmd == "campaign":
        from .learner import Config
        mods = None if a.modules == "all" else ([] if a.modules == "none" else a.modules.split(","))
        lr, _, _, viol = campaign.run(a.rounds, a.drives, mods, Config(mode=a.mode))
        print(f"hard-limit violations during learning: {viol:.4%} of samples")
        if a.save:
            lr.save(a.save)
    elif a.cmd == "export":
        from .learner import Learner
        from .tables import TableSet
        ts = Learner.load(a.state).tables if a.state else TableSet()
        with open(a.out, "w") as f:
            f.write(ts.to_c_header())
        if a.json:
            with open(a.json, "w") as f:
                f.write(ts.to_json())
        print(f"wrote {a.out}")


main()
