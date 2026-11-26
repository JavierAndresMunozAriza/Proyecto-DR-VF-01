# Pitch deck

Source of the 10-slide pitch of the PCB application note (Spanish, about 5 minutes).

```bash
latexmk -pdf main.tex     # pdfLaTeX with TeX Gyre Heros; images come from ../application_note_pcb/figures/
```

The cover can show the author line: fill the `\autor`, `\curso` and `\fecha` macros at the top of `main.tex`.
The published copy is `docs/presentacion/` (PDF, one PNG per slide and the speaker script).
After changing the deck, rebuild and refresh it:

```bash
pdftoppm -r 110 -png main.pdf ../../docs/presentacion/slides/slide
cp main.pdf ../../docs/presentacion/DR-VF-01_Pitch.pdf
```

To also refresh the PowerPoint (one image per slide, speaker notes from the script table in `docs/presentacion/README.md`):

```bash
python3 -m venv /tmp/venv && /tmp/venv/bin/pip install python-pptx
/tmp/venv/bin/python make_pptx.py
```
