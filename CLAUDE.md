# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

DR-VF-01 is a virtual-fencing collar for cattle: embedded C firmware, hardware design notes, and an academic LaTeX report. Docs and the report are in Spanish. All builds and git commands run from Bash in WSL2 (Ubuntu).

## Work rules

- Read `docs/` for context before planning. Plan in plan mode, which runs on Opus 5.5, and make every change outside plan mode, on Sonnet 5.5. `.claude/settings.json` sets `opusplan`, which switches between them automatically. A subagent that writes files runs with `model: sonnet`.
- Never edit Altium files (`.PcbDoc`, `.SchDoc`, `.PrjPcb`). Record hardware changes in `hardware/BOM.md` or `hardware/pinout.md`.
- When a compile fails, read the full GCC/Clang log.
- C code follows the Linux kernel, Zephyr, or FreeRTOS coding standards.

## Before committing

1. Review the staged diff. The worktree may hold unrelated changes, so stage only what the task needs.
2. Run the checks for what changed: host tests for `firmware/`. `reports/` has no local check; Overleaf compiles it.
3. Commit without `--no-verify`. The pre-commit hook runs `gga run`, which reviews staged `*.c`, `*.h`, `*.conf` and `*.overlay` files against `AGENTS.md` using the `claude` provider. It blocks on violations but does not fix them: fix the findings, stage again, and retry. Commits with no matching files pass with a warning.

The hook lives in `.git/hooks/`, so a new clone needs `gga install`. Commit messages use Conventional Commits, and the history is written in Spanish.

## Commands

Host tests run from `firmware/src/geofence`. There is a single test file, `test_geofence.c`, and it exits non-zero on any failed check:

```bash
make test
```

`make debug` adds debug flags and `make clean` removes every build artifact. Host timing printed by the tests proves nothing about the 50 ms target (RF-02); that needs a measurement on the board.

No linter or formatter is configured; GCC warnings are the only static check.

Report, from `reports/` (wraps `latexmk`, output in `reports/output/`):

```bash
./build.sh
```

The final PDF is compiled in Overleaf, not here, and committed as `docs/DR-VF-01_Memoria_Tecnica.pdf` (the README links to it). Keep the `.tex` sources consistent, but do not spend time making `./build.sh` work: it exits 1 at `\spanishdeactivate{~<>}` (`reports/Configuracion.tex:8`) and that is accepted.

## Architecture

- **Specs:** `docs/PROJECT_CONTEXT.md` is the master document. It links to `docs/REQUIREMENTS.md` (RF-xx and RNF-xx requirements with traceability) and `docs/ARCHITECTURE.md` (power topology and component calculations). Firmware design is in `docs/firmware_docs/`.
- **Firmware:** the code today is only the geofence engine (`firmware/include/geofence.h`, `firmware/src/geofence/geofence.c`) and its host tests. The RTOS application, drivers, FSM and board configuration exist only as designs in `docs/firmware_docs/`.
- **Report:** `reports/main.tex` is the root. It loads `Configuracion.tex` and one folder per domain under `reports/secciones/` (requirements, hardware, firmware, mechanical, commercial).
- **Hardware and mechanical:** `hardware/` and `mechanical/` track Markdown specs only, with no EDA binaries.

Firmware design rules, based on Elecia White's *Making Embedded Systems*:

- `geofence.c` stays pure C with no ESP32 or Zephyr SDK dependency, so it remains testable on the host.
- HAL and drivers stay separate from real-time task logic. Peripheral drivers (LoRa, GNSS, IMU) use the Adapter pattern with a POSIX-style interface (`open`, `read`, `write`, `ioctl`, `test`).
- The main loop is an explicit FSM. Keep global state minimal, using static variables or encapsulated structs.
- The MCU stays in deep sleep as long as possible, and wakes on a GPIO interrupt from the LSM6DSOX IMU after significant movement. The polygon check takes under 50 ms (RF-02).
- Logging is encapsulated, with INFO, DEBUG, WARNING and ERROR levels and a global off switch.

## Where the sources disagree

Do not assume either side. Ask when the answer affects the work.

- **Target (decided 2026-10-30):** two boards, one codebase on Zephyr. Proof of concept on the Nordic Thingy:91 X (nRF9151, nRF Connect SDK, LTE-M/NB-IoT/GNSS), then the final PCB on the ESP32-S3-WROOM-1U-N16R8 (LoRa SX1262, external GNSS, LSM6DSOX). `geofence.c`, the FSM and logging are shared; each board has its own overlay, `prj.conf` and `gnss`/`comms`/`imu` adapters. The README and `docs/` still describe one target or the other, and that contradiction is being removed. FreeRTOS is no longer the plan.
- **Local toolchain:** `west`, Zephyr SDK 1.0.1 (`~/zephyr-sdk-1.0.1`) and upstream Zephyr (`~/zephyrproject`) are installed. nRF Connect SDK, `nrfutil` and `nrfjprog` are not, and `zephyr/boards/nordic` has no `thingy91x`.
- **Geofence math:** the README claims a fixed-point kernel with no `malloc`. `geofence.c` uses `double`, `malloc`, and ray casting.
- **Layout:** the README's `docs/3_Firmware/`, `docs/5_Comercial/` and `firmware/include/drivers/` are out of date. The real paths are `docs/firmware_docs/`, `docs/comercial_docs/` and `firmware/include/`.
