# ECS-.327-12.5-34B-TR (ECS Inc.) - símbolo, huella y modelo 3D

**Estado: VERIFICADO (2026-11-05).** Símbolo, huella y 3D coinciden con el datasheet del ECX-31B. Falta copiar símbolo y huella a la librería `DR-VF-01`.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/ECS-.327-12.5-34B-TR.IntLib` | SnapMagic (SnapEDA), página del ECS-.327-12.5-34B-TR; la página marca la pieza como "Created with ECS Inc.". |
| `ECS-.327-12.5-34B-TR_ECS_Inc.pdf` | Datasheet del ECX-31B (rev. 2016), botón "See Datasheet PDF" de esa página. Copia también en `hardware/datasheets/ecs_ecs-.327-12.5-34b-tr.pdf`. |

El `.IntLib` trae el símbolo `ECS-.327-12.5-34B-TR`, la huella `XTAL_ECS-.327-12.5-34B-TR` y su cuerpo 3D (STEP incrustado).

## Cómo se verificó

Se leyeron los registros del símbolo y de la huella dentro del `.IntLib` con `hardware/libraries/tools/altium_lib_check.py` (sin abrir Altium) y se compararon con el datasheet del ECX-31B, p.1.

### Símbolo

Dos pines, 1 y 2, de tipo Passive. El datasheet los define como "In/Out" y "Out/In": el cristal no tiene polaridad. El designador por defecto es `Y?` y el enlace de huella no tiene ruta de archivo.

### Huella (cotas en mm; datasheet, figura 2 "Suggested Land Pattern")

| Cota | Datasheet | Huella | Resultado |
|------|-----------|--------|-----------|
| Número de pads | 2 | 2 | OK |
| Tamaño del pad | 1.1 x 1.9 | 1.1 x 1.9 | OK |
| Separación entre los bordes de los pads | 1.4 | 1.4 (centros en x = ±1.25) | OK |
| Máscara y pasta | | Rule en los 2 pads | OK |
| Texto suelto en la serigrafía | | Ninguno | OK |

### Modelo 3D

El STEP mide 3.2 x 1.5 mm de planta y 0.9 mm de alto, igual que el cuerpo del datasheet (3.2 ± 0.1 x 1.5 ± 0.1 x 0.9 máx.).

## Datos del cristal (datasheet)

ESR de 70 kΩ máximo (igual al límite del ESP32-S3, sin margen); ±20 ppm a 25 °C; carga de 12.5 pF; coeficiente de temperatura de -0.034 ppm/°C² típico; potencia de excitación de 0.1 a 1.0 µW; capacitancia en paralelo de 1.05 pF; operación de -40 a +85 °C.
