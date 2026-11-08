# Hojas de datos

Esta carpeta contiene **solo** hojas de datos (PDF) y este índice. Los PDF no se versionan (`*.pdf` está en `.gitignore`): pesan varios MB y su redistribución depende de cada fabricante. El índice dice cuál descargar, para qué sirve y qué dato del repo se verificó con ella.

## Reglas de la carpeta

1. Nombre del archivo: `<pieza>_datasheet.pdf` en minúsculas. Otros documentos del fabricante llevan su tipo: `_integration_manual.pdf`, `_vendor_brochure.pdf`.
2. Todo PDF de la carpeta tiene una fila en este índice, con su URL y un dato leído en el PDF (con página). Un PDF sin fila se borra.
3. Cuando la URL no es del fabricante, la columna lo dice (espejo o copia). Un espejo puede ser una revisión anterior a la vigente: antes de cerrar un diseño, comparar con la revisión actual del fabricante.
4. Si una pieza sale del BOM, se borra su PDF y pasa a "Descartadas" con el motivo.
5. Lo que no es una hoja de datos no va aquí: libros y referencias en `docs/references/`, documentos de producto en `docs/comercial_docs/`.

## 1. Procesador y módulo (PCB final)

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `esp32-s3-wroom-1_datasheet.pdf` | ESP32-S3-WROOM-1 & WROOM-1U Datasheet v1.8 | https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf | Tabla 3-1 (pines del módulo), Tabla 4-1 (strapping), variantes N16R8 |
| `esp32-s3_datasheet.pdf` | ESP32-S3 Series Datasheet | https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf | Tabla 5-11 (flash: 100 000 ciclos, retención 20 años); lista el controlador USB Serial/JTAG entre sus periféricos |
| `esp32-s3_hardware_design_guidelines.pdf` | ESP32-S3 Hardware Design Guidelines | https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/esp-hardware-design-guidelines-en-master-esp32s3.pdf | Guía de esquemático y layout del módulo (leer antes de Altium) |

## 2. RF y GNSS

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `lr30-900m22s_vendor_brochure.pdf` | Folleto del vendedor del módulo DX-LR30-900M22S (11 páginas, solo imágenes) | Guardado a mano desde el listado de AliExpress (ver `hardware/COMPRAS.md`, A1) | Es marketing, no el datasheet de Semtech. De él se tomó el pinout del módulo de `hardware/pinout.md` (sesión del 30-oct); al ser imágenes, no se puede buscar texto |
| `dx-lr30-900m22s_module_manual.pdf` | Manual técnico del módulo DX-LR30-900M22S, versión 2.0 del 2024-12-17 (23 páginas): parámetros, pines, circuito de referencia, cotas, huella recomendada (fig. 4) y soldadura | Dentro de `DX-LR30-900Mhz Development & User Information.rar` (paquete del fabricante, bajado por el usuario el 2026-11-07), carpeta `03 Module Technical Manual`. Mismo paquete: archivos de Altium de la huella y el símbolo, en `libraries/dx-lr30/altium_nativo/` |
| `sx1262_datasheet.pdf` | Semtech SX1261/2 Data Sheet Rev. 1.2 (junio 2019) | https://cdn.sparkfun.com/assets/6/b/5/1/4/SX1262_datasheet.pdf (copia en el CDN de SparkFun; el enlace oficial de semtech.com abre un visor con JavaScript y no se pudo descargar) | VDDop (VBAT y VBAT_IO) 1.8 a 3.7 V (condiciones de operación, p. 14) |
| `max-m10s_datasheet.pdf` | u-blox MAX-M10S Data Sheet | https://content.u-blox.com/sites/default/files/MAX-M10S_DataSheet_UBX-20035208.pdf | Pines TXD, RXD, EXTINT, VCC, V_BCKP, RESET_N; la referencia de pedido MAX-M10S-00B-01 es la versión de producción en masa de MAX-M10S-00B |
| `max-m10s_integration_manual.pdf` | u-blox MAX-M10S Integration Manual (UBX-20053088) | https://content.u-blox.com/sites/default/files/MAX-M10S_IntegrationManual_UBX-20053088.pdf | V_IO admite 1.8 V o 3.3 V; con 1.8 V el pin VIO_SEL va a GND (p. 73) |

