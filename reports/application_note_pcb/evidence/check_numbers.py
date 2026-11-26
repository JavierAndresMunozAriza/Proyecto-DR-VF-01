"""Check the numbers of the PCB application note against their sources.

Usage (from reports/application_note_pcb/evidence):  python3 -I check_numbers.py

Sources: the Gerber zip, the BOM CSV, drc_summary.txt, docs/REQUIREMENTS.md, the PoC data.tex
and the sections themselves. Exits non-zero if any check fails.
"""
import csv, io, math, re, sys, zipfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
NOTE = HERE.parent
REPO = NOTE.parent.parent
checks = failed = 0


def check(name, ok, detail=""):
    global checks, failed
    checks += 1
    if not ok:
        failed += 1
    print(("ok   " if ok else "FAIL ") + name + (f"  [{detail}]" if detail else ""))


def macros(path):
    out = {}
    for m in re.finditer(r"\\newcommand\{\\([A-Za-z]+)\}\{(.*?)\}\s*(?:%.*)?$", path.read_text(), re.M):
        out[m.group(1)] = m.group(2).replace("\\,", "")
    return out


def num(v):
    return float(v)


d = macros(NOTE / "data.tex")
poc = macros(REPO / "reports/engineering_report_poc/data.tex")

# --- Gerbers: outline and drill report ---
z = zipfile.ZipFile(REPO / "hardware/pcb/DR-VF-01_gerbers.zip")
gm = z.read("PCB.GM").decode("latin1")
body = gm[gm.index("D66*"):]
xs = [int(v) / 1e4 for v in re.findall(r"X(\d+)", body)]
ys = [int(v) / 1e4 for v in re.findall(r"Y(\d+)", body)]
check("board width", abs((max(xs) - min(xs)) - num(d["bBoardWidth"])) < 0.06, f"{max(xs)-min(xs):.2f}")
check("board height", abs((max(ys) - min(ys)) - num(d["bBoardHeight"])) < 0.06, f"{max(ys)-min(ys):.2f}")
names = z.namelist()
check("two copper layers (GTL, GBL) and no inner layer", "PCB.GTL" in names and "PCB.GBL" in names and not any(n.endswith(".G1") for n in names))
drr = z.read("PCB.DRR").decode("latin1")
tot = re.search(r"Totals\s+(\d+)", drr)
check("drill total", tot and int(tot.group(1)) == int(d["bHoles"]), tot.group(1) if tot else "")
t1 = re.search(r"T1\s+0\.305mm.*?Round\s+(\d+)", drr)
check("0.305 mm holes", t1 and int(t1.group(1)) == int(d["bViaHoles"]))
counts = [int(x) for x in re.findall(r"^T\d\s+.*?(?:Round|Slot)\s+(\d+)", drr, re.M)]
check("drill rows add up to the total", sum(counts) == int(d["bHoles"]), str(sum(counts)))

# --- BOM ---
rows = list(csv.reader(open(REPO / "hardware/pcb/DR-VF-01_BOM.csv", encoding="utf-8")))
h = rows[0]
q = h.index("Quantity")
check("BOM lines", len(rows) - 1 == int(d["bBomLines"]), str(len(rows) - 1))
check("BOM parts", sum(int(r[q]) for r in rows[1:] if r[q].isdigit()) == int(d["bParts"]))
des = " ".join(r[h.index("Designator")] for r in rows[1:])
for ref, part in [("U1", "USBLC6"), ("U2", "4.2V 1A"), ("U3", "CN3791"), ("U4", "TLV75733P"), ("U5", "ESP32-S3-WROOM-1U"),
                  ("U6", "MAX-M10S"), ("U7", "DX-LR30"), ("U8", "LSM6DS3TR-C"), ("U9", "MAX17048"), ("U10", "VO617A"),
                  ("BZ1", "PS1240P02BT"), ("Y1", "ECS-.327")]:
    ok = any(ref in [x.strip() for x in r[h.index("Designator")].split(",")] and part in r[1] for r in rows[1:])
    check(f"BOM {ref} is {part}", ok)

