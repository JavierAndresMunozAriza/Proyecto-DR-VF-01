# BOM audit (read-only), branch chore/repo-audit

Legend: [F] verified fact (repo evidence or well-known ESP32-S3 fact), [A] assumption, [D] needs-datasheet-check. No repo file edited. Overlaps with odd/tasks/repo-audit-report.md (E7-E17, C1-C8) are noted, not repeated in full.

## A. BOM.md internal consistency
| ID | Sev | Where | Finding |
|---|---|---|---|
| B1 | major | BOM.md:97-99, 115-118, 166-173, 191-204 | File is structurally corrupted: "Panel Solar" heading (97) has an empty table; its rows (201-204) are at EOF after the cost-lot table. "Costos Estimados" header (117-118) is split from its rows (167-173). Resistor rows (203-204) sit under the solar table. [F] |
| B2 | major | BOM.md:26-28 | ESP32-S3-WROOM-1U-N16R8 already integrates 16 MB flash + 8 MB PSRAM; separate GD25Q128CWIGR (27) and IS62WVS5128GBLL (28) are redundant and have no free GPIO on this module (GPIO26-37 are taken). IS62WVS5128 is, I believe, a 4 Mbit part, not 8 Mbit [D]. Never referenced elsewhere. |
| B3 | major | BOM.md:74 | "Cristal 40MHz" has MPN ECS-.327-... (a 32.768 kHz series code [D]). Module has its own 40 MHz crystal; SX1262 needs 32 MHz XTAL/TCXO. Role of the crystal and of 4x 22 pF (75) is undefined. |
| B4 | minor | BOM.md:60, 94 | USB-C connector (qty 1) and "Tapas ciegas USB-C" (qty 2) share the same Digi-Key number 1240-6813-1-ND; a DK number is not an MPN, and a receptacle is not a blind cap. [F] |
| B5 | minor | BOM.md:61-62, 69, 203-204 | Qty without a designated use: 2x JST PH, 4x JST SH 4-pin (no schematic/pinout use), 3x indicator LED (pinout.md has only RGB, no GPIO for them), 3x 10k + 2x 100k (pinout needs 2x 10k pull-ups for GPIO15/16 at pinout.md:154-155; 100k unexplained). No designators anywhere. [F] |
| B6 | minor | BOM.md:117-173, 191-199 | Arithmetic OK: 24.50+8.75+3.25+5.50+1.25 = 43.25; x1000 = 43,250. But lot cost is exactly unit x1000 (no volume break), and 01_esquematicos_y_bom.tex says "lote de 100" while BOM says lot of 1000. [F] |
| B7 | minor | BOM.md:24-48,53-56 | Rating columns (V/I/package) do not exist; only qty/desc/MPN/maker/supplier. Package inferable only from MPN. |
| B8 | minor | BOM.md:152-157 | Revision 1.1 dated 2024-06; files elsewhere dated 2026-10. Stale. |