## 3. Energía

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `tlv757p_datasheet.pdf` | TI TLV757P, regulador LDO de 1 A (SBVS322C) | https://www.ti.com/lit/ds/symlink/tlv757p.pdf | Entrada 1.45 a 5.5 V, salida de 1 A, Iq 25 µA típica (p. 1); caída máxima 425 mV a 1 A; RθJA del DBV de 100.8 °C/W en la EVM y 231.1 °C/W en JEDEC, y del DYD de 60.3 y 92.5 °C/W (tabla 5.4) |
| `cn3791_datasheet.pdf` | Consonance CN3791, cargador Li-ion de 4 A con MPPT (Rev 1.0) | https://radiokot.ru/forum/download/file.php?id=363931 (espejo en un foro; no es el sitio del fabricante) | VCC 4.5 a 28 V (tabla Electrical Characteristics, p. 4); VMPPT = 1.205 × (1 + R3/R4) y el ciclo de carga inicia solo con el pin MPPT por encima de 1.23 V; consumo desde la batería en sleep de unos 30 µA; sin pines de temperatura, habilitación ni power-good (pines VG, GND, CHRG, DONE, COM, MPPT, BAT, CSP, VCC, DRV) |
| `tp4056_datasheet.pdf` | NanJing Top Power TP4056, cargador lineal Li-ion de 1 A | https://dlnmh9ip6v2uc.cloudfront.net/datasheets/Prototyping/TP4056.pdf (copia en el CDN de SparkFun) | VCC 4.0 a 8.0 V (tabla Electrical Characteristics, p. 2); 500 mA con RPROG de 2.4 kΩ y 1000 mA con 1.2 kΩ; terminación a C/10; regulación térmica a 145 °C |
| `max17048_datasheet.pdf` | Analog Devices (Maxim) MAX17048/MAX17049, Rev 7 (11/16) | https://cdn.sparkfun.com/assets/b/b/2/c/b/MAX17048.pdf (copia en el CDN de SparkFun; analog.com no respondió) | VDD 2.5 a 4.5 V con condiciones eléctricas de -20 a +70 °C (tabla Electrical Characteristics, p. 2); 3 µA en hibernación y 23 µA activo (p. 1); TDFN de 8 pines; dirección I2C de 7 bits 0110110 (0x36) |
| `tp4056_yongyutai_datasheet.pdf` | YONGYUTAI TP4056, cargador lineal Li-ion de 1 A (Rev 2.0, en chino; es el fabricante del chip que tiene el docente) | Descargado con el navegador desde LCSC C20199357 (www.yongyutai.com en el pie) | Dibujo del encapsulado ESOP-8 con cotas en mm: cuerpo 3.8 a 4.0 de ancho (E), 4.7 a 5.1 de largo (D), paso 1.27, pad térmico 3.2 a 3.4 x 2.3 a 2.5 (D1 x E2) (p. 9); 400 mA típicos con RPROG de 2.4 kΩ (p. 3) |
| `ds8570f8_datasheet.pdf` | DSTECH DS8570, regulador LDO de 3 A, versión ESOP-8 `F8` (Preliminary) | Descargado con el navegador desde LCSC C5884245 | Iq con EN activo y sin carga de 0.6 mA típica y 1.2 mA máx., VDD de 3 a 5.5 V (p. 5); corriente en apagado de 1 µA máx. (p. 6). Salida ajustable y con alimentación VDD aparte: no es un reemplazo directo del TLV75733P, y 0.6 mA supera el presupuesto de sueño de 124 µA |
| `psbd1df40v1h_datasheet.pdf` | Prisemi PSBD1DF20V1H a PSBD1DF200V1H, Schottky de 1 A en SOD-123FL (una sola hoja para la serie) | Descargado con el navegador desde LCSC C110730 | Fuga inversa máxima con la tensión nominal: 0.3 mA a 25 °C y 10 mA a 125 °C en el primer grupo de columnas, que incluye al 40V1H (p. 1); no da fuga a 4.2 V. La curva típica (Fig. 2, p. 2) al 10 % de la tensión nominal marca unos 1.7 µA a 25 °C y unos 80 µA a 75 °C. VF de 0.55 V a 1 A en las columnas de 20 y 40 V |

