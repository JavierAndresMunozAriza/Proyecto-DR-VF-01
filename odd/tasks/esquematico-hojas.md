# Esquemático DR-VF-01 por hojas: imágenes estilo Altium, bloques 1 a 5
Objetivo: una imagen estilo Altium por bloque (5 hojas), con todas las conexiones de `hardware/pinout.md`, sin errores.
Rama: `chore/repo-audit` (sin commits hasta que el usuario lo pida). Entrega: ask-on-risk. Runner: `python3` con venv (`olefile`, `cairosvg`).

## Specs
- S1: "una imagen estilo Altium por bloque": "bloque por bloque en imagenes de estilo altium, de manera puntual y profesional con la informacion del pinout.md".
- S2: "darme las imagenes para solo imitar las conexiones, de manera organizada y profesional de industria".
- S3: "sin cometer errores en las conexiones": cada pin de cada símbolo con red asignada o NC explícito; comprobado contra `pinout.md`, `pin_connections.csv`, los pines del símbolo y los datasheets.
- S4: "decide si se hace por diferentes hojas de esquematico por bloque o todos los bloques en uno solo": decidido, una hoja por bloque (01 Potencia, 02 MCU, 03 RF, 04 Sensores, 05 Interfaz), `Net Identifier Scope = Global` (supuesto, ver Log).
- S5: "hazlo desde el bloque 1 hasta el 5 completo con toda la exigencia posible": las 5 hojas en una entrega, con verificador independiente.

## Tasks
- [x] T1 (S3) Lector de geometría de símbolos `hardware/esquematico/symlib.py` — inline (sin commit)
- [x] T2 (S1, S2) Modelo (`model.py`) + renderizador SVG estilo Altium (`render.py`) — inline (sin commit)
- [x] T3 (S1, S3) Especificación de las 5 hojas (`sheets.py`): 92 piezas, 174 pines de CI, 70 redes — inline (sin commit)
- [x] T4 (S3) Comprobaciones (`checks.py`): 0 errores de estructura/eléctricos; 336 pines coinciden con `pinout.md` y el CSV, 0 difieren, 14 revisados a mano (USB) — inline (sin commit)
- [x] T5 (S5) Verificador independiente (2 agentes de solo lectura, sin ver la especificación): 19 componentes, todos los pines coinciden; hallazgos en las fuentes (TP4056 PROG 2.4k = 400 mA con el chip Yongyutai, AP2301 sin numeración impresa, datos de apoyo erróneos en pinout.md) — delegated
- [x] T6 (S1, S2) Artifact privado con las 5 hojas, detalles, piezas, tablas de pines y verificación (https://claude.ai/artifact/SbLNXFkfJaNsQVD132mg1r) — inline
- [x] T7 (S3) Revisión visual (PNG) de cada hoja y de sus detalles; ajustes de solapes — inline (la página HTML no se renderizó en un navegador)

Ruta: todo inline salvo T5 (verificador pedido por el usuario con "toda la exigencia posible").

## Log
- L1 (2026-11-08): "si puedes darme a detalle bloque por bloque en imagenes de estilo altium, de manera puntual y profesional con la informacion del pinout.md, si lohaces, asegurate de no cometer erroes y darme las imagenes para solo imitar las conexiones, de manera organizada y profesional de industria. dime q modelo necesitas"
- L2 (2026-11-08): "dame cada bloque completo, decide si se hace por diferentes hojas de esquematico por bloque o todos los bloques en uno solo. sin cometer errores en las conexiones."
- L3 (2026-11-08): "hazlo desde el bloque 1 hasta el 5 completo con toda la exigencia posible"
- 2026-11-08: Supuesto: `Net Identifier Scope = Global` para que las etiquetas que el usuario ya puso en la hoja 1 (USB_DP, USB_DM) sigan valiendo entre hojas. Los nombres de red internos deben ser únicos en todo el proyecto.
- 2026-11-08: Polaridad leída de la geometría de los símbolos: `D_Schottky_40V_1A_SOD_123F` pin 1 = cátodo, pin 2 = ánodo; `LED_0805_*` pin 1 = ánodo, pin 2 = cátodo; `Cap_100uF_ALU_TH_35V` pin 1 = positivo.
- 2026-11-08: El zumbador de la librería es `PS1240P02BT`; `pinout.md` aún describe el Murata. Mismas conexiones.
- 2026-11-08: RPROG del TP4056 pasa a 1.8k + 100R en serie (1.9 kΩ, ~500 mA) por el datasheet de Yongyutai; docs sincronizados.
- 2026-11-08: Usuario: "pq no directamente por botones??" Se añaden botones EN (SW2) y BOOT (SW3) en la hoja 02, en paralelo con TP1 y TP2; SW1 sigue siendo el botón de usuario (hoja 05). 3 TSC015 en total (cantidad por confirmar con el docente). Docs sincronizados (pinout v2.11, BOM 1.18, COMPRAS D13, CSV).
- 2026-11-08: Comprobación símbolo/footprint (40 símbolos): 39 coinciden; el footprint del ESP32 tiene 12 vías térmicas (`41_10` a `41_21`) sin pin en el símbolo (el usuario debe nombrarlas `41_1` en la PcbLib). Decidido (plan mode, aprobado): sin fusible PTC en el USB-C, sin diodo ESD LESD5Z3.3T1G y sin supervisor de EN en la primera placa (pinout v2.12, BOM 1.19). Título 5A renombrado a "Optoacoplador de salida (RF-03)". Artifact v4.
- 2026-11-09: Cambios de hardware tras las auditorías MCU y POWER (plan aprobado): GPIO21 = CHG_INHIBIT (N-FET Q3 en MPPT, 100 kΩ a GND), GPIO48 = GNSS_RESET (0 Ω R34), EXTINT del GNSS sin conectar; AP2301 Q2 contra batería invertida entre J2 y VBAT; pull-up NSS (R37), pull-downs RXEN/TXEN (R38, R39), CHRG_STAT 100 kΩ, I2C 3.3 kΩ, STDBY/CHRG del TP4056 a GND, C1 X7R, DNP en D+/D- y SCK, puentes de 0 Ω (R33, R35, R36), pads TP8 a TP11. Designadores nuevos explícitos y altos para no renumerar lo ya dibujado (R32 a R40, C31 a C33, Q2, Q3). 3V3_GNSS y 3V3_LORA se dibujan como Power Port. 115 piezas, 180 pines de CI, 78 redes, 0 errores, 330 pines coinciden con pinout y CSV. Docs sincronizados: pinout 2.13, BOM 1.20 (sección 1.9), COMPRAS O27, CSV, ARCHITECTURE, PROJECT_CONTEXT, .tex. Artifact v5.
- 2026-11-09: N-FET de la biblioteca = DN2302S (Doeshare, LCSC C2931750): datasheet guardado en hardware/datasheets/dn2302s_datasheet.pdf; VGS(th) 0.4 a 1.0 V, RDS(on) 70 mΩ máx. con 3.3 V: conduce con la puerta a 3.3 V (verificado). Condensador 10 µF X7R 25 V 0805 anotado como compra pendiente con enlace pendiente (COMPRAS O28; O27 ya era la cinta de cobre). Las hojas v5 no cambian por esto.
- 2026-11-08: Hoja 01: el usuario ya dibujó J1, U1, U2, C1, C2, R1 a R6 (R1 = CC2, R2 = CC1, R3 = USB_DP, R4 = USB_DM, R5 = 3k, R6 = 12k). Se conservan esos designadores.
