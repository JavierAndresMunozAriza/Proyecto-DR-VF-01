#!/usr/bin/env python3
"""Build docs/presentacion/DR-VF-01_Pitch.pptx.

Each slide is the full-bleed PNG exported from main.pdf (docs/presentacion/slides/),
and the speaker notes come from the script table in docs/presentacion/README.md.
The text inside the slides is part of the image: to change it, edit main.tex and rebuild.

Usage (python-pptx is the only dependency):
    python3 -m venv /tmp/venv && /tmp/venv/bin/pip install python-pptx
    /tmp/venv/bin/python make_pptx.py
"""
import re
from pathlib import Path

from pptx import Presentation
from pptx.util import Inches

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "docs" / "presentacion"

readme = (OUT / "README.md").read_text(encoding="utf-8")
rows = re.findall(r"^\| (\d+) \| (.*?) \| (\d:\d\d) \| (.*) \|$", readme, re.M)
if not rows:
    raise SystemExit("no script rows found in docs/presentacion/README.md")

prs = Presentation()
prs.slide_width = Inches(13.333)
prs.slide_height = Inches(7.5)
blank = prs.slide_layouts[6]

for n, title, start, script in rows:
    image = OUT / "slides" / f"slide-{int(n):02d}.png"
    slide = prs.slides.add_slide(blank)
    pic = slide.shapes.add_picture(str(image), 0, 0, width=prs.slide_width, height=prs.slide_height)
    pic._element.nvPicPr.cNvPr.set("descr", f"Diapositiva {n}: {title}")
    slide.notes_slide.notes_text_frame.text = f"[{start}] {title}. {script}"

prs.core_properties.title = "DR-VF-01 - Nota de aplicación (pitch)"
prs.core_properties.author = "Javier Andres Muñoz Ariza"
prs.save(OUT / "DR-VF-01_Pitch.pptx")
print(f"{len(rows)} slides written to {OUT / 'DR-VF-01_Pitch.pptx'}")
