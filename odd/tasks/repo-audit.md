# Repo audit

Objective: make the repository correct and consistent, per the user's request. Branch: `chore/repo-audit`. Delivery: `single-pr`; push and PR stay with the user. Checks: host tests in `firmware/src/geofence`, `reports/build.sh`.
Evidence: `odd/tasks/repo-audit-report.md`. Remove both files when the feature closes.

## Specs

- S1: "quiero que revises todo el repositorio, que todo este correcto, todos los documentos, todos los archivos, todo es todo"
- S2: "si hay un archivo que no sirva desechalo"
- S3: "revisa que no haya contradicciones"
- S4: "revisa que las reglas de gentle ai se lleven a cabo"
- S5: "que clinerules.md agents.md y claude.md no se contradigan y que tengan lo que debe ser"
- S6: "si dices que no es necesario, deja todo en claude.md"
- S7: "para modo plan sera opus 5.5 y para realizar cualquier cambio asea sonnet 5.5"

## Tasks

- [x] T1 (S5, S6, S7) Consolidate rules in `CLAUDE.md`, remove `.clinerules`, set `opusplan`. Route: delegated, user's Sonnet rule. Commit: 9cc2ee6 (after e21e4ad, which holds the user's reorganization).
- [x] T2 (S2) Remove unused files with no unique content. Route: delegated, user's Sonnet rule. Commit: 4bbd982.
- [x] T3 (S1) Report build: dropped by the user. The PDF is compiled in Overleaf and committed to `docs/`; only `.tex` content consistency matters. `CLAUDE.md` updated. Route: inline.
- [x] T3b (S3) BOM audit (done; report in scratchpad, findings applied in T7a): `hardware/BOM.md` against every other document, read-only report in the scratchpad for the user to review item by item. Route: delegated read-only explorer (mapping trigger).
- [x] T6 (S3, done by a delegated Sonnet writer; leftovers listed in L7) Record the two-board decision in README, `docs/PROJECT_CONTEXT.md`, `docs/ARCHITECTURE.md`, `docs/firmware_docs/` (PoC Thingy:91 X, then ESP32-S3, Zephyr on both, no FreeRTOS). Route: delegated, user's Sonnet rule.
- [x] T7a (S1, S3) `hardware/pinout.md` v2.0 rewritten from the WROOM-1 datasheet v1.8 and `hardware/BOM.md` v1.2 restructured. Draft, pending the user's item-by-item review. Datasheet index in `hardware/datasheets/README.md`. Route: inline.
- [ ] T7b (S1) Open BOM decisions marked POR DECIDIR: 915 MHz antenna, SX1262 RF section, regulator (200 mA LDO too small), battery 1500 vs 1200 mAh, panel voltage, CN3791/BQ2975 external parts, USB-C parts, Phase 2 modem. Blocked: user review.
- [x] T5 (S1, S3, done by a delegated Sonnet writer; leftovers in L8) Repair scrambled Markdown, stale references, arithmetic, and claims of measurements that have no data. Route: delegated, user's Sonnet rule.
- [x] T4 (S1, S2, commit f8a7d15) Fix `geofence.c` bugs, port what only the backup holds, merge the tests, remove `simple_test.c` and the backup. Route: delegated, user's Sonnet rule.
- [ ] T8 (S4) Independent verification at feature end (high-risk item 6: reassembled docs have no test).

## Log

