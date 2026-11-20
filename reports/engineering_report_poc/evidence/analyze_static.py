#!/usr/bin/env python3
"""Precision statistics of a static GNSS test, without publishing coordinates.

Reads a raw console capture (private, never committed), keeps only the last boot,
and computes how much the fixes scatter around their own mean. Only derived numbers
and offsets relative to the mean position are printed or written, so the output can
be committed without revealing where the test was done.

This measures precision (scatter around the mean), not absolute accuracy: without a
reference point of known coordinates there is no way to measure the absolute error.

Usage:
  python3 -I analyze_static.py <raw.log> [offsets.csv]   analyze a capture
  python3 -I analyze_static.py --selftest                check the maths on synthetic data
"""
import csv
import math
import re
import sys

METERS_PER_DEG_LAT = 111195.0  # same constant as firmware/include/geo_constants.h
TS_RE = re.compile(r"^\[(\d+):(\d+):(\d+)\.(\d+)")
FIX_RE = re.compile(r"\bfix (-?\d+\.\d+), (-?\d+\.\d+) acc=(\d+\.\d+) m")


def uptime_s(line):
    m = TS_RE.match(line)
    if not m:
        return None
    h, mi, s, ms = (int(x) for x in m.groups())
    return h * 3600 + mi * 60 + s + ms / 1000.0


def last_boot(lines):
    """Lines of the last boot: the uptime goes backwards where a reset happened."""
    stamped = [(uptime_s(line), line) for line in lines if TS_RE.match(line)]
    start = 0
    for i in range(1, len(stamped)):
        if stamped[i][0] < stamped[i - 1][0] - 5:
            start = i
    return stamped[start:]


def parse_fixes(lines):
    fixes = []
    for t, line in last_boot(lines):
        m = FIX_RE.search(line)
        if m:
            fixes.append((t, float(m.group(1)), float(m.group(2)), float(m.group(3))))
    return fixes


def nearest_rank(sorted_values, fraction):
    return sorted_values[max(0, math.ceil(fraction * len(sorted_values)) - 1)]


def statistics(fixes):
    """Scatter of the fixes around their mean. Returns None if there are no fixes."""
    if not fixes:
        return None
    n = len(fixes)
    lat0 = sum(f[1] for f in fixes) / n
    lon0 = sum(f[2] for f in fixes) / n
    scale_e = METERS_PER_DEG_LAT * math.cos(math.radians(lat0))
    offsets = [(f[0], (f[2] - lon0) * scale_e, (f[1] - lat0) * METERS_PER_DEG_LAT, f[3])
               for f in fixes]
    radial = sorted(math.hypot(e, nn) for _, e, nn, _ in offsets)
    intervals = [b[0] - a[0] for a, b in zip(fixes, fixes[1:])]
    accs = sorted(f[3] for f in fixes)
    return {
        "fixes": n,
        "duration_s": fixes[-1][0] - fixes[0][0],
        "interval_mean_s": sum(intervals) / len(intervals) if intervals else float("nan"),
        "interval_max_s": max(intervals) if intervals else float("nan"),
        "cep50_m": nearest_rank(radial, 0.50),
        "cep95_m": nearest_rank(radial, 0.95),
        "drms_m": math.sqrt(sum(r * r for r in radial) / n),
        "sigma_east_m": math.sqrt(sum(e * e for _, e, _, _ in offsets) / n),
        "sigma_north_m": math.sqrt(sum(nn * nn for _, _, nn, _ in offsets) / n),
        "reported_acc_median_m": nearest_rank(accs, 0.50),
        "reported_acc_min_m": accs[0],
        "reported_acc_max_m": accs[-1],
        "offsets": offsets,
    }


def report(stats):
    if stats is None:
        print("no fixes in the last boot")
        return
    for key in ("fixes", "duration_s", "interval_mean_s", "interval_max_s", "cep50_m", "cep95_m",
                "drms_m", "sigma_east_m", "sigma_north_m", "reported_acc_median_m",
                "reported_acc_min_m", "reported_acc_max_m"):
        v = stats[key]
        print(f"{key:24s} {v if isinstance(v, int) else round(v, 2)}")


def selftest():
    """Points on a circle of radius 5 m around a mean: every statistic is known exactly."""
    lat0, lon0, radius = 5.0, -75.0, 5.0
    fixes = []
    for i in range(100):
        a = 2 * math.pi * i / 100
        lat = lat0 + radius * math.cos(a) / METERS_PER_DEG_LAT
        lon = lon0 + radius * math.sin(a) / (METERS_PER_DEG_LAT * math.cos(math.radians(lat0)))
        fixes.append((float(i), lat, lon, 8.0))
    s = statistics(fixes)
    checks = [
        ("100 fixes", s["fixes"] == 100),
        ("CEP50 equals the radius", abs(s["cep50_m"] - radius) < 0.01),
        ("CEP95 equals the radius", abs(s["cep95_m"] - radius) < 0.01),
        ("DRMS equals the radius", abs(s["drms_m"] - radius) < 0.01),
        ("sigma east and north are radius / sqrt(2)",
         abs(s["sigma_east_m"] - radius / math.sqrt(2)) < 0.05 and
         abs(s["sigma_north_m"] - radius / math.sqrt(2)) < 0.05),
        ("1 s interval", abs(s["interval_mean_s"] - 1.0) < 1e-9),
        ("reported accuracy median", s["reported_acc_median_m"] == 8.0),
        ("no fixes gives None", statistics([]) is None),
    ]
    # A reset in the middle must be ignored: only the last boot counts.
    lines = ["[00:00:10.000,000] <inf> main: fix 1.000000, 1.000000 acc=9.0 m",
             "[00:00:01.000,000] <inf> main: fix 5.000000, -75.000000 acc=7.0 m",
             "[00:00:02.000,000] <inf> main: fix 5.000010, -75.000000 acc=7.0 m"]
    checks.append(("only the last boot is used", len(parse_fixes(lines)) == 2))
    failed = [name for name, ok in checks if not ok]
    for name, ok in checks:
        print(("ok   " if ok else "FAIL ") + name)
    print("self-test passed" if not failed else f"{len(failed)} check(s) failed")
    return 0 if not failed else 1


def main(argv):
    if len(argv) == 2 and argv[1] == "--selftest":
        return selftest()
    if len(argv) not in (2, 3):
        print(__doc__)
        return 2
    with open(argv[1], errors="replace") as f:
        text = re.sub(r"\x1b\[[0-9;]*m", "", f.read()).replace("\r", "").replace("\x00", "")
    stats = statistics(parse_fixes(text.splitlines()))
    report(stats)
    if stats is not None and len(argv) == 3:
        with open(argv[2], "w", newline="") as out:
            writer = csv.writer(out)
            writer.writerow(["uptime_s", "east_m", "north_m", "reported_accuracy_m"])
            for t, e, nn, acc in stats["offsets"]:
                writer.writerow([f"{t:.1f}", f"{e:.2f}", f"{nn:.2f}", f"{acc:.1f}"])
        print(f"offsets relative to the mean written to {argv[2]}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