# --- DRC summary ---
drc = (HERE / "drc_summary.txt").read_text()
rules = [(r, n) for r, n in re.findall(r"^(?!#)(.+)\t(\d+)$", drc, re.M) if not r.startswith(("rules checked", "violations detected", "waived"))]
n_rules = int(re.search(r"rules checked\t(\d+)", drc).group(1))
check("DRC rules", n_rules == int(d["bDrcRules"]))
check("DRC violations", re.search(r"violations detected\t(\d+)", drc).group(1) == d["bDrcViolations"])
check("every DRC rule has 0 violations", len(rules) == n_rules and all(v == "0" for _, v in rules), f"{len(rules)} rules")
def pref(pattern):
    m = re.search(pattern, drc)
    return m.group(1) if m else None
check("clearance", pref(r"Clearance Constraint \(Gap=([\d.]+)mm\) \(All\),\(All\)") == d["rClearance"])
check("via hole", pref(r"MinHoleWidth=([\d.]+)mm") == d["rViaHole"])
check("via diameter", pref(r"PreferedWidth=([\d.]+)mm") == d["rViaDiameter"])
check("RF50 width", pref(r"Preferred=([\d.]+)mm\) \(InNetClass\('RF50'\)\)") == d["rRfWidth"])
check("USB width", pref(r"Preferred=([\d.]+)mm\) \(InNetClass\('USB'\)\)") == d["rUsbWidth"])
check("PWR width", pref(r"Preferred=([\d.]+)mm\) \(InNetClass\('PWR'\)\)") == d["rPwrWidth"])
check("PWR_CN3791 width", pref(r"Preferred=([\d.]+)mm\) \(InNetClass\('PWR_CN3791'\)\)") == d["rChargerWidth"])

# --- Calculations ---
vin, vbat, fsw, L = num(d["cVin"]), num(d["cVbat"]), num(d["cFsw"]) * 1e3, num(d["cLused"]) * 1e-6
rcs = num(d["cRcs"])
ich = 0.120 / rcs * 1e3
check("charge current (120 mV / R12)", abs(ich - num(d["cIcs"])) < 1, f"{ich:.1f} mA")
ripple = vbat * (vin - vbat) / (vin * fsw * L) * 1e3
check("inductor ripple", abs(ripple - num(d["cRipple"])) < 1, f"{ripple:.1f} mA")
check("ripple as a percentage of the charge current", abs(ripple / ich * 100 - num(d["cRipplePct"])) < 1, f"{ripple/ich*100:.1f}")
check("inductor datasheet rule 5 (8 V - 3 V)", abs(5 * (8 - 3) - num(d["cLmin"])) < 0.01 and num(d["cLused"]) > num(d["cLmin"]))
vm = 1.205 * (1 + num(d["cRtop"]) / num(d["cRbottom"]))
check("MPPT voltage", abs(vm - num(d["cVmppt"])) < 0.01, f"{vm:.3f}")
check("MPPT start threshold", abs(1.23 * (1 + num(d["cRtop"]) / num(d["cRbottom"])) - num(d["cVstart"])) < 0.01)
check("MPPT divider: 390 k + 10 k = 400 k", 390 + 10 == int(d["cRtop"]))
check("PROG resistor: 1.8 k + 100 ohm = 1.9 k", abs(1.8 + 0.1 - num(d["cRprog"])) < 1e-9)
check("TP4056 current scaled from 0.91 k -> 1 A", abs(0.91 / num(d["cRprog"]) * 1000 - num(d["cIcharge"])) < 25, f"{0.91/num(d['cRprog'])*1000:.0f}")
check("TP4056 dissipation at 500 mA", abs((5 - 3.7) * 0.5 - num(d["cPtp"])) < 0.005)
check("LDO dissipation at 300 mA", abs((4.2 - 3.3) * 0.3 - num(d["cPldo"])) < 0.005)
check("deep-sleep margin", num(d["cIlimit"]) - num(d["cIsleep"]) == num(d["cMargin"]))

