# Plan paso a paso: lo que falta por definir (preparado el 2026-11-05)

Orden recomendado. **Quién:** *Tú* = navegador, compras o Altium; *Yo* = Claude. Los archivos que bajes del navegador van a `Descargas`: yo los copio a `hardware/datasheets/` y los reviso.

## Cómo funciona la placa (para tener claro el punto de partida)

- **Carga:** por **USB-C** con el TP4056 (limitado a 500 mA) y por **panel solar** con el CN3791. Un USB de 5 V nunca arranca al CN3791, así que cada entrada tiene su cargador.
- **Programación:** por el **USB-C**, con el USB nativo del ESP32-S3 (GPIO19 y GPIO20) y el botón BOOT a GND para el modo descarga.
- **Ojo:** el USB-C **no alimenta** al regulador de 3.3 V. Sin batería conectada, la placa no arranca desde el USB (`docs/ARCHITECTURE.md`). Para programar hace falta la **batería conectada**.
- **Batería provisional:** cualquier 1S Li-ion o LiPo de 4.2 V cargada, **con protección integrada**, de al menos **500 mAh** (el TP4056 carga a 500 mA, y una celda más chica se cargaría a más de 1C), y con conector **JST PH de 2 pines**. La definitiva se consigue después.

## Estado al 2026-11-07

| Bloque | Estado |
|--------|--------|
| 1. Descargas | Hecho |
| 2. Datasheets del docente | Hecho (filas 6 a 15). La fila 16 es opcional y no se bajó |
| 3. Revisión con lo descargado | Hecho (pasos 17 a 21) |
| 4. Elegir | Pasos 22, 23 y 25 hechos con piezas de prueba; el 24 (zumbador) y el 26 (bulk de 3.3 V) siguen abiertos. Ver "Resultados del bloque 4" al final |
| 5. Compras | **Cambio del 2026-11-07:** todo se compra por AliExpress, no por Digi-Key ni Mouser. El pedido es un solo cuadro al inicio de `COMPRAS.md`: 10 piezas con enlace, 10 sin enlace y el bulk de 3.3 V por decidir |
| 6. Librerías | Faltan el CN3791, el módulo LoRa y el zumbador; lo demás ya está en la biblioteca propia `DR-VF-01` |
| 7. Limpieza de documentos | Pasos 38 y 39 hechos el 2026-11-07; el 40 (borrar el datasheet viejo del cristal) es tuyo |
| 8. Antes del layout | Pendiente |

## Bloque 1. Descargas en el navegador (tú) — prioridad alta

