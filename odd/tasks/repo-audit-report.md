# DR-VF-01 repository audit (read-only)

Audit date: 2026-10-30. Base: HEAD `d0da942` (in sync with `origin/main`), plus the uncommitted worktree.
Nothing in the repo was changed: `git status --short` still shows the same 22 lines as at the start.
All builds ran in the scratch copy `/tmp/claude-1000/-home-xavi-Proyecto-DR-VF-01/8116d6ba-1dbb-41aa-9e02-23a24130aed2/scratchpad/tmp.vvan7vsa11`.

Severity: **B** blocker (wrong, or would mislead a reader or break a build), **M** minor, **C** cosmetic.

## Overall picture

- There is one engine (`geofence.c`) and four test programs. No application firmware, no schematic and no PCB exist. `hardware/gerber`, `hardware/altium_project` and `firmware/src/tasks` are empty.
- Despite that, many documents state measured results and certifications (see E12, E20).
- 12 Markdown documents are structurally scrambled: blocks were inserted in the wrong places and tables are split, or content was appended after the closing line (see E1). Git holds no clean version of them. They first appeared already scrambled in commit `99efbaa`, and the Cline checkpoint refs do not contain an older copy.
- Three incompatible design generations coexist: ESP32-S3 DevKit (`docs/ARCHITECTURE.md`), ESP32-S3 bare BOM (`hardware/`, `docs/firmware_docs/`) and Thingy:91 X (`README.md`). Report pin tables are a leftover from a classic ESP32 design.

---

## 1. File verdicts

"Refs" means the result of `git grep` plus grep over the untracked docs. "Recoverable" means present at HEAD.

### 1.1 Delete candidates

| File | Verdict | Refs | Information found nowhere else | Recoverable |
| :--- | :--- | :--- | :--- | :--- |
| `FIRMWARE_IMPLEMENTATION_SUMMARY.md` | delete | None (only mentions itself) | Only the history of the nested-function bug, which the backup itself shows. It claims "Proper handling of edge cases and boundary conditions" (L35) and "Comprehensive error checking on all function inputs" (L57), both false (see B1, B5, B6) | Yes, `99efbaa` |
| `REPOSITORY_REORGANIZATION_SUMMARY.md` | delete | None | None. Its tree (L13-35) matches `reports/secciones/`. Its "Next steps" (L84-88) were never closed, and the build still fails | Yes |
| `RESUMEN_IMPLEMENTACION.md` | delete | Only itself (L78) | Next steps L110-113 (HAL for both platforms, DeviceTree overlays). Copy those lines elsewhere if wanted. All paths are stale (section 4) and L91 claims 200+ days | Yes |
| `docs/references/.clineignore` | delete, if Cline is no longer used | None | None. It sits in a subfolder and ignores PDFs for one tool. Its location was not tested with Cline | Yes |
| `docs/references/Informe de Selección de IA para Cline.pdf` | delete | None | Only a ranking of LLM models for Cline (2 pages). No project engineering data | Yes |
| `docs/references/Making Embedded Systems...{Elecia White}.pdf` (ignored on disk) | delete from repo, keep outside | Cited by title only, never by path | Third-party book marked "© 2024 Elecia White. All rights reserved" | Yes: same sha256 as the long-named blob at HEAD. **Included in `origin/main` (local tracking ref), so removing it from HEAD does not purge history.** Repo visibility was not checked. See 1.4 |
| `firmware/src/geofence/simple_test.c` | delete | `FIRMWARE_IMPLEMENTATION_SUMMARY.md:80`, `CLAUDE.md:29`, `.gitignore:38` (binary) | None. Calls only `geofence_init` and `geofence_deinit`, a strict subset of every other test | Yes |
| `firmware/src/geofence/geofence.c.backup` | delete **after porting**, see 1.3 | `CLAUDE.md:31`, `.gitignore:37` (`*.backup`) | Yes, 13 items, see 1.3 | Yes: byte-identical to HEAD and staged for deletion |
| `reports/main.pdf` (ignored on disk) | delete | Only `reports/output/main.pdf` is used by `build.sh`, not this file | None. Normalized text is identical to the deleted `DR-VF-01_Reporte_Final.pdf`, and it has an unresolved "Figura ??" that the Memoria does not have | Yes, staged for deletion |
| `docs/references/referencia.pdf` | **needs user decision** (archive or delete) | None | Yes. An older SRD (9 pages) with ideas found nowhere else: watchdog TPS3823, FRAM MB85RS64V, Gore vent, MOSFET power gating, BOM $56.80, LiFePO4 3200 mAh. It is the origin of the report's invalid pin table (section 2, C11) | Yes |

### 1.2 Merge candidates

| Files | Verdict | Evidence |
| :--- | :--- | :--- |
| `comprehensive_test.c`, `fixed_test_geofence.c`, `test_geofence.c` | merge into `test_geofence.c` | Coverage in 1.3b. `comprehensive_test.c` has no assertions, but it is the only caller of `geofence_calculate_distance`. `fixed_test_geofence.c` is the only one testing the south edge point `{40.7128,-74.0030}` |
| `docs/PROJECT_CONTEXT.md` L77-240 into `docs/ARCHITECTURE.md` | merge | The same sections appear twice. `ARCHITECTURE.md:187` says it was generated from them. Values found only in `PROJECT_CONTEXT`: L104 (`f_sw` 300 kHz stated in text), L229 ("más de 5 minutos" before sleep). Found only in `ARCHITECTURE`: L179-184 (open findings). `PROJECT_CONTEXT` also has duplicated section numbers 3, 4 and 5 (L49/77, L147/244, L207/259) |
| `reports/02_Manual_Operativo.tex` into the report's commercial chapter | merge or keep standalone | Not used by anything. Holds the only description of the offline walk-the-corners method ("Fijar Vértice", L40) |

### 1.3 `geofence.c.backup` versus `geofence.c`

Only the backup contains these items:

1. `geofence_validate_polygon` (L263-281), required by `test_geofence.c:74`.
2. `geofence_is_point_in_polygon_winding_number` (L163-286, interleaved with other functions).
3. `geofence_is_point_in_polygon_optimized` (L288-292), just a wrapper over ray casting.
4. `geofence_get_closest_boundary_point` (L224-261). Checks vertices only ("For simplicity", L244), not edges, so it cannot give a correct distance to the border.
5. `cross_product` helper (L348-351), needed by item 2.
6. `geofence_add_vertex` growth via `realloc` (L305-316). Its absence causes bug B3.
7. `geofence_add_vertex` coordinate validation (L300-303).
8. `geofence_init` validation: rejects double init and zero limits, and stores the config (L72-87).
9. `geofence_create_polygon` separate `NOT_INITIALIZED`/`INVALID_PARAM` results and the NULL `vertices` check (L99-113). Its absence causes B2 and B4.
10. Ray-casting coordinate validation of the point (L138-141).
11. `geofence_calculate_distance` coordinate validation (L204-208).
12. `RAD_TO_DEG` macro (L51), unused, and `g_config` (L56).
13. Orphan comment "Check if point is on the line segment" (L353-355) with no function under it.

It cannot be used as it stands. `create_polygon` never closes (L127-128), so everything after it nests inside it, and the header block repeats (L1-23, L25-47). GCC compiles it as nested functions, and `nm` shows only 6 exported symbols: `init`, `deinit`, `create_polygon`, `add_vertex`, `clear_polygon`, `destroy_polygon`. The ray-casting, winding-number, distance, closest-point and validate functions are not exported, so each must be extracted by hand.

**`CLAUDE.md:31` only mentions item 1.** Items 2-4 are declared in `geofence.h` and have no implementation elsewhere. Items 6-11 are behaviour regressions.

Item 1 is enough to link. With `geofence_validate_polygon` ported in scratch, `test_geofence` gives 7/7 and exit 0.

### 1.3b Test coverage

| Function or check | simple | comprehensive | fixed_test | test_geofence |
| :--- | :--- | :--- | :--- | :--- |
| `geofence_init`, `geofence_deinit` | yes | yes | yes | yes |
| `geofence_create_polygon`, `geofence_destroy_polygon` | - | yes | yes | yes |
| `geofence_validate_polygon` | - | - | - | yes (link failure at present) |
| Ray casting | - | 2 points, printed, **no assertion** | 5 cases | 7 cases |
| South edge `{40.7128,-74.0030}` | - | - | **only here** | - |
| Outside north, west, east | - | - | - | **only here** |
| `geofence_calculate_distance` | - | **only here, no assertion** | - | - |
| `geofence_add_vertex`, `geofence_clear_polygon` | - | - | - | - |
| Performance | - | - | avg < 50 **µs** (L144), comment says 50 ms (L143) | avg < 50 ms (L156) |
| Non-zero exit on wrong geometry | n/a | **No, always 0** | yes | yes |
| Builds today | yes | yes | yes | **no** |

`add_vertex` and `clear_polygon` are untested everywhere, and that is where B3 hides. No test checks the north or east edge, where B1 would show up. All performance runs are on the host and say nothing about the 50 ms target requirement.

### 1.4 All other files

