#!/usr/bin/env python3
"""Read-only netlist extractor for an Altium .SchDoc (OLE file with text records).

Prints each component with the net of every pin and then the list of nets. Connectivity comes from wires,
net labels and power ports; the pin connection point is the pin location plus the pin length in the pin
direction. Use it to compare a schematic you saved against hardware/esquematico/sheets.py.
Requires olefile:  python3 -m venv /tmp/v && /tmp/v/bin/pip install olefile
Usage: altium_schdoc_nets.py FILE.SchDoc
"""
import sys
import olefile

path = sys.argv[1]
ole = olefile.OleFileIO(path)
data = ole.openstream("FileHeader").read()

recs = []
i = 0
while i + 4 <= len(data):
    ln = int.from_bytes(data[i:i + 2], "little")
    flag = data[i + 3]
    body = data[i + 4:i + 4 + ln]
    i += 4 + ln
    if flag != 0:
        continue
    txt = body.rstrip(b"\x00").decode("latin-1", "replace")
    d = {}
    for part in txt.split("|"):
        if "=" in part:
            k, v = part.split("=", 1)
            d[k.upper()] = v
    for k in list(d):
        if k.startswith("%UTF8%"):
            d[k[6:]] = d[k]
    recs.append(d)


def f(d, k):
    v = float(d.get(k, "0") or 0)
    fr = float(d.get(k + "_FRAC", "0") or 0) / 100000.0
    return round(v + fr, 3)


comps = {}
for idx, d in enumerate(recs):
    if d.get("RECORD") == "1":
        comps[idx] = {"lib": d.get("LIBREFERENCE", d.get("DESIGNITEMID", "?")), "des": "?", "val": "", "pins": [], "x": f(d, "LOCATION.X"), "y": f(d, "LOCATION.Y")}
# the owner index is the index of the component record among all records (0 = header)
owner_map = {}
for idx, d in enumerate(recs):
    if d.get("RECORD") == "1":
        owner_map[idx - 1] = idx  # OWNERINDEX counts records after the header record
for idx, d in enumerate(recs):
    r = d.get("RECORD")
    oi = d.get("OWNERINDEX")
    if oi is None:
        continue
    key = owner_map.get(int(oi))
    if key is None:
        continue
    c = comps[key]
    if r == "34":
        c["des"] = d.get("TEXT", "?")
    elif r == "41" and d.get("NAME", "").lower() in ("comment", "value"):
        c["val"] = d.get("TEXT", "")
    elif r == "2":
        rot = int(d.get("PINCONGLOMERATE", "0")) & 3
        L = f(d, "PINLENGTH"); X = f(d, "LOCATION.X"); Y = f(d, "LOCATION.Y")
        dx, dy = [(L, 0), (0, L), (-L, 0), (0, -L)][rot]
        c["pins"].append({"des": d.get("DESIGNATOR", "?"), "name": d.get("NAME", ""), "x": round(X + dx, 3), "y": round(Y + dy, 3), "rot": rot})

wires, labels, powers, junc, noerc = [], [], [], [], []
for d in recs:
    r = d.get("RECORD")
    if r == "27":
        n = int(d.get("LOCATIONCOUNT", "0"))
        pts = [(f(d, f"X{k}"), f(d, f"Y{k}")) for k in range(1, n + 1)]
        wires.append(pts)
    elif r == "25":
        labels.append((d.get("TEXT", ""), f(d, "LOCATION.X"), f(d, "LOCATION.Y")))
    elif r == "17":
        powers.append((d.get("TEXT", ""), f(d, "LOCATION.X"), f(d, "LOCATION.Y")))
    elif r == "29":
        junc.append((f(d, "LOCATION.X"), f(d, "LOCATION.Y")))
    elif r == "22":
        noerc.append((f(d, "LOCATION.X"), f(d, "LOCATION.Y")))

parent = {}


def find(a):
    parent.setdefault(a, a)
    while parent[a] != a:
        parent[a] = parent[parent[a]]
        a = parent[a]
    return a


def union(a, b):
    parent[find(a)] = find(b)


def on_seg(p, a, b):
    (px, py), (ax, ay), (bx, by) = p, a, b
    if abs(ax - bx) < 0.01 and abs(px - ax) < 0.01:
        return min(ay, by) - 0.01 <= py <= max(ay, by) + 0.01
    if abs(ay - by) < 0.01 and abs(py - ay) < 0.01:
        return min(ax, bx) - 0.01 <= px <= max(ax, bx) + 0.01
    return False


pts_all = set()
for w in wires:
    for a, b in zip(w, w[1:]):
        union(a, b)
    pts_all.update(w)
pinpts = {}
for k, c in comps.items():
    for p in c["pins"]:
        pt = (p["x"], p["y"])
        pinpts[(k, p["des"])] = pt
        pts_all.add(pt)
for _, x, y in labels + powers:
    pts_all.add((x, y))
for pt in pts_all:
    find(pt)
for pt in list(pts_all):
    for w in wires:
        for a, b in zip(w, w[1:]):
            if on_seg(pt, a, b):
                union(pt, a)
name_of = {}
for t, x, y in labels + powers:
    name_of.setdefault(find((x, y)), set()).add(t)

netid = {}


def net(pt):
    root = find(pt)
    if root in name_of:
        return "/".join(sorted(name_of[root]))
    if root not in netid:
        netid[root] = f"N{len(netid) + 1}"
    return netid[root]


print(f"componentes: {len(comps)} | cables: {len(wires)} | etiquetas: {len(labels)} | puertos de alimentación: {len(powers)} | No ERC: {len(noerc)}")
noerc_pts = set(noerc)
byroot = {}
for k, c in sorted(comps.items(), key=lambda kv: (kv[1]["des"], kv[0])):
    print(f"\n== {c['des']}  {c['lib']}  {c['val']}")
    for p in sorted(c["pins"], key=lambda p: (len(p["des"]), p["des"])):
        pt = (p["x"], p["y"])
        root = find(pt)
        n = net(pt)
        others = sum(1 for q in pts_all if find(q) == root)
        tag = "" if pt in noerc_pts else ""
        print(f"  {p['des']:>5} {p['name'][:14]:<14} -> {n}")
        byroot.setdefault(n, []).append(f"{c['des']}.{p['des']}")
print("\n== REDES (pines por red)")
for n, pl in sorted(byroot.items()):
    print(f"  {n}: {' '.join(pl)}")

if len(sys.argv) > 2:
    print("\n== DEBUG cables")
    for w in wires:
        print("  ", w)
    print("== DEBUG power/labels", powers, labels)
    for k, c in comps.items():
        print("==", c["des"], c["lib"], c["x"], c["y"])
        for p in c["pins"][:4]:
            print("    pin", p["des"], (p["x"], p["y"]), "rot", p["rot"])
    for d in recs:
        if d.get("RECORD") == "2":
            print("PINREC", {k: d[k] for k in d if k in ("PINLENGTH", "PINCONGLOMERATE", "LOCATION.X", "LOCATION.Y")}); break