## 4. Sensores y actuadores

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `lsm6dsox_datasheet.pdf` | ST LSM6DSOX Datasheet, DS12814 Rev 4 (junio 2024) | https://github.com/kodediy/kode_lsm6dsox/blob/main/LSM6DSOX_Datasheet_Rev4.pdf (copia en un repositorio de GitHub; st.com no respondió) | VDD 1.71 a 3.6 V (p. 12); dirección I2C 1101010b con SDO/SA0 a GND y 1101011b con SDO/SA0 a VDD (p. 18) |
| `lsm6ds3tr-c_datasheet.pdf` | ST LSM6DS3TR-C Datasheet, DocID030071 Rev 3 (mayo 2017) | https://cdn-shop.adafruit.com/product-files/4503/4503_LSM6DS3TR-C_datasheet.pdf (copia de Adafruit; st.com no respondió) | Vdd 1.71 a 3.6 V (p. 24). Es la IMU que el docente tiene físicamente; el BOM usa LSM6DSOX |
| `vo617a_datasheet.pdf` | Vishay VO617A, optoacoplador (Rev. 2.8, 22-ene-2025) | https://www.vishay.com/docs/83430/vo617a.pdf | Aislamiento 5300 VRMS (p. 1); CTR a 5 mA de 50 a 600 % sin bin y bins -1 a -9 (tabla de pedido, p. 2) |
| `pkmcs0909e4000-r1_specification.pdf` | Murata PKMCS0909E4000-R1, zumbador piezoeléctrico (JGB40-1594B, 11 páginas, 22-may-2020) | Descargado con el navegador desde murata.com | Presión sonora de 65 dB mín. con señal de ±1.5 Vo-p, 4 kHz, onda cuadrada (p. 1 y 2); entrada máxima ±12.5 Vo-p y temperatura de -40 a +85 °C (p. 1) |