## B. Cross-file contradictions
| ID | Sev | Files:lines | Finding / suggested resolution |
|---|---|---|---|
| X1 | blocker | pinout.md:27-28 vs :33 | GPIO05 assigned to both GNSS UART1 RX and SPI SCK of SX1262. [F] Reassign one. |
| X2 | blocker | pinout.md:46-47 (GPIO22, GPIO23), 06_diagramas.tex:17,23,52,54 (GPIO22, 23) | ESP32-S3 has no GPIO22-25. [F] CHARGE_STAT/POWER_GOOD/ENABLE_CHG and tex I2C/SPI refs are unroutable. |
| X3 | blocker | 06_diagramas.tex:27,57 | BG95-M3 UART on GPIO26/27 = SPI flash pins on WROOM-1U-N16R8. [F] |
| X4 | major | pinout.md:43-44 | Lists D+=GPIO19, D-=GPIO20. On ESP32-S3 GPIO19=USB D-, GPIO20=USB D+. [F] |
| X5 | major | pinout.md:55 | GPIO03 (LED_RED) is a strapping pin (JTAG source); GPIO00 also used for BOOT (:182). GPIO45/46 unused (good). Strapping use is legal but needs check of pull states at reset. [F] |
| X6 | major | pinout.md:101-102 | Diagram says MAX-M10S TX -> GPIO04 "UART1 TX" and RX -> GPIO05 "UART1 RX": TX->TX direction is wrong (GNSS TX must go to ESP RX). Table (27-28) lists ESP side TX=GPIO04, so diagram contradicts table. [F logic] |
| X7 | major | pinout.md:112-127 pin numbers | LSM6DSOX INT1 shown at pin 6, VDD pin 10, GND pin 1; my recollection of the LGA-14 pinout is INT1 = pin 4, VDD = pin 8, VDDIO = pin 5 [D]. SX1262 QFN-24 pins (NSS 4, RST 3, DIO1 11, BUSY 12, VCC 10) also look wrong (recollection: DIO1 13, BUSY 14, NRESET 15, SPI 16-19) [D]. Same for MAX-M10S pins 10-12 [D]. Re-derive from datasheets. |
| X8 | major | 3 incompatible pin maps | pinout.md/BOOT_SEQUENCE.md:22,29 (UART 4/5, I2C 17/18, SPI 5-8, IMU INT 15, opto 1) vs ARCHITECTURE.md:100-115 and PROJECT_CONTEXT.md:166-181 (SPI 11/12/13/10, I2C 1/2, UART 43/44, IMU INT 4, opto 5) vs 06_diagramas.tex:16-55 (UART 16/17, I2C 21/22, SPI 5/18/19/23, opto 13, buzzer 14). REQUIREMENTS.md:15 says opto = GPIO5. [F] ARCH/PROJECT_CONTEXT are DevKit/prototype; decide which is the PCB map. |
| X9 | major | BOM.md:53 vs ARCHITECTURE.md:11-33,70, PROJECT_CONTEXT.md:54,100 | PCB BOM: external TPS7A0533 (200 mA). Docs: LDO internal to DevKit (AMS1117, 500 mA). Bare WROOM module has no VIN pin / LDO; pinout.md:73 lists a "VIN" pin that a bare module does not have. [F] |
| X10 | major | BOM.md:56 vs REQUIREMENTS.md:23, ARCHITECTURE.md:130, PROJECT_CONTEXT.md:42,194, POWER_BUDGET.md:201 | Battery 1500 mAh (BOM) vs 1200 mAh (requirement, prototype). DATASHEET.md:40 "1200-1500". [F] |
| X11 | major | BOM.md:201 vs REQUIREMENTS.md:23, ARCHITECTURE.md:131 | Panel 2 W (BOM, DATASHEET:27, POWER_BUDGET:256) vs 6 V / 1.5 W (REQ, ARCH). Panel voltage of HTJ-2W-D not stated anywhere. [F] |
| X12 | major | BOM.md (no row) vs REQUIREMENTS.md:17, ARCHITECTURE.md:16,69,128, PROJECT_CONTEXT.md:21,60,192 | MAX17048 fuel gauge required by RF-05 but not in BOM (nor in tex BOM table). Also missing from BOM: CN3791 inductor, P-MOSFET, sense resistor, divider, 330 ohm opto resistor, 4.7k I2C pull-ups, NTC (THERMAL.md:203), PTC (pinout.md:170), USB-C CC 5.1k, USB ESD, antenna for the ESP32 U.FL, 915 MHz LoRa antenna. [F] |
| X13 | blocker | BOM.md:40-41 vs mechanical/ANTENNA_PLACEMENT.md:5,11,54,107, DATASHEET.md:41 | Antenna is 2.4 GHz FPC, but LoRa is 915 MHz (USER_MANUAL:69, DATASHEET:15) and GNSS is 1.575 GHz. No 868/915 MHz antenna in BOM. ANTENNA_PLACEMENT claims it serves "LoRa y GNSS" (:5). DATASHEET says 8 dBi vs 2.5 dBi in ANTENNA_PLACEMENT:13. 2015301200/6521011000 part identity unverified [D]. |
| X14 | major | A7670SA (PROJECT_CONTEXT.md:24,136; ARCH:72) vs BG95-M3 (06_diagramas.tex:27, 01_Defensa) | Phase-2 cellular module inconsistent; neither is in BOM (acceptable if Phase 2, but name one). |
| X15 | minor | ARCHITECTURE.md:125-127, PROJECT_CONTEXT.md:189 | Prototype parts (DevKitC-1, Waveshare Core1262-HF, breakouts) are not in BOM.md, by design; label that doc "prototype BOM" to avoid confusion with PCB BOM. |
| X16 | major | README.md:61,10 | Thingy:91 X is stated as "Primary Industrial Target", ESP32-S3 as "Secondary"; contradicts intended roles (PCB = ESP32-S3, Thingy = PoC only). BOM.md has no Thingy mention (correct). REAL_TIME_RISKS.md:168,255 uses Thingy as test bench (acceptable as PoC). |
| X17 | minor | THERMAL.md:18 | LDO input 5.0 V "USB/CN3791"; power tree has VBAT 3.0-4.2 V (see audit report E7-E9). |

## C. BOM components never referenced outside BOM.md
GD25Q128CWIGR, IS62WVS5128GBLL, ANNMS-420.A, 2015301200, 6521011000, PKMCS0909E000-R1 (buzzer appears only as "Buzzer" in pinout.md:54), BQ2975DRBR, PL063-1500, B2B-PH-K, B4B-SH-SM4-TB, PTS645SL50SMTR92, L-154A4SURK/G5T, LTST-C193KGKT, crystal, caps (C0G2A220J5GACA024, CGA2B1X7R1E104K050BC, C1608X5R1A106K080AE, TAJA107K016RNJ), HTJ-2W-D, RC0603 resistors, 1240-6813-1-ND, mechanical parts. Only 467MP appears elsewhere (ANTENNA_PLACEMENT.md). Core ICs (ESP32-S3, MAX-M10S, SX1262, LSM6DSOX, VO617A, CN3791, TPS7A0533) are consistently named across pinout, POWER_BUDGET, THERMAL, DATASHEET, tex 01_esquematicos_y_bom.tex:99-104. The tex BOM table lists only 4 of ~35 BOM lines.

