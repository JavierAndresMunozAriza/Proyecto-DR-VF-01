# geofence-hardening

Objective: harden the pure-C geofence engine (`firmware/src/geofence/geofence.c`) with no board attached.
Branch: `feat/geofence-hardening`. Delivery strategy: ask-on-risk. Test runner: `make test` in `firmware/src/geofence`.

## Specs

- S1: "Haz que el codigo c de geofence.c funcione totalmente, un codigo muy escalable y portable"
- S2: "probaras miles o cientos de miles de casos y que no falle"
- S3: "recuerda ir guardando en memoria y haciendo commits para no perder nada por si se apaga el pc"
- S4: "que te revisen todo lo que hiciste" with independent sub-agents, "uno por uno", looking for "10 lineas en lo q se peude hacer en 1 o 2", lines without sense, non-scalable or non-portable code, "muy profesional de nivel senior de industria"
- S5: "slo tocaras geofence.c ... o tocaras algo mas que tenga q ver con zephyr" -> only the engine and its docs; no Zephyr code tonight.
- S6 (assumption, needed by the next step): `geofence_distance_to_boundary()` returns meters to the nearest polygon side, `-1.0` on invalid input.
- S7 (assumption, needed by the GNSS step): `geofence_check()` reports inside/outside and distance with an error code, so "outside" is never confused with "invalid input".

## Tasks

- [x] U0 Branch, this document, Engram mirror (route: inline) - commit 3476f9f (CLAUDE.md) + this document
- [x] U1 `geofence_distance_to_boundary()` (S6, inline) - commit 50b9d1b
- [x] U2 `geofence_check()` (S7, inline)
- [x] U3 Stress tests >= 200000 cases + `make sanitize` (S2, inline)
- [x] U4 `make cross-check` for ARM and ESP32-S3 (S1, inline)
- [ ] U4b Independent reviewers, one by one (S4, delegated: independent verifier requested by the user)
- [ ] U5 Docs and morning report (S3, inline)

## Log

- L1 (2026-11-17): "como me hire a la cama y no podre conectar la placa, haras esto y me diras si es recomendable con criterio y como experto. Haz que el codigo c de geofence.c funcione totalmente, un codigo muy escalable y portable, probaras miles o cientos de miles de casos y que no falle, luego manana que el codigo c este correcto y muy bien hecho, sigo con lo que necesites, conectar la placa y demas cosas para luego el paso 2 con el GNSS. ademas recuerda ir guardando en memoria y haciendo commits para no perder nada por si se apaga el pc mientrs estas ejecuntando algo."
- L2 (2026-11-17): "y te recuerdo que tus agentes o con lo que puedas hacer de manera critica y experta, te revisen el codigo luego de que funcione ... haras q te revisen todo lo que hiciste y vean cosas tipo: Que no hasgas 10 lineas en lo q se peude hacer en 1 o 2, linea que no tengan sentido, que no sea escalable ni tan portable, que sea muy profesional de nivel senior de industria ... manda uno por uno para mermar el consumo de los tokens"
- L3: Answer to the user's question: the same `geofence.c` serves step 1 (simulated point) and step 2 (GNSS); only the Zephyr side changes, where the point comes from.
- L4 (2026-11-17): U0 done. Next: U1.
- L5 (2026-11-18): U1 and U2 done, 13 tests / 2840 checks green. Next: U3 stress tests.
- L6 (2026-11-18): U3 done. `make stress`: ~3.6M cases per seed, 6 seeds, 0 failures; `make sanitize` (ASan+UBSan+leaks) clean; 3 injected mutations all caught. Finding: ray casting and winding number can differ for points only within rounding of an edge (expected, header already warns); the test excludes those. Next: U4 cross-check.
- L7 (2026-11-18): U4 done. `make cross-check`: geofence.c compiles with -Werror and strict warnings on host, Cortex-M33 (3248 B text, 24 B bss) and ESP32-S3 (3349 B text, 17 B bss). Next: U4b reviewers one by one.
- L8 (2026-11-18): U4b reviewer 1 (readability, general-purpose agent; the review-* agent types are native-RAR actors and returned "incomplete"): no engine defects; fixes pending in one batch (stress test comment/inner point/garbage picker coupling, header "Invalid input" + boundary stance, Makefile header, EXPECT context duplication, float == in check tests).
- L9 (2026-11-18): U4b reviewer 2 (reliability): no blockers; 40 mutations, 7 survivors (check guards `< 3`, `!vertices`, point validation in check, t cap 0.999, is_valid_coordinate exact limits, len_sq threshold, inside via winding); should-fix: projection error grows near poles (0.15% at lat 85, 0.76% at 89, 8.2% at 89.9) so state a latitude limit and test it; no test for zero-length edges; antimeridian returns SUCCESS with nonsense distance (39,419,776 m). Reviewer 3 (risk/portability) running.
