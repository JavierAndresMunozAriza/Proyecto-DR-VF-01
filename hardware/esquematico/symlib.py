"""Read symbol geometry (pins and graphics) from an Altium .SchLib, read-only.

Reuses the stream/record handling of hardware/libraries/tools/altium_lib_check.py.
Coordinates are in Altium schematic units (10 mil), y axis pointing up.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "libraries", "tools"))
import altium_lib_check as L  # noqa: E402

LIB_PATH = ("/mnt/c/Users/Usuario/OneDrive/Documentos/Universidad/Diseño Electronico/"
            "Libreria para DR-VF_01/DR-VF-01/shared_lib.SchLib")
ELEC = L.PIN_TYPES


def _kv(text):
    out = {}
    for item in text.strip("|").split("|"):
        if "=" in item:
            k, v = item.split("=", 1)
            out[k.upper()] = v
    return out


def _num(d, key, default=0.0):
    v = d.get(key.upper())
    if v is None or v == "":
        base = default
    else:
        base = float(v)
    frac = d.get(key.upper() + "_FRAC")
    if frac:
        base += float(frac) / 100000.0
    return base


class Symbol:
    def __init__(self, name):
        self.name = name
        self.pins = []        # dicts: des,name,x,y,rot,len,elec,show_name,show_num,hx,hy
        self.prims = []       # dicts: kind + geometry (+ colours)
        self.designator = None
        self.comment = None
        self.value = None
        self.libref = name
        self.footprint = None

    def pin(self, des):
        for p in self.pins:
            if p["des"] == des:
                return p
        raise KeyError("%s: no pin %s" % (self.name, des))

    def bbox(self):
        xs, ys = [], []
        for p in self.pins:
            xs += [p["x"], p["hx"]]
            ys += [p["y"], p["hy"]]
        for pr in self.prims:
            for (x, y) in pr.get("pts", []):
                xs.append(x)
                ys.append(y)
        return (min(xs), min(ys), max(xs), max(ys))


_DIRS = {0: (1, 0), 1: (0, 1), 2: (-1, 0), 3: (0, -1)}


def load(names=None, path=LIB_PATH):
    ole = L.open_lib(path, "sch")
    items = L._items(ole)
    out = {}
    for key in items:
        if names is not None and key not in names:
            continue
        data = ole.openstream([key, "Data"]).read()
        sym = Symbol(key)
        pos = 0
        while pos + 4 <= len(data):
            word = struct.unpack_from("<I", data, pos)[0]
            size, flag = word & 0xFFFFFF, word >> 24
            rec = data[pos + 4:pos + 4 + size]
            pos += 4 + size
            if flag == 1 and rec[:4] == b"\x02\x00\x00\x00":
                p = 13 + rec[12]
                elec = rec[p + 1]
                cong = rec[p + 2]
                ln = struct.unpack_from("<h", rec, p + 3)[0]
                x = struct.unpack_from("<h", rec, p + 5)[0]
                y = struct.unpack_from("<h", rec, p + 7)[0]
                q = p + 13
                name, q = L._pascal(rec, q)
                des, _ = L._pascal(rec, q)
                rot = cong & 3
                dx, dy = _DIRS[rot]
                sym.pins.append({
                    "des": des, "name": name, "x": x, "y": y, "rot": rot, "len": ln,
                    "elec": ELEC.get(elec, str(elec)), "show_name": bool(cong & 8),
                    "show_num": bool(cong & 16) or ln >= 20,
                    "hx": x + dx * ln, "hy": y + dy * ln})
            elif flag == 0:
                text = rec.rstrip(b"\x00").decode("latin-1")
                if not text.startswith("|RECORD="):
                    continue
                d = _kv(text)
                r = d.get("RECORD")
                if r == "1":
                    sym.libref = d.get("LIBREFERENCE", key)
                elif r == "34":
                    sym.designator = {"text": d.get("TEXT", "?"), "x": _num(d, "LOCATION.X"), "y": _num(d, "LOCATION.Y")}
                elif r == "41" and d.get("NAME", "").lower() == "comment":
                    sym.comment = {"text": d.get("TEXT", ""), "x": _num(d, "LOCATION.X"), "y": _num(d, "LOCATION.Y"),
                                   "hidden": d.get("ISHIDDEN") == "T"}
                elif r == "45":
                    sym.footprint = d.get("MODELNAME")
                elif r in ("14", "10"):
                    x1, y1 = _num(d, "LOCATION.X"), _num(d, "LOCATION.Y")
                    x2, y2 = _num(d, "CORNER.X"), _num(d, "CORNER.Y")
                    sym.prims.append({"kind": "rect", "round": r == "10", "pts": [(x1, y1), (x2, y2)],
                                      "fill": d.get("AREACOLOR"), "solid": d.get("ISSOLID") == "T",
                                      "color": d.get("COLOR"), "rx": _num(d, "CORNERXRADIUS")})
                elif r == "6":
                    n = int(d.get("LOCATIONCOUNT", "0"))
                    pts = [(_num(d, "X%d" % i), _num(d, "Y%d" % i)) for i in range(1, n + 1)]
                    sym.prims.append({"kind": "poly", "pts": pts, "color": d.get("COLOR")})
                elif r == "7":
                    n = int(d.get("LOCATIONCOUNT", "0"))
                    pts = [(_num(d, "X%d" % i), _num(d, "Y%d" % i)) for i in range(1, n + 1)]
                    sym.prims.append({"kind": "polygon", "pts": pts, "fill": d.get("AREACOLOR"),
                                      "color": d.get("COLOR")})
                elif r == "13":
                    sym.prims.append({"kind": "poly", "pts": [(_num(d, "LOCATION.X"), _num(d, "LOCATION.Y")),
                                                              (_num(d, "CORNER.X"), _num(d, "CORNER.Y"))],
                                      "color": d.get("COLOR")})
                elif r == "12":
                    sym.prims.append({"kind": "arc", "pts": [(_num(d, "LOCATION.X"), _num(d, "LOCATION.Y"))],
                                      "radius": _num(d, "RADIUS"), "a0": _num(d, "STARTANGLE"),
                                      "a1": _num(d, "ENDANGLE", 360.0), "color": d.get("COLOR")})
                elif r == "4":
                    sym.prims.append({"kind": "label", "pts": [(_num(d, "LOCATION.X"), _num(d, "LOCATION.Y"))],
                                      "text": d.get("TEXT", ""), "color": d.get("COLOR")})
        out[key] = sym
    return out


def bgr(value, default="#000080"):
    """Altium colours are BGR integers."""
    if value in (None, ""):
        return default
    v = int(value)
    return "#%02x%02x%02x" % (v & 255, (v >> 8) & 255, (v >> 16) & 255)


# Pin roles of polarised two-pin symbols, read from their graphics (see build notes):
#  D_Schottky_*: triangle apex and bar on top -> pin 1 is the cathode, pin 2 the anode.
#  LED_0805_*:   triangle points right, bar on the right -> pin 1 is the anode, pin 2 the cathode.
#  Cap_*_ALU_*:  the "+" sign is drawn at the top -> pin 1 is positive.
ROLES = {
    "D_Schottky_40V_1A_SOD_123F": {"K": "1", "A": "2"},
    "LED_0805_RED": {"A": "1", "K": "2"},
    "LED_0805_Green": {"A": "1", "K": "2"},
    "Cap_100uF_ALU_TH_35V": {"+": "1", "-": "2"},
    "Sw_PUSH_4.5x4.5x5.0xmm": {"N": "2", "G": "1"},   # pin 1 = GND side by convention (pinout.md), pin 2 = net
}

if __name__ == "__main__":
    syms = load(set(sys.argv[1:]) or None)
    for n, s in sorted(syms.items()):
        print(n, s.footprint, "bbox", s.bbox(), "pins", len(s.pins))
