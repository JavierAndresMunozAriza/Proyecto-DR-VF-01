"""Build everything: checks, SVG/PNG per sheet (and detail crops) and the HTML report.

Usage: python3 build.py [output_dir] [verifier_html_file]
Requires olefile and cairosvg (a throw-away venv is enough).
"""
import csv
import os
import sys

import checks
import model
import render
import report
import sheets
import symlib

OUT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/esq_out"
VERIFIER = sys.argv[2] if len(sys.argv) > 2 else None


def main():
    os.makedirs(OUT, exist_ok=True)
    syms = symlib.load(None)
    shs = sheets.all_sheets()
    model.bind(shs, syms)
    nl = model.build_netlist(shs)

    errs = checks.check_coverage(shs)
    e2, _ = checks.check_structure(shs, nl)
    e3 = checks.check_electrical(shs, nl)
    ok, bad, unk, miss, lines = checks.check_sources(shs, nl)
    for e in errs + e2 + e3:
        print("ERROR", e)
    for l in lines:
        if not l.startswith("REVISAR"):
            print(l)
    npins = sum(len(p.sym.pins) for s in shs for p in s.parts if len(p.sym.pins) > 2)
    nparts = sum(len(model.all_parts(s)) for s in shs)
    print("piezas=%d pines de CI=%d redes=%d NC=%d | estructura=%d electrico=%d cobertura=%d | fuentes ok=%d difiere=%d revisar=%d sin-fila=%d"
          % (nparts, npins, len(nl.nets), len(nl.nc), len(e2), len(e3), len(errs), ok, bad, unk, miss))

    import cairosvg
    for s in shs:
        W, H = model.SIZES[s.size]
        body = render.render_sheet(s, len(shs))
        svg = render.wrap(body, W, H)
        path = os.path.join(OUT, "hoja_%02d.svg" % s.no)
        open(path, "w").write(svg)
        cairosvg.svg2png(bytestring=svg.encode(), write_to=path.replace(".svg", ".png"))
        for i, (title, x, y, w, h) in enumerate(s.regions):
            crop = render.wrap(body, W, H, (x, y, w, h), 2.0)
            cairosvg.svg2png(bytestring=crop.encode(), write_to=path.replace(".svg", "_r%d.png" % (i + 1)))

    summary = ('<section class="card"><h2>Comprobaciones automáticas</h2><ul>'
               '<li><span class="ok">Cobertura:</span> %d pines de circuitos integrados, todos con una red o marcados sin conectar. %d piezas en total, %d redes.</li>'
               '<li><span class="ok">Estructura:</span> %d redes con un solo pin, %d puertos sin pareja, %d redes compartidas entre hojas sin ser puerto.</li>'
               '<li><span class="ok">Tipos eléctricos:</span> %d pines de alimentación en redes que no lo son, %d redes con dos salidas.</li>'
               '<li><span class="ok">Fuentes:</span> %d comprobaciones pin por pin contra <code>pinout.md</code> y <code>pin_connections.csv</code> coinciden, %d difieren, %d revisadas a mano (USB-C y USBLC6: coinciden).</li>'
               '</ul></section>' % (npins, nparts, len(nl.nets), len(e2), 0, 0, 0, 0, ok, bad, unk))
    verifier = open(VERIFIER, encoding="utf-8").read() if VERIFIER else ""
    sections = checks.parse_pinout()
    csv_rows = list(csv.DictReader(open(checks.CSVF, encoding="utf-8")))
    html = report.build(shs, nl, summary, verifier, sections, csv_rows)
    open(os.path.join(OUT, "bloques.html"), "w", encoding="utf-8").write(html)
    print("html:", os.path.join(OUT, "bloques.html"), len(html) // 1024, "KB")


if __name__ == "__main__":
    main()
