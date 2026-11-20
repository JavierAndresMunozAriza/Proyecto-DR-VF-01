#!/usr/bin/env python3
"""Check that every figure of the Engineering Report matches its evidence.

Recomputes the numbers of data.tex from the files in evidence/ and compares them, and checks
that the Spanish and English versions use the same figures and have the same structure.

Usage: python3 -I check_numbers.py        (from reports/engineering_report_poc/evidence)

Exits non-zero if any check fails. The static-test checks are skipped, with a warning, when
static_offsets.csv is missing.
"""
import csv
import math
import os
import re
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPORT = os.path.dirname(HERE)
LOGS = os.path.join(HERE, "logs")

failures = []
checks = 0


def check(name, ok, detail=""):
    global checks
    checks += 1
    print(("ok   " if ok else "FAIL ") + name + (f"  [{detail}]" if detail and not ok else ""))
    if not ok:
        failures.append(name)


def close(a, b, tol):
    return abs(a - b) <= tol


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


# --- data.tex -------------------------------------------------------------------
def load_data():
    macros = {}
    for m in re.finditer(r"\\newcommand\{\\(d[A-Za-z]+)\}\{([^}]*(?:\{[^}]*\}[^}]*)*)\}", read(os.path.join(REPORT, "data.tex"))):
        macros[m.group(1)] = m.group(2).strip()
    return macros


def num(macros, name):
    """The numeric value of a macro: thin spaces and TeX spacing removed."""
    raw = macros[name].replace("\\,", "").replace("~", "").split("%")[0]
    return float(re.sub(r"[^0-9.\-]", "", raw))


# --- benchmark ------------------------------------------------------------------
def check_benchmark(m):
    text = read(os.path.join(LOGS, "geofence_benchmark.log"))
    boots = re.split(r"^## boot \d+", text, flags=re.M)[1:]
    means = {}
    for boot in boots:
        for n, mean, wrong in re.findall(r"bench:\s+(\d+) vertices, \d+ checks, total \d+ us, mean ([\d.]+) us per check, (\d+) wrong", boot):
            means.setdefault(int(n), []).append(float(mean))
            check(f"benchmark {n} vertices: no wrong classification", int(wrong) == 0)
    mean_of = {n: sum(v) / len(v) for n, v in means.items()}
    names = {4: "dBenchUsFour", 16: "dBenchUsSixteen", 64: "dBenchUsSixtyFour", 256: "dBenchUsTwoFiftySix", 1024: "dBenchUsThousand"}
    check("benchmark: three boots recorded", all(len(v) == 3 for v in means.values()) and len(means) == 5)
    for n, name in names.items():
        check(f"data.tex {name} = mean of 3 boots at {n} vertices", close(num(m, name), mean_of[n], 0.06), f"{num(m, name)} vs {mean_of[n]:.2f}")
    ns = sorted(mean_of)
    xs, ys = ns, [mean_of[n] for n in ns]
    mx, my = sum(xs) / len(xs), sum(ys) / len(ys)
    slope = sum((a - mx) * (b - my) for a, b in zip(xs, ys)) / sum((a - mx) ** 2 for a in xs)
    icpt = my - slope * mx
    check("data.tex fit offset", close(num(m, "dBenchFitOffsetUs"), icpt, 0.1), f"{icpt:.2f}")
    check("data.tex fit slope", close(num(m, "dBenchFitSlopeUs"), slope, 0.01), f"{slope:.3f}")
    check("data.tex vertices that reach 50 ms", abs(num(m, "dBenchLimitVertices") - (50000 - icpt) / slope) <= 1)
    derived = {"dBenchMsSixteen": (16, 1000), "dBenchMsSixtyFour": (64, 1000), "dBenchMsTwoFiftySix": (256, 1000), "dBenchMsThousand": (1024, 1000)}
    for name, (n, div) in derived.items():
        check(f"data.tex {name} in ms", close(num(m, name), mean_of[n] / div, 0.006))
    margins = {"dBenchMarginFour": 4, "dBenchMarginSixteen": 16, "dBenchMarginSixtyFour": 64, "dBenchMarginTwoFiftySix": 256, "dBenchMarginThousand": 1024}
    for name, n in margins.items():
        check(f"data.tex {name} margin against 50 ms", close(num(m, name), 50000 / mean_of[n], 0.6 if 50000 / mean_of[n] > 20 else 0.06), f"{50000 / mean_of[n]:.2f}")


