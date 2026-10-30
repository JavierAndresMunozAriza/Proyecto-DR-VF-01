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
2. Run the checks for what changed: host tests for `firmware/`, `./build.sh` for `reports/`.
3. Commit without `--no-verify`. The pre-commit hook runs `gga run`, which reviews staged `*.c`, `*.h`, `*.conf` and `*.overlay` files against `AGENTS.md` using the `claude` provider. It blocks on violations but does not fix them: fix the findings, stage again, and retry. Commits with no matching files pass with a warning.

The hook lives in `.git/hooks/`, so a new clone needs `gga install`. Commit messages use Conventional Commits, and the history is written in Spanish.

## Commands

Host tests run from `firmware/src/geofence`. Each test file is its own executable, so this also runs a single test:

```bash
gcc -std=c99 -Wall -Wextra -I../../include geofence.c comprehensive_test.c -o comprehensive_test -lm && ./comprehensive_test
```

The working test files are `simple_test.c`, `comprehensive_test.c` and `fixed_test_geofence.c`.

`make` and `make test` build only `test_geofence.c`, and they currently fail at link: `geofence_validate_polygon` is implemented only in `geofence.c.backup`, which is git-ignored. Port that function to `geofence.c` before deleting the backup. The README's `make` then `./comprehensive_test` steps do not work, because the Makefile never builds that binary.

No linter or formatter is configured; GCC warnings are the only static check.

Report, from `reports/` (wraps `latexmk`, output in `reports/output/`):

```bash
./build.sh
```

The report currently does not build: `./build.sh` exits 1 with the first error at `\spanishdeactivate{~<>}` (`reports/Configuracion.tex:8`), and `latexmk -f` produces no PDF either.

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

- **Target:** the README says Nordic Thingy:91 X, Zephyr with nRF Connect SDK, and LTE-M/NB-IoT. `docs/PROJECT_CONTEXT.md` and `docs/ARCHITECTURE.md` say ESP32-S3, FreeRTOS, and LoRa SX1262.
- **Local toolchain:** `west`, Zephyr SDK 1.0.1 (`~/zephyr-sdk-1.0.1`) and upstream Zephyr (`~/zephyrproject`) are installed. nRF Connect SDK, `nrfutil` and `nrfjprog` are not, and `zephyr/boards/nordic` has no `thingy91x`.
- **Geofence math:** the README claims a fixed-point kernel with no `malloc`. `geofence.c` uses `double`, `malloc`, and ray casting.
- **Layout:** the README's `docs/3_Firmware/`, `docs/5_Comercial/` and `firmware/include/drivers/` are out of date. The real paths are `docs/firmware_docs/`, `docs/comercial_docs/` and `firmware/include/`.
