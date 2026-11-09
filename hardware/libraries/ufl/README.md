# U.FL-R-SMT-1 (Hirose) - símbolo, huella y modelo 3D

**Estado: EN CURSO (2026-11-05).** El archivo descargado no trae modelo 3D y la huella no se ha comparado con el dibujo de Hirose.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/U.FL-R-SMT-1_10_.IntLib` | SnapMagic (SnapEDA), página del `U.FL-R-SMT-1(10)`, descarga en formato Altium. |

El `.IntLib` trae el símbolo `U.FL-R-SMT-1(10)` y la huella `HRS_U.FL-R-SMT-1(10)`. Los sufijos `(01)`, `(10)` y `(80)` parecen códigos de empaque de Hirose (confirmado para `(01)`: bolsa de 100); la huella sería la misma para todos. Pendiente de confirmar con el dibujo de Hirose.

## Lo leído de los archivos (con `hardware/libraries/tools/altium_lib_check.py`)

### Símbolo

Tres pines, todos Passive: 1 GND, 2 SIG, 3 GND. Designador `J?`. **El enlace de huella lleva una ruta de la máquina que generó el paquete** (`C:\altium-converter\conversion_dir\Temp_37388\HRS_U.FL-R-SMT-1(10).PcbLib`): hay que dejarlo sin ruta (origen "Any") en Altium.

### Huella

| Pad | Posición (mm) | Tamaño (mm) |
|-----|---------------|-------------|
| 1 (GND) | x = -1.475, y = 0 | 1.05 x 2.2 |
| 2 (señal) | x = 0, y = -1.525 | 1.0 x 1.05 |
| 3 (GND) | x = +1.475, y = 0 | 1.05 x 2.2 |

Máscara y pasta en Rule; sin texto suelto en la serigrafía.

### Modelo 3D

El `.IntLib` descargado **no traía** modelo 3D. El usuario bajó el STEP desde la pestaña "3D Model" de SnapMagic (`altium_nativo/U.FL-R-SMT-1_10_.step`, 510 puntos, unidades en mm, altura de 1.25 mm) y lo añadió a la huella: en la librería propia (`shared_lib.PcbLib`, guardada a las 15:30) la huella ya tiene un cuerpo 3D.

El tamaño del STEP no se pudo comprobar con el script: el cajón de sus puntos sale de unos 7.9 x 8.85 mm de planta, demasiado para un conector de unos 3 mm, probablemente por puntos que no son del cuerpo. Hay que mirarlo en la vista 3D de Altium.

## Estado en la librería propia (`shared_lib`, guardada a las 15:30)

- Símbolo `U.FL-R-SMT-1(10)` con designador `J?`, pines 1 GND, 2 SIG, 3 GND, y **enlace de huella sin ruta** (corregido).
- Huella `U.FL-R-SMT-1(10)` con sus 3 pads sin cambios, máscara y pasta en Rule y un cuerpo 3D.

## Pendiente

- Bajar de `hirose.com` el dibujo del U.FL-R-SMT-1 (el repo solo tiene un escaneo sin texto) y comparar el tamaño y la posición de los pads con su huella recomendada.
- Mirar en la vista 3D de Altium (tecla `3` en la huella) que el conector quede centrado sobre los 3 pads y con un tamaño de unos 3 mm.
- Decidir el nombre del componente en la librería propia. Sugerencia: `U.FL-R-SMT-1` (sin sufijo de empaque) y el MPN comprable como parámetro.
