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
- [x] U4b Independent reviewers, one by one (S4, delegated: independent verifier requested by the user)
- [x] U5 Docs and morning report (S3, inline)

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
- L10 (2026-11-18): U4b correction batch applied (commits b2f4885, 6906495). Fixed: use-after-free in geofence_add_vertex when the argument aliases the vertex array (ASan-confirmed, alias test added); hypot/fmin/fmax removed from the per-edge distance loop (squared distances, one sqrt); geofence_check guards (NULL vertex array is INVALID_PARAM, <3 vertices is EMPTY_POLYGON); header docs (latitude limit 85 deg, antimeridian, thread safety, slow get_closest_boundary_point, even-odd semantics); tests for duplicate vertices, exact coordinate limits, pentagram, collinear end; stress refactor (context in failures, duplicate vertices, lat up to 85, exact-limit garbage, 65535-vertex polygon). Recheck of the blockers: 7 earlier-surviving mutants re-run, 5 killed, 2 equivalent (t cap 0.999 is masked by the next edge; len_sq > 1e-6 only affects edges < 1 mm).
- L11 (2026-11-18) MORNING REPORT. State: 14 unit tests / 3067 checks, stress ~3.8M cases per seed (6 seeds, 0 failures), ASan+UBSan+leaks clean, cross-check OK for Cortex-M33 (3336 B text) and ESP32-S3 (3429 B text). Engine is ready to be called from Zephyr; geofence_check() is the entry point.
  Pending decisions for the user (not changed without them): (1) antimeridian: reject or wrap longitudes (today documented as unsupported; distance can be nonsense across +/-180); (2) error-code consistency: add_vertex returns INVALID_PARAM when not initialized while create/check return NOT_INITIALIZED, and create_polygon does not validate vertices; (3) static buffers instead of malloc for the final firmware; (4) float or fixed-point kernel only after measuring on the board; (5) shared test_common.h for the two test files (taste).
  Caveats: timings for the 50 ms budget are estimates, not measured (about 41 ms for 1000 vertices on soft double before the hypot fix; the MVP polygon is at most 32 vertices); cross-check only compiles, so libm (sin, cos, atan2, sqrt) and a malloc arena must be enabled in prj.conf and confirmed at link time in NCS v3.4.1; review-* agent types are native RAR actors and cannot be used as generic reviewers.
  Next (needs the board): Zephyr app skeleton with simulated route, RGB LED (pwm-led0 red, pwm-led1 green, pwm-led2 blue on the Thingy:91 X), build with ~/nordic/ncs-shell.sh, flash over USB-C with usbipd bind/attach + west thingy91x-dfu; then GNSS.
