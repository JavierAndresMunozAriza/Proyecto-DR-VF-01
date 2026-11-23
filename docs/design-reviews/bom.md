STATUS: complete

# DR-VF-01 BOM audit (independent, read-only)

Date: 2026-11-18. Scope: BOM only (MPN, package, quantity, supplier, purchase completeness, teacher library vs BOM, ratings and derating at +65 C, MPN existence/lifecycle for gaps, cost tables, master parts list). Repo: `/home/xavi/Proyecto DR-VF-01` (branch `feat/geofence-hardening`); nothing in the repo was edited, staged or committed. Paths below are repo-relative unless absolute. Datasheet pages are PDF pages of `hardware/datasheets/` (read with `pdftotext -layout`).

Extra read-only evidence (same method as `docs/design-reviews/rf.md`): the live Altium project in OneDrive (`.../Diseño Electronico/PCB  DR-VF-01/PCB_Project/`: five `.SchDoc` saved 2026-11-10 to 2026-11-14, `PCB.PcbDoc` 2026-11-15 19:11, ActiveBOM `DR-VF-01.BomDoc` 2026-11-16 00:08, `DR-VF-01.PrjPcb`) and the project library `.../Libreria para DR-VF_01/DR-VF-01/shared_lib.SchLib` (84 symbols). All were copied to the scratchpad and parsed there with olefile. Scripts (scratchpad `scripts/`): `enum_parts.py` (binds `hardware/esquematico/sheets.py` to the library and lists all parts with designators), `dump_params.py` (symbol parameters and footprints), `schdoc_parts.py` (components and parameters of the live SchDocs), `pcb_comps.py` (components and footprints of the live PcbDoc).

## 1. Scope and checklist

### 1.1 Master parts inventory (every placed part)

Source: `hardware/esquematico/sheets.py` bound to `shared_lib.SchLib` (112 parts). The live SchDocs and the live PcbDoc hold the same 112 designators with the same footprints except where noted in the findings (C25 symbol, C31/C32 footprint, DNP marking).

| Group | Designators | Qty | Part (sheets.py) | BOM.md line | Source line |
|-------|-------------|-----|------------------|-------------|-------------|
| MCU | U5 | 1 | ESP32-S3-WROOM-1U-N16R8 | 1.1 | O2 |
| GNSS | U6 | 1 | MAX-M10S-00B (buy -00B-01) | 1.2 | O11 |
| LoRa | U7 | 1 (+1 receiver) | DX-LR30-900M22S | 1.3 | A1 (x2) |
| IMU | U8 | 1 | LSM6DS3TR-C | 1.4 | teacher (classroom) D4 |
| Gauge | U9 | 1 | MAX17048G+T10 | 1.4 | A5 |
| Opto | U10 | 1 | symbol VO617A-1X017T; purchase LTV-817S-TA1-B | 1.4 | O13 |
| USB ESD | U1 | 1 | USBLC6-2SC6 | 1.6 | teacher D3 |
| USB charger | U2 | 1 | TP4056 (YONGYUTAI, ESOP-8) | 1.5 | teacher D1 |
| Solar charger | U3 | 1 | CN3791 | 1.5 | O23 |
| LDO | U4 | 1 | TLV75733PDBVR | 1.5 | O1 |
| Crystal | Y1 | 1 | ECS-.327-12.5-34B-TR (generic 3215 bought) | 1.8 | O8 |
| Buzzer | BZ1 | 1 | PS1240P02BT (TZT copy) | 1.4 | O14 |
| RGB LED | D3 | 1 | RS-3535MWAM | 1.7 | teacher D7 |
| Red LED | D4 | 1 | XL-2012SURC | 1.7 | teacher D11 |
| Schottky | D1, D2 | 2 | PSBD1DF40V1H | 1.8 | teacher (O20 closed) |
| P-FET | Q1, Q2 | 2 | AP2301 | 1.5 text, 1.9 | teacher D5 (Q1 only) |
| N-FET | Q3 | 1 | DN2302S | 1.9 | teacher (no D-row) |
| Inductor | L1 | 1 | CY54-47UH | 1.5 text | teacher D6 |
| USB-C | J1 | 1 | TYPE-C-3.1-16PIN | 1.6 | teacher D2 |
| JST PH | J2, J3 | 2 | S2B-PH-K-S(LF)(SN) | 1.6 | teacher D8 |
| u.FL | J4, J5 | 2 | U.FL-R-SMT-1(10) (compatible bought) | 1.3 | O4 |
| Buttons | SW1-SW3 | 3 | TSC015A05026A | 1.6 | teacher D13 |
| Test points | TP1-TP11 | 11 | TestPoint `TP_TH_15mil` (no part) | 1.9 (TP8-TP11 only) | none needed |
| 100 nF | C8, C15, C21, C24, C27-C30 | 8 | X7R wanted (library symbol is Y5V) | 1.8 (10) | O21 |
| 10 uF 25 V 0805 | C2, C4, C6, C7, C11 | 5 | CL21A106KAYNNNE X5R | 1.8 (8, mixed) | teacher D10 |
| 10 uF 25 V X7R | C1 | 1 | to buy | 1.9 | O28 (link pending) |
| 10 uF 10 V 0603 | C16, C22, C23 | 3 | TCC0603X5R106K100CT | 1.8 (in the 8) | teacher D10 |
| 1 uF | C12, C13 | 2 | CL10A105KB8NNNC | none | teacher D10 |
| 22 uF | C14, C25 | 2 | HGC0805R5226M160NSLJ | 1.8 (2 rows) | teacher D10 |
| 33 pF | C17-C20 (+C31, C32, C33 DNP) | 4 + 3 DNP | CC0603JRNPO9BN330 | 1.8 (4) + 1.9 (3) | teacher D10 |
| 470 nF | C5, C9, C10 | 3 | HGC0603R7474K500NTHJ | 1.8 (2) | teacher D10 |
| 100 uF 35 V el. | C3 | 1 | HRK 35V100uF CD110 | 1.5 text only | teacher D12 |
| 100 uF 1206 | C26 | 1 DNP | Cap_100uF_1206_DNP | 1.8 text | O25 closed |
| 0 ohm | R17, R18, R33-R36, R40 (+R19 DNP) | 7 + 1 DNP | FRC0603P000 | 1.9 (5) | teacher D9 |
| 100 ohm | R6, R29 | 2 | kit / teacher | 1.8 (R6 only) | O19 / D9 |
| 120 ohm | R7 | 1 | FRC0603F1200TS | 1.8 | teacher D9 |
| 150 ohm | R16 | 1 | RTT031500FTP | text only (plan B) | teacher (not in D9) |
| 330 ohm | R15, R22, R28 | 3 | FRC0603J331 | none | teacher D9 |
| 1 k | R24, R25, R31 | 3 | FRC0603J102 / kit | none | D9 / O19 |
| 3 k | R26 | 1 | kit | none | O19 |
| 470 ohm | R27 | 1 | kit | none | O19 |
| 3.3 k | R20, R21 | 2 | FRC0603J332 | 1.9 | teacher D9 |
| 10 k | R9, R13, R14, R30 | 4 | FRC0603J103TS | 1.8 (3 + 1) | teacher D9 |
| 100 k | R10, R11, R23, R32, R37-R39 | 7 | kit | 1.8 (2) + 1.9 (5) | O19 |
| 390 k | R8 | 1 | RC0603FR-07390KL | 1.8 | teacher |
| 1.8 k | R5 | 1 | kit | 1.8 | O19 |
| 5.1 k | R1, R2 | 2 | kit | 1.8 | O19 |
| 22 ohm | R3, R4 | 2 | kit | 1.8 ("22 o 33") | O19 |
| 0.47 ohm | R12 | 1 | kit | 1.8 | O22 |
| Off-board | LoRa antenna, GNSS patch (x2), FPC 2.4 GHz, battery, panel, panel lead, copper tape | 1 each | — | 1.2, 1.3, 1.5, 2.3 | A2, O3, O9, O6, O7, O26, O27 |

