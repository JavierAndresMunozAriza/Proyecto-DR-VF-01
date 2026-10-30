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
- [~] T6 (S3, in progress by a delegated Sonnet writer) Record the two-board decision in README, `docs/PROJECT_CONTEXT.md`, `docs/ARCHITECTURE.md`, `docs/firmware_docs/` (PoC Thingy:91 X, then ESP32-S3, Zephyr on both, no FreeRTOS). Route: delegated, user's Sonnet rule.
- [x] T7a (S1, S3) `hardware/pinout.md` v2.0 rewritten from the WROOM-1 datasheet v1.8 and `hardware/BOM.md` v1.2 restructured. Draft, pending the user's item-by-item review. Datasheet index in `hardware/datasheets/README.md`. Route: inline.
- [ ] T7b (S1) Open BOM decisions marked POR DECIDIR: 915 MHz antenna, SX1262 RF section, regulator (200 mA LDO too small), battery 1500 vs 1200 mAh, panel voltage, CN3791/BQ2975 external parts, USB-C parts, Phase 2 modem. Blocked: user review.
- [ ] T5 (S1, S3) Repair scrambled Markdown, stale references, arithmetic, and claims of measurements that have no data. Route: delegated, user's Sonnet rule.
- [ ] T4 (S1, S2) Fix `geofence.c` bugs, port what only the backup holds, merge the tests, remove `simple_test.c` and the backup. Route: delegated, user's Sonnet rule.
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