- L1 (2026-10-30, user): "antes de continuar, quiero que revises todo el repositorio, que todo este correcto, todos los documentos, todos los archivos, todo es todo, si hay un archivo que no sirva desechalo, revisa que no haya contradicciones,ademas revisa que las reglas de gentle ai se lleven a cabo, que clinerules.md agents.md y claude.md no se contradigan y que tengan lo que debe ser, ademas debe esatr clinerules.md? ya que su contenido reclaca las reglas de gentle ai, revisalo, y si dices que no es necesario, deja todo en claude.md y para modo plan sera opus 5.5 y para realizar cualquier cambio asea sonnet 5.5, tengo entendido que cada uno de estos modelos es mejor en ese ambito que especifique."
- L2 (2026-10-30): `.clinerules` is not needed for the Claude Code and Gentle AI workflow: only Cline reads it, and its repo-specific rules now live in `CLAUDE.md`. `AGENTS.md` stays because GGA reads it. `referencia.pdf` stays: it holds content found nowhere else.
- L3 (2026-10-30): next steps kept from `RESUMEN_IMPLEMENTACION.md` before deleting it:
  > 1. **Implementación del Firmware**:
  >    - Desarrollar drivers HAL para ESP32-S3 y Nordic Thingy:91
  >    - Implementar algoritmo Point-in-Polygon optimizado
  >    - Crear aplicación Zephyr con DeviceTree overlays

- L4 (2026-10-30, user): decisions. Report PDF comes from Overleaf, not built here. Two boards: "la ESP32-S3-WROOM-1U-N16R8 que va al deseño de la pcb y la thingy:91X con nFR9151, con zephyr, para poder reciclar la mayor parte del codigo C". Proof of concept first on the Thingy:91 X, then the PCB. Work order follows project progress: documentation, then components, then Altium schematic and PCB with a connection guide, PoC in parallel. Push updates to GitHub (the user pushes). Single repo, no fork of Nordic: `firmware/` becomes a west application importing NCS (assistant proposal, accepted).
- L5 (2026-10-30, user): "especial cuidado con el BOM.md ya que ahi esta todo lo que se necesita, quiero que corrobores que si esta correcto, que todos los componentes son los que se deben usar y que entre todos los documentos del repo, no se contradigan o tengan otro componentes, luego yo reviso uno a uno".
- L6 (2026-10-30, user): asked for the pin map proposal, the BOM fixes, datasheets in a datasheets folder, flash-endurance and crystal answers, a repo-structure review, a commit and a push to GitHub, and prompt advice. Authorized the commit and push of this branch.
- L7 (2026-10-30): T6 leftovers, not fixed: optocoupler 330 ohm / 10k pull-down vs pinout (calculated resistor, 100k pull-down); power tree still assumes the DevKit AMS1117; carrier-board wording (RNF-04, NF-08) vs a bare-module PCB; roadmap and `01_descripcion.tex` omit the Thingy PoC; `lora_management.cpp`/`power_management.cpp` names; README fixed-point/no-malloc claim and unmeasured figures; Task_* core split and MVP Wi-Fi SoftAP assume the ESP32 stack; `reports/01_Defensa_Arquitectura.tex` not updated; MVP deliverable 1 wording is the writer's interpretation. All go to T5/T7b.
- L8 (2026-10-30): T4 done (commit f8a7d15). Open firmware decisions: B7 (bool cannot tell invalid from outside; public API change), `closest_boundary_point` measures to vertices only, config limits stored but not enforced, header should say the caller serializes access (GGA advisory). T5 done: `POWER_BUDGET.md` arithmetic corrected (31.30 A.s/day, 0.362 mA average, 117 days at 1200 mAh and 147 days at 1500 mAh); this conflicts with 80 days in DATASHEET/USER_MANUAL and 7 days in RNF-01. Left for the user (T7b): battery, panel, LDO and input voltage, autonomy, sleep current (5/15/50/150/190 uA), GNSS accuracy, LoRa range, modem A7670SA vs BG95-M3, polygon storage, vertex limit 32 vs 64, carrier-board wording, phases, IP rating, default password, strap material, LoRa protocol, antenna band and gain (ANTENNA_PLACEMENT designs 2.4 GHz for LoRa/GNSS), temperature range, cost, GNSS rate, opto 50 mA vs pinout, `reports/build.sh` still names the deleted root PDF, `01_Defensa_Arquitectura.tex` lacks the two-board decision, `PROJECT_CONTEXT.md` has duplicate section numbers.
