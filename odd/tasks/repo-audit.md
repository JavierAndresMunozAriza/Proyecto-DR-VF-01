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
- [ ] T3 (S1) Make the LaTeX report build. Route: delegated, user's Sonnet rule.
- [ ] T4 (S1, S2) Fix `geofence.c` bugs, port what only the backup holds, merge the tests, remove `simple_test.c` and the backup. Route: delegated, user's Sonnet rule.
- [ ] T5 (S1, S3) Repair scrambled Markdown, stale references, arithmetic, and claims of measurements that have no data. Route: delegated, user's Sonnet rule.
- [ ] T6 (S3) Resolve contradictions that depend on the hardware target. Blocked: user decision.
- [ ] T7 (S1) Resolve hardware design defects. Blocked: user decision.
- [ ] T8 (S4) Independent verification at feature end (high-risk item 6: reassembled docs have no test).

## Log

- L1 (2026-10-30, user): "antes de continuar, quiero que revises todo el repositorio, que todo este correcto, todos los documentos, todos los archivos, todo es todo, si hay un archivo que no sirva desechalo, revisa que no haya contradicciones,ademas revisa que las reglas de gentle ai se lleven a cabo, que clinerules.md agents.md y claude.md no se contradigan y que tengan lo que debe ser, ademas debe esatr clinerules.md? ya que su contenido reclaca las reglas de gentle ai, revisalo, y si dices que no es necesario, deja todo en claude.md y para modo plan sera opus 5.5 y para realizar cualquier cambio asea sonnet 5.5, tengo entendido que cada uno de estos modelos es mejor en ese ambito que especifique."
- L2 (2026-10-30): `.clinerules` is not needed for the Claude Code and Gentle AI workflow: only Cline reads it, and its repo-specific rules now live in `CLAUDE.md`. `AGENTS.md` stays because GGA reads it. `referencia.pdf` stays: it holds content found nowhere else.
- L3 (2026-10-30): next steps kept from `RESUMEN_IMPLEMENTACION.md` before deleting it:
  > 1. **Implementación del Firmware**:
  >    - Desarrollar drivers HAL para ESP32-S3 y Nordic Thingy:91
  >    - Implementar algoritmo Point-in-Polygon optimizado
  >    - Crear aplicación Zephyr con DeviceTree overlays
