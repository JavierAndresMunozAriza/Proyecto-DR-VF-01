# Hojas de datos

Los PDF **no se versionan** (`*.pdf` está en `.gitignore`): pesan varios MB y su redistribución depende de cada fabricante. Este índice dice cuál descargar, para qué sirve y qué dato del repo se verificó con ella. Guardar los PDF en esta carpeta con el nombre de la columna "Archivo".

## Descargadas y usadas

| Archivo | Documento | URL | Qué se verificó |
|---------|-----------|-----|-----------------|
| `esp32-s3-wroom-1_datasheet.pdf` | ESP32-S3-WROOM-1 & WROOM-1U Datasheet v1.8 | https://www.espressif.com/sites/default/files/documentation/esp32-s3-wroom-1_wroom-1u_datasheet_en.pdf | Tabla 3-1 (pines del módulo), Tabla 4-1 (strapping), variantes N16R8 |
| `esp32-s3_datasheet.pdf` | ESP32-S3 Series Datasheet | https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf | Tabla 5-11 (flash: 100 000 ciclos, retención 20 años) |
| `esp32-s3_hardware_design_guidelines.pdf` | ESP32-S3 Hardware Design Guidelines | https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/esp-hardware-design-guidelines-en-master-esp32s3.pdf | Guía de esquemático y layout del módulo (leer antes de Altium) |
| `max-m10s_datasheet.pdf` | u-blox MAX-M10S Data Sheet | https://content.u-blox.com/sites/default/files/MAX-M10S_DataSheet_UBX-20035208.pdf | Pines TXD, RXD, EXTINT, VCC, V_BCKP, RESET_N |
| `tps7a05_datasheet.pdf` | TI TPS7A05 | https://www.ti.com/lit/ds/symlink/tps7a05.pdf | Corriente máxima 200 mA |

## Pendientes de descargar a mano

La descarga automática falló o no se intentó. Se necesitan para cerrar los puntos POR DECIDIR de `BOM.md`.

| Documento | Para qué | Fabricante |
|-----------|----------|------------|
| SX1261/2 Datasheet | Sección RF, cristal/TCXO, pines, diseño de referencia | Semtech |
| LSM6DSOX Datasheet | Pines físicos, dirección I2C, configuración de INT1 | STMicroelectronics |
| CN3791 Datasheet | Componentes externos, pines CHRG/DONE/TEMP, divisor MPPT | Consonance |
| BQ2975 Datasheet | MOSFET externos y resistencia de sensado | Texas Instruments |
| VO617A Datasheet | V_F, I_F y CTR para dimensionar la resistencia | Vishay |
| MAX17048 Datasheet | Encapsulado, dirección I2C, pin ALRT | Analog Devices |
| MAX-M10S Integration Manual | Antena, alimentación, plano de masa | u-blox |

## Cómo usarlas con el asistente

Pedir "verifica X contra `hardware/datasheets/<archivo>`" para que la respuesta cite el dato de la hoja de datos y no la memoria del modelo.