## D. Datasheet sanity checks
| ID | Sev | Status | Finding |
|---|---|---|---|
| D1 | blocker | F (loads from repo) / D (LDO limits) | TPS7A0533 is 200 mA. Repo's own loads: ESP32 active 80 mA avg (POWER_BUDGET:16-20) with peaks ~240 mA (ARCH:182), SX1262 TX 118-120 mA, GNSS 35-65 mA -> 221-394 mA sum (ARCH:70,182) > 200 mA. ESP32-S3 with Wi-Fi/BT also needs a supply able to handle several hundred mA peaks (Espressif guideline ~500 mA) [D]. Also LDO dissipation and dropout at 200 mA with VBAT near 3.4 V (audit E7). Needs buck/higher-current LDO or a verified duty profile (no radio overlap). |
| D2 | blocker | F/D | SX1262 as a bare chip needs: 32 MHz XTAL or TCXO, DC-DC inductor/cap, RF switch (DIO2/TXEN/RXEN), LC matching + harmonic filter. None in BOM. |
| D3 | major | D | BQ2975DRBR: I recall a bq297xx-family protector that drives external back-to-back FETs; no FETs, sense resistor, or NTC in BOM. Confirm that PL063-1500 already has a PCM (ARCH:130 says "con PCM"); if so, BQ2975 is redundant. Battery part PL063-1500 (PowerStream) not verified [D]. |
| D4 | major | D | CN3791: 1-cell Li-ion MPPT charger requires external P-MOSFET, inductor, sense resistor, MPPT divider and optional NTC; none in BOM. It is not a USB charger by design, yet pinout.md:41-47, DATASHEET.md:146 describe USB-C charging. No VBUS path/diode-OR in BOM. Vin range stated as 4.5-24 V (POWER_BUDGET:72) and 5 V (THERMAL:34) vs 6 V panel; VREF 2.0 vs ~1.04 V is doubted in ARCH:54 [D]. LiPo cannot be charged below 0 C; requirement is -20 C (BOM:161): NTC/cutoff is needed [A]. |
| D5 | major | F | IMU wake: LSM6DSOX INT1 on GPIO15 (pinout) is RTC-capable (ESP32-S3 RTC GPIO = GPIO0-21), valid for ext0/ext1 wake. GPIO4 (ARCH) also valid. But tex uses 21/22 (22 nonexistent). Pinout adds 10k external pull-up to an INT line (pinout:154); LSM6DSOX INT is push-pull by default so use pull-down or none; ensure active-high polarity matches ext wake level [D]. Register values in firmware docs already flagged in audit E17. |
| D6 | major | F | N16R8 pins: GPIO26-32 flash, GPIO33-37 octal PSRAM (35/36/37 certainly reserved), GPIO0/3/45/46 strapping, GPIO19/20 USB, GPIO43/44 UART0. pinout.md avoids 26-37 (good) but uses 0 (boot), 3, 22, 23 (see X2, X5). |
| D7 | major | A/D | GNSS: MAX-M10S supply is nominally 1.8-3.6 V and wants a clean 3.3 V rail; sharing the LDO with the LoRa TX burst needs bulk cap/ferrite. Passive ceramic patch ANNMS-420.A [D] needs ground plane and keep-out; no matching/bias-T needed for passive, but no ESD/LNA filtering stage is in BOM. Antenna placement doc covers only 2.4 GHz (X13). |
| D8 | minor | F | USB path: native USB on GPIO19/20 gives programming/JTAG without a UART bridge; BOM lacks CC 5.1k resistors, ESD array, series resistors, and BOOT/EN buttons (only one tactile button, used as user button on GPIO14). Auto-download needs USB-Serial-JTAG or manual BOOT/EN. [A] |
| D9 | minor | F | VO617A driven directly from GPIO1 at 50 mA (POWER_BUDGET:68, pinout:53,134) vs 12 mA/pin claim (pinout:160) vs 330 ohm limiting 6.6 mA (ARCH:80). Resolve with series resistor + stated CTR; ESP32-S3 per-pin limits [D]. |
| D10 | minor | F | GNSS "INT_GNSS" GPIO16 (pinout:65) without an equivalent MAX-M10S pin named (EXTINT/TIMEPULSE?) [D]. |

## E. Proposed next steps for the user (not decided)
1. Declare which pin map is the PCB map; regenerate pinout.md from the ESP32-S3 GPIO table.
2. Decide power architecture (buck vs bigger LDO) given D1.
3. Decide the 915 MHz antenna and complete SX1262 RF/clock BOM lines.
4. Repair BOM.md structure and add missing support parts.
