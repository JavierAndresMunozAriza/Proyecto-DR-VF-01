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

## 4. Sensores y actuadores

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `lsm6dsox_datasheet.pdf` | ST LSM6DSOX Datasheet, DS12814 Rev 4 (junio 2024) | https://github.com/kodediy/kode_lsm6dsox/blob/main/LSM6DSOX_Datasheet_Rev4.pdf (copia en un repositorio de GitHub; st.com no respondió) | VDD 1.71 a 3.6 V (p. 12); dirección I2C 1101010b con SDO/SA0 a GND y 1101011b con SDO/SA0 a VDD (p. 18) |
| `lsm6ds3tr-c_datasheet.pdf` | ST LSM6DS3TR-C Datasheet, DocID030071 Rev 3 (mayo 2017) | https://cdn-shop.adafruit.com/product-files/4503/4503_LSM6DS3TR-C_datasheet.pdf (copia de Adafruit; st.com no respondió) | Vdd 1.71 a 3.6 V (p. 24). Es la IMU que el docente tiene físicamente; el BOM usa LSM6DSOX |
| `vo617a_datasheet.pdf` | Vishay VO617A, optoacoplador (Rev. 2.8, 22-ene-2025) | https://www.vishay.com/docs/83430/vo617a.pdf | Aislamiento 5300 VRMS (p. 1); CTR a 5 mA de 50 a 600 % sin bin y bins -1 a -9 (tabla de pedido, p. 2) |

## 5. Conectores y pasivos

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `jst_b2b-ph-k-s.pdf` | JST serie PH (incluye B2B-PH-K-S en p. 3 y B4B-PH-SM4-TB en p. 4) | https://www.jst-mfg.com/product/pdf/eng/ePH.pdf | Corriente nominal 2 A y tensión nominal 100 V (p. 1) |
| `ecs_ecs-.327-12.5.pdf` | ECS ECX-306X, cristal SMD de 32.768 kHz (guía de numeración ECS-.327-12.5-17X-TR) | https://ecsxtal.com/store/pdf/ECX-306X.pdf | Frecuencia 32.768 kHz y capacitancia de carga 12.5 pF (p. 1) |

## Descartadas (sin PDF local)

| Documento | URL | Motivo |
|-----------|-----|--------|
| TI TPS7A05 (TPS7A0533) | https://www.ti.com/lit/ds/symlink/tps7a05.pdf | Entrega 200 mA como máximo y las cargas suman 224 a 394 mA. Sustituido por el TLV75733P en el BOM v1.4 |
| "Manual de usuario" del vendedor del USBLC6-2SC6 | (guardado a mano, borrado el 2026-10-31) | Texto genérico de advertencias sin ningún parámetro: no es una hoja de datos |

## Pendientes de descargar a mano

Se necesitan para cerrar los puntos POR DECIDIR de `BOM.md`. Las descargas automáticas fallaron (403, tiempos de espera o visores con JavaScript) y no se intentó evadir esos controles: bajarlos desde el navegador.

| Documento | Para qué | Fabricante | Estado |
|-----------|----------|------------|--------|
| DX-LR30-900M22S (variante SMD "LR30", no la "SP") | Pines, alimentación y potencia del módulo LoRa | DX-SMART | No se encontró un datasheet oficial. Solo hay el folleto de la sección 2 y una página de manual en https://manuals.plus/ae/1005009103292012 (responde 403 a la descarga automática). Pedirlo al vendedor |
| USBLC6-2 (USBLC6-2SC6) | Pines y capacitancia de la protección ESD de D+/D- y VBUS | STMicroelectronics | st.com no respondió desde esta red. Probar https://www.st.com/resource/en/datasheet/usblc6-2.pdf desde el navegador. Las copias de LCSC devuelven HTML |
| BQ2975 | MOSFET externos y resistencia de sensado de la protección de batería | Texas Instruments | **El código `BQ2975DRBR` de `BOM.md` no existe en las fuentes consultadas.** `ti.com/lit/ds/symlink/bq2975.pdf` devuelve 404 y dos búsquedas no lo encontraron. El documento de la familia BQ2970 a BQ2973 (SLUSBU9I) no lo menciona. Hay que identificar la pieza correcta (protector de 1 celda con MOSFET externos) antes de descargar |
| PKMCS0909E4000-R1 (zumbador) | Tensión, frecuencia y consumo | Murata | El BOM decía `PKMCS0909E000-R1`, que no aparece en distribuidores; el código que sí aparece en Digi-Key y LCSC es `PKMCS0909E4000-R1` (9 x 9 mm, 1.5 V nominal, 4 kHz). Descargar su PDF desde murata.com en el navegador. Un listado de Digi-Key anota una fecha de última compra (31-mar-2027): comprobar disponibilidad |
| L-154A4SURK/G5T (LED) | Corriente y tensión directa | Kingbright | No se encontró ningún datasheet con ese código. Solo aparece L-154A4SURKQBDZGC, un LED RGB distinto |
| LTST-C193KGKT (LED) | Corriente y tensión directa | Lite-On | Los distribuidores lo listan como LTST-C193KGKT-5A; confirmar el sufijo. No se alcanzó un PDF directo de Lite-On |
| PTS645SL50SMTR92 (pulsador) | Fuerza, vida útil y huella | C&K (ahora Littelfuse) | https://www.ckswitches.com/media/1471/pts645.pdf responde 403 a la descarga automática |
| 2015301200 (antena) | Ganancia, adaptación y huella | Würth Elektronik | https://www.we-online.com/components/products/datasheet/2015301200.pdf responde 403 a la descarga automática |
| 6521011000 (conector SMA) | Huella y datos del conector | Würth Elektronik | https://www.we-online.com/components/products/datasheet/6521011000.pdf responde 403 a la descarga automática |
| U.FL-R-SMT-1 (conector U.FL) | Huella y altura acoplada | Hirose | Solo se alcanzó un escaneo sin texto extraíble (media.digikey.com, EDC3-302540-10.tif.pdf) y se descartó. Bajar el PDF desde hirose.com en el navegador |
| B4B-SH-SM4-TB (conector JST) | Huella del conector de 4 pines | JST | El código no aparece en el PDF de la serie SH ni en los listados consultados. Existe B4B-PH-SM4-TB(LF)(SN) de la serie PH (2.0 mm, en `jst_b2b-ph-k-s.pdf`, p. 4). Confirmar serie y código antes de comprar |
| GSB1C41110SSHR (USB-C) | Huella, corriente y pines del receptáculo | Amphenol | https://cdn.amphenol-cs.com/media/wysiwyg/files/drawing/gsb1c41x10sshr-ax1.pdf (URL de un listado de terceros, sin verificar) responde 403 a la descarga automática |

## Cómo usarlas con el asistente

Pedir "verifica X contra `hardware/datasheets/<archivo>`" para que la respuesta cite el dato de la hoja de datos y no la memoria del modelo.