Totals: 112 placed designators = 96 fitted parts + 5 DNP (C26, C31, C32, C33, R19) + 11 test points (pads, no part); 54 distinct part groups. Mechanical and packaging lines (BOM.md sections 2-3) were checked only for cost and consistency.

### 1.2 Checklist

| # | Check | Items | Result |
|---|-------|-------|--------|
| 1 | Inventory every BOM line (BOM.md 1.1-1.9, 2, 3) | 48 BOM rows + 112 designators | done |
| 2 | MPN / package / qty / supplier across BOM, COMPRAS, pinout, CSV, .tex, sheets.py, ARCHITECTURE, PROJECT_CONTEXT | 54 part groups | see findings |
| 3 | Purchase or stated source for every placed part | 54 groups | see findings |
| 4 | Teacher library entry vs BOM (MPN, footprint, value) | 33 symbols used | see findings |
| 5 | Live Altium SchDoc/PcbDoc/ActiveBOM vs repo BOM | 112 designators | see findings |
| 6 | Ratings and derating at +65 C (V, I, P, T) | 40 parts | see findings |
| 7 | MPN existence and lifecycle (online only for gaps) | 4 gaps | see Unverified |
| 8 | Cost tables (BOM.md 4, .tex, COMPRAS total) | 3 tables | recomputed |
| 9 | Hardware changes from the POWER/MCU/RF audits reflected in BOM/COMPRAS | 12 items | see findings and Verified OK |

## 2. Findings