| File | Verdict | Reason |
| :--- | :--- | :--- |
| `.clinerules` | fix | L37 "menos de 50 ms tras despertar" (see C13). L31 "Estados Finos". L10 requires a build that fails today |
| `.gga` | keep | Retargeted to C, consistent with `CLAUDE.md:17`. L2 has a placeholder URL (C) |
| `.gitignore` | fix | L21 comment says "en la raíz", but the PDF is in `docs/`. L36 repeats L27 (`*.o`). `firmware/src/geofence/geofence_test` and `..._debug` (Makefile output) are not ignored (checked with `git check-ignore`) |
| `.vscode/settings.json` | keep | Standard LaTeX Workshop config. Not exercised in VS Code |
| `.atl/skill-registry.md`, `.atl/.skill-registry.cache.json` | keep | Generated, ignored |
| `AGENTS.md` | fix | L31 "within 50 ms of waking (RF-02)". RF-02 does not say "of waking" (C13) |
| `CLAUDE.md` | fix after cleanup | Facts checked in section 5.4; update paths and status once fixes land |
| `README.md` | fix | Entirely uncommitted (HEAD has 0 bytes). Stale paths, false claims, see C1, C14, section 4, section 5 |
| `docs/ARCHITECTURE.md` | fix | Depends on target. L65 Wi-Fi disabled vs SoftAP. L70 total not corrected despite its own L182. L111 typo "haya"; L79-80 "and" |
| `docs/REQUIREMENTS.md` | fix | Parallel ID scheme to the report (C33). Its L30-38 status (all pending) is the only honest status in the repo |
| `docs/DR-VF-01_Memoria_Tecnica.pdf` | keep, regenerate | Untracked, cannot be reproduced from HEAD (section 6.6), shows "¿1 Hz" and "¡50 ms", and has an empty Bibliografía page |
| `docs/firmware_docs/BOOT_SEQUENCE.md` | fix | Byte-identical to HEAD's old path. C11, C30 |
| `docs/firmware_docs/FSM_DESIGN.md` | fix | Scrambled (E1). C12 |
| `docs/firmware_docs/MVP_DEFINITION.md` | fix | Scrambled. API not real (5.1) |
| `docs/firmware_docs/REAL_TIME_RISKS.md` | fix | Scrambled, including a code fence swallowing L145-215. Invented-looking measurements (E12) |
| `docs/comercial_docs/DATASHEET.md` | fix | Unsupported claims (E20). Tail appended after the closing line |
| `docs/comercial_docs/USER_MANUAL.md` | fix | Scrambled. C24, C25, E23 |
| `firmware/include/geofence.h` | fix | 4 declared functions have no implementation. L128-136 claim caching and spatial indexing, which do not exist |
| `firmware/src/geofence/Makefile` | fix | Section 5.3 |
| `firmware/src/geofence/geofence.c` | fix | Bugs in 5.2 |
| `firmware/src/geofence/test_geofence.c` | keep | Survivor after merge |
| `hardware/BOM.md` | fix | Scrambled. C20, C21, E14 |
| `hardware/POWER_BUDGET.md` | fix | Scrambled. E2-E6 |
| `hardware/THERMAL.md` | fix | Scrambled. E9, E10, E12 |
| `hardware/pinout.md` | fix | Scrambled tail. C11 |
| `mechanical/ANTENNA_PLACEMENT.md` | fix | Wrong band. C17 |
| `mechanical/IP67_SPEC.md` | fix | C18, C26, E18 |
| `mechanical/TOLERANCES.md` | fix | Scrambled. E16 |
| `reports/01_Defensa_Arquitectura.tex` | keep until target is decided | Standalone, builds, used by nothing. The only explicit decision record in the repo, and it rejects a Nordic part (L32-37) |
| `reports/Configuracion.tex` | fix | Section 6 |
| `reports/build.sh` | fix | L46, L49 |
| `reports/main.tex` | keep | Bibliography commented out at L60-61 (6.4) |
| `reports/references.bib` | fix or delete | Unused: no `\cite` anywhere, 3 bare entries, `esp32_datasheet` names the ESP32 series, not S3. Recoverable at HEAD |
| `secciones/01_requerimientos/01_descripcion.tex` | fix | C20 cost, L14 "10 Hz" |
| `.../02_casos_de_uso.tex` | fix | L17 "7 µA", L1 "Arquitectura V4.1" has no meaning elsewhere |
| `.../03_requerimientos_producto.tex` | fix | Text-mode `>`/`<` render wrong (6.5). C19, C20 |
| `.../04_requerimientos_srd.tex` | fix | Same rendering bug |
| `.../05_priorizacion.tex` | keep | Consistent with the Fase 1/2 scheme |
| `.../06_diagramas.tex` | fix | Invalid GPIOs (C11) |
| `.../07_conclusiones.tex` | fix | L5 FreeRTOS vs Zephyr API in the next dossier |
| `.../08_revision.tex` | fix | L14 "Aprobado... Línea base oficial" without evidence |
| `secciones/02_hardware/01_esquematicos_y_bom.tex` | fix | No schematics despite filename; E2, E9, C20 |
| `secciones/03_firmware/01_arquitectura_y_fsm.tex` | fix | C2, C12, C13, E17 |
| `secciones/04_mecanico/01_carcasa_y_tolerancias.tex` | fix | C17, C26, E19 |
| `secciones/05_comercial/01_ensayos_y_cumplimiento.tex` | fix | C7, C23, C24, E20 |

**Already deleted or moved by the user, confirmed safe:**

- `docs/3_Firmware/*`, `docs/5_Comercial/*`: all 6 new copies are byte-identical to HEAD.
- `.github/workflows/compile_latex.yml`: no remaining references.
- `DR-VF-01_Reporte_Final.pdf`: only `build.sh:46,49` still refers to it (a rebuild re-creates it at the root).
- `firmware/src/geofence/{simple_test,comprehensive_test,fixed_test_geofence}` binaries: staged for deletion, ignored, not on disk.

**Empty directories** (git tracks none of them, so removing them has no git effect):

| Directory | Referenced by |
| :--- | :--- |
| `docs/datasheets` | Nothing |
| `docs/math_models` | `PROJECT_CONTEXT.md:268` |
| `firmware/platform` | `RESUMEN_IMPLEMENTACION.md:59` |
| `firmware/include/drivers` | `README.md:86`, `RESUMEN_IMPLEMENTACION.md:55` |
| `firmware/src/tasks` | `RESUMEN_IMPLEMENTACION.md:57`, `MVP_DEFINITION.md:79` |
| `hardware/altium_project` | `PROJECT_CONTEXT.md:278` |
| `hardware/gerber` | `README.md:89`, `PROJECT_CONTEXT.md:279` |
| `reports/figures` | `RESUMEN_IMPLEMENTACION.md:77` (no `\includegraphics` exists anywhere) |

**Git residue, not a file:** 35 local refs under `refs/cline/checkpoints/*`. One of them holds `bdb65e4`, an "index on main" commit from 2026-10-27 with the same 5 staged deletions. They hold no earlier version of the scrambled docs: only one ref contains any of them, and it is identical to HEAD. Also, `git stash list` is empty, and the book and test binaries remain in pushed history.

---

## 2. Contradictions between files

