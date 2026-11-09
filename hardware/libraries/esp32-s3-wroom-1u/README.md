# ESP32-S3-WROOM-1U (Espressif) - símbolo, huella y modelo 3D

**Estado: VERIFICADO EN LO ESENCIAL (2026-11-05).** Símbolo y huella coinciden con el datasheet. El tamaño y la posición del modelo 3D siguen sin confirmar.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/ESP32-S3-WROOM-1U.IntLib` | SnapMagic (SnapEDA), página del ESP32-S3-WROOM-1U, descarga "Symbol and Footprint" en formato Altium. La página marca la pieza como "Created by SnapMagic". |

El `.IntLib` trae dentro `ESP32-S3-WROOM-1U.SchLib` (símbolo), `XCVR_ESP32-S3-WROOM-1U.PcbLib` (huella) y el modelo 3D (STEP) incrustado. Ya se extrajeron las fuentes en Altium (`ESP32-S3-WROOM-1U.LibPkg`).

Nota: el listado de SnapMagic es del módulo sin sufijo. La huella de la serie ESP32-S3-WROOM-1U es la misma para todas las variantes de memoria, y el N16R8 del BOM es una de ellas (figura 11-2 del datasheet, p.46).

## Cómo se verificó

Se leyeron los registros del símbolo y de la huella dentro del `.IntLib` con un script (sin abrir Altium) y se compararon con `hardware/esp32-s3-wroom-1u_pins.csv` (tabla 3-1 del datasheet) y con la figura 11-2 de `datasheets/esp32-s3-wroom-1_datasheet.pdf` (v1.8, p.46).

### Símbolo

| Comprobación | Resultado |
|--------------|-----------|
| Los 40 pines numerados 1 a 40 tienen el mismo nombre que el CSV | OK, sin diferencias |
| Tipo eléctrico (GND y 3V3 de alimentación, EN entrada, el resto E/S) | OK, igual que el CSV |
| Pin 41 (almohadilla térmica, GND) | El símbolo no tiene un pin 41, sino nueve pines `41_1` a `41_9`, uno por cada pad de la huella. Hay que conectar los nueve a GND. |

### Huella (cotas en mm, datasheet fig. 11-2)

| Cota | Datasheet | Huella | Resultado |
|------|-----------|--------|-----------|
| Pads de los bordes (40) | 1.5 x 0.9 | 1.5 x 0.9 | OK |
| Paso | 1.27 | 1.27 en los tres lados | OK |
| Distancia entre las columnas de pads | 17.5 | 17.5 (x = ±8.75) | OK |
| Entre los centros de los pads 1 y 14 | 16.51 | 16.51 | OK |
| Almohadilla térmica | 3 x 3 pads de 0.9, separados 0.5, extensión total 3.7 | 9 pads de 0.9 x 0.9 con paso de 1.4 (extensión 3.7) | OK |
| Vías de la almohadilla térmica | "Via for thermal pad" | 12 vías de 0.4 mm con agujero de 0.2 mm (`41_10` a `41_21`) | OK |
| 7.5, 10.29, 1.19 y 2.015 | Distancias al borde del módulo | Coinciden si el módulo mide 18 x 19.2 mm y sus bordes están en x = ±9 e y = ±9.6 | OK (supuesto mío sobre los bordes) |
| Numeración | 1-14 a la izquierda, 15-26 abajo, 27-40 a la derecha | Igual | OK |

## Sin verificar

- **Modelo 3D.** Los puntos del STEP dan un cajón de 24 x 5.6 x 26 mm, que no cuadra con un módulo de 18 x 19.2 mm. Probablemente incluye puntos que no son del cuerpo, así que ese método no sirve aquí. Revisarlo a ojo en la vista 3D de Altium (módulo sobre los pads, antena u.FL arriba a la derecha).
- **Apertura de la máscara y de la pasta** (0.05 mm que pide el docente) y la pasta de la almohadilla térmica.
- **Zona libre de cobre bajo la antena.** Con la variante 1U (conector u.FL) no aplica la zona de antena impresa. Se define en el diseño de la placa.
