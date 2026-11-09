#!/usr/bin/env python3
"""Read Altium library files without opening Altium.

Supports .SchLib, .PcbLib and .IntLib (the SchLib/PcbLib embedded in an IntLib
are inflated in memory). Used to verify downloaded symbols and footprints
against the manufacturer datasheets.

Requires olefile. Use a throw-away virtual environment:
    python3 -m venv /tmp/v && /tmp/v/bin/pip install olefile

Usage:
    altium_lib_check.py list FILE                 component and footprint names
    altium_lib_check.py sch  FILE [COMPONENT]     pins, electrical types, designator, footprint link
    altium_lib_check.py pcb  FILE [FOOTPRINT]     pads (mm), mask modes, texts, regions, 3D bodies

The binary offsets were reverse engineered and checked on the LSM6DS3TR-C,
ESP32-S3-WROOM-1U, MAX-M10S-00B, TLV75733PDBVR, MAX17048G+T10 and VO617A
libraries (2026-11-05). Treat results as evidence, not as proof: a pad that
looks wrong here must still be confirmed against the datasheet drawing.
"""
import io
import struct
import sys
import zlib

import olefile

MM_PER_UNIT = 0.0254 / 10000.0          # PcbLib coordinates: 1/10000 mil
PIN_TYPES = {0: "Input", 1: "I/O", 2: "Output", 3: "OpenCollector",
             4: "Passive", 5: "HiZ", 6: "OpenEmitter", 7: "Power"}
MASK_MODES = {0: "rule(0)", 1: "rule(1)", 2: "manual"}  # 0 se ve como "Rule Expansion" en Altium 26.11 (verificado en SOIC_8_3.9mm_EP); solo 2 = Manual esta documentado
# number of length-prefixed blocks that follow each PcbLib record type
BLOCKS = {1: 1, 2: 6, 3: 1, 4: 1, 5: 2, 6: 1, 11: 1, 12: 1}
RECORD_NAMES = {1: "arc", 2: "pad", 3: "via", 4: "track", 5: "text",
                6: "fill", 11: "region", 12: "3d_body"}


def _inflate(data):
    for offset in range(16):
        try:
            return zlib.decompress(data[offset:])
        except zlib.error:
            continue
    return None


def open_lib(path, kind):
    """Return an OleFileIO for the SchLib ('sch') or PcbLib ('pcb') in path."""
    ole = olefile.OleFileIO(path)
    key = "SchLib" if kind == "sch" else "PCBLib"
    streams = ole.listdir(streams=True, storages=False)
    if any(entry[0] == key for entry in streams):        # IntLib container
        entry = [e for e in streams if e[0] == key][0]
        inner = _inflate(ole.openstream(entry).read())
        if inner is None:
            sys.exit("cannot inflate %s inside the IntLib" % "/".join(entry))
        return olefile.OleFileIO(io.BytesIO(inner))
    return ole


def _items(ole, skip=("FileVersionInfo", "Library")):
    """Names of the per-component (or per-footprint) 'Data' streams."""
    return sorted(e[0] for e in ole.listdir(streams=True, storages=False)
                  if len(e) == 2 and e[1] == "Data" and e[0] not in skip)


def _pascal(buf, pos):
    length = buf[pos]
    return buf[pos + 1:pos + 1 + length].decode("latin-1"), pos + 1 + length


def read_symbol(ole, name):
    data = ole.openstream([name, "Data"]).read()
    pos, pins, info = 0, [], {"designator": None, "footprint": None, "path": ""}
    while pos + 4 <= len(data):
        word = struct.unpack_from("<I", data, pos)[0]
        size, flag = word & 0xFFFFFF, word >> 24
        rec = data[pos + 4:pos + 4 + size]
        pos += 4 + size
        if flag == 1 and rec[:4] == b"\x02\x00\x00\x00":            # binary pin
            p = 13 + rec[12]                                         # skip description
            electrical = rec[p + 1]
            q = p + 3 + 6 + 4                                        # conglomerate, length/x/y, colour
            pin_name, q = _pascal(rec, q)
            designator, _ = _pascal(rec, q)
            pins.append((designator, pin_name, PIN_TYPES.get(electrical, electrical)))
        elif flag == 0:
            text = rec.rstrip(b"\x00").decode("latin-1")
            if text.startswith("|RECORD=34|") or text.startswith("|RECORD=45|"):
                kv = dict(item.split("=", 1) for item in text.strip("|").split("|") if "=" in item)
                if text.startswith("|RECORD=34|"):
                    info["designator"] = kv.get("Text")
                else:
                    info["footprint"] = kv.get("ModelName")
                    info["path"] = kv.get("ModelDatafile0", "")
    pins.sort(key=lambda p: (len(p[0]), p[0]))
    return pins, info