| ID | Topic | Side A | Side B (and others) | Assessment | Sev |
| :--- | :--- | :--- | :--- | :--- | :--- |
| C1 | MCU / target | `README.md:61` "Nordic Thingy:91 X (PCA20065)" | `docs/PROJECT_CONTEXT.md:57` "ESP32-S3 Dual-Core"; `docs/ARCHITECTURE.md:124`; `hardware/BOM.md:26`; `reports/01_Defensa_Arquitectura.tex:35` "No se reemplazará el núcleo ESP32-S3" (rejected part is nRF52, not nRF91); `REAL_TIME_RISKS.md:168` "Pruebas en Nordic Thingy:91 X" | **Needs user decision.** See section 7 | B |
| C2 | RTOS | `README.md:62` Zephyr; `MVP_DEFINITION.md:122` "Zephyr RTOS" | `REQUIREMENTS.md:24` RNF-02 FreeRTOS; `07_conclusiones.tex:5` "firmware en FreeRTOS"; `ARCHITECTURE.md:145`. Report contradicts itself: `03_firmware:192` uses `K_THREAD_DEFINE` (Zephyr). `MVP_DEFINITION.md:156` says firmware for ESP32-S3 | **Needs user decision.** Tied to C1 | B |
| C3 | Radio | `README.md:61` LTE-M/NB-IoT; L56 "LTE-M / NB-IoT / LoRa" | `REQUIREMENTS.md:16` LoRa 915 MHz; `01_Defensa:43` "LoRa SX1262 como red primaria". Thingy:91 X listings mention cellular, Wi-Fi, BLE, 802.15.4, DECT NR+; none mention LoRa (not confirmed against Nordic's hardware guide) | **User decision.** README L56 is not achievable on Thingy alone if it has no LoRa | B |
| C3b | Future modem | `PROJECT_CONTEXT.md:24`, `ARCHITECTURE.md:72` A7670SA | `01_descripcion:8`, `01_Defensa:45`, `06_diagramas:27` Quectel BG95-M3 | BG95-M3 is more plausible: newer docs, and commit `37e2ec4`'s message says it | M |
| C3c | LoRa protocol | `DATASHEET.md:17` "Protocolo Propietario" | `05_comercial:24` "LoRaWAN o P2P"; `03_firmware:184`; `02_casos:14` Gateway | Needs user decision | M |
| C4 | GNSS accuracy | `REQUIREMENTS.md:13` "< 2.5 m"; `04_srd:17` "≤ 2.5 m CEP" | `DATASHEET.md:10` "< 3 metros RMS"; `MVP_DEFINITION.md:153`; `REAL_TIME_RISKS.md:184` "< 3m"; `05_comercial:17` | The requirement governs (2.5 m CEP). CEP and RMS are also different measures | B |
| C5 | Deep-sleep current, system | `REQUIREMENTS.md:25` "< 150 μA"; `ARCHITECTURE.md:70` "< 50 μA" | `POWER_BUDGET.md:139` "190.09μA"; `README.md:65` "< 15 µA"; `03_firmware:108` "< 20 µA"; `DATASHEET.md:26`, `FSM_DESIGN.md:121`, `MVP_DEFINITION.md:92`, `05_comercial:41` "< 5 µA"; `REAL_TIME_RISKS.md:182` "Real 3.2μA PASS" | The only itemized total is 190 µA, which **fails RNF-03 by its own numbers**. Even without the CN3791 it is 40 µA, so "< 5 µA" is unsupported. The report states 190, < 20 and < 5 | B |
| C5b | ESP32 deep sleep alone | `ARCHITECTURE.md:65` 10 µA | `POWER_BUDGET.md:17` 5 µA; `02_casos:17` 7 µA | Needs datasheet | M |
| C5c | Component currents | `ARCHITECTURE.md:65-69`: ESP 70 mA active, GNSS 15 µA / 12 mA / 35 mA, SX1262 0.6 µA / 5.3 / 118 mA | `POWER_BUDGET.md:18-40`: ESP 80 mA, GNSS 25 µA / 45 / 65 mA, SX1262 0.7 µA / 5.0 / 120 mA | GNSS differs by nearly 4×, and is the largest term in energy per event. Needs datasheet | B |
| C6 | Battery | `REQUIREMENTS.md:23`, `ARCHITECTURE.md:130` 1200 mAh | `BOM.md:56`, `USER_MANUAL.md:128`, `05_comercial:65` 1500 mAh; `DATASHEET.md:40`, `01_descripcion:13` 1200-1500 | Needs user decision. `DATASHEET.md` states both (L25, L40) | M |
| C7 | Autonomy | `REQUIREMENTS.md:23` 7 days dark; `03_req:18` "7 a 10 días" | `DATASHEET.md:25` "Hasta 80 días"; `USER_MANUAL.md:129` "60-80 días"; `MVP_DEFINITION.md:152` "> 60 días"; `POWER_BUDGET.md:177` "más de 200 días"; `02_hardware:61` "superior a 278 días"; `05_comercial:40` "80 días" | Report says 278+ and 80 days. No document derives 80. Recomputed with the repo's own inputs: 117 days (1200 mAh), 147 days (1500 mAh), see E2 | B |
| C8 | 3.3 V supply | `ARCHITECTURE.md:19-31` DevKit internal LDO, 500 mA | `BOM.md:53`, `02_hardware:16` TPS7A0533, 200 mA; `06_diagramas:78` "LDO 3.3V (500 mA)"; `02_Manual:48` dedicated LDO | Needs user decision. Peak `ARCHITECTURE.md:70` 221 mA is above the 200 mA part | B |
| C8b | LDO input | `THERMAL.md:18` "5.0V (USB/CN3791)" | `06_diagramas:92` VBAT 3.7-4.2 V; `ARCHITECTURE.md:30` 3.6-4.2 V; `pinout.md:73` 3.7-5.0 V | Power tree says VBAT, so THERMAL is wrong (E10) | B |
| C9 | Panel | `REQUIREMENTS.md:23` 6V/1.5W; `04_srd:11` 1.5-3.0 W | `BOM.md:201`, `DATASHEET.md:27`, `04_mecanico:135` 2 W; `POWER_BUDGET.md:257`, `04_mecanico:136` Voc 5.5 V; `THERMAL.md:34` 5.0 V | **Voc 5.5 V is below the 6.0 V MPPT setpoint** (`ARCHITECTURE.md:49`): the point can never be reached. USB-C 5 V (`03_req:16`) has the same problem. User decision on panel, then recalculate | B |
| C10 | Charge current | `ARCHITECTURE.md:57` 1 A (R_CS 0.12 Ω) | `THERMAL.md:37` "250mA máxima"; `POWER_BUDGET.md:73` | 2 W at 3.7 V is about 0.54 A, so neither fits. `USER_MANUAL.md:25` "2 horas" for 1500 mAh needs ≥ 0.75 A, and 250 mA would need about 6 h | M |
| C11 | Pins | see table below | | | B |
| C12 | FSM | see list below | | | B |
| C13 | 50 ms meaning | `REQUIREMENTS.md:14` "Tiempo de procesamiento < 50 ms" | `REAL_TIME_RISKS.md:13` "menos de 50ms después de salir del modo Deep Sleep"; `03_firmware:118`; `.clinerules:37`; `AGENTS.md:31`; `CLAUDE.md:55` | Same doc L124-125: wake takes ~200 ms and total ~211 ms. `FSM_DESIGN.md:49` waits up to 30 s for a fix. Only RF-02's wording (processing time) is achievable | B |
| C13b | PiP timing | `README.md:71` "under 1.2 ms" | `REAL_TIME_RISKS.md:54,72` 12 ms (8 vertices), 41 ms (32) | No measurement exists for any figure | B |
| C13c | Vertex limit | `MVP_DEFINITION.md:40` "hasta 32 vértices" | `03_firmware:153` "hasta 64 vértices"; `REAL_TIME_RISKS.md:73` "64 vértices 78ms ❌FAIL" | The report's limit fails the repo's own table | B |
| C14 | Fence model | `README.md:42-46` Haversine, radius zones | `ARCHITECTURE.md:172` polygon ray casting; `REQUIREMENTS.md:14` | Code uses polygon ray casting, so README is wrong | B |
| C14b | Math | `README.md:64,71` fixed point, no FPU | `geofence.h:29-30` `double`; `03_firmware:126` "unidad FPU"; `REAL_TIME_RISKS.md:36` `float` | README wrong | B |
| C14c | Polygon storage | `PROJECT_CONTEXT.md:237` Flash | `03_firmware:153` PSRAM; `03_firmware:183` "RAM"; `03_req:24` NVS | Needs user decision | M |
| C15 | GNSS rate | `README.md:75-76` 15 min to 5 s | `FSM_DESIGN.md:88` 60 min; `POWER_BUDGET.md:195` 6 h; `FSM_DESIGN.md:156` 24 h; `03_req:10` "> 1 Hz"; `02_casos:11` 10 Hz | Continuous tracking at 12-45 mA drains 1020 mAh in 3.5 to 0.9 days, against RNF-01 (7 days). Needs user decision | B |
| C16 | LoRa range | `DATASHEET.md:14` 15 km | `MVP_DEFINITION.md:151` "> 5km" | No link budget exists, though `README.md:16` promises one | M |
| C17 | Antenna | `ANTENNA_PLACEMENT.md:5,11` 2.4 GHz "para aplicaciones LoRa y GNSS"; `BOM.md:40` "FPC 2.4GHz" | `REQUIREMENTS.md:16` 915 MHz; `04_mecanico:106` both bands on one antenna; `DATASHEET.md:41` "8 dBi" vs `ANTENNA_PLACEMENT.md:13` 2.5 dBi; `ANTENNA_PLACEMENT.md:21` soldered vs `04_mecanico:126` U.FL; `ANTENNA_PLACEMENT.md:26` 12.5 µm vs `04_mecanico:125` 50 µm | **No 915 MHz antenna in the BOM, and the document designs the wrong band.** GNSS is about 1.575 GHz | B |
| C18 | IP rating | `DATASHEET.md:51`, `README.md:6` IP67 | `03_req:20` "salpicaduras (IP67 en Fase 2)"; `01_descripcion:8`, `05_priorizacion:25` IP68 Fase 2; `04_mecanico:3` "IP67/IP68"; `PROJECT_CONTEXT.md:26` IP67 only in Fase 4; `USER_MANUAL.md:155` "No sumerja" | Phase 1 is a splash-proof 3D print (`05_priorizacion:18`). Needs user decision | B |
| C19 | Temperature | `03_req:22` -10 to 50 °C | `DATASHEET.md:50`, `05_comercial:87`, `IP67_SPEC.md:125` -20 to +65 °C | The report has both. Needs user decision. LiPo at +65 °C should be checked against the cell datasheet | B |
| C20 | Cost | `03_req:21` "< \$150.000 COP (~35-40 USD)" | `01_descripcion:16` "\$50.00 USD"; `02_hardware:111` \$43.25 "lote de 100"; `BOM.md:191` "1000 unidades"; `referencia.pdf` \$56.80 | No per-line prices exist anywhere, so \$24.50 for electronics cannot be checked | B |
| C21 | Board type | `ARCHITECTURE.md:85`, `01_descripcion:7`, `03_req:25` 2-layer Carrier Board with modules | `BOM.md:39,46` bare ICs; `02_hardware:116` "4 capas"; `THERMAL.md:21` "PCB de 4 capas"; `ARCHITECTURE.md:124` DevKitC-1 vs `pinout.md:5` WROOM-1U | Needs user decision | B |
| C22 | Fuel gauge | `REQUIREMENTS.md:17`, `ARCHITECTURE.md:128`, `06_diagramas:53` MAX17048 | **Zero mentions in `hardware/*`, `mechanical/*`, `firmware_docs/*`, `comercial_docs/*`** | BOM, pinout and power budget omit a Must requirement | B |
| C23 | Strap | `DATASHEET.md:57`, `BOM.md:86`, `USER_MANUAL.md:125` nylon | `05_comercial:110` poliuretano | Needs user decision | M |
| C24 | Default password | `USER_MANUAL.md:47` `geofence2024` | `05_comercial:124` `geofence2026` | Needs user decision. Also a published fixed default credential | M |
| C25 | LED meaning | `USER_MANUAL.md:25`, `05_comercial:103` charge until green | `USER_MANUAL.md:77` and `05_comercial:130` "Verde fijo: Cargando" | The instruction ends the charge as soon as charging starts | M |
| C26 | Screws and flatness | `IP67_SPEC.md:87` "autofresantes"; L68-70 ±0.1/±0.05/±0.05 | `BOM.md:87` allen plus inserts; `04_mecanico:157` Torx T6; `TOLERANCES.md:30-32` 0.05/0.03/0.02 | Report uses TOLERANCES. `BOM.md:88` has 2 inserts for 4 screws | M |
| C27 | Tasks | `ARCHITECTURE.md:158-160`: 5 tasks on 2 cores | `BOOT_SEQUENCE.md:59-61` and `MVP_DEFINITION.md:128-134`: 3 different tasks, Zephyr-style | `REQUIREMENTS.md` traceability names ARCHITECTURE's tasks | M |
| C28 | Wi-Fi | `ARCHITECTURE.md:65` "WiFi deshabilitado en firmware" | `MVP_DEFINITION.md:103`, `04_srd:21`, `USER_MANUAL.md:44` SoftAP | SoftAP is a Must (`05_priorizacion:15`). ARCHITECTURE is wrong, and the power budget ignores SoftAP | M |
| C29 | Phases | `PROJECT_CONTEXT.md:20-26`: 4 phases | `01_descripcion:7-8`: 2 phases; `MVP_DEFINITION.md:173-177,190-193`: two other 4-phase lists | Needs user decision. REQUIREMENTS uses Fases 1-3 | M |
| C30 | Boot | `03_firmware:19` total 5.0 s | `BOOT_SEQUENCE.md:42` "recepción de satélites", L48 message plus confirmation; `DATASHEET.md:11` TTFF < 30 s | Boot checks cannot fit in 5 s. Watchdog 1 s (`BOOT_SEQUENCE.md:63`) is shorter than the 2 s TX and 30 s fix unless fed elsewhere | M |
| C31 | Stimulus | `DATASHEET.md:63` "Feedback Háptico", `README.md:51` "Haptic" | No motor in `BOM.md` or `pinout.md`; `REQUIREMENTS.md:15` LED only | Unsupported | M |
| C32 | Features | `DATASHEET.md:69-70`, `USER_MANUAL.md:55` multi-polygon, exclusion zones | `geofence.h` single polygon; `MVP_DEFINITION.md:40` | Unsupported | M |
| C33 | Requirement IDs | `REQUIREMENTS.md` RF-01..05, RNF-01..04 | `03_req` F-01..08, NF-01..08; `04_srd` SR-xx; no mapping | Values agree where they overlap, apart from C4, C7 and C5 (the report has no RNF-03 equivalent) | M |
| C34 | Dates | `BOM.md:156`, `pinout.md:199`, `TOLERANCES.md:138` 2024 revisions; `USER_MANUAL.md:215` "© 2024" | First commit 2026-10-18; `08_revision:12` Sept 2026 | The 2024 histories look invented | M |
| C35 | Status | `README.md:3,10` "firmware-passing", "Production-grade" | `REQUIREMENTS.md:30-38` all pending | REQUIREMENTS is right | B |
| C36 | LoRa OTA | `05_comercial:207` OTA via LoRaWAN | `02_Manual:34` "LoRa es demasiado lento" | Manual is more plausible | M |

### C11 pin table

| Signal | `hardware/pinout.md` | `docs/ARCHITECTURE.md` | Report `06_diagramas.tex` | `BOOT_SEQUENCE.md` |
| :--- | :--- | :--- | :--- | :--- |
| GNSS | UART1 GPIO4/5 (L27-28) | I2C plus UART0 43/44 (L107-112) | UART1 16/17 (L51) | - |
| I2C SDA/SCL | 17/18 (L29-30) | 1/2 (L108-109) | 21/**22** (L52) | 17/18 (L22) |
| LoRa MOSI/MISO/SCK/CS | 7/6/5/8 (L31-34) | 11/12/13/10 (L100-103) | **23**/19/18/5 (L54) | 7/6/5/8 (L29) |
| LoRa DIO1/BUSY/RST | 9/10/11 (L35-37) | 14/9/21 (L104-106) | 4/2/none (L54) | - |
| Opto | GPIO1 (L53) | GPIO5 (L115) | GPIO13 (L55) | - |
| Buzzer | GPIO2 (L54) | none | GPIO14 (L55) | - |
| IMU interrupt | GPIO15 (L64) | GPIO4 (L114) | none | - |
| Fuel gauge | none | shared I2C | shared I2C | - |
| Phase 2 modem | none | none | **26/27** (L57) | - |

- ESP32-S3 has no GPIO22-25. Espressif's pin table skips them. GPIO26-32 serve flash and PSRAM, and GPIO33-37 serve octal PSRAM on R8 parts ([Espressif GPIO docs](https://docs.espressif.com/projects/esp-idf/en/v4.4.2/esp32s3/api-reference/peripherals/gpio.html)).
- So the report's pins 22, 23, 26, 27 are impossible or reserved, and `pinout.md:46-47` uses non-existent GPIO22 and GPIO23.
- The report's set matches the classic ESP32 table in `referencia.pdf` (GPIO 16/17, 21/22, 5/18/19/23, 26/27), which is its likely origin.
- `pinout.md` also assigns GPIO05 twice (L28 UART RX and L33 SPI SCK), and wires GNSS TX to the MCU's TX (L101).
- **If ESP32-S3 stays, `ARCHITECTURE.md` is the only set that avoids invalid pins** (its own L118 notes UART0 conflict).

### C12 FSM comparison

| Item | `README.md:28-56` | `FSM_DESIGN.md` | `03_firmware:68-114` |
| :--- | :--- | :--- | :--- |
| State names | DEEP_SLEEP, GNSS_ACQUIRE, GEOFENCE_EVAL, zones, AUDIO_CUE_TRIGGER, CORRECTION_STIMULUS, TELEMETRY_DISPATCH | INIT, SLEEP, GEOFENCE_CHECK, WARNING, DISUASION, GNSS_UPDATE, ERROR | Same 7 as FSM_DESIGN |
| INIT and ERROR | Neither exists | Both | Both |
| WARNING means | Near the perimeter | **Already outside** (L54, L61), 10 s timer | **Approaching**, < 5 m (L111) |
| GNSS acquisition | Own state, before evaluation | Inside GEOFENCE_CHECK (L48); GNSS_UPDATE is a 60-min telemetry cycle (L88) | GEOFENCE_CHECK to GNSS_UPDATE on stale fix |
| Exit from ERROR | n/a | To INIT or SLEEP (L258-259) | **None drawn: a sink state** |
| CHECK to DISUASION direct | n/a | "Alarma activada" (L55), trigger undefined | Not drawn |
| Telemetry | Every cycle | Only on alert or hourly | Not shown |

The report's WARNING cannot be evaluated by today's code, since distance to the border is unimplemented (5.1).

---

## 3. Errors inside single documents

**E1 (B). Scrambled documents.** Inserted blocks broke tables and sections. Examples:

| File | Evidence |
| :--- | :--- |
| `hardware/POWER_BUDGET.md` | Table headers at L167, L198, rows at L280-287, L270-275. Conclusions twice (L175, L242). Component specs twice (L14-89, L92-125) |
| `hardware/BOM.md` | Cost header L117-118, rows L167-173. Panel header L97-98, rows L200-204 (resistors end up under Panel Solar) |
| `hardware/THERMAL.md` | Table header L100-101, rows L149-151. Conclusions twice (L140, L178) |
| `hardware/pinout.md` | L190 sentence splits the table L187-193 |
| `docs/firmware_docs/FSM_DESIGN.md` | All states twice (L12-114, L161-236). Orphan fragments L109-114, L155-160 |
| `docs/firmware_docs/REAL_TIME_RISKS.md` | Table split L52-54 / L71-73. Code fence opened at L130 closes at L225, swallowing L145-215 as code. Sections 1, 2, 3 and validation twice |
| `docs/firmware_docs/MVP_DEFINITION.md` | Title twice, Objective twice (L10, L162), closing sentence twice |
| `docs/comercial_docs/USER_MANUAL.md` | L119 problem, answer at L239; Warranty L145, body L216; Reset L244, steps L264 |
| `mechanical/TOLERANCES.md` | O-ring table L84-85 continues at L160; GD&T L104 continues at L149; L50 row has too few columns |
| `docs/comercial_docs/DATASHEET.md`, `mechanical/IP67_SPEC.md`, `mechanical/ANTENNA_PLACEMENT.md` | Content appended after the closing line |

The report chapters 02-05 are coherent and can serve as a guide to the intended order.

**E2 (B). Power budget total is wrong.** `POWER_BUDGET.md:271-279`: rows are 16.42, 10.75, 2.80, 1.30 and the stated total is 16.46.

- Correct sum: **31.27 A·s/day**. The stated total effectively ignores all active events.
- Average current: 31.27 / 86400 = **0.362 mA, not 0.191 mA**.
- Autonomy with the doc's own factor of 0.85: 1020 mAh / 0.362 = **117 days** (1200 mAh), 1275 / 0.362 = **147 days** (1500 mAh), against the stated 222 and 278. `02_hardware:57-61` inherits the error ("327 días", "278 días").
- Adding the 4 daily GNSS checks that the doc assumes at L195 but leaves out of the table: 0.412 mA (103 and 129 days).
- With realistic SF12 TX time (E5): 0.441 mA.

**E3 (M). Event sums and units.** Stated vs recomputed: GEOFENCE_CHECK 1074.63 vs **1077.63** mAs (L151). LORA_TX 140.175 vs 140.185 (L162). DISUASION 650.8825 vs 650.9325 (L287). "Ah*s" (L271, L275) should be A·s. State labels contradict the component tables: SX1262 "Sleep" 1.5 µA (L149, L284) is the standby figure, LSM6DSOX "Sleep" 10 µA (L159, L283) is the low-power figure.

**E4 (M). Duty cycle.** "98.5%" (L57). Events total 10×22 + 20×1 + 2×5 = 250 s/day, so sleep is 99.71%. The budget also leaves out MAX17048 and the LDO.

**E5 (B). LoRa TX time.** `POWER_BUDGET.md:59,160` and `THERMAL.md:52` use 0.5 s. At the documented settings (SF12, 125 kHz, 20-byte payload, `MVP_DEFINITION.md:68-69,81`; CR 4/5, preamble 8, explicit header and CRC assumed), the standard LoRa formula gives **1.32 s**, so 158 mAs of radio energy per TX instead of 60. `FSM_DESIGN.md:120` "< 2 s" is consistent with this.

**E6 (B). Solar balance and fit.** `POWER_BUDGET.md:263-264`: arithmetic is right (2430 mAh/day) but ignores efficiency, tilt and shading on an animal. Fill factor 2 W / (5.5 V × 0.4 A) = 0.91 is unusually high. Panel area for 2 W at 20% and 1000 W/m² is **100 cm²**, while the enclosure top is 12 × 8 = **96 cm²**, which also holds the FPC antenna (`ANTENNA_PLACEMENT.md:32`, `04_mecanico:142`).

**E7 (B). LDO dropout.**

- ARCHITECTURE topology: with ~1 V dropout (L33), output is at most 3.2 V even at 4.2 V, so it never reaches "3.3V Estrictos" (L31). At 3.6 V it is 2.6 V. L33 and L181 understate this.
- TPS7A0533: dropout 235 mV max at 200 mA (secondary source, not checked against TI's table) needs VBAT ≥ about 3.54 V, while `05_comercial:133` sets critical at ≤ 3.40 V and `BOOT_SEQUENCE.md:10` accepts > 3.0 V. Part of the LiPo range is therefore out of regulation.

**E8 (B). MPPT divider.** Arithmetic in `ARCHITECTURE.md:50` holds for V_ref = 2.0 V. `PROJECT_CONTEXT.md:114` states 2.0 V as fact, while `ARCHITECTURE.md:54,183` doubt it. A CN3791 datasheet excerpt (search snippet only; the PDF could not be opened, and the same excerpt holds a stray "1.23V") says the pin regulates to **1.205 V**. Then 200k/100k fixes the panel at **3.6 V, not 6 V**, and 6 V needs R1/R2 ≈ 3.98. L (14 µH) and R_CS (0.12 Ω) arithmetic is correct. The 120 mV figure was not confirmed.

**E9 (B). LDO thermal resistance.** `THERMAL.md:21` and `02_hardware:75` use 60.9 °C/W. TI lists **185.6 °C/W** for TPS7A05 DBV ([TI thermal table](https://www.ti.com/document-viewer/TPS7A05/datasheet/thermal-information-sbvs2546647-1)).

| Case | Vin | I | P | ΔT (TI) | Tj at 65 °C |
| :--- | :--- | :--- | :--- | :--- | :--- |
| Doc's case | 5.0 V | 150 mA | 0.255 W | 47.3 °C | 112.3 °C |
| Doc's overload (L154, stated 85.7 °C) | 5.0 V | 200 mA | 0.340 W | 63.1 °C | **128.1 °C, over the 125 °C limit in L29** |
| Power tree input | 4.2 V | 150 mA | 0.135 W | 25.1 °C | 90.1 °C |
| Power tree input | 4.2 V | 200 mA | 0.180 W | 33.4 °C | 98.4 °C |

**E10 (M). THERMAL scenario.** "Operación Normal" (L61-67) assumes 150 mA continuous, while POWER_BUDGET sleeps > 98% of the time. CN3791 Tj (L150) reuses the LDO's RθJA. L155 P = 0.172 W, not 0.168. L64-66 percentages sum to 100.3%. L74 SX1262 TX 0.024 W vs L55 0.396 W.

**E11 (M). Peak current.** `ARCHITECTURE.md:70` and `PROJECT_CONTEXT.md:66,134` say 221 mA. The table sums to 224 mA, as L182 admits, without fixing the table. 394 mA with the 240 mA ESP32 figure.

**E12 (B). Measurements with no supporting data.** No hardware exists.

- `THERMAL.md:104-114` "Pruebas Térmicas Realizadas" with FLIR and chamber. Its L112 "10% menor que cálculo": calculation at 25 °C is 40.5 °C, so 45 °C is **11% higher**. L114 "Coincidencia exacta" has no calculation to match.
- `REAL_TIME_RISKS.md:51-54,71-73` "Profiling en Hardware Real", and L181-184, L268-271 "Real ... PASS".
- 41 ms for 32 edges at 240 MHz is about 307,000 cycles per edge, which is implausible for this loop.
- L49 "~10ns vs 150ns" treats external PSRAM as faster than internal memory, while L78 caches in SRAM to speed up.

**E13 (M). Host result used as evidence.** `FIRMWARE_IMPLEMENTATION_SUMMARY.md:50-52` "0.01 microseconds... Suitable for embedded system deployment". I reproduced 0.01 µs, but on x86 with 4 vertices. `fixed_test_geofence.c:144` tests < 50 **µs** while its comment at L143 says 50 ms.

**E14 (B). BOM parts.**

- L27-28: Flash and SRAM are redundant, since the module has 16 MB and 8 MB, and "8Mbit" is not 8 MB.
- L74: "Cristal 40MHz" with MPN `ECS-.327-...`, whose code suggests a 32.768 kHz part (not checked).
- L60, L94: USB-C connector and blanking cap share `1240-6813-1-ND`, a Digi-Key number, not an MPN.
- L54: CN3791 listed from "ChipsBase"; distributors list Consonance as maker.
- Missing: MAX17048, CN3791 inductor and MOSFET, R_CS, 200k, 4.7k and 330 Ω resistors, 915 MHz antenna, NTC (`THERMAL.md:203`), PTC fuse (`pinout.md:170`).

**E15 (M). Opto current.** `POWER_BUDGET.md:68` and `pinout.md:53,134` drive VO617A directly from GPIO1 at 50 mA, while `pinout.md:160` limits pins to 12 mA and `ARCHITECTURE.md:80` uses 330 Ω for 6.6 mA ((3.3−1.1)/330 = 6.67 mA, correct).

**E16 (M). Tolerance classes.** Classes are defined as A ±0.05, B ±0.1, C ±0.2 (`TOLERANCES.md:10-12`). Rows disagree: L49 ±0.05 labelled C, L72 ±0.1 labelled C, L90 ±5 C, L91 ±1 B, L98 ±0.5 C, L177 ±0.5 C, L178 ±0.2 B. Flatness and parallelism with "±" (`IP67_SPEC.md:68-69`) is not valid GD&T usage.

**E17 (B). IMU wake configuration** (`03_firmware:160-174`, `REAL_TIME_RISKS.md:104-118`, `MVP_DEFINITION.md:27-31`). Checked against [ST's register header](https://raw.githubusercontent.com/STMicroelectronics/lsm6dsox-pid/master/lsm6dsox_reg.h):

| Line of code | Comment says | Header says |
| :--- | :--- | :--- |
| `INT1_CTRL, 0x20` | Routes wake-up to INT1 | Bit 5 is `int1_fifo_full`. Wake-up is `MD1_CFG` bit 5, which is never written. **As written the wake interrupt is not routed to INT1** |
| `WAKE_UP_DUR, 0x10` | 16 consecutive samples | Bit 4 is `wake_ths_w` (threshold weight). `wake_dur` is bits 5-6 |
| `CTRL1_XL, 0x10` | 26 Hz | `odr_xl` = 1 is 12.5 Hz; 26 Hz is 2 |
| `WAKE_UP_THS, 0x20` | 32 LSB = 0.5 g | With `wake_ths_w`=1 and ±2 g: 32 × 2/256 = 0.25 g (with weight 0: 1 g) |

**E18 (M). Standards misuse.** `IP67_SPEC.md:12` cites ASTM F2412 for dust and L97 calls it "para pisos" (it is a footwear standard from memory; either way, not ingress). L100 cites R&TTE while `ANTENNA_PLACEMENT.md:118` and `05_comercial:146` cite RED. `IP67_SPEC.md:24,75` "silicona EPDM" names two different elastomers.

**E19 (M). M2 torque.** 0.6-0.8 N·m (`IP67_SPEC.md:90`, `TOLERANCES.md:73`, `04_mecanico:159`). Estimate T = 0.2 × (0.75 × 450 MPa × 2.07 mm²) × 2 mm ≈ **0.28 N·m** for A2-70 in metal, and lower in plastic. The spec is about 2.5× that, while the report says it avoids cracks. Engineering estimate, not datasheet.

**E20 (B). Certifications stated as fact with no data.** `05_comercial:144` "cumple", L158 (a Phase 2 stage that does not exist), L182, L188 "superó las pruebas", L213 "garantizan"; `DATASHEET.md:98-99`; `ANTENNA_PLACEMENT.md:113,120` vs L119 "Pendiente"; `05_comercial:79` "Garantizado"; `02_hardware:61` "se garantiza"; `08_revision:14`.

**E21 (M). Regulatory.** "+22 dBm (máximo legal)" (`BOOT_SEQUENCE.md:46`, `DATASHEET.md:16`, `USER_MANUAL.md:70`) is the chip's limit, not a legal one. From memory, FCC Part 15.247 limits channel dwell to 0.4 s for narrowband hopping in 902-928 MHz, which conflicts with SF12/125 kHz at 1.32 s and `05_comercial:144`. Not verified, and Colombian (ANE) rules were not checked.

**E22 (M). RF details.** `ANTENNA_PLACEMENT.md:79-80` microstrip 0.2 mm on 0.1 mm, εr 3.8: IPC formula gives about 43 Ω, not 50 Ω, and a 0.1 mm dielectric does not fit a 2-layer 1.6 mm board. L24 "Duroid 5880LCP" εr 2.95 is not a part I could identify. L26 "Polimida", and `04_mecanico:125` "poliamida (Kapton)": Kapton is a polyimide.

**E23 (M). User manual.** L157 "No intente abrir" vs L163 "Remueva la batería". L175 "Oriente hacia el sur" for a device on a moving animal, near the equator. Contact data (L228-230, phone, domain) look real but `README.md:95` calls the datasheet fictional. Same concern for prices (`DATASHEET.md:140-141`).

**E24 (M). "Elecia White" attributions.** More than 80 lines mention her across code and docs, with quoted principles such as "Check everything twice", "Limit dangerous actions", "Log everything". None of 28 sampled phrases appear in the 76-page excerpt in the repo, which stops early in Chapter 3. **This is not proof they are absent from the whole book,** but an evaluator cannot trace them. `.clinerules` chapter references (6, 11, 12, 13) point beyond the excerpt.

**E25 (M). Thermal design logic.** `THERMAL.md:160-162` passive ventilation and "áreas ventiladas" in an IP67 sealed case. L95 FR4 "buena conductividad térmica". L136 temperature sensor "en próxima revisión" vs L203 "Sensor NTC en PCB".

**E26 (C).** "Estados Finos" (`FSM_DESIGN.md:1`, `.clinerules:31`) vs "Finitos" in the report. `RESUMEN_IMPLEMENTACION.md:9`. `TOLERANCES` repeats closing lines. `README.md:75` stray "- ". `PROJECT_CONTEXT.md:73-75` double rule. Report footer "Septiembre, 2026" with `\today` on the title page. `04_mecanico:140` "Encapuslado", L135 "Nomínal".

---

## 4. Stale references

| Reference | Where | Reality | Sev |
| :--- | :--- | :--- | :--- |
| `docs/3_Firmware/`, `docs/5_Comercial/` | `README.md:83-84`; `RESUMEN_IMPLEMENTACION.md:8-15,45,50` | Now `docs/firmware_docs/`, `docs/comercial_docs/` | B |
| `firmware/include/drivers/` "(geofence.h)" | `README.md:86`; `RESUMEN_IMPLEMENTACION.md:55` | Empty. Header is in `firmware/include/` | B |
| `DR-VF-01_Reporte_Final.pdf` | `reports/build.sh:46,49` | Deleted. Replacement is `docs/DR-VF-01_Memoria_Tecnica.pdf`, which `build.sh` never updates | B |
| `.github/workflows/compile_latex.yml` | No references | Clean | - |
| `make` then `./comprehensive_test` | `README.md:104,107` | Make does not build it, and fails anyway | B |
| "RF link budget ... hardware schematics" | `README.md:16` | Neither is in the report. `01_esquematicos_y_bom.tex` holds no schematic | B |
| "EDA Design Files (Altium / KiCad)", "gerber" | `README.md:88-89` | Directories empty | M |
| "License-MIT" | `README.md:8,110` | No LICENSE file | M |
| `hal_gpio`, `hal_gnss` | `README.md:69` | Do not exist | M |
| `schematics/`, `main.cpp`, `geofence.cpp`, `lora_management.cpp`, `power_management.cpp`, `platformio.ini` | `PROJECT_CONTEXT.md:267,272-276` | Do not exist. Real engine is `firmware/src/geofence/geofence.c` | M |
| `firmware/src/drivers/imu_lsm6dsox.c`, `firmware/src/tasks/lora_task.c`, `firmware/src/main.c` | `MVP_DEFINITION.md:24,79,127`; `REAL_TIME_RISKS.md:103,131` | Do not exist. Presented as implementation | M |
| `firmware/src/geofence/pip_ray_casting.c`, `firmware/include/system_log.h` | `REAL_TIME_RISKS.md:19,241` | Do not exist | M |
| `QUICK_START.md`, `WARRANTY_CARD` | `BOM.md:112-113` | Do not exist | M |
| `USER_MANUAL.md` (bare) | `BOM.md:111` | In `docs/comercial_docs/` | C |
| `pinout.md` (bare) | `BOOT_SEQUENCE.md:17` | In `hardware/` | C |
| `REAL_TIME_RISKS.md` (bare) | `FSM_DESIGN.md:118,264`; `test_geofence.c:155` | In `docs/firmware_docs/` | C |
| `1_SRD/`, `secciones/02_Hardware.tex`, `03_Firmware.tex`, `04_Mecanico.tex`, `05_Comercial.tex` | `RESUMEN_IMPLEMENTACION.md:33-36,44,72-75` | Do not exist | M |
| `.gitignore:21` comment "en la raíz" | `.gitignore` | PDF is in `docs/`; the rule still matches it (verified) | C |

All relative Markdown links in `ARCHITECTURE.md`, `PROJECT_CONTEXT.md` and `REQUIREMENTS.md` resolve. The `README.md` PDF links resolve.

---

## 5. Code against docs

### 5.1 Claims versus code

| Claim | Where | Code | Sev |
| :--- | :--- | :--- | :--- |
| Fixed-point kernel | `README.md:64,71` | `geofence.h:29-30` `double`; `geofence.c:138-140,161-164` | B |
| No `malloc` | `README.md:71` | `geofence.c:60` | B |
| < 1.2 ms | `README.md:71` | Never measured on a target | B |
| Haversine zones | `README.md:42-46` | Distance exists (`geofence.c:151`) but is not used by PiP. No zone API | B |
| Warning near the border (< 5 m) | `03_firmware:111`; `02_casos:11` | `geofence_get_closest_boundary_point` is unimplemented | B |
| Ray casting | `ARCHITECTURE.md:172`, `geofence.c:122` | Matches | - |
| 4 declared functions | `geofence.h:125,136,155,164` | **Not in `geofence.c`** | B |
| "con caching ... spatial indexing" | `geofence.h:128-136` | Not anywhere, even in the backup wrapper | M |
| `is_point_in_geofence`, `lat`/`lon` | `MVP_DEFINITION.md:46-62` | Real names: `geofence_is_point_in_polygon_ray_casting`, `latitude`/`longitude` | M |
| `pip_optimized(point_t, polygon_t)` "se implementó" | `03_firmware:126-148`; `REAL_TIME_RISKS.md:20` | Not in the repo. Real code uses `double`, not `float vt` | B |
| Config limits and tolerance | `geofence.h:46-48` | `geofence.c:33-40` ignores them all | M |
| "Comprehensive error checking" | `FIRMWARE_IMPLEMENTATION_SUMMARY.md:57` | B2, B4, B5, B6 | M |

From memory and not verified: ESP32-S3 and nRF9151 FPUs are single precision, so `double` would run in software on both, which matters for the 50 ms budget.

### 5.2 Bugs in `geofence.c`, shown with concrete input

Compiled and run in scratch, probes in `.../tmp.vvan7vsa11/probes/`. Test polygon: lat 40.7128-40.7188, lon -74.0060 to -74.0000.

| ID | Input | Result | Why it matters | Sev |
| :--- | :--- | :--- | :--- | :--- |
| B1 | Boundary points | SW corner, S edge, W edge: **inside**. SE, NE, NW corners, N edge, E edge: **outside** | Half-open ray-casting rule. The tests assert "on edge = inside" but only try S and W edges, so they pass by chance. An N or E edge test would fail | M |
| B2 | `geofence_create_polygon(&g, NULL, 3)` | **Segfault** (exit 139) | `memcpy` from NULL at L65, no check. Backup had one | B |
| B3 | `create(&e, NULL, 0)` returns 0, then `add_vertex` | **-3 TOO_MANY_VERTICES, every time**. Also -3 after `create` with 4 vertices | `add_vertex` only works after `clear_polygon` on a polygon that already had capacity. The header says `vertices` "can be NULL" (L72) | B |
| B4 | `create(NULL, f, 4)` | -5 NOT_INITIALIZED, while initialized | Wrong error code (L53) | M |
| B5 | `geofence_init` with all-zero config; double init | Both return 0 | L33-40 only checks NULL | M |
| B6 | `add_vertex` after clear with `{999,999}` | Returns 0, accepted | `is_valid_coordinate` is unused (the one GCC warning). `geofence_calculate_distance` with lat 999 returns 9006789.1 m | M |
| B7 | Point `lat 999`, `NaN`, or after `geofence_deinit` on a truly inside point | All **outside** | `bool` cannot separate "invalid" from "outside". An FSM would fire the stimulus on a bad fix or uninitialized engine. Animal-welfare risk once integrated | B |
| B8 | Square across ±180° | (0, 179.5) **outside**, (0, 0) **inside** (both wrong) | Not relevant in Huila (lon about -75°), but undocumented | C |
| B9 | Exact antipodes on a grid | 61374 of about 1.5 M give **NaN** | `sqrt(1-a)` with `a` slightly > 1 (L164). Not relevant at field scale | C |
| B10 | Second `create` on the same struct | Leaks the first array | `memset` at L57 overwrites the pointer (from reading the code, not run under a leak checker) | M |

Also: no test covers `add_vertex`, `clear_polygon` or the distance value. Distance itself is right: 0.0058° of latitude gives 644.93 m, matching the expected figure.

### 5.3 Makefile

- `make` fails at link (`geofence_validate_polygon`), confirmed with exit 2.
- Builds only `test_geofence.c`. `simple_test`, `comprehensive_test`, `fixed_test_geofence` are unreachable from it, contrary to `README.md:104-107`.
- `debug`, `help`, `depend` are not in `.PHONY` (L33).
- `clean` (L50) does not remove `geofence_test_debug`.
- L8 says "Cross-platform" but L72 hard-codes `gcc`.
- `geofence_test` output is not git-ignored.
- `-std=c99 -O2 -I../../include`, `-lm` and header dependency are all fine.

### 5.4 Factual check of `CLAUDE.md`, `AGENTS.md`, `.clinerules`

| Claim | Result |
| :--- | :--- |
| `CLAUDE.md:26` test command | Works |
| `CLAUDE.md:29` working test files | True, but `comprehensive_test.c` asserts nothing and `simple_test.c` only inits |
| `CLAUDE.md:31` Makefile and link failure; README steps fail | True |
| `CLAUDE.md:31` "Port that function ... before deleting the backup" | **Incomplete**: 13 items are backup-only, see 1.3 |
| `CLAUDE.md:41` `./build.sh` exit 1, first error at `Configuracion.tex:8`, `latexmk -f` no PDF | True (exit 12 and "no output PDF"). **Incomplete**: fixing L8 is not enough, see section 6 |
| `CLAUDE.md:17,19` hook, patterns | True. `.git/hooks/pre-commit` runs `gga run || exit 1`; `.gga:33,44` match. Behaviour of `gga` itself not exercised |
| `CLAUDE.md:19` Conventional Commits | Partly: 5 of 10 commits use the format. Spanish: true |
| `CLAUDE.md:45-48` architecture | True |
| `CLAUDE.md:55`, `AGENTS.md:31`, `.clinerules:37` "50 ms of waking (RF-02)" | **False as stated**: RF-02 is processing time, see C13 |
| `CLAUDE.md:62-65` disagreements | True, but understates it. Pins, power, FSM and others also disagree |
| `CLAUDE.md:63` local toolchain | True: `west`, `~/zephyr-sdk-1.0.1`, `~/zephyrproject` exist; `nrfutil`, `nrfjprog` not found; `boards/nordic` has `thingy52`, `thingy53`, no `thingy91x` |
| `.clinerules:10` compiles with `latexmk` | False today |

---

## 6. LaTeX

### 6.1 Does `reports/build.sh` compile?

No: exit 1, 182 errors, no PDF. `latexmk -f` exits 12 with "no output PDF file produced".

### 6.2 Root cause of the first error

1. `Configuracion.tex:7` loads babel with `es-noquoting`, so `<` and `>` never become Spanish shorthands, and `\es@savecat<` and `\es@savecat>` are never defined.
2. `Configuracion.tex:8` calls `\spanishdeactivate{~<>}`. For each character, `spanish.ldf:522-536` runs `\catcode`#1=\csname es@savecat\string#1\endcsname\relax`.
3. For `<` and `>` the undefined name gives TeX "Missing number, treated as zero" (the log shows exactly these two, at l.8), and it sets **`\catcode`<=0` and `\catcode`>=0`**.
4. Catcode 0 is the escape character, so from then on every `>` in any package starts a control sequence. The next hit is `\ifnum\pdfoutput>0` while `geometry` loads, and the 182 errors cascade from there.

### 6.3 Smallest fix, confirmed in scratch

| Step | Change to `reports/Configuracion.tex` | Result |
| :--- | :--- | :--- |
| 1 | Delete L8. TikZ is already protected by the `babel` tikzlibrary at L32 | 1 error left: `Missing \begin{document}` at `\setlength{\cftfigaftersnum}{:}` |
| 2 | L103-104: `\setlength` to `\renewcommand` (`\cftfigaftersnum`, `\cfttabaftersnum` are macros, not lengths) | **`build.sh` exit 0**, 32 pages, but **page 1 is junk "Figura Cuadro"** |
| 3 | L108-109: same change for `\cftfigpresnum`, `\cfttabpresnum` | Exit 0, 0 errors, 31 pages, no junk page. **This is the minimum correct fix: 5 lines** |

Steps 1 and 2 alone would give a passing build with a wrong first page, so a passing exit code is not enough to confirm this.

Optional, also confirmed (built with exit 0, 0 errors, 7 overfull boxes, TOC checked by eye):

- L99 `\cftchapnumwidth` from 2em to 6em, and add `\cftfignumwidth` and `\cfttabnumwidth` at 5.5em. Without this the TOC shows "Capítulo" overlapping "Dossier1".
- `\usepackage[T1]{fontenc}` plus `\usepackage{lmodern}` (see 6.5).

Also needed in `build.sh:46,49`: copy to `docs/DR-VF-01_Memoria_Tecnica.pdf`. As written a rebuild re-creates the deleted root PDF. Not tested.

### 6.4 Structure

- **All 13 `\input` targets exist**: `Configuracion.tex` plus the 12 section files in `main.tex:34-57`.
- **`01_Defensa_Arquitectura.tex`, `02_Manual_Operativo.tex`**: standalone (own `\documentclass`), both build with 0 errors to 1 page. **Used by nothing**: no grep hits outside themselves.
- **`references.bib`**: unused. Zero `\cite` or `\nocite`, and `main.tex:60-61` are commented out.
- **Figures**: zero `\includegraphics`, all diagrams are TikZ, so none are missing. `reports/figures/` is unneeded.
- **Labels**: 9 defined, only `fig:bloques_hw` is referenced. Harmless.
- **Chapter 1** uses `\section*` only, so it has no TOC entries, unlike chapters 2-5 (C).
- **Warnings left**: 4 hyperref "Token not allowed" (math in headings) (C).

### 6.5 Visible wrong characters (B)

With no T1 encoding, text-mode `<` and `>` print as `¡` and `¿`. The Memoria PDF reads "**Latencia ¡50 ms**", "**(¿80**", "**¡\$150.000 COP**" and "actualización ¿1 Hz". Sources: `03_req:10,21`, `04_srd:16,18,24`, `02_casos:20`. Requirements values read wrong. The T1 fix above was confirmed in scratch (`> 1 Hz`, `< 50 ms`). T1 alone gave bitmap fonts here, and adding `lmodern` gave Type 1 fonts.

### 6.6 The Memoria PDF does not come from HEAD

- `docs/DR-VF-01_Memoria_Tecnica.pdf`: created 2026-10-26 15:57, pdfTeX 1.40.29. The local TeX is 1.40.28, the version that produced the older PDFs.
- Its TOC has "1. Dossier" with no "Capítulo" prefix, and it has an **empty "Bibliografía" page 26**, while HEAD's `main.tex` has the bibliography commented out.
- Body text matches HEAD apart from line wrapping and page breaks (word-level diff), so content is current, but its preamble was different. How it was built is unknown. The deleted workflow's `-f` option does not rescue the build on the local toolchain.
- It has zero mentions of Thingy, nRF or Zephyr (only 3 `K_THREAD_DEFINE`), though the README presents it as the specification for a Thingy:91 X product.

---

## 7. Target evidence

Dates are the last commit touching the file. **`99efbaa` (2026-10-26) was a bulk commit**, and many of those files were written on 2026-10-25, so dates separate only a few groups. The mtime column helps with that. No target is recommended here.

| Document | Assumes | Last commit | On-disk mtime |
| :--- | :--- | :--- | :--- |
| `README.md` | **Thingy:91 X, Zephyr/NCS, LTE-M/NB-IoT.** ESP32-S3 as secondary | **uncommitted** (HEAD empty) | 10-26 |
| `RESUMEN_IMPLEMENTACION.md` | **Mixed**: Thingy:91 X and Zephyr (L83-84), pinout for ESP32-S3 (L22), HAL for both (L111) | 10-26 | 10-25 |
| `docs/firmware_docs/REAL_TIME_RISKS.md` | **Mixed**: ESP32-S3 (L27, L124), Zephyr API (L132), tests on Thingy:91 X (L168, L255) | 10-26 (moved, uncommitted) | 10-25 |
| `docs/firmware_docs/MVP_DEFINITION.md` | **Mixed**: ESP32-S3 (L97, L156), Zephyr (L122, L128) | 10-26 (moved, uncommitted) | 10-25 |
| `docs/firmware_docs/BOOT_SEQUENCE.md` | ESP32-S3 (L11), RTOS unnamed | 10-26 (moved, uncommitted) | 10-25 |
| `docs/firmware_docs/FSM_DESIGN.md` | ESP32 (L40), RTOS unnamed | 10-26 (moved, uncommitted) | 10-25 |
| `docs/PROJECT_CONTEXT.md` | ESP32-S3 DevKitC-1, FreeRTOS, SX1262, A7670SA | 10-18 | 10-18 |
| `docs/ARCHITECTURE.md` | Same | 10-18 | 10-18 |
| `docs/REQUIREMENTS.md` | ESP32-S3, FreeRTOS | 10-18 | 10-18 |
| `docs/comercial_docs/DATASHEET.md` | ESP32-S3-WROOM-1U, LoRa | 10-26 (moved, uncommitted) | 10-25 |
| `docs/comercial_docs/USER_MANUAL.md` | Implied ESP32 (Wi-Fi SoftAP, LoRa); no MCU named | 10-26 (moved, uncommitted) | 10-25 |
| `hardware/BOM.md`, `POWER_BUDGET.md`, `pinout.md` | ESP32-S3-WROOM-1U | 10-26 | 10-25 |
| `hardware/THERMAL.md` | No MCU named; TPS7A0533, CN3791, SX1262 | 10-26 | 10-25 |
| `mechanical/ANTENNA_PLACEMENT.md` | LoRa and 2.4 GHz; no MCU | 10-26 | 10-25 |
| `mechanical/IP67_SPEC.md`, `TOLERANCES.md` | Neutral | 10-26 | 10-25 |
| `reports/01_Defensa_Arquitectura.tex` | ESP32-S3-WROOM-1U. **Explicitly rejects replacing it with nRF52**. BG95-M3 for Phase 2 | 10-20 | 10-20 |
| `reports/02_Manual_Operativo.tex` | ESP32, LoRa | 10-20 | 10-20 |
| `reports/secciones/01_requerimientos/*` | ESP32-S3-N16R8, SX1262, BG95-M3. L5 of `07_conclusiones` FreeRTOS | 10-26 | 10-25 |
| `reports/secciones/02_hardware/*` | ESP32-S3-WROOM-1U | 10-26 | 10-25 |
| `reports/secciones/03_firmware/*` | **Mixed**: ESP32-S3 FPU (L126), Zephyr `K_THREAD_DEFINE` (L192) | 10-26 | 10-25 |
| `reports/secciones/04_mecanico/*` | LoRa, Wi-Fi/BLE; no MCU | 10-26 | 10-25 |
| `reports/secciones/05_comercial/*` | ESP32-S3-WROOM-1U, LoRa | 10-26 | 10-25 |
| `docs/DR-VF-01_Memoria_Tecnica.pdf` | ESP32-S3, LoRa, FreeRTOS and `K_THREAD_DEFINE`; no Thingy | **uncommitted** | 10-26 |
| `docs/references/referencia.pdf` | ESP32-S3-N16R8 in text; ESP32-WROOM, SIM800L, SX1278 in tables | 10-26 (moved) | 10-18 |
| `Informe de Selección de IA para Cline.pdf` | ESP32-S3, C/Zephyr | 10-26 | 10-25 |
| `firmware/include/geofence.h`, `geofence.c` | Neutral, host-only | 10-26 | 10-25 |
| `.clinerules` | Neutral: "ESP32 o Zephyr" (L28), "Zephyr / FreeRTOS" (L17) | 10-26 | 10-25 |
| `CLAUDE.md`, `AGENTS.md` | Neutral, flags the disagreement | **uncommitted** | 10-30 |

**Facts for the decision (no recommendation):**

- Thingy:91 X appears as the target only in the README (uncommitted) and in partial lines of two older files.
- Everything concrete (BOM, pinout, power, thermal, antenna, report) is ESP32-S3 and LoRa. Moving to Thingy:91 X would void most of it: the board reportedly has its own 1350 mAh battery, PMICs, sensors and enclosure, and no listing mentions LoRa. One distributor listing gives 0-35 °C operating range. These come from search snippets ([cnx-software](https://cnx-software.com/2024/12/15/nordic-thingy91-x-multi-sensor-cellular-iot-platform-combines-nrf9151-lte-iot-sip-nrf5340-ble-soc-and-nrf7002-wifi-6-ic), [Mouser](https://www.mouser.lt/nordic-thingy91-x-prototyping-platform)) and were not checked against Nordic's hardware guide.
- Local tools: `west`, Zephyr SDK and upstream Zephyr are installed. `~/ncs`, `~/esp/esp-idf` and `~/.espressif` do not exist, `nrfutil` and `nrfjprog` are not on the PATH, and `boards/nordic` has no `thingy91x`. Upstream Zephyr's `boards/espressif` has `esp32s3_devkitc`, `esp32s3_box3` and `esp32s3_eye`. `esptool` exists only inside the Zephyr venv.

---

## What I did not read or could not verify

- **Read in full:** all Markdown, TeX, C, header, Makefile, script and config files. **PDFs:** `Informe` (all 2 pages), `referencia.pdf` (text, all 9 pages, diagrams not viewed), `Memoria` (pages 1-2 as images, all text extracted), the book (classified from text extraction, 76 pages, not read), `reports/main.pdf` and the deleted `DR-VF-01_Reporte_Final.pdf` (text only).
- **External facts:** no component datasheets are in the repo. Verified from primary sources: TPS7A05 RθJA (TI), ESP32-S3 GPIO (Espressif), LSM6DSOX registers (ST driver header). From secondary or snippet only: CN3791 1.205 V (the PDF fetch failed), TPS7A05 dropout, Thingy:91 X specs. From my own memory, flagged where used: FCC dwell, FPU precision, ASTM F2412 subject, ECS crystal code. Datasheet current figures for GNSS, CN3791 standby and MAX-M10S were not checked.
- **Not run:** `gga`, `gentle-ai`, `claude`, VS Code, any target hardware, leak checker. `build.sh`'s suggested PDF path change was not tested.
- **Remote:** repository visibility on GitHub was not checked (no remote calls).
- **Not compared by instruction:** `.clinerules`, `CLAUDE.md` and `AGENTS.md` with each other.
- **Judgement calls:** "scrambled" is from reading structure, not a tool. Severity is my assessment. Engineering estimates (E19, E22) are approximations.

## Scratch contents (not deleted)

`/tmp/claude-1000/-home-xavi-Proyecto-DR-VF-01/8116d6ba-1dbb-41aa-9e02-23a24130aed2/scratchpad/tmp.vvan7vsa11`

- `firmware/` builds and test binaries; `reports/` unmodified copy (fails to build); `r2/` the 5-line minimum fix (builds); `r3/` the fix plus fonts and TOC widths (builds).
- `probes/` the bug probes; `txt/` extracted PDF text; `scripts/wdiff.py`; `build*.log`, `latexmk_f.log`.
- `DR-VF-01_Reporte_Final.pdf`, written there by the scratch `build.sh` runs.