# --- build and host tests ---------------------------------------------------------
def check_build(m):
    text = read(os.path.join(LOGS, "build_memory.log"))
    block = re.search(r"FLASH:\s+(\d+) B\s+810858 B\s+([\d.]+)%\s+RAM:\s+(\d+) B\s+(\d+) B\s+([\d.]+)%", text)
    check("build_memory.log has the application block", block is not None)
    if block:
        flash, fpct, ram, region, rpct = block.groups()
        check("data.tex flash bytes", num(m, "dFlashBytes") == float(flash))
        check("data.tex flash percent", close(num(m, "dFlashPercent"), float(fpct), 0.005))
        check("data.tex flash region", num(m, "dFlashRegion") == 810858)
        check("data.tex RAM bytes", num(m, "dRamBytes") == float(ram))
        check("data.tex RAM region", num(m, "dRamRegion") == float(region))
        check("data.tex RAM percent", close(num(m, "dRamPercent"), float(rpct), 0.005))


def check_host_tests(m):
    text = read(os.path.join(LOGS, "host_tests.log"))
    tests = re.search(r"(\d+) tests, (\d+) checks, 0 failed", text)
    check("host_tests.log: engine tests all pass", tests is not None)
    if tests:
        check("data.tex engine tests", num(m, "dHostTests") == float(tests.group(1)))
        check("data.tex engine checks", num(m, "dHostChecks") == float(tests.group(2)))
    stress = re.search(r"(\d+) cases, 0 failed, worst flat-vs-haversine error ([\d.]+)%", text)
    check("host_tests.log: stress test all pass", stress is not None)
    if stress:
        check("data.tex stress cases", num(m, "dStressCases") == float(stress.group(1)))
        check("data.tex worst haversine error", close(num(m, "dStressWorstErrorPct"), float(stress.group(2)), 0.0006))
    sizes = re.findall(r"^\s*(\d+)\s+0\s+(\d+)\s+\d+\s+[0-9a-f]+\s+geofence_(cm33|esp32s3)\.o", read(os.path.join(LOGS, "cross_check.log")), re.M)
    got = {name: int(text_) for text_, _, name in sizes}
    check("cross_check.log has both object sizes", set(got) == {"cm33", "esp32s3"})
    if got:
        check("data.tex engine size Cortex-M33", num(m, "dEngineBytesCmThree") == got["cm33"])
        check("data.tex engine size ESP32-S3", num(m, "dEngineBytesEsp") == got["esp32s3"])
    check("cross_check.log covers the five other pure modules",
          all(mod in read(os.path.join(LOGS, "cross_check.log")) for mod in ("alert_color", "fence_square", "app_fsm", "demo_route", "bench_polygon")))


# --- cold-start times ---------------------------------------------------------------
def check_ttff(m):
    rows = [l for l in read(os.path.join(HERE, "field_ttff_manual.csv")).splitlines() if l and not l.startswith("#")]
    secs = [float(r[2]) for r in list(csv.reader(rows))[1:]]
    check("field_ttff_manual.csv has 3 runs", len(secs) == 3)
    for name, v in zip(("dTtffRunA", "dTtffRunB", "dTtffRunC"), secs):
        check(f"data.tex {name}", num(m, name) == v)
    check("data.tex TTFF mean", close(num(m, "dTtffMean"), statistics.mean(secs), 0.05))
    check("data.tex TTFF median", num(m, "dTtffMedian") == statistics.median(secs))
    check("data.tex TTFF standard deviation", close(num(m, "dTtffSd"), statistics.stdev(secs), 0.05))
    spec = num(m, "dTtffSpecCold")
    check("data.tex TTFF ratios to the datasheet", close(num(m, "dTtffRatioMin"), min(secs) / spec, 0.06) and close(num(m, "dTtffRatioMax"), max(secs) / spec, 0.06))