# --- Requirements: table 1 matches docs/REQUIREMENTS.md and table 2 covers every one ---
req = (REPO / "docs/REQUIREMENTS.md").read_text()
ids = re.findall(r"^\| \*\*(R(?:F|NF)-\d\d)\*\* \|", req, re.M)[:16]
check("REQUIREMENTS.md has 16 requirements", len(ids) == 16, str(len(ids)))
sec1 = (NOTE / "sections/es/01_requerimientos.tex").read_text()
sec7 = (NOTE / "sections/es/07_cumplimiento.tex").read_text()
for i in ids:
    check(f"{i} in table 1", re.search(rf"^{i} &", sec1, re.M) is not None)
    check(f"{i} in table 2", re.search(rf"^{i} &", sec7, re.M) is not None)
status = {"Cumple por diseño": 0, "Medido en PoC": 0, "Abierto": 0, "Sin métrica definida": 0, "Pendiente": 0}
for i in ids:
    row = re.search(rf"^{i} &(.*?)\\\\ \\hline", sec7, re.M | re.S).group(1)
    cells = row.split(" & ")
    st = cells[2] if len(cells) >= 3 else ""
    for k in status:
        if st.startswith(k):
            status[k] += 1
            break
    else:
        check(f"{i} has a known status", False, st)
check("every requirement has a status", sum(status.values()) == 16, str(status))
check("reading paragraph: 8 by design, 1 PoC, 1 open, 4 without metric, 2 pending",
      (status["Cumple por diseño"], status["Medido en PoC"], status["Abierto"], status["Sin métrica definida"], status["Pendiente"]) == (8, 1, 1, 4, 2), str(status))

# --- mil values equal the mm values (1 mil = 0.0254 mm) ---
for mm, mil in [("bBoardWidth", "mBoardWidth"), ("bBoardHeight", "mBoardHeight"), ("bThickness", "mThickness"),
                ("rClearance", "mClearance"), ("rViaHole", "mViaHole"), ("rViaDiameter", "mViaDiameter"),
                ("rRfWidth", "mRfWidth"), ("rUsbWidth", "mUsbWidth"), ("rPwrWidth", "mPwrWidth"),
                ("rChargerWidth", "mChargerWidth")]:
    check(f"{mil} is {mm} in mil", abs(num(d[mm]) / 0.0254 - num(d[mil])) < 0.5, f"{num(d[mm]) / 0.0254:.2f} vs {d[mil]}")
check("mRestWidth is 0.203 mm in mil", abs(0.203 / 0.0254 - num(d["mRestWidth"])) < 0.5)

# --- autonomy figures ---
cap = 1200 * 0.85
check("effective capacity", abs(cap - num(d["cCapEff"])) < 1)
check("autonomy, POWER_BUDGET profile", abs(cap / num(d["cIdoc"]) / 24 - num(d["cDays"])) < 1.5, f"{cap / num(d['cIdoc']) / 24:.1f}")
check("autonomy, FSM profile", abs(cap / num(d["cIfsm"]) / 24 - num(d["cDaysLow"])) < 1, f"{cap / num(d['cIfsm']) / 24:.1f}")
check("autonomy, sleep only (upper bound)", abs(cap / 0.12376 / 24 - num(d["cDaysSleep"])) < 1, f"{cap / 0.12376 / 24:.1f}")
check("energy of 7 dark days", abs(7 * 25.3 - num(d["cEnergySeven"])) < 1)

# --- Thingy figures equal those of the PoC report ---
pairs = {"pBenchUsFour": "dBenchUsFour", "pBenchMsThousand": "dBenchMsThousand", "pBenchLimitVertices": "dBenchLimitVertices",
         "pCepLow": "dStaticRunTwoCepFifty", "pCepHigh": "dStaticBothCepFifty", "pReqCep": "dReqCep",
         "pWalkAccMin": "dWalkAccMin", "pWalkAccMax": "dWalkAccMax", "pTtffA": "dTtffRunB", "pTtffB": "dTtffRunA",
         "pTtffC": "dTtffRunC", "pFlashBytes": "dFlashBytes", "pRamBytes": "dRamBytes"}
for mine, theirs in pairs.items():
    check(f"{mine} = PoC {theirs}", d[mine] == poc[theirs], f"{d[mine]} vs {poc[theirs]}")

print(f"{checks} checks, {failed} failed")
sys.exit(1 if failed else 0)