| ID | Severity | Finding | Evidence | Fix |
|----|----------|---------|----------|-----|
| BOM-01 | MAJOR | **No complete master parts list exists in the repo.** `BOM.md` is called "Lista completa de materiales para compra", but its tables have no row for 19 of the 101 fitted/DNP parts and for 7 of the 11 test points: Q1 (AP2301, M1 of the CN3791), L1 (CY54-47UH), C3 (100 uF 35 V electrolytic), C5 (third 470 nF, CN3791 C1 bank), C12 and C13 (1 uF: LDO output and EN RC; no 1 uF row at all), R15 + R16 (330 + 150 ohm UART0 TX), R17 and R18 (0 ohm RXEN/TXEN links) and R19 (0 ohm DIO2, DNP), R22 (330 ohm opto input), R24 (1 k opto LED), R25 + R26 (1 k // 3 k = 750 ohm), R27 (470 ohm), R28 (330 ohm), R29 (100 ohm buzzer), R31 (1 k button), TP1-TP7. Q1, L1 and C3 appear only in prose (1.5 "POR DECIDIR" paragraph), the LED/opto resistors only in the stale "Faltan además (se añaden al dibujar el esquemático)" note. Quantity rows that do exist disagree with the schematic: 0 ohm "5" vs 7 fitted + 1 DNP; 470 nF "2" vs 3; the 10 uF X5R row "8" lists "VCC del TP4056" (now the X7R C1 of row 1.9, so it is double counted) and omits the GNSS 10 uF C22, so its total matches only by coincidence. The .tex report sends readers to `BOM.md` for every passive. Combined with BOM-02 (the Altium BOM has wrong MPNs), no single list can be used to kit or order the board. | `docs/PROJECT_CONTEXT.md:291`; `hardware/BOM.md:62-65`, `:71`, `:101-114`, `:122`, `:128-135`; `hardware/esquematico/sheets.py:83`, `:87`, `:97-99`, `:106`, `:142-144`, `:185`, `:194-196`, `:250-268`; `reports/secciones/02_hardware/01_esquematicos_y_bom.tex:153`; enumeration in scratchpad `scripts/enum_parts.py` (112 designators) | Regenerate the BOM.md electronic tables from the schematic: one row per value/MPN with designators, quantity (fitted / DNP), package, MPN, source code (D#/O#). Easiest: export the Altium ActiveBOM after BOM-02/BOM-03 are fixed and paste it into `BOM.md`, keeping the prose decisions below the tables. |
| BOM-02 | MAJOR | **The live Altium design carries wrong MPN/supplier parameters on 25 placed parts, so its ActiveBOM (columns: Comment, Manufacturer Part Number 1, Supplier Part Number 1) would order wrong parts.** Values were edited on copies of other library parts without changing their parameters: 16 resistors carry the MPN/LCSC of another value: R1, R2 (5.1 k) and R12 (0.47 ohm current sense) and R26 (3 k) = FRC0603J102 (1 k, C2907113); R3, R4 (22 ohm), R5 (1.8 k), R6 (100 ohm) = RTT031500FTP (150 ohm, C103302); R10, R11, R32, R37, R38, R39 (100 k) = FRC0603J103TS (10 k, C2930027); R23 (100 k) and R27 (470 ohm) = FRC0603J331 (330 ohm, C2907152). All eight 100 nF (C8, C15, C21, C24, C27-C30) carry the teacher's Y5V Walsin 0603F104Z500CT (C83060), the part BOM.md itself rejects (loses 40 % at 45 C and 77 % at 85 C). C1 (TP4056 VCC, must be X7R since 2026-11-09) carries X5R CL21A106KAYNNNE. In addition U5's MPN parameter is "ESP32-S3-WROOM-1U" (no -N16R8: N4/N8/N16R8 variants differ in flash, PSRAM and temperature rating), and U10 is VO617A-1X017T while LTV-817S-TA1-B is bought (BOM-07). The generator reproduces the same defect: `R()` falls back to the 1 k symbol for every value not in `RES_LIB` and `C("100nF")` maps to the Y5V symbol. | Live SchDocs (parsed, scratchpad `scripts/schdoc_parts.py`): R1-R6, R10-R12, R23, R26, R27, R32, R37-R39, C1, C8, C15, C21, C24, C27-C30, U5, U10 parameter records; live `DR-VF-01.BomDoc` VisibleOrderOptions (columns listed) and CatalogItem records; `shared_lib.SchLib` symbols `Cap_0.1uF_0603_50V` (desc "Y5V -20%~+80%"), `Res_1k/10k/150R/330R_0603_1_10W`; `hardware/esquematico/sheets.py:12-31`, `:19`; `hardware/BOM.md:116` and `hardware/COMPRAS.md:165` (Y5V rejected); `hardware/BOM.md:134` | In Altium, for every edited part set Comment, Value, Manufacturer, Manufacturer Part Number and Supplier Part Number to the real part (kit parts: MPN "genérico kit O19", supplier "AliExpress O19"); add library parts `Cap_0.1uF_0603_50V_X7R` (O21 lot) and `Cap_10uF_0805_25V_X7R` (O28) and swap C8/C15/C21/C24/C27-C30 and C1; set U5 MPN to ESP32-S3-WROOM-1U-N16R8. Mirror the same in `sheets.py` (`RES_LIB`/`CAP_LIB` entries per value) so the generated sheets stop propagating the 1 k/Y5V parameters. Re-export the ActiveBOM and diff it against BOM.md. |
| BOM-03 | MAJOR | **"Do not populate" is not encoded anywhere in the live design, and two DNP parts are not even labelled.** The project has no variant (`Variant=[No Variations]`), the ActiveBOM lists every DNP part as `Component Kind=Standard` (fitted), and in the live SchDoc R19 (comment "0R 1/10W") and C33 (comment "33pF 50V") carry no DNP mark at all; `sheets.py` marks both `dnp=True`. R19 is the 0 ohm between SX1262 DIO2 and the TXEN line that GPIO47 drives; `pinout.md` keeps it unfitted precisely to avoid two outputs fighting. A BOM- or pick-and-place-driven assembly fits it. C26 (Cap_100uF_1206_DNP) also has its Value parameter set to "100nF" and no MPN or description. | `DR-VF-01.PrjPcb:180` (`Variant=[No Variations]`, live copy); live `DR-VF-01.BomDoc` CatalogItem lines for `Cap_100uF_1206_DNP` (`"Component Kind=Standard"`); live SchDoc parameters of R19, C33, C26, C31, C32; `shared_lib.SchLib` `Cap_100uF_1206_DNP` (Value=100nF, no MPN); `hardware/esquematico/sheets.py:60-61`, `:196`, `:202`, `:206`; `hardware/pinout.md:213`, `:224` | Create an Altium variant (e.g. "Primera placa") with C26, C31, C32, C33 and R19 "Not Fitted" and make it the BOM/assembly output variant; add "DNP" to the Comment of R19 and C33; fix the library part `Cap_100uF_1206_DNP` (Value 100uF, description, a real MPN or "por elegir"). Add a DNP column (or section) to BOM.md (BOM-01). |
| BOM-04 | MAJOR | **The ESP32-S3-WROOM-1U-N16R8 is rated -40 to 65 C ambient, exactly the design maximum, and the repo's own thermal note says the sealed enclosure in sun exceeds 65 C; no mitigation is chosen.** The octal-PSRAM (R8) variants are the only 65 C parts of the family: with PSRAM ECC enabled the limit becomes 85 C (1/16 of PSRAM lost), and the pin-compatible WROOM-1U-N16R2 (16 MB flash, 2 MB quad PSRAM) and -N16 are rated 85 C. Every other active part in the BOM is rated 85 C or more (Verified OK, section 3), so this module sets the product's thermal limit with 0 C of margin. RNF-09 defines +65 C as the in-enclosure ambient, while THERMAL adds "hasta 15 C adicional en exposición directa". The firmware docs do not mention PSRAM ECC and nothing in the repo needs 8 MB of PSRAM (the geofence engine uses small heap buffers). | ESP32-S3-WROOM-1 DS v1.8 p3 (R8: -40-65 C; ECC raises it to 85 C), Table 1-2 p3 (N16R8 -40-65; N16R2 -40-85), p27 (TA 65 C version); `hardware/THERMAL.md:200-201`; `docs/REQUIREMENTS.md:32`; `hardware/BOM.md:13`; `reports/secciones/02_hardware/01_esquematicos_y_bom.tex:104`; `docs/firmware_docs/BOOT_SEQUENCE.md:36` | Decide before buying O2 (not bought yet): either (a) order ESP32-S3-WROOM-1U-N16R2 (same footprint and pinout, GPIO35-37 stay NC as drawn, 85 C), or (b) keep N16R8 and make "PSRAM ECC enabled" a firmware requirement (check that Zephyr's ESP32-S3 PSRAM driver supports it, Unverified U-7). Record the choice in BOM.md 1.1, RNF-09 and THERMAL.md. |
| BOM-05 | MINOR | C31 and C32 (DNP 33 pF on USB D+/D-) are drawn in the live design with the `Cap_100uF_1206_DNP` symbol, so the PCB has 1206 pads on the USB data lines, while BOM.md, pinout.md and sheets.py specify the teacher's 0603 33 pF NP0 (CC0603JRNPO9BN330). If they are ever fitted, an 0603 part does not sit properly on 1206 pads, and the large pads add stub capacitance on D+/D-. | Live SchDoc C31/C32 (comment "DNP"/"Cap_100uF_1206_DNP", value "33pF DNP"); live PcbDoc C31, C32 pattern `1206_CAP` (scratchpad `scripts/pcb_comps.py`); `hardware/BOM.md:133`; `hardware/pinout.md:404`; `hardware/esquematico/sheets.py:60-61` | Replace C31/C32 with `Cap_33pF_0603_50V` (footprint `0603_cap`) and mark them Not Fitted in the variant (BOM-03). |
| BOM-06 | MINOR | C25 (second 22 uF, next to the LoRa) is a different part in the live design than in every document: live = `Cap_22uF_0805_6V3`, Samsung CL21A226MQQNNNE 6.3 V (C5674); BOM.md, COMPRAS O25 and sheets.py = Chinocera HGC0805R5226M160NSLJ 16 V. Both are teacher stock and 0805 X5R, so the board works, but the BOM and the assembled part differ, and at 3.3 V the 6.3 V part runs at 52 % of rating with a larger DC-bias capacitance loss than the 16 V one. | Live SchDoc C25 parameters; `hardware/BOM.md:105`; `hardware/COMPRAS.md:168`; `hardware/esquematico/sheets.py:205` | Pick one (16 V Chinocera preferred for DC-bias) and align the live SchDoc or the documents. |
| BOM-07 | MINOR | The optocoupler MPN is split across files. Purchase and BOM table: Lite-On LTV-817S-TA1-B (O13). Live SchDoc, library, sheets.py, ARCHITECTURE, pinout section title and the report table: Vishay VO617A-1X017T, and the report says the Vishay part is bought on AliExpress although COMPRAS says it cannot be found there. `BOM.md:210` still says "la PCB usa el VO617A". The LTV-817S datasheet used for the CTR/VCE(sat) calculation ("una copia de la hoja de Lite-On") is not in `hardware/datasheets/` and has no README row. BOM.md and COMPRAS name the footprint `SOIC254P1016X440-4N`; the library and PCB call it `VO617A-1X017T` (same 10.16 mm lead span, so only the name differs). The MPN LTV-817S-TA1-B exists (distributor listing). | `hardware/BOM.md:51`, `:210`; `hardware/COMPRAS.md:36`, `:162`; live SchDoc U10; `hardware/esquematico/sheets.py:247`; `reports/secciones/02_hardware/01_esquematicos_y_bom.tex:133`; `hardware/pinout.md:313`, `:324`; `hardware/datasheets/README.md` (no LTV-817 row); `hardware/libraries/vo617a/README.md:11`, `:34`; VO617A DS p8 (option 7 lead span 10.16 +- 0.3 mm), p2 (order code VO617A-1X017T); https://za.rs-online.com/web/p/optocouplers/1689666 | Set U10 Comment/MPN to LTV-817S-TA1-B (keep the footprint), add the Lite-On datasheet with a README row, fix the report table row and BOM.md:210; name the footprint as the library does. |
| BOM-08 | MINOR | BOM.md attributes named MPNs and Yageo to generic AliExpress parts. The 100 nF row says "CC0603KRX7R9BB104, YAGEO, lote con MPN nombrado", but COMPRAS O21 says the LECHEE listing "no nombra marca ni MPN (es genérico)". The kit rows for 100 k and 5.1 k say "por definir (serie RC0603FR-07), Yageo", but kit O19's seller is unknown and the kit has no MPN. | `hardware/BOM.md:102`, `:107`, `:110`; `hardware/COMPRAS.md:38`, `:164-165` | Write "genérico (lote O21) / sin marca" and "genérico (kit O19)"; keep Yageo MPNs only as the production reference in a separate column. |
| BOM-09 | MINOR | COMPRAS section 1.1 (teacher stock, the "stated source" for most parts) was not updated for the 2026-11-09 changes. There is no row or usage for the DN2302S (Q3) nor for the second AP2301 (Q2); D5 still describes the AP2301 only as M1 of the CN3791. D9 omits the 150 ohm (R16, UART0 TX) and the UART0 330 ohm, although section 1.2 says the teacher's 150 ohm is a 1 % library part; D10 lists the 1 uF only for the EN RC (C13), not the LDO output (C12). The stock audit still says "El DN2302S ya no hace falta". The teacher quantities of AP2301 (2 needed), DN2302S and TSC015 (3) remain unconfirmed. | `hardware/COMPRAS.md:46`, `:67`, `:71-72`, `:79`; `hardware/course-notes/AUDITORIA_STOCK_DOCENTE.md:150`; `hardware/BOM.md:128-129`, `:253`, `:255`; `hardware/esquematico/sheets.py:93-94`, `:105-106`, `:142-144` | Add D-rows (or extend D5/D9/D10) for DN2302S (Q3, LCSC C2931750), AP2301 x2 (Q1, Q2), 150 ohm (R16) and 1 uF x2 (C12, C13); fix the AUDITORIA line; keep the quantity confirmations as open items. |
| BOM-10 | MINOR | `pinout.md` (GPIO map and module table) and `pin_connections.csv` still give CHRG_STAT a 10 kΩ pull-up; the CN3791 section, BOM 1.9, COMPRAS D9 and sheets.py use 100 kΩ (R11, changed 2026-11-09). Anyone drawing from the CSV row moves one part between the 10 k and 100 k counts. | `hardware/pinout.md:49`, `:98` vs `:287`; `hardware/pin_connections.csv:35`; `hardware/BOM.md:106`, `:130`; `hardware/esquematico/sheets.py:74` | Change the three 10 kΩ mentions to 100 kΩ. |
| BOM-11 | MINOR | The report's BOM table and related text are behind the repo BOM: optocoupler "VO617A-1X017T, Vishay, AliExpress" (BOM-07); buzzer "POR DECIDIR" although the PS1240 test part was chosen on 2026-11-07; FPC antenna "2015301200, Würth, AliExpress" (twice) although a generic 42 x 12 mm FPC was bought (O9) and the Würth code is unverified; u.FL "Hirose" although a compatible part is bought; PCB "número de capas por confirmar, 2 o 4" although 2 layers were fixed on 2026-11-07. ARCHITECTURE.md still says the USB-C connector MPN is "POR DECIDIR" and CC1/CC2 "por definir" (TYPE-C-3.1-16PIN and 5.1 kΩ are decided). | `reports/secciones/02_hardware/01_esquematicos_y_bom.tex:17`, `:130`, `:132`, `:133`, `:135`, `:162`; `hardware/BOM.md:38-39`, `:52`; `hardware/COMPRAS.md:24`, `:37`; `hardware/PCB_REGLAS.md:3`, `:9`; `docs/ARCHITECTURE.md:38` | Update the report table (or generate it from the fixed BOM.md) and ARCHITECTURE.md:38. |
| BOM-12 | MINOR | Stale statements that contradict the current design (each is small; together they make the BOM files untrustworthy as a source): BOM header "REVISIÓN v1.16 (2026-11-07)" while its history reaches 1.20 (2026-11-09); COMPRAS header "actualizado el 2026-11-07" and order intro "pedido completo: 20 piezas" while O28 (2026-11-09) is pending and not in the total; BOM 1.6 "Falta bajar el datasheet de ST" (Rev 5 is in the repo and D3 is verified) and "Falta un conector o pads para el cristal RTC y los puntos de prueba" (drawn); BOM 1.8 22 uF "falta leer la hoja de Chinocera", COMPRAS D10 condition and pinout "leer la hoja del 22 uF" (the Chinocera sheet is in the repo: R5 = X5R, -55 to +85 C); BOM 1.8 22/33 ohm "por definir" (22 ohm fixed); COMPRAS D1 condition "la huella SOIC_8_3.9mm ... hay que hacer la huella" (the library `SOIC_8_3.9mm_EP` is used on U2); BOM 7 and COMPRAS 1.3 "Faltan el CN3791, el módulo LoRa y el zumbador" (all three are in the library and placed); BOM 6 "PC817 ... la PCB usa el VO617A"; COMPRAS section 3 status "pendiente" for O9 and O13 vs "elegido" in the order table; datasheets README "el BOM usa LSM6DSOX" (it uses the LSM6DS3TR-C) and keeps PDFs of LESD5Z3.3T1G and DS8570F8 although both left the BOM (its rule 4 says delete); COMPRAS O14 "Cant. 2 (se usan 2)" while the board has one buzzer (BZ1). | `hardware/BOM.md:3`, `:85`, `:87`, `:104`, `:112`, `:210`, `:229`, `:255`; `hardware/COMPRAS.md:3`, `:15`, `:34`, `:37`, `:42`, `:63`, `:72`, `:101`, `:160`, `:162`; `hardware/pinout.md:138`; `hardware/datasheets/README.md:10`, `:40`, `:48`, `:70`; live PcbDoc U2 `SOIC_8_3.9mm_EP`; `hardware/esquematico/sheets.py:57-58`, `:261` | Sweep the listed lines in one commit after BOM-01 (the regenerated tables remove most of them). |
| BOM-13 | MINOR | The 1575 MHz SAW footprint recommended by RF-02 has no BOM or purchase consequence written anywhere. If it is accepted before the PCB order, the BOM needs: (1) a chosen SAW MPN before layout, because the footprint is part-specific (the u-blox manual gives no part number); (2) a DNP row for the SAW and a fitted 0 ohm bypass row (+1 0 ohm, teacher FRC0603P000, or an 0402 RF jumper if the line is 0402); (3) library symbol and footprint (not in shared_lib); (4) a COMPRAS line (not in teacher stock; a named-MPN distributor such as LCSC/Digi-Key is more reliable than AliExpress for an RF filter). Today the PCB routes GNSS_RF straight from U6.11 to J4.2. | `docs/design-reviews/rf.md:60` (RF-02 fix); MAX-M10S IM p77 and p90 (external SAW recommended, no part number given); `hardware/pinout.md:180`; `hardware/BOM.md` and `hardware/COMPRAS.md` (no SAW line, grep); live PcbDoc (no part between U6 and J4) | Decide RF-02 first; if accepted, add the four items above in the same change as the layout edit. Open item, not an error today. |
| BOM-14 | ADVISORY | BOM selection criterion 3 ("-20 C a +65 C para todos los componentes") is stated as met but is not: the PS1240 buzzer is rated -10 to +70 C (5 C margin at the top), the TSC015 buttons declare no range (known condition), and the purchased battery has no datasheet. The product range is 0 to +65 C, so only the top end matters. | `hardware/BOM.md:13`; `hardware/COMPRAS.md:37` (PS1240 -10 to +70 C), `:75` (TSC015); `docs/REQUIREMENTS.md:32` | Reword the criterion to "0 to +65 C, with 5 C margin where available" and list the three exceptions. |
| BOM-15 | ADVISORY | The 3.3 V rail capacitance in the schematic is 75.5 uF, not the 55 uF documented nor the 64.5 uF of POWER PE-1: the schematic adds a 10 uF on 3V3_GNSS (C22) and the 1 uF on 3V3_LDO (C12). C12 1 + C14 22 + C15 0.1 + C16 10 + C21 0.1 + C22 10 + C23 10 + C24 0.1 + C25 22 + C27/C28 0.2 = 75.5 uF; with C26 = 100 uF fitted, 175.5 uF, still under the TLV757P 200 uF limit, so the largest C26 that fits is 124.5 uF. | `hardware/esquematico/sheets.py:105-107`, `:152-154`, `:185`, `:203-206`, `:229`; `hardware/pinout.md:241`; `hardware/BOM.md:105`, `:120`; `docs/design-reviews/power.md:61` (PE-1); TLV757P DS p4 (COUT 1-200 uF) | Use 75.5 uF in pinout/BOM; note that C26 must stay at or below about 120 uF. |
| BOM-16 | ADVISORY | O28 (X7R 10 uF 25 V 0805) still has no part. Two verified candidates with a named MPN: Samsung CL21B106KAYQNNE (LCSC C3039694) and Murata GRM21BZ71E106KE15L (LCSC C237493), both 10 uF, 25 V, ±10 %, X7R, 0805. A named MPN from a distributor avoids the "is it really X7R?" problem COMPRAS describes for generic lots (the dielectric cannot be measured on receipt). | `hardware/COMPRAS.md:34`, `:165`, `:185`; https://www.lcsc.com/product-detail/C3039694.html ; https://www.lcsc.com/product-detail/C237493.html | Pick one, fill O28, create the library part (BOM-02) and add its price to the COMPRAS total. |
<!-- findings-end -->

**Counts:** 16 findings: BLOCKER 0, MAJOR 4 (BOM-01 to BOM-04), MINOR 9 (BOM-05 to BOM-13), ADVISORY 3 (BOM-14 to BOM-16). Unverified: 9. Verified-OK groups: 8.

## 3. Verified OK

### 3.1 Library entry vs BOM/COMPRAS (MPN, manufacturer, LCSC code, footprint)
Every teacher-library part the design uses has the same MPN and LCSC code in `shared_lib.SchLib`, `BOM.md` and `COMPRAS.md` section 1: TYPE-C-3.1-16PIN C7507405 (`CON_USB_C_16P_RA`), USBLC6-2SC6 C7519 (`SOT_23_6`), TSC015A05026A C2888509, RS-3535MWAM C842778, XL-2012SURC C965812, S2B-PH-K-S(LF)(SN) C173752 (side entry, as BOM 1.6 says), AP2301 C385296, DN2302S C2931750, CY54-47UH C2929434, TP4056 YONGYUTAI C20199357 (`SOIC_8_3.9mm_EP`), HRK 35V100uF CD110 C2960381, HGC0805R5226M160NSLJ C33546008, HGC0603R7474K500NTHJ C33546006, CC0603JRNPO9BN330 C107047, CL21A106KAYNNNE C15850, TCC0603X5R106K100CT C380314, CL10A105KB8NNNC C15849, FRC0603F1200TS, RC0603FR-07390KL, FRC0603J103TS, PSBD1DF40V1H C110730. Own-library parts match the BOM MPNs: TLV75733PDBVR, MAX17048G+T10 (TDFN-8), MAX-M10S-00B, LSM6DS3TR-C, ECS-.327-12.5-34B-TR (ECX-31B), U.FL-R-SMT-1(10), CN3791 (`SOP-10_3.9x4.9mm_P1.00mm`, 1.00 mm pitch as pinout.md:281 requires), DX-LR30-900M22S, PS1240P02BT. The project library holds 84 symbols, as the brief states, including `TestPoint` (`TP_TH_15mil`, as BOM 1.9 says) and `Cap_100uF_1206_DNP` (`1206_CAP`).
- D1/D2: in the project library copy read here and in the live PCB, `D_Schottky_40V_1A_SOD_123F` links footprint `SOD_123FL`. The SOD_123 link reported earlier is fixed in this copy (not re-reported).
- VO617A-1X017T footprint: option 7 lead span 10.16 ± 0.3 mm (VO617A DS p8) matches the `P1016` footprint; LTV-817S-TA1-B is an SMD-lead 4-pin DIP of the same family (lead span to be measured on receipt, COMPRAS O13).

### 3.2 Live design vs generated sheets
The live SchDocs (5 sheets) and the live PcbDoc each hold the same 112 designators as `sheets.py` (U1-U10, Q1-Q3, D1-D4, J1-J5, L1, Y1, BZ1, SW1-SW3, C1-C33, R1-R40, TP1-TP11), with the same footprints except C31/C32 (BOM-05). Values match `sheets.py` except C25 (BOM-06); parameters and DNP are the problems (BOM-02, BOM-03).

### 3.3 Quantities that agree across BOM, COMPRAS, pinout, sheets and the live design
ESP32 module 1; GNSS 1; GNSS patch 1 (+1 spare, BOM 1.2, O3, report "1 (+1)"); u.FL 2 (J4, J5; lot of 20); JST PH 2 (J2, J3); TSC015 3 (SW1-SW3); 33 pF 4 fitted + 3 DNP; 22 uF 2; 100 nF 8 fitted vs 10 in BOM (2 spares, from a 100-piece lot); 100 k 7 (R10, R11, R23, R32, R37-R39 = BOM 2 + 5); 10 k 4 (R9, R13, R14, R30 = BOM 3 + 1); 3.3 k 2; Schottky 2; RPROG 1.8 k + 100 ohm; 390 k 1; 120 ohm 1; 5.1 k 2; 22 ohm 2 (value fixed); 0.47 ohm 1; TP4056, CN3791, TLV75733P, MAX17048, LSM6DS3TR-C, USBLC6, LEDs, crystal, buzzer 1 each. LoRa: 1 on the PCB, 2 bought (A1: collar + RF-04 receiver), explained in COMPRAS A1.

### 3.4 Ratings at +65 C ambient (operating temperature)
85 C or more: CN3791 -40 to 85 C (DS p3); TP4056 YONGYUTAI -40 to 85 C, Tj 145 C (DS p1); TLV757P TJ -40 to 125 C spec (DS p4); MAX-M10S -40 to 85 C (DS p11, p12); LSM6DS3TR-C -40 to 85 C (DS p22); DX-LR30 -40 to 85 C (manual p6); ECX-31B -40 to 85 C (DS p1); VO617A -55 to 110 C (DS p2); USBLC6 Tj -40 to 125 C (DS p2); DN2302S Tj -50 to 150 C (DS p1); PSBD1DF40V1H Tj -55 to 150 C, IF(AV) 1 A rated at Ta 65 C (DS p1); CY54 -55 to 125 C (DS p4); JST PH -40 to 105 C (DS p1); u.FL -40 to 105 C (Hirose spec p1); TYPE-C -25 to 85 C (spec p1); RS-3535MWAM -30 to 85 C; XL-2012SURC -40 to 85 C; Chinocera X5R -55 to 85 C, X7R -55 to 125 C (spec p3); Samsung/CCTC X5R -55 to 85 C (COMPRAS D10); YAGEO NP0 -55 to 125 C; HRK CD110 -40 to 105 C (series sheet p1); HX patch -40 to 80 C (COMPRAS O3); MAX17048 electrical table -20 to 70 C (DS p2). Exceptions: the ESP32 module (BOM-04), the PS1240 (-10 to 70 C, BOM-14), and three parts without data (Unverified).

### 3.5 Voltage, current and power derating
- Capacitor voltage: VPANEL (Voc about 8 V, 16 V hot-plug ringing estimate from `docs/design-reviews/power.md:85`) on 25 V ceramics, 35 V electrolytic and 50 V 470 nF; C8 (VG to VPANEL, at most 8 V gate clamp) 50 V; VBUS (5.25 V, 7.36 V hot-plug spike) on 25 V; VBAT (at most 4.25 V) on 25 V; 3.3 V rail on 10 V and 16 V (C25 live 6.3 V, 52 %, BOM-06). All at or below 50 % of rating except C25 live.
- Semiconductors: DN2302S VDS 20 V vs at most 1.3 V on MPPT, VGS ±10 V vs 3.3 V (DS p1); AP2301 Q2 VGS = -VBAT, at most 4.25 V, vs ±12 V, VDS -20 V vs 4.25 V reversed (DS p1); AP2301 Q1 |VGS| at most 8 V (`docs/design-reviews/power.md:88); USBLC6 VRM 5.25 V vs vSafe5V.
- Current and power (own arithmetic): RCS R12 at VCS max 130 mV and 0.277 A gives 36 mW (36 % of a 1/10 W 0603; rating of the O22 kit unverified, U-2); RPROG 1 V^2 / 1.9 k = 0.5 mW; opto R22 (3.3 - 1.1) V x 6.7 mA = 15 mW; LED resistors and R24 below 4 mW; Q2 at the worst bench case 0.885 A x 0.095 ohm (RDS at -2.5 V, DS p2) = 74 mW in SOT-23; L1 peak 0.39 A vs 0.71 A IDC (`docs/design-reviews/power.md:87); J2 0.885 A vs 2 A; TSC015 0.33 mA vs 50 mA; RGB LED 2 mA vs 20-25 mA max; D4 0.9 mA vs 20 mA.

### 3.6 Cost tables
- BOM.md section 4: 24.50 + 8.75 + 3.25 + 5.50 + 1.25 = 43.25 USD per unit and 24,500 + 8,750 + 3,250 + 5,500 + 1,250 = 43,250 USD per 1000. Correct. The report repeats 43.25 USD with the same five items (`.tex:157-164`). Both documents already flag the figures as unsupported and the lot size (100 vs 1000) as open; not re-reported.
- COMPRAS order total "unos $209.897 COP": reproduces exactly with the listed prices, 20 rows with a link, A1 x2 at $25.431, O3 x2 at $3.541 plus $11.724 shipping, and O11 plus $13.441 shipping (own script). It excludes O28 (no price yet) and IVA (COMPRAS 3.1), and 14 rows use one-per-buyer new-customer prices (already warned at COMPRAS:3).

### 3.7 Hardware changes from the other audits, as reflected in BOM/COMPRAS
| Change (source) | BOM.md | COMPRAS | Schematic / live | Status |
|-----------------|--------|---------|------------------|--------|
| AP2301 reverse-battery Q2 (POWER PD-1) | 1.9 row | no D-row (BOM-09) | Q2 present | partly |
| DN2302S solar inhibit Q3 + R32 (POWER PD-2) | 1.9 rows | no D-row (BOM-09) | present, datasheet in repo | partly |
| X7R 10 uF at TP4056 VCC, O28 (POWER PB-3) | 1.9 row; 1.8 row still counts it as X5R (BOM-01) | O28, link pending | live C1 has the X5R MPN (BOM-02) | partly |
| 0 ohm links R33-R36, R40 (MCU I-3, E-5, F-1) | 1.9 row | D9 | present | OK |
| GNSS RESET_N via R34 to GPIO48 (MCU F-1) | 1.9 row | D9 | present | OK |
| I2C pull-ups 3.3 k (MCU D-1) | 1.9 row | D9 | R20, R21 | OK |
| NSS pull-up R37 (MCU E-1) | 1.9 row | O19 | present | OK |
| RXEN/TXEN pull-downs R38, R39 (MCU E-2) | 1.9 row | O19 | present | OK |
| CHRG_STAT pull-up 100 k (MCU G-1) | 1.9 row | D9 note | R11 | pinout/CSV stale (BOM-10) |
| DNP 33 pF on D+/D- and SCK (MCU C-2, E-5) | 1.9 row | D10 | C31-C33 | footprint and DNP problems (BOM-03, BOM-05) |
| Test pads TP8-TP11 (MCU F-2, I-6) | 1.9 row | not needed | present | OK; TP1-TP7 unlisted (BOM-01) |
| STDBY/CHRG of TP4056 to GND (POWER PB-4) | no part | no part | sheets.py:64 | OK |
| 1 % 10 k in R3 (POWER PA-1) | not adopted (R9 = FRC0603J103TS, 5 %) | — | R9 5 % | open, not re-reported |
| SAW footprint (RF RF-02) | absent | absent | absent | open (BOM-13) |

### 3.8 MPN existence
All MPNs in the BOM tables exist in the repo datasheets or as library parts with an LCSC code, except the gaps checked online: LTV-817S-TA1-B (RS Components stock no. 168-9666, https://za.rs-online.com/web/p/optocouplers/1689666) and the O28 candidates (BOM-16). Lifecycle data is not in the repo: the ActiveBOM shows `STATUS=Unknown` for every catalog item and no lifecycle column is populated; the only recorded lifecycle event is the Murata PKMCS0909E4000-R1 last-time-buy of 31-Mar-2027 (backup buzzer only, COMPRAS O14).
<!-- verified-end -->

## 4. Unverified

- U-1: LTV-817S-TA1-B datasheet (CTR rank B 130-260 %, VCE(sat), operating range, lead span, CTR at 65 C). The pinout calculation quotes "una copia de la hoja de Lite-On" that is not in the repo; the MPN exists (3.8).
- U-2: Power rating and tolerance of the 0.47 ohm from kit O22 (the listing says "1 % o 5 %" and gives no power). 36 mW is needed (3.5).
- U-3: Rated current and maximum resistance of the FOJAN FRC0603P000 0 ohm jumpers. R33 carries the whole 3.3 V rail (peak below about 0.6 A, own estimate) and R35 counts toward the 0.2 ohm GNSS supply limit (`pinout.md:176`); no datasheet in the repo.
- U-4: PS1240 buzzer. The TDK datasheet is not in the repo. The library description (SnapEDA/Digi-Key text) says "60dB @ 3V, 10cm", while TDK's table gives 70 dB min at 10 cm (https://www.digikey.com/en/products/detail/tdk/PS1240P02BT/935924 vs https://octopart.com/part/tdk/PS1240P02BT). BOM.md uses 70 dBA. The TZT copy that is bought has no data of its own.
- U-5: Purchased battery (O6). There is no datasheet. A typical LiPo is rated for discharge only up to about 60 C (assumption); the in-enclosure +65 C may exceed it. POWER PD-2 covers charging temperature only.
- U-6: Teacher quantities: 2 AP2301 (Q1, Q2), 1 DN2302S, 3 TSC015 (open, `BOM.md:253`, `:255`; the TSC015 quantity is a listed open decision).
- U-7: Whether Zephyr's ESP32-S3 PSRAM driver supports PSRAM ECC (needed for option (b) of BOM-04).
- U-8: Generic 32.768 kHz crystal (O8): ESR, temperature range and pad layout against the ECX-31B footprint. Already listed as a check on receipt in COMPRAS O8.
- U-9: Final cost: 14 of the 20 priced rows use one-per-buyer new-customer prices; their normal prices are mostly not recorded.
<!-- unverified-end -->

## 5. Cross-domain notes

- **MCU:** `docs/PROJECT_CONTEXT.md:178` still puts the phase-2 modem PWRKEY on GPIO48, which is GNSS_RESET since 2026-11-09 (`ARCHITECTURE.md:159` is updated). BOM-04 is a module-variant choice: N16R2 keeps the same pins; PSRAM ECC is a firmware setting.
- **POWER:** PA-1's 1 % 10 k for R3 is not adopted (R9 is still the teacher's 5 % FRC0603J103TS in BOM 1.8 and in the live SchDoc). The rail-capacitance figure of PE-1 grows to 75.5 uF with C22 and C12 (BOM-15). Battery discharge temperature at +65 C (U-5) is not covered by PD-2.
- **RF:** if RF-02 is accepted, the SAW needs an MPN before layout (BOM-13). The bought u.FL is a compatible part, and the report table still names Hirose (BOM-11).
- **FIRMWARE:** if BOM-04 is closed with option (b), "PSRAM ECC enabled" becomes a firmware requirement and 1/16 of the PSRAM is lost.
- **ALTIUM / LAYOUT:** BOM-02, BOM-03 and BOM-05 are fixes in the live SchDoc and PcbDoc (C31/C32 footprint swap, variant, parameters). Per the project rules, Altium files are edited only by the author.
- **REPORT:** `.tex` table rows for the optocoupler, buzzer, FPC antenna and u.FL, and the layer count (BOM-11).
<!-- cross-end -->