## 5. Conectores y pasivos

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `jst_b2b-ph-k-s.pdf` | JST serie PH (incluye B2B-PH-K-S en p. 3 y B4B-PH-SM4-TB en p. 4) | https://www.jst-mfg.com/product/pdf/eng/ePH.pdf | Corriente nominal 2 A y tensión nominal 100 V (p. 1) |
| `ecs_ecs-.327-12.5-34b-tr.pdf` | ECS ECX-31B, cristal SMD de 32.768 kHz de 3.2 x 1.5 x 0.9 mm (ECS-.327-12.5-34B-TR) | Descargado con el navegador desde la página de SnapMagic del ECS-.327-12.5-34B-TR (botón "See Datasheet PDF"); sitio del fabricante: https://ecsxtal.com | ESR de 70 kΩ máx., ±20 ppm a 25 °C, CL 12.5 pF, coeficiente de temperatura -0.034 ppm/°C², pads 1 y 2 sin polaridad y huella sugerida (p. 1) |
| `bzcn_tsc015.pdf` | BZCN serie TSC015, interruptor táctil SMD de 4.5 x 4.5 mm (el TSC015A05026A del docente, LCSC C2888509) | Descargado con el navegador desde la página de LCSC de la pieza (botón del datasheet) | 50 mA a 12 V DC, resistencia de contacto ≤ 100 mΩ, vida ≥ 100 000 ciclos, código de altura (5.0 mm) y de fuerza (260 gf), diagrama de circuito y huella recomendada (p. 1) |
| `ufl-r-smt-1_drawing.pdf` | Hirose U.FL-R-SMT-1(01), dibujo 2D (EDC3-302540-01, código CL331-0472-2-01) | Descargado con el navegador desde la página del producto en hirose.com (Documents, Drawing 2D, English) | Huella recomendada con cotas de 4, 2.2, 1.9, 1.05 y 1 mm (±0.05) y zona de corte prohibida, máscara de pasta de 0.10 mm y perfil de reflujo de 260 °C pico, hasta 2 ciclos |
| `ufl-r-smt-1_specification.pdf` | Hirose U.FL-R-SMT-1(01), hoja de especificaciones (ELC4-302540-01) | Descargado con el navegador desde hirose.com (Documents, Specification Sheet, English) | 50 Ω de 0 a 8 GHz; VSWR ≤ 1.3 hasta 6 GHz y ≤ 1.4 de 6 a 8 GHz; -40 a +105 °C |
| `ufl_series_catalog.pdf` | Hirose serie U.FL, catálogo (34 páginas, sep-2024) | Descargado con el navegador desde hirose.com (Documents, Series Catalogs, English) | Altura acoplada de 1.9 a 2.4 mm; sirve de consulta de la serie y de los cables y conectores de acople |
| `type-c-3.1-16pin_specification.pdf` | Shou Han TYPE-C-3.1-16PIN, hoja de aprobación con dibujo (6 páginas) | Descargado con el navegador desde LCSC C7507405 | 5 V 3 A, -25 a +85 °C, asignación de los 16 pines y layout recomendado para PCB de **0.80 mm** (p. 1) |
| `usblc6-2sc6_datasheet.pdf` | ST USBLC6-2, DocID 11265 **Rev 5** (oct-2011) | Copia de LCSC C7519; la revisión vigente (Rev 7) sigue pendiente, ver el final | VRM 5.25 V y fuga de 10 nA típ. y 150 nA máx. (tabla 2, p. 2); capacitancia máx. de 3.5 pF (p. 1) |
| `ap2301_datasheet.pdf` | ALLPOWER AP2301 V1.1, MOSFET P de 20 V (solo imagen, sin texto) | Descargado con el navegador desde LCSC C385296 | Vds -20 V, Vgs ±12 V, Id -3 A (p. 1); Vth de -0.5 a -1.0 V y Rds(on) de 57 mΩ típ. y 75 mΩ máx. a -4.5 V (p. 2). El resumen de la p. 1 da 74 mΩ típ. a -4.5 V, pero la tabla de la p. 2 lo da a -2.5 V: vale la tabla |
| `cy54-47uh_datasheet.pdf` | Shou Han CY54, inductores de potencia (hoja de aprobación; el 47 µH es el CY54-470K) | Descargado con el navegador desde LCSC C2929434 | CY54-470K: 47 µH, DCR de 355 mΩ e IDC de 710 mA (p. 3); -55 a +125 °C |
| `rs-3535mwam_specification.pdf` | NationStar RS-3535MWAM, LED RGB 3535 de 6 pines (spec B2, 14 páginas) | Descargado con el navegador desde LCSC C842778 | IF máx. de 25, 25 y 20 mA (R, G, B), -30 a +85 °C y VF por color (p. 3) |
| `hrk-cd110_series_specification.pdf` | HRK serie CD110, electrolítico de aluminio con terminales (8 páginas). **Es la hoja genérica de la serie con una muestra de 33 µF 400 V, no la del 100 µF 35 V** | Descargado con el navegador desde LCSC C2960381 | Serie de 6.3 a 500 VDC y -40 a +105 °C (p. 1); no trae la vida útil ni la ESR del 100 µF 35 V de 6.3 x 12 mm |
| `xl-2012surc_datasheet.pdf` | XINGLIGHT XL-2012SURC, LED rojo 0805 (14 páginas) | Descargado con el navegador desde LCSC C965812 | VF de 1.8 a 2.2 V a 20 mA, IF máx. de 20 mA y -40 a +85 °C (p. 3) |
| `lesd5z3.3t1g_datasheet.pdf` | LRC LESD5Z2.5T1G, serie de TVS para ESD (incluye el LESD5Z3.3T1G; 6 páginas) | Descargado con el navegador desde LCSC C136161 | LESD5Z3.3T1G: VRWM 3.3 V, IR máx. de 1 µA, VBR mín. de 5.0 V y 105 pF típ. (p. 2); -40 a +125 °C (p. 1) |
| `chinocera-hgc_mlcc_specification.pdf` | Chinocera serie HGC, condensadores cerámicos multicapa, hoja de aprobación (22 páginas). Cubre el `HGC0805R5226M160NSLJ` (22 µF) y el `HGC0603R7474K500NTHJ` (470 nF) | Descargado con el navegador desde LCSC C33546008 (el C33546006 trae el mismo PDF) | R5 = X5R (-55 a +85 °C, ±15 %) y R7 = X7R (-55 a +125 °C, ±15 %) (p. 3). No trae curva de pérdida de capacidad con tensión continua |

## Descartadas (sin PDF local)