# --- static test ----------------------------------------------------------------------
def nearest_rank(sorted_values, fraction):
    return sorted_values[max(0, math.ceil(fraction * len(sorted_values)) - 1)]


def stats_of(points):
    """points: list of (east, north). Scatter around their own mean."""
    n = len(points)
    me = sum(p[0] for p in points) / n
    mn = sum(p[1] for p in points) / n
    pts = [(e - me, nn - mn) for e, nn in points]
    radial = sorted(math.hypot(e, nn) for e, nn in pts)
    return {
        "n": n,
        "cep50": nearest_rank(radial, 0.5),
        "cep95": nearest_rank(radial, 0.95),
        "drms": math.sqrt(sum(r * r for r in radial) / n),
        "se": math.sqrt(sum(e * e for e, _ in pts) / n),
        "sn": math.sqrt(sum(nn * nn for _, nn in pts) / n),
    }


def check_static(m):
    path = os.path.join(HERE, "static_offsets.csv")
    if not os.path.exists(path):
        print("warn static_offsets.csv missing: static-test checks skipped")
        return
    rows = list(csv.DictReader(read(path).splitlines()))
    fixes = [(float(r["uptime_s"]), float(r["east_m"]), float(r["north_m"]), float(r["reported_accuracy_m"])) for r in rows]
    runs, cur = [], [fixes[0]]
    for f in fixes[1:]:
        if f[0] - cur[-1][0] > 30:
            runs.append(cur)
            cur = []
        cur.append(f)
    runs.append(cur)
    check("static_offsets.csv holds two runs", len(runs) == 2)
    both = stats_of([(f[1], f[2]) for f in fixes])
    tol = 0.03
    check("data.tex fixes used (both runs)", num(m, "dStaticBothUsed") == both["n"])
    check("data.tex CEP50 both", close(num(m, "dStaticBothCepFifty"), both["cep50"], tol), f"{both['cep50']:.2f}")
    check("data.tex CEP95 both", close(num(m, "dStaticBothCepNinetyFive"), both["cep95"], tol), f"{both['cep95']:.2f}")
    check("data.tex DRMS both", close(num(m, "dStaticBothDrms"), both["drms"], tol), f"{both['drms']:.2f}")
    check("data.tex sigma east/north both", close(num(m, "dStaticBothSigmaEast"), both["se"], tol) and close(num(m, "dStaticBothSigmaNorth"), both["sn"], tol))
    for tag, run in zip(("One", "Two"), runs):
        s = stats_of([(f[1], f[2]) for f in run])
        check(f"data.tex run {tag} fixes used", num(m, f"dStaticRun{tag}Used") == s["n"])
        check(f"data.tex run {tag} CEP50", close(num(m, f"dStaticRun{tag}CepFifty"), s["cep50"], tol), f"{s['cep50']:.2f}")
        check(f"data.tex run {tag} CEP95", close(num(m, f"dStaticRun{tag}CepNinetyFive"), s["cep95"], tol), f"{s['cep95']:.2f}")
        check(f"data.tex run {tag} DRMS", close(num(m, f"dStaticRun{tag}Drms"), s["drms"], tol), f"{s['drms']:.2f}")
        acc = sorted(f[3] for f in run)
        check(f"data.tex run {tag} reported accuracy median", close(num(m, f"dStaticRun{tag}Acc"), nearest_rank(acc, 0.5), 0.06))
    acc_all = sorted(f[3] for f in fixes)
    check("data.tex reported accuracy median both", close(num(m, "dStaticBothAcc"), nearest_rank(acc_all, 0.5), 0.06))
    me_ = [sum(f[1] for f in r) / len(r) for r in runs]
    mn_ = [sum(f[2] for f in r) / len(r) for r in runs]
    check("data.tex separation of the two runs' means", close(num(m, "dStaticRunSeparation"), math.hypot(me_[1] - me_[0], mn_[1] - mn_[0]), 0.1))
    req = num(m, "dReqCep")
    check("data.tex ratios to the requirement", close(num(m, "dStaticRatioTwo"), num(m, "dStaticRunTwoCepFifty") / req, 0.06)
          and close(num(m, "dStaticRatioOne"), num(m, "dStaticRunOneCepFifty") / req, 0.06)
          and close(num(m, "dStaticRatioBoth"), num(m, "dStaticBothCepFifty") / req, 0.06))
    stable = [(f[1], f[2]) for f in fixes if f[3] <= num(m, "dStableLimit")]
    st = stats_of(stable)
    check("data.tex stable subset (reported accuracy <= limit)", num(m, "dStableUsed") == st["n"] and close(num(m, "dStableCepFifty"), st["cep50"], tol)
          and close(num(m, "dStableCepNinetyFive"), st["cep95"], tol) and close(num(m, "dStableDrms"), st["drms"], tol),
          f"{st['n']} {st['cep50']:.2f} {st['cep95']:.2f} {st['drms']:.2f}")