| # | Qué bajar | Enlace o dónde buscar | Para qué lo necesito |
|---|-----------|-----------------------|----------------------|
| 1 | Datasheet del Schottky `PSBD1DF40V1H` | [LCSC C110730](https://www.lcsc.com/product-detail/Switching-Diode_Shanghai-Prisemi-Elec-PSBD1DF40V1H_C110730.html) | Leer su fuga a 4.2 V. Decide si se compran los 2 Schottky de O20 |
| 2 | Datasheet del regulador `DS8570F8` (DSTECH) | lcsc.com, buscar `C5884245` | Ver su corriente en reposo y si reemplaza al TLV75733 |
| 3 | Datasheet del TP4056 **de YONGYUTAI** (el del docente) | [LCSC C20199357](https://lcsc.com/product-detail/Battery-Management_YONGYUTAI-TP4056_C20199357.html) | El que tenemos es de otro fabricante. Necesito el dibujo del encapsulado ESOP-8 |
| 4 | Dibujo del conector `U.FL-R-SMT-1` | hirose.com, página del producto | Verificar la huella del U.FL (y ver si ofrece el STEP) |
| 5 | Datasheet del zumbador `PKMCS0909E4000-R1` | murata.com, buscar el código | Presión sonora, frecuencia y consumo |

## Bloque 2. Datasheets de las piezas del docente que usamos (tú) — prioridad media

Abre cada enlace y baja el PDF del datasheet. Las piezas de las filas 6 a 11 son las que más importan.

| # | Pieza | Enlace |
|---|-------|--------|
| 6 | USB-C Shou Han `TYPE-C-3.1-16PIN` | [LCSC C7507405](https://lcsc.com/product-detail/USB-Connectors_SHOU-HAN-TYPE-C-3-1-16PIN_C7507405.html) |
| 7 | `USBLC6-2SC6` de ST (Rev 7 desde st.com) | [LCSC C7519](https://www.lcsc.com/product-detail/ESD-Protection-Devices_STMicroelectronics-USBLC6-2SC6_C7519.html) |
| 8 | MOSFET `AP2301` | [LCSC C385296](https://www.lcsc.com/product-detail/MOSFETs_ALLPOWER-ShenZhen-Quan-Li-Semiconductor-AP2301_C385296.html) |
| 9 | Inductor `CY54-47UH` | [LCSC C2929434](https://lcsc.com/product-detail/Inductors-SMD_SHOU-HAN-CY54-47UH_C2929434.html) |
| 10 | LED RGB `RS-3535MWAM` | [LCSC C842778](https://www.lcsc.com/product-detail/Light-Emitting-Diodes-LED_Foshan-NationStar-Optoelectronics-RS-3535MWAM_C842778.html) |
| 11 | Electrolítico `35V100uF CD110` (HRK) | [LCSC C2960381](https://lcsc.com/product-detail/Aluminum-Electrolytic-Capacitors-Leaded_HRK-35V100uF-CD110_C2960381.html) |
| 12 | LED rojo `XL-2012SURC` | [LCSC C965812](https://www.lcsc.com/product-detail/Light-Emitting-Diodes-LED_XINGLIGHT-XL-2012SURC_C965812.html) |
| 13 | TVS `LESD5Z3.3T1G` (para el diodo ESD de la alimentación) | [LCSC C136161](https://lcsc.com/product-detail/ESD-and-Surge-Protection-TVS-ESD_LRC-LESD5Z3-3T1G_C136161.html) |
| 14 | Chinocera 22 µF `HGC0805R5226M160NSLJ` | [LCSC C33546008](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_Chinocera-HGC0805R5226M160NSLJ_C33546008.html) |
| 15 | Chinocera 470 nF `HGC0603R7474K500NTHJ` | [LCSC C33546006](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_Chinocera-HGC0603R7474K500NTHJ_C33546006.html) |
| 16 | Opcionales (una hoja por familia): `CL21A106KAYNNNE` [C15850](https://lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_Samsung-Electro-Mechanics-CL21A106KAYNNNE_C15850.html), `CL10A105KB8NNNC` [C15849](https://www.lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_Samsung-Electro-Mechanics-CL10A105KB8NNNC_C15849.html), `TCC0603X5R106K100CT` [C380314](https://www.lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_CCTC-TCC0603X5R106K100CT_C380314.html), `CC0603JRNPO9BN330` [C107047](https://www.lcsc.com/product-detail/Multilayer-Ceramic-Capacitors-MLCC-SMD-SMT_YAGEO-CC0603JRNPO9BN330_C107047.html), resistencia `FRC0603J103TS` [C2930027](https://www.lcsc.com/product-detail/Chip-Resistor-Surface-Mount_FOJAN-FRC0603J103TS_C2930027.html) | Solo para tener la ficha completa |

## Bloque 3. Revisión con lo descargado (yo)

| # | Qué hago | Resultado |
|---|----------|-----------|
| 17 | Leo la fuga del PSBD1DF40V1H a 4.2 V y 65 °C | Decido si se compran o no los 2 Schottky (O20) |
| 18 | Comparo el DS8570F8 con el TLV75733 (corriente en reposo contra los 124 µA del presupuesto de sueño) | Regulador definitivo |
| 19 | Comparo la huella `SOIC_8_3.9mm_EP` del docente con el dibujo del ESOP-8 del TP4056 | Huella del TP4056 sin crear una nueva |
| 20 | Verifico la huella del U.FL contra el dibujo de Hirose | U.FL cerrado |
| 21 | Verifico contra sus datasheets las piezas del docente que usamos | Informe corto de qué cumple y qué no |

## Bloque 4. Elegir (yo busco candidatos con datasheet y precio; tú eliges)

| # | Qué elegir | Criterio |
|---|------------|----------|
| 22 | **Panel solar** | 6 V, de 1 W a 1.5 W; tensión de máxima potencia cerca de 6 V y de circuito abierto de unos 8 V; que quepa en el collar y aguante IP67; cable con JST PH de 2 pines. Con él fijo R3, R4 y RCS |
| 23 | **Parche GNSS pasivo** | 18 x 18 x 4 mm, 1575.42 MHz, pasivo, con cable corto y u.FL, con datasheet de un fabricante conocido |
| 24 | **Zumbador** | Hoy el elegido da unos 52 dB a 1 m y `IP67_SPEC.md` pide más de 80 dBA. Cambiar el zumbador o el requisito |
| 25 | **Batería provisional** | Local, 1S de 4.2 V, **con protección integrada**, de 500 mAh o más, JST PH de 2 pines (o adaptable). Mide la polaridad al recibirla |
| 26 | **Bulk de 3.3 V** y **diodo ESD de la alimentación** | Tantalio (el código `TAJA107K016RNJ` parece no existir) o el electrolítico del docente; y el `LESD5Z3.3T1G` del docente |

## Bloque 5. Compras (tú), con los links que yo te dejo

| # | Dónde | Qué comprar | Antes de pagar |
|---|-------|-------------|----------------|
| 27 | **AliExpress** (ya elegidos; los enlaces están en `COMPRAS.md`) | Módulo LoRa LR30 900M22S (x2), antenas LoRa (lote de 10), MAX17048G+T10 (5 piezas), chip CN3791, kit de resistencias 0603 (660 piezas) y lote de 100 nF X7R | Comprobar la variante: en el CN3791 debe decir **CN3791** (no CN3762), en el LoRa **LR30 900M22S** (no la "SP") |
| 28 | **Digi-Key** | ESP32-S3-WROOM-1U-N16R8, MAX-M10S-00B, TLV75733PDBVR, ECS-.327-12.5-34B-TR, U.FL-R-SMT-1(80) en cinta cortada (4 piezas) | Comparar el envío con Mouser y agrupar para pagar un solo envío |
| 29 | **Mouser** | VO617A-1X017T y la antena Würth 2015301200 | Idem |
| 30 | Tras el bloque 4 | Panel solar, parche GNSS, zumbador y batería | Que cumplan lo elegido |
| 31 | Yo | Anoto todas las URL en `COMPRAS.md` | — |

## Bloque 6. Librerías de Altium (tú en Altium; yo verifico los archivos guardados)

| # | Pieza | Qué hacer |
|---|-------|-----------|
| 32 | Cristal ECS-.327-12.5-34B-TR | Copiar símbolo y huella a `shared_lib` y guardar (ya está verificado) |
| 33 | TP4056 | Usar la huella que resulte del paso 19 |
| 34 | CN3791 (SSOP-10) | No hay archivo de Altium. Probar el generador IPC de Altium con las cotas del datasheet y dibujar el símbolo con los pines de `pinout.md` |
| 35 | Módulo LoRa DX-LR30-900M22S | Dibujar la huella con el folleto del vendedor (pines y medidas, p.5 y p.11). Yo te preparo la tabla |
| 36 | Zumbador elegido | Bajar o crear su librería después del paso 24 |
| 37 | Final | Compilar `DR-VF-01.IntLib` (`Project » Compile Integrated Library`) y avisarme para verificarlo |

## Bloque 7. Limpieza de documentos (yo)

| # | Qué corrijo |
|---|-------------|
| 38 | Las contradicciones entre `BOM.md` y `COMPRAS.md` (sección 4 de `AUDITORIA_STOCK_DOCENTE.md`), y la línea de `ARCHITECTURE.md` que aún dice "Protección de batería: POR DECIDIR" |
| 39 | `datasheets/README.md`: una fila por cada datasheet nuevo |
| 40 | Tú: borrar el datasheet viejo del cristal con `! rm "/home/xavi/Proyecto DR-VF-01/hardware/datasheets/ecs_ecs-.327-12.5.pdf"` |

## Bloque 8. Antes del layout (otro día)

| # | Qué | Detalle |
|---|-----|---------|
| 41 | Elegir fabricante de PCB y apilado | La PCB es de 0.8 mm (por el USB-C). Con eso, anchos, vías y separaciones mínimas. Hoy no hay fabricante elegido |
| 42 | Reglas de diseño (`PCB_REGLAS.md`) | Yo extraigo de los datasheets y guías (Espressif, u-blox, CN3791): qué va lejos de qué, anchos, vías, línea de 50 Ω. Tú las cargas en Altium para que el DRC avise en vivo |
| 43 | Mecánica IP67 | Fijación y sellado del panel, paso del cable y tapas ciegas del USB-C |
| 44 | Esquemático y revisión final | Dibujarlo con `pinout.md`, imprimir las huellas a escala 1:1, y la revisión final antes de fabricar (las 6 auditorías cuando digas que todo es definitivo) |

## Resultados del bloque 3 (2026-11-06)

| # | Resultado | Evidencia |
|---|-----------|-----------|
| 17 | **Se usa el PSBD1DF40V1H del docente y no se compra el O20 (decidido el 2026-11-06).** A 65 °C no cumple, pero la noche en Colombia no pasa de 25 °C de ambiente (40 a 45 °C internos): unos 11 a 16 µA entre D1 y D2 contra unos 47 µA disponibles. Solo garantiza fuga a la tensión nominal de 40 V (0.3 mA a 25 °C, 10 mA a 125 °C), no a 4.2 V ni a 65 °C. Su curva típica (Fig. 2, p. 2), al 10 % de la tensión nominal, marca unos 1.7 µA a 25 °C y unos 80 µA a 75 °C: unos 40 µA por diodo a 65 °C (interpolación estimada), unos 80 µA entre D1 y D2. Con D1 montado el CN3791 baja a unos 9 µA, y a D1 y D2 les quedan unos 47 µA en total antes de pasar los 150 µA de RNF-03 (123.76 - 30 + 9 = 102.76 µA; los pendientes de `POWER_BUDGET.md` la reducen). Son valores típicos: se mide la corriente de sueño de la primera placa y, si pasa de 150 µA, se cambian los diodos | `psbd1df40v1h_datasheet.pdf` p. 1; `cn3791_datasheet.pdf` p. 9; `POWER_BUDGET.md` |
| 18 | **Se queda el TLV75733P. El DS8570F8 no sirve.** Iq de 0.6 mA típica (1.2 mA máx.) contra 25 µA del TLV: solo ese regulador pasaría de 150 µA. Además es ajustable, necesita VDD de 3 V como mínimo aparte de VIN y la batería llega a unos 3 V | `ds8570f8_datasheet.pdf` p. 5; `tlv757p_datasheet.pdf` p. 1 |
| 19 | **La huella `SOIC_8_3.9mm_EP` del docente sirve para el TP4056.** Paso 1.27 mm; centros de pads a ±2.725 con largo de 1.45 mm: llegan a ±3.45, es decir, 0.35 mm más allá de la punta máxima de la pata (E1 máx. 6.2, ±3.1); pad central de 2.71 x 3.4 mm contra 2.31 a 2.51 x 3.20 a 3.40 del pad térmico. Falta en Altium: cambiar la huella del TP4056 de `SOIC_8_3.9mm` a la `_EP`, y conectar el pad 9 a GND. La máscara y la pasta de esa huella ya están en "Rule Expansion" (verificado en el panel de propiedades de Altium 26.11 sobre el pad 8, que además muestra rotación de 90°): no hay que cambiarlas. La herramienta `altium_lib_check.py` las etiquetaba "none" y esa etiqueta era errónea para el valor 0. El fabricante dice "底部带有散热片需接地" (el pad de abajo se conecta a tierra), así que no es opcional | `tp4056_yongyutai_datasheet.pdf` p. 2 y 9; `altium_lib_check.py` sobre `shared_lib.PcbLib` |
| 20 | **La huella del U.FL coincide con el dibujo de Hirose.** Pads de masa de 1.05 x 2.2 mm con 4.0 mm entre bordes exteriores y 1.9 mm entre los interiores; pad de señal de 1.0 x 1.05 mm con su borde superior 1.0 mm bajo el eje. Diferencia: la pasta. Hirose pide aberturas de 0.9 x 2.0 mm (masa) y 0.8 mm de ancho (señal) con plantilla de 0.10 mm; con pasta en Rule y expansión 0 salen del tamaño del pad (28 % más de pasta en la masa). Recomendado: expansión de pasta de -0.075 mm en los tres pads | `ufl-r-smt-1_drawing.pdf`; `hardware/libraries/ufl/README.md` |
| 21 | **Las piezas del docente cumplen, con 2 datos que faltan (2026-11-07).** USB-C: 5 V 3 A, -25 a +85 °C y layout para PCB de 0.80 mm. USBLC6-2SC6 (Rev 5): VRM 5.25 V y fuga de 150 nA máx. AP2301: Vds -20 V, Vth -0.5 a -1.0 V, 57 mΩ típ. a -4.5 V. CY54-47UH: 710 mA y 355 mΩ contra unos 0.25 A del panel. LED rojo: VF de 1.8 a 2.2 V y 20 mA máx. TVS LESD5Z3.3T1G: VRWM 3.3 V, IR de 1 µA máx. a 25 °C (cuenta en el presupuesto de sueño y no hay valor en caliente). RGB: IF máx. de 25, 25 y 20 mA; ojo, su VF máx. de verde y azul es 3.4 V a 8 y 5 mA, por encima de 3V3, y el diseño depende de la curva típica a 2 mA. **Faltan:** (a) el electrolítico HRK es la hoja genérica de la serie (-40 a +105 °C) sin vida útil del 100 µF 35 V; (b) los Chinocera no traen curva de capacidad contra tensión continua (22 µF X5R). La Rev 7 del USBLC6 sigue pendiente | `type-c-3.1-16pin_specification.pdf`, `usblc6-2sc6_datasheet.pdf`, `ap2301_datasheet.pdf`, `cy54-47uh_datasheet.pdf`, `xl-2012surc_datasheet.pdf`, `lesd5z3.3t1g_datasheet.pdf`, `rs-3535mwam_specification.pdf`, `hrk-cd110_series_specification.pdf`, `chinocera-hgc_mlcc_specification.pdf` |

## Resultados del bloque 4 (2026-11-07)

Las piezas elegidas son para la prueba de la primera placa (la PCB llega a inicios de diciembre). Las definitivas se eligen después. Los enlaces, variantes y precios están en `COMPRAS.md`.

| # | Resultado |
|---|-----------|
| 22 | **Panel de prueba:** SunMoonMall, variante `6V 220mA 120x90Cable` (6 V, 220 mA, 1.32 W, con cable; O7). No declara la tensión de circuito abierto: medirla al sol al recibirlo (el CN3791 necesita más de unos 6.2 V). **Definitivo:** POR DECIDIR, candidato Voltaic P124 (6 V, 1.22 W, tensión de máxima potencia de 6.07 V, cubierta ETFE; falta su datasheet) |
| 23 | **Parche GNSS de prueba:** HXANT `HX-TWGPS-184-CV0`, 2 unidades (18 x 18 mm, cerámica de 4 mm, 6.5 mm de alto en total, pasivo, cable RF1.13 con IPEX1; O3). Sin datasheet propio: la hoja del vendedor es del modelo de 15 mm. Prueba: cobre de unos 50 x 50 mm debajo y C/N0 al aire libre. **Definitivo:** POR DECIDIR, candidato C&T RF `CTRF-ANTENNA-GPS-1575-181804-IPEX` |
| 24 | **Zumbador:** se mantiene el requisito de más de 80 dBA a 1 m si se consigue uno que lo cumpla. Falta buscarlo; el Murata PKMCS0909E4000-R1 queda como respaldo |
| 25 | **Batería de prueba:** LiPo 603450 de 1200 mAh con protección (PCM) y JST PH 2.0, de Nasal Store (O6). La protección la declara la imagen del vendedor, sin verificar. Medir la polaridad antes de enchufarla. **Definitiva:** POR DECIDIR (1200 o 1500 mAh) |
| 26 | Abierto: el código TAJA107K016RNJ parece no existir; la alternativa es el electrolítico de 100 µF 35 V del docente. El diodo ESD de la alimentación es el LESD5Z3.3T1G del docente (revisado en el paso 21) |

