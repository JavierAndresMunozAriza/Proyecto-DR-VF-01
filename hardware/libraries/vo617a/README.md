# VO617A-1X017T (Vishay) - símbolo y huella SMD-4 (opción 7)

**Estado: SÍMBOLO VERIFICADO; HUELLA CON DIFERENCIAS FRENTE A LA RECOMENDADA POR VISHAY; SIN MODELO 3D (2026-11-05).**

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/VO617A-1X017T.IntLib` | SnapMagic (SnapEDA), página del VO617A-1X017T, descarga en formato Altium. |

El `.IntLib` trae `VO617A-1X017T.SchLib` (símbolo) y `SOIC254P1016X440-4N.PcbLib` (huella). No trae modelo 3D.

## Cómo se verificó

Se leyeron los registros del símbolo y de la huella dentro del `.IntLib` con un script (sin abrir Altium) y se compararon con `datasheets/vo617a_datasheet.pdf` (Vishay, documento 83430, rev. 2.8): configuración de pines (p.1), dibujos de paquete y huella recomendada (fig. 17, p.8).

### Símbolo

| Pin | Datasheet | Símbolo | Resultado |
|-----|-----------|---------|-----------|
| 1 | A (ánodo del LED) | A | OK |
| 2 | C (cátodo del LED) | C | OK |
| 3 | E (emisor del transistor) | E | OK |
| 4 | C (colector del transistor) | CO | OK (se llama CO para distinguirlo del cátodo) |

Los cuatro pines son de tipo Passive. El designador por defecto es `U?`.

**Enlace de huella roto en la fuente:** el símbolo apunta a `SOIC254P1016X440-4N` con la ruta `c:\altium-converter\conversion_dir\Temp\SOIC254P1016X440-4N.PcbLib`, que no existe en este equipo, y Altium muestra "no preview available". En la librería propia no debe quedar ninguna ruta: el enlace debe buscar la huella por nombre.

### Huella (cotas en mm; datasheet fig. 17, opción 7 = sufijo X017T)

| Cota | Vishay, huella recomendada (opción 7) | Huella descargada | Resultado |
|------|---------------------------------------|-------------------|-----------|
| Distancia entre las puntas de los pines | 10.16 ± 0.3 | 10.16 (nombre IPC `P1016`) | OK, es la variante correcta (la opción 8 mide 12.1) |
| Paso entre los pads de una columna | 2.54 | 2.54 (y = ±1.27) | OK |
| Tamaño del pad (x por y) | 1.52 x 1.78 | 1.7 x 1.45 | Distinto |
| Separación entre los bordes interiores de las columnas | 8 mín. | 7.7 (centros en x = ±4.7) | Menor que la recomendada |
| Ancho total | 11.05 | 11.1 | OK |
| Orden | 1 y 2 a la izquierda, 3 y 4 a la derecha, el 4 arriba | Pad 1 arriba a la izquierda, 2 abajo a la izquierda, 3 abajo a la derecha, 4 arriba a la derecha | OK |
| Máscara y pasta | | Rule en los 4 pads | OK |
| Texto suelto en la serigrafía | | Ninguno | OK |

El datasheet indica que la opción 7 tiene líneas de fuga y distancia de aire de 7 mm o más, y que 8 mm o más solo se consigue con las opciones 6 y 8 (p.1 y p.5). Con 7.7 mm entre los bordes de los pads se cumple 7 mm, pero queda por debajo de los 8 mm mínimos que recomienda Vishay para la huella.

**Recomendación:** ajustar los pads a la huella de Vishay: tamaño 1.52 x 1.78 mm, con los centros en x = ±4.76 mm e y = ±1.27 mm (con eso la separación interior es de 8.0 mm y el ancho total de 11.04 mm).

## Sin verificar / pendiente

- **Modelo 3D:** la huella no tiene. Es opcional; se puede mirar en la pestaña "3D Model" de la página de SnapMagic.
- **Resultado del ajuste de pads:** comprobarlo en el archivo guardado.