# --- field walks (transcribed from the phone console) -------------------------------------
def csv_rows(name):
    lines = [l for l in read(os.path.join(HERE, name)).splitlines() if l.strip() and not l.startswith("#")]
    return list(csv.DictReader(lines))


def check_walks(m):
    walk = csv_rows("field_walk_open_sky.csv")
    t = [int(r["uptime_s"]) for r in walk]
    acc = [float(r["reported_accuracy_m"]) for r in walk]
    dist = [float(r["distance_to_edge_m"]) for r in walk]
    check("open-sky walk: one row per second, no gaps", all(b - a == 1 for a, b in zip(t, t[1:])))
    check("data.tex walk fixes and seconds", num(m, "dWalkFixes") == len(walk) and num(m, "dWalkSeconds") == t[-1] - t[0])
    check("data.tex walk accuracy min, max, median", num(m, "dWalkAccMin") == min(acc) and num(m, "dWalkAccMax") == max(acc)
          and close(num(m, "dWalkAccMedian"), statistics.median(acc), 0.001))
    limit = num(m, "dReqCep")
    within = sum(a <= limit for a in acc)
    check("data.tex walk fixes within the requirement figure", num(m, "dWalkWithin") == within and num(m, "dWalkAbove") == len(acc) - within)
    check("data.tex walk percentage within", close(num(m, "dWalkWithinPct"), 100 * within / len(acc), 0.5))
    check("data.tex walk distance range", close(num(m, "dWalkDistMin"), min(dist), 0.06) and close(num(m, "dWalkDistMax"), max(dist), 0.06))
    check("data.tex walk largest step between fixes", close(num(m, "dWalkStepMax"), max(abs(b - a) for a, b in zip(dist, dist[1:])), 0.006))

    rows = csv_rows("field_walk_signal_loss.csv")
    fixes = [r for r in rows if r["event"] == "fix"]
    ignored = [r for r in rows if r["event"] == "ignored"]
    lost = [r for r in rows if r["event"] == "fix_lost"][0]
    sats = [r for r in rows if r["event"] == "sats"]
    check("data.tex crossing fixes and ignored fixes", num(m, "dCrossFixes") == len(fixes) and num(m, "dCrossIgnored") == len(ignored))
    check("data.tex crossing accuracy of accepted fixes", num(m, "dCrossAccFrom") == float(fixes[0]["reported_accuracy_m"])
          and num(m, "dCrossAccTo") == float(fixes[-1]["reported_accuracy_m"]) and num(m, "dCrossIgnoredLast") == float(ignored[-1]["reported_accuracy_m"]))
    check("every accepted fix has accuracy 50 m or better and every ignored one worse",
          all(float(r["reported_accuracy_m"]) <= 50 for r in fixes) and all(float(r["reported_accuracy_m"]) > 50 for r in ignored))
    check("data.tex FIX_LOST after the last accepted fix (s)", close(num(m, "dCrossLostS"), float(lost["uptime_s"]) - float(fixes[-1]["uptime_s"]), 0.05))
    signed = [float(r["distance_m"]) * (1 if r["inside"] == "1" else -1) for r in fixes]
    steps = [round(a - b, 2) for a, b in zip(signed, signed[1:])]
    n_const = int(num(m, "dCrossConstSteps"))
    check("data.tex crossing: steps of the last fixes are constant", min(steps[-n_const:]) == num(m, "dCrossConstLow") and max(steps[-n_const:]) == num(m, "dCrossConstHigh"))
    check("data.tex crossing: earlier steps vary", min(steps[:-n_const]) == num(m, "dCrossVarLow") and max(steps[:-n_const]) == num(m, "dCrossVarHigh"))
    check("data.tex satellites at FIX_LOST and 14 s later", num(m, "dCrossSatsFrom") == float(sats[0]["satellites"]) and num(m, "dCrossSatsTo") == float(sats[14]["satellites"])
          and close(num(m, "dCrossSatsSeconds"), float(sats[14]["uptime_s"]) - float(sats[0]["uptime_s"]), 0.5))
    inside = {int(r["check_us"]) for r in fixes if r["inside"] == "1"}
    outside = {int(r["check_us"]) for r in fixes if r["inside"] == "0"}
    check("data.tex check time inside and outside", inside == {int(num(m, "dCrossCheckInUs"))} and outside == {int(num(m, "dCrossCheckOutUs"))})


