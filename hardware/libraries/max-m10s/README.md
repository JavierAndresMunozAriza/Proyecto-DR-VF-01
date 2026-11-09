# MAX-M10S-00B (u-blox) - símbolo, huella y modelo 3D

**Estado: VERIFICADO EN LO ESENCIAL (2026-11-05).** Símbolo y huella coinciden con el datasheet y con el manual de integración. Quedan detalles menores.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/MAX-M10S-00B.IntLib` | SnapMagic (SnapEDA), página del MAX-M10S-00B, descarga en formato Altium. |

El `.IntLib` trae `MAX-M10S-00B.SchLib` (símbolo), `XCVR_MAX-M10S-00B.PcbLib` (huella con cuerpo 3D) y el modelo STEP incrustado. Las fuentes ya se extrajeron en Altium (`MAX-M10S-00B.LibPkg`).

## Cómo se verificó

Se leyeron los registros del símbolo y de la huella dentro del `.IntLib` con un script (sin abrir Altium) y se compararon con `datasheets/max-m10s_datasheet.pdf` (UBX-20035208 R08) y `datasheets/max-m10s_integration_manual.pdf` (UBX-20053088 R05).

### Símbolo (tabla 10 del datasheet, p.9)

Los 18 pines coinciden en número y nombre: 1 GND, 2 TXD, 3 RXD, 4 TIMEPULSE, 5 EXTINT, 6 V_BCKP, 7 V_IO, 8 VCC, 9 RESET_N, 10 GND, 11 RF_IN, 12 GND, 13 LNA_EN, 14 VCC_RF, 15 VIO_SEL, 16 SDA, 17 SCL, 18 SAFEBOOT_N. Las direcciones de TXD, RXD, TIMEPULSE, EXTINT, RF_IN, LNA_EN, VIO_SEL, SDA y SCL también coinciden.

### Huella (cotas en mm; datasheet fig. 4 y tabla 21; manual fig. 30 y tabla 44)

| Cota | Fuente | Huella | Resultado |
|------|--------|--------|-----------|
| Número de pads | 18 | 18 | OK |
| Paso (E, p.17) | 1.1 | 1.1 | OK |
| Ancho de los pads normales (K, tabla 44) | 0.8 | 0.8 | OK |
| Ancho de los pads de las esquinas (L, tabla 44) | 0.7 | 0.7 (pads 1, 9, 10 y 18) | OK |
| Largo del pad (M + N, tabla 44) | 1.0 dentro del módulo + 0.8 fuera = 1.8 | 1.8, con centros en x = ±4.75 y módulo de 9.7 mm (bordes en ±4.85) | OK |
| Margen de la esquina al borde del módulo (E, tabla 44) | 0.3 | 0.3 (pad de la esquina llega a y = ±4.75; el módulo mide 10.1) | OK |
| Separación entre el pad de la esquina y el siguiente (H, tabla 44) | 0.35 | 0.35 | OK |
| Orden | Pin 1 en la esquina marcada | Pad 1 arriba a la izquierda (x -4.75, y +4.4); 1-9 bajan por la izquierda y 10-18 suben por la derecha | OK |
| Máscara de soldadura | Mismo tamaño y posición que el cobre (manual, p.81) | Modo Rule en los 18 pads | Ver "Sin verificar" |
| Pasta (tabla 45) | Patrón en T: 0.9 dentro y 1.4 fuera del módulo (M y N), ancho de 0.8 fuera y 0.6 dentro en los pads normales (K y P), 0.7 fuera y 0.5 dentro en los de las esquinas (L y R), 7.9 entre los extremos interiores (S), 12.5 en total (T) | 18 regiones de 8 vértices en Top Paste con exactamente esas medidas, y expansión de pasta manual de -2 mm en cada pad para que solo pongan pasta las regiones | OK |

### Modelo 3D

El STEP mide 9.7 mm de ancho (B, tabla 21) y 10.4 mm de largo, que es 10.1 mm del cuerpo más 0.3 mm de la pestaña de separación (P). Altium muestra una altura de 106.3 mil (2.7 mm), igual al máximo de C (tabla 21).

## Sin verificar

- **Máscara de 0.05 mm del docente.** u-blox recomienda que la apertura de la máscara tenga el mismo tamaño que el cobre. Con 0.3 mm entre pads, 0.05 mm de expansión no causa problemas. Es una desviación pequeña respecto a la recomendación.
- **Tipo eléctrico de VCC_RF.** El símbolo lo marca como alimentación, y el datasheet dice que es una salida (voltaje para un LNA externo). Si queda sin conectar, el chequeo eléctrico de Altium puede avisar. Se puede cambiar a Output o Passive.
- **Nombres RESET_N y SAFEBOOT_N.** El símbolo muestra `RESET` y `SAFEBOOT` con barra encima (activo en bajo), sin el sufijo `_N`. Es cosmético, pero los nets del esquemático se llamarán `RESET_N` y `SAFEBOOT_N`.
- **Zona de exclusión.** El manual (fig. 30) indica un área de 11.1 x 10.1 mm. Se tiene en cuenta en el diseño de la placa.
