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

Host tests run from `firmware/src/geofence`. The unit tests are in `test_geofence.c` and exit non-zero on any failed check:

```bash
make test         # unit tests
make stress       # ~3.8M randomized cases, fixed seed (SEED=n to change it)
make sanitize     # unit + stress tests under ASan and UBSan (also finds leaks)
make cross-check  # compile geofence.c for Cortex-M33 and ESP32-S3 with strict warnings
```

`make cross-check` uses the Zephyr SDK compilers in `~/zephyr-sdk-1.0.1` (`ZEPHYR_SDK=` to change the path); it compiles but does not link. `make debug` adds debug flags and `make clean` removes every build artifact. Host timing printed by the tests proves nothing about the 50 ms target (RF-02); that needs a measurement on the board.

For the application, `geofence_check()` is the call to use: it returns inside/outside plus the distance to the nearest edge, and an error code (never "outside") for a bad GNSS fix. `geofence_distance_to_boundary()` is accurate up to 85 degrees of latitude, and the antimeridian is not supported (see `geofence.h`).

Thingy:91 X proof-of-concept app, from `firmware/`. Always go through the script: Zephyr does not support spaces anywhere in the application path (it splits `CONF_FILE` on spaces) and the repository folder name has one, so `build.sh` builds through a space-free symlink. Renaming the folder is planned and makes the symlink unnecessary:

```bash
./build.sh        # build; image in ~/.cache/dr-vf-01/build/dfu_application.zip
./build.sh flash  # flash over USB-C (MCUboot); it prints almost nothing, check the console
./build.sh clean
```

The app log is on `/dev/ttyACM0` at 115200 baud. The pure modules have their own host tests (`make test` in `firmware/src/alert`, `fence`, `app` and `demo`); `gnss_nrf91.c`, `button.c`, `rgb_led.c` and `main.c` can only be checked on the board (read the console on `/dev/ttyACM0`). Flashing from WSL2 needs a udev rule, `SUBSYSTEM=="usb", ATTR{idVendor}=="1915", MODE="0666"` in `/etc/udev/rules.d/`, or `pyusb` fails with "The device has no langid".

No linter or formatter is configured; GCC warnings are the only static check.

Report, from `reports/` (wraps `latexmk`, output in `reports/output/`):

```bash
./build.sh
```

The final PDF is compiled in Overleaf, not here, and committed as `docs/DR-VF-01_Memoria_Tecnica.pdf` (the README links to it). Keep the `.tex` sources consistent, but do not spend time making `./build.sh` work: it exits 1 at `\spanishdeactivate{~<>}` (`reports/Configuracion.tex:8`) and that is accepted.

## Architecture

- **Specs:** `docs/PROJECT_CONTEXT.md` is the master document. It links to `docs/REQUIREMENTS.md` (RF-xx and RNF-xx requirements with traceability) and `docs/ARCHITECTURE.md` (power topology and component calculations). Firmware design is in `docs/firmware_docs/`.
- **Firmware:** the geofence engine (`firmware/include/geofence.h`, `firmware/src/geofence/geofence.c`) with its host tests, plus steps 1 and 2 of the Thingy:91 X proof of concept. `firmware/src/main.c` reads real GNSS fixes (`gnss` adapter, nRF9151 backend), builds a 60 m square fence around the first accurate fix (`fence_square`), runs the application FSM (`app_fsm`: SEARCHING, MONITORING, FIX_LOST, FAULT, DEMO) and shows the distance to the edge on the RGB LED (`alert_color`, `rgb_led`, `timing`). The board's Button 1 (`button` adapter, alias `sw0`) toggles a demo mode that plays a simulated walk (`demo_route`) so the alert can be shown without GNSS signal; its logic is host-tested and it was checked on the board (the first press starts the walk, the second returns to SEARCHING). It runs on the board and was checked by eye outdoors in Manizales (green after the fix, red when walking about 25 m away). `geofence_check()` took 275 us for a 4-vertex polygon (measured with the simulated route of step 1). The MAX-M10S GNSS backend, comms, IMU, deep sleep and the ESP32-S3 board configuration exist only as designs in `docs/firmware_docs/`.
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
- **Local toolchain:** `west`, Zephyr SDK 1.0.1 (`~/zephyr-sdk-1.0.1`) and upstream Zephyr (`~/zephyrproject`) are installed. nRF Connect SDK v3.4.1 is also installed (toolchain in `~/ncs`, workspace in `~/nordic/asset-tracker-template`). The upstream `west` in the `~/zephyrproject` venv cannot build Thingy:91 X targets, so use `~/nordic/ncs-shell.sh` (for example `~/nordic/ncs-shell.sh west build -p -b thingy91x/nrf9151/ns`). `nrfutil` has `sdk-manager` and `device` (installed 2026-11-19; `west thingy91x-dfu` calls `nrfutil device program`); `nrfjprog` is not installed. Flash over USB-C with `firmware/build.sh flash` (it wraps `west thingy91x-dfu`); from WSL2 the board needs `usbipd bind` once and `usbipd attach` (it re-enumerates in MCUboot mode).
- **Geofence math:** the README now matches the code (`double`, ray casting, `malloc`/`realloc`); a fixed-point kernel is only planned.