# --- the two languages -----------------------------------------------------------------
def check_languages(m):
    used = {}
    counts = {}
    for lang in ("es", "en"):
        folder = os.path.join(REPORT, "sections", lang)
        text = "".join(read(os.path.join(folder, f)) for f in sorted(os.listdir(folder)) if f.endswith(".tex"))
        used[lang] = set(re.findall(r"\\(d[A-Z][A-Za-z]+)", text))
        counts[lang] = [len(re.findall(p, text)) for p in (r"\\section\{", r"\\subsection\{", r"begin\{table\}", r"begin\{figure\}", r"begin\{equation\}")]
    check("both languages use the same data.tex figures", used["es"] == used["en"], f"only es: {sorted(used['es'] - used['en'])}, only en: {sorted(used['en'] - used['es'])}")
    check("both languages have the same sections, tables, figures and equations", counts["es"] == counts["en"], str(counts))
    undefined = sorted((used["es"] | used["en"]) - set(m))
    check("every figure used is defined in data.tex", not undefined, str(undefined))
    unused = sorted(set(m) - used["es"] - used["en"])
    if unused:
        print("warn data.tex macros defined but not used: " + ", ".join(unused))
    for lang in ("es", "en"):
        main = read(os.path.join(REPORT, f"main_{lang}.tex"))
        for f in re.findall(r"\\input\{(sections/[^}]+)\}", main):
            check(f"main_{lang}.tex includes an existing file {f}", os.path.exists(os.path.join(REPORT, f)))


def main():
    m = load_data()
    check("data.tex parsed", len(m) > 50, f"{len(m)} macros")
    for step in (check_benchmark, check_build, check_host_tests, check_ttff, check_static, check_walks, check_languages):
        step(m)
    print(f"{checks} checks, {len(failures)} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