def read_footprint(ole, name):
    data = ole.openstream([name, "Data"]).read()
    pos = 4 + struct.unpack_from("<I", data, 0)[0]                   # skip the footprint-name block
    pads, texts, counts = [], [], {}
    while pos < len(data):
        rtype = data[pos]
        pos += 1
        if rtype not in BLOCKS:
            counts["unknown_%d" % rtype] = 1
            break
        blocks = []
        for _ in range(BLOCKS[rtype]):
            size = struct.unpack_from("<I", data, pos)[0]
            pos += 4
            blocks.append(data[pos:pos + size])
            pos += size
        counts[RECORD_NAMES[rtype]] = counts.get(RECORD_NAMES[rtype], 0) + 1
        if rtype == 2:
            g = blocks[4]
            x, y, sx, sy = (v * MM_PER_UNIT for v in struct.unpack_from("<llll", g, 13))
            pads.append({
                "des": blocks[0][1:1 + blocks[0][0]].decode("latin-1"), "x": x, "y": y,
                "sx": sx, "sy": sy, "hole": struct.unpack_from("<l", g, 45)[0] * MM_PER_UNIT,
                "paste": (MASK_MODES.get(g[101]), struct.unpack_from("<l", g, 86)[0] * MM_PER_UNIT),
                "mask": (MASK_MODES.get(g[102]), struct.unpack_from("<l", g, 90)[0] * MM_PER_UNIT)})
        elif rtype == 5:
            height = struct.unpack_from("<l", blocks[0], 21)[0] * MM_PER_UNIT
            texts.append((blocks[0][0], round(height, 3), blocks[1][1:1 + blocks[1][0]].decode("latin-1")))
    return pads, texts, counts


def main(argv):
    if len(argv) < 3 or argv[1] not in ("list", "sch", "pcb"):
        sys.exit(__doc__)
    cmd, path = argv[1], argv[2]
    only = argv[3] if len(argv) > 3 else None
    if cmd == "list":
        print("components:", _items(open_lib(path, "sch")) if _has(path, "sch") else [])
        print("footprints:", _items(open_lib(path, "pcb")) if _has(path, "pcb") else [])
    elif cmd == "sch":
        ole = open_lib(path, "sch")
        for name in [n for n in _items(ole) if only in (None, n)]:
            pins, info = read_symbol(ole, name)
            print("== %s | designator %s | footprint %s | path %s" %
                  (name, info["designator"], info["footprint"], info["path"] or "(none)"))
            for des, pin_name, ptype in pins:
                print("  %-5s %-14s %s" % (des, pin_name, ptype))
    else:
        ole = open_lib(path, "pcb")
        for name in [n for n in _items(ole) if only in (None, n)]:
            pads, texts, counts = read_footprint(ole, name)
            print("== %s | pads %d | records %s | texts %s" % (name, len(pads), counts, texts or "none"))
            for p in sorted(pads, key=lambda q: (len(q["des"]), q["des"])):
                print("  %-5s x=%8.3f y=%8.3f size=%.3f x %.3f hole=%.3f  solder=%s/%.4f paste=%s/%.4f" %
                      (p["des"], p["x"], p["y"], p["sx"], p["sy"], p["hole"],
                       p["mask"][0], p["mask"][1], p["paste"][0], p["paste"][1]))


def _has(path, kind):
    ole = olefile.OleFileIO(path)
    streams = ole.listdir(streams=True, storages=False)
    if any(e[0] in ("SchLib", "PCBLib") for e in streams):           # IntLib
        return any(e[0] == ("SchLib" if kind == "sch" else "PCBLib") for e in streams)
    return path.lower().endswith(".schlib" if kind == "sch" else ".pcblib")


if __name__ == "__main__":
    main(sys.argv)