| Documento | URL | Motivo |
|-----------|-----|--------|
| TI TPS7A05 (TPS7A0533) | https://www.ti.com/lit/ds/symlink/tps7a05.pdf | Entrega 200 mA como máximo y las cargas suman 224 a 394 mA. Sustituido por el TLV75733P en el BOM v1.4 |
| "Manual de usuario" del vendedor del USBLC6-2SC6 | (guardado a mano, borrado el 2026-10-31) | Texto genérico de advertencias sin ningún parámetro: no es una hoja de datos |
| TI TMP102 (SBOS397I) | https://www.ti.com/lit/ds/symlink/tmp102.pdf | Sin uso desde el 2026-11-03 (no hay corte de carga por frío). Verificado: 1.4 a 3.6 V, 3.2 µA a 1 conversión/s, ±2 °C máx. de -25 a +85 °C |
| Schottky SS34-TD, TVS SMF5.0CA, ferrita HCB1608KF-600T20 | copias de LCSC (ver `COMPRAS.md` 1.2) | No aprobados: fuga inversa de hasta 0.5 mA a 25 °C (SS34-TD), 400 µA máx. a 5 V (SMF5.0CA) y la ferrita no figura en el catálogo de TAI-TECH. El PSBD1DF40V1H ya tiene PDF local (sección 3) y se usa en D1 y D2 (O20, decidido el 2026-11-06) |
| Kingbright L-154A4SURK/G5T y Lite-On LTST-C193KGKT | - | Reemplazados por el LED RGB del docente (RS-3535MWAM) en el BOM v1.7; el primero no tiene datasheet con ese código |
| Amphenol GSB1C41110SSHR (USB-C) | - | Reemplazado por el USB-C del docente (Shou Han TYPE-C-3.1-16PIN) en el BOM v1.6 |

## Consultadas el 2026-11-03 para las piezas del docente (sin PDF local)

Hojas leídas para decidir las piezas de `COMPRAS.md` sección 1. Casi todas son copias del fabricante alojadas en LCSC, no el PDF de su sitio. No se guardan en la carpeta (regla 2).

| Documento | URL | Qué se verificó |
|-----------|-----|-----------------|
| Samsung CL10A105KB8NNNC y CL21A106KAYNNNE; CCTC TCC0603X5R106K100CT | https://product.samsungsem.com/mlcc/CL21A106KAYNNN.do ; https://datasheet.lcsc.com/datasheet/pdf/71a9f8eccc4da164324d4a716182c5e3.pdf?productCode=C380314 | X5R, tensión nominal y rango de -55 a +85 °C |
| Walsin 0603F104Z500CT | https://datasheet.lcsc.com/datasheet/pdf/c647af9500c0421cb071e40d1cbbf836.pdf?productCode=C83060 | Y5V: -40 % a 45 °C y -77 % a 85 °C (p.1). No se usa |
| Resistencia de PROG del TP4056 | `tp4056_datasheet.pdf` (local) | I = 1200 V / RPROG (p.2); TEMP a GND desactiva el corte por temperatura (p.2). El PDF no trae dibujo de empaque ESOP-8; el del fabricante YONGYUTAI está en `tp4056_yongyutai_datasheet.pdf` (p. 9) |

## Pendientes de descargar a mano

Se necesitan para cerrar los puntos POR DECIDIR de `BOM.md`. Las descargas automáticas fallaron (403, tiempos de espera o visores con JavaScript) y no se intentó evadir esos controles: bajarlos desde el navegador.

| Documento | Para qué | Fabricante | Estado |
|-----------|----------|------------|--------|
| DX-LR30-900M22S (variante SMD "LR30", no la "SP") | Pines, alimentación y potencia del módulo LoRa | DX-SMART | No se encontró un datasheet oficial. Solo hay el folleto de la sección 2 y una página de manual en https://manuals.plus/ae/1005009103292012 (responde 403 a la descarga automática). Pedirlo al vendedor |
| USBLC6-2 (USBLC6-2SC6), revisión vigente | Confirmar que la Rev 7 no cambia el pinout ni la capacitancia que se usaron de la Rev 5 | STMicroelectronics | st.com no respondió desde esta red. Bajar https://www.st.com/resource/en/datasheet/usblc6-2.pdf desde el navegador. La decisión del 2026-11-03 usó la Rev 5 (copia de LCSC, sección "Consultadas")
| BQ2975 | MOSFET externos y resistencia de sensado de la protección de batería | Texas Instruments | **El código `BQ2975DRBR` de `BOM.md` no existe en las fuentes consultadas.** `ti.com/lit/ds/symlink/bq2975.pdf` devuelve 404 y dos búsquedas no lo encontraron. El documento de la familia BQ2970 a BQ2973 (SLUSBU9I) no lo menciona. Hay que identificar la pieza correcta (protector de 1 celda con MOSFET externos) antes de descargar |
| 2015301200 (antena) | Ganancia, adaptación y huella | Würth Elektronik | https://www.we-online.com/components/products/datasheet/2015301200.pdf responde 403 a la descarga automática |

## Cómo usarlas con el asistente

Pedir "verifica X contra `hardware/datasheets/<archivo>`" para que la respuesta cite el dato de la hoja de datos y no la memoria del modelo.
