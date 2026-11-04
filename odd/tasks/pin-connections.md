# Pin connections for every accepted part

Objective: give the Altium schematic a verified, pin-by-pin connection guide for every accepted module and component. Branch `chore/repo-audit`. Delivery: work-unit commits on this branch, push as the user authorized for it. Test runner: none (documentation); checks are scripted cross-checks against the datasheets.

## Specs

- S1: Connect every pin of every accepted module and component, based on the datasheets and the research already done, choosing the connection an expert would choose. User words (2026-11-04): "has lo de los piens, apra todo los modulos y componentes q hemos aceptado" and "elije bien como conectar todos los piens de todo lo que llevamos".
- S2: Keep track of what remains pending and update the `.md` files where it is recorded. User words: "recuerd abien qq queda pendeinte yve actualizabdo los .md donde anotes todo".
- S3: Save to memory. User words: "guarda en memoria".
- S4: Purchases and the other pending items are resumed when the user returns; this work does not decide them.

## Tasks

| ID | Specs | Route | Status | Evidence |
|----|-------|-------|--------|----------|
| T1 | S1 | explorer (read-only) + inline write | done | power chain: TP4056, CN3791 with its power stage, TLV75733P, MAX17048, battery and panel connectors |
| T2 | S1 | explorer + inline write | done | GNSS MAX-M10S with passive patch on u.FL |
| T3 | S1 | explorer + inline write | done | IMU LSM6DS3TR-C |
| T4 | S1 | explorer + inline write | done | LoRa module DX-LR30-900M22S |
| T5 | S1 | explorer + inline write | done | VO617A, buzzer, LED RGB, button, USB-C with USBLC6, JST, crystal, u.FL |
| T6 | S1 | inline | done | `hardware/pin_connections.csv`, 152 rows; every GPIO cross-checked between the module and the peripherals |
| T7 | S2, S3 | inline | done | pending list in `pinout.md`, memory update |
| T8 | S1 | inline | done | two commits pushed to `chore/repo-audit` (hashes in the Engram mirror) |

Route evidence: the mapping trigger fired (more than ten datasheets to read), so three read-only explorers extract pin tables and application circuits; the parent writes inline because all output goes into one shared file (`hardware/pinout.md`).

## Log

- L1 (2026-11-04, user): "mientras estoy fuera, has lo de los piens, apra todo los modulos y componentes q hemos aceptado, cuando vuelva seguimos con las compras y lo demas pendiente, eso si recuerd abien qq queda pendeinte yve actualizabdo los .md donde anotes todo. guarda en memoria y ahora hablamos. RECUERDA eres un eperto y con agua de toda la investigacion q tenemos y datasheets, elije bien como conectar todos los piens de todo lo que llevamos."
- L2: ESP32-S3-WROOM-1U section already done in commit 57f61ca (pinout.md v2.3).
- L3 (2026-11-04): explorers hit the 5-hour usage limit and were resumed with SendMessage after the reset.
- L4 (2026-11-04): decisions taken from the datasheets: CN3791 inductor 47 µH (L > 5(VCC - VBAT) µH needs > 25 µH at 8 V open circuit); D1 fitted (cuts the night battery path through the MPPT divider and the panel); RGB LED active-low with 750/470/330 Ω (VF at 2 mA from the VF-IF curve); buzzer driven directly with 100 Ω; opto output drives a teacher red 0805 LED with 1 kΩ; LoRa RX/TX through 0 Ω links plus a DNP 0 Ω to DIO2; MAX-M10S V_BCKP open.
- L5 (2026-11-04): findings left for the user: buzzer cannot meet 80 dBA at 1 m; C1 electrolytic height; 3.3 V bulk under 200 µF; battery with built-in protection recommended. Unified pending list in `hardware/pinout.md`.
