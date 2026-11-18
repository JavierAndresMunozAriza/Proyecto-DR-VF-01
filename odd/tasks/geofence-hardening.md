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
- [ ] U1 `geofence_distance_to_boundary()` (S6, inline)
- [ ] U2 `geofence_check()` (S7, inline)
- [ ] U3 Stress tests >= 200000 cases + `make sanitize` (S2, inline)
- [ ] U4 `make cross-check` for ARM and ESP32-S3 (S1, inline)
- [ ] U4b Independent reviewers, one by one (S4, delegated: independent verifier requested by the user)
- [ ] U5 Docs and morning report (S3, inline)

## Log

- L1 (2026-11-17): "como me hire a la cama y no podre conectar la placa, haras esto y me diras si es recomendable con criterio y como experto. Haz que el codigo c de geofence.c funcione totalmente, un codigo muy escalable y portable, probaras miles o cientos de miles de casos y que no falle, luego manana que el codigo c este correcto y muy bien hecho, sigo con lo que necesites, conectar la placa y demas cosas para luego el paso 2 con el GNSS. ademas recuerda ir guardando en memoria y haciendo commits para no perder nada por si se apaga el pc mientrs estas ejecuntando algo."
- L2 (2026-11-17): "y te recuerdo que tus agentes o con lo que puedas hacer de manera critica y experta, te revisen el codigo luego de que funcione ... haras q te revisen todo lo que hiciste y vean cosas tipo: Que no hasgas 10 lineas en lo q se peude hacer en 1 o 2, linea que no tengan sentido, que no sea escalable ni tan portable, que sea muy profesional de nivel senior de industria ... manda uno por uno para mermar el consumo de los tokens"
- L3: Answer to the user's question: the same `geofence.c` serves step 1 (simulated point) and step 2 (GNSS); only the Zephyr side changes, where the point comes from.
- L4 (2026-11-17): U0 done. Next: U1.
