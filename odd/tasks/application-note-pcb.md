# Application note of the DR-VF-01 PCB (Altium design)
Objective: a professional application note, modeled on the TI/Infineon references, about the custom PCB designed in Altium. Branch: `docs/repo-reorg`. Delivery: ask-on-risk (no push). Runner: `latexmk -pdf`, `python3 -I evidence/check_numbers.py`.

## Specs
- S1: "la nota de aplicacion es para el diseno que yo hice ... al altium, de mi pcb".
- S2: "solo sugirio tener la tabla de requerimientos y luego ps la tabla de caracteristicas".
- S3: "la segunda tabla dice q si se cumplio de la tabla de requerimientos" and "decir q se cumplio de la tabla de requerimientos q ya teniamos ... la q esta en memoria tecnica".
- S4: "q se parezca a las referencias de notas de aplicacion" (TI reference designs, Infineon engineering report in `docs/references/`).
- S5: "usemos los datos de la Thingy" as indicative data from another platform; the MAX-M10S module is not bought, so no PCB measurements exist.
- S6 (assumption, from the earlier decision of 2026-11-20): Spanish first, English later; every number comes from one `data.tex` and is checked against its source.

## Tasks
- [x] T1 (S4) Scaffold `reports/application_note_pcb/`: preamble, `data.tex`, `main_es.tex`, evidence folder. Inline.
- [x] T2 (S1, S2) Sections 0 to 2 (about, requirements table 1, system). Inline. Commit: see log.
- [x] T3 (S1) Sections 3 and 4 (design theory, PCB layout). Inline.
- [x] T4 (S3, S5) Sections 5 to 7 (firmware, verification, compliance table 2). Inline.
- [x] T5 (S1) Sections 8 and 9 (limitations, files), `check_numbers.py` (88 checks, 0 failed, mutation-tested), compile (19 pages, 0 overfull), visual check. Inline.
- [ ] T6 (S1) Pending from the user: ERC report and 3D views; then update sections 2, 4 and 9.
- [ ] T7 (S6) English version.

Route: all inline (no Writer trigger: sequential sections that share one data file).

## Log
- L1 (2026-11-22): "el docente me dijo q la nota de aplicacion es para el diseno que yo hice, lo q tu decias, q la nota solo es para cuando se disena, asi que toca ahcerlo asi apra el diseno q yo hice al altium".
- L2 (2026-11-22): "no tengo el modulo todavia, usemos los datos de la Thingy".
- L3 (2026-11-22): "recuerda que el engineering report queda para otro proyecto donde el docente solo exige usar zephyr y ahi se hace de sobra, solo faltaria ... una tabla de requerimientos y luego mostrar en Engineering_Report que requerimientos se cumplieron y ahora son caracteristicas". Done in the PoC report rev. 1.0.1 (commit 21ea162).
- L4 (2026-11-22): the PCB outline from the Gerber profile is 64.4 x 88.8 mm, not 100 x 100 mm (the README said so); README fixed. Altium BOM export has wrong part parameters on 25 parts (design review BOM-02): the CSV in `hardware/pcb/` documents the design and must not be used to order parts.
- L5 (2026-11-22): the Altium BOM names the buzzer PS1240P02BT (BZ1) while `docs/REQUIREMENTS.md` RF-03 names PKMCS0909E4000-R1; the note follows the Altium BOM and flags the mismatch.
- L6 (2026-11-22): the schematic (sheet 1, read as an image) differs from `docs/ARCHITECTURE.md` 2.2/2.3: MPPT divider is R8 390 k + R9 10 k over R10 100 k (6.03 V), not 402 k / 100 k; sense resistor R12 is 0.47 ohm (255 mA), not 0.12 ohm (1 A); TP4056 PROG is R5 1.8 k + R6 100 ohm = 1.9 k. The note follows the schematic.
- L7: Draft Rev. 0.9 complete. Waiting for the user: ERC report and 3D views (sections 2, 4, 9), decisions on RF-01, RF-02, BOM-02/03/04, and review of the draft PDF.
