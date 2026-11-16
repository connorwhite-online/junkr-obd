"""python3 -m pytest tools/test_junkrlog.py  (or just run it)

Decodes test/fixtures/sim_drive.jlog, which the firmware's own telemetry
code wrote during a simulated drive (pio test -e native -f test_log with
JLOG_OUT set), so this checks the C++ writer and the Python reader agree.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
import junkrlog  # noqa: E402

FIX = os.path.join(os.path.dirname(__file__), "..", "test", "fixtures", "sim_drive.jlog")


def test_crc_check_value():
    assert junkrlog.crc16(b"123456789") == 0x29B1


def test_fixture_decodes():
    log = junkrlog.read(FIX)
    assert log.bad_frames == 0
    assert 940 <= len(log.samples) <= 960
    assert log.meta["format"] == "1" and "cal_crc" in log.meta
    names = [c.name for c in log.channels]
    for n in ("rpm", "q", "adv", "pedal", "map", "ect", "flags", "dtc"):
        assert n in names
    idle = log.samples[440]
    assert 500 < idle["rpm"] < 1000 and idle["f_synced"] == 1
    assert abs(log.samples[740]["pedal"] - 40) < 0.01
    segs = [i["segment"] for i in log.injections]
    assert segs == list(range(segs[0], segs[0] + len(segs)))
    assert log.events[0]["event"] == "boot"


def test_survives_corruption():
    buf = bytearray(open(FIX, "rb").read())
    good = len(junkrlog.parse(bytes(buf)).samples)
    for k in range(1000, len(buf), 5000):
        buf[k] ^= 0xFF
    log = junkrlog.parse(bytes(buf))
    assert good - 30 < len(log.samples) < good
    assert log.bad_frames > 0


if __name__ == "__main__":
    for name, fn in list(globals().items()):
        if name.startswith("test_"):
            fn()
            print("ok", name)
