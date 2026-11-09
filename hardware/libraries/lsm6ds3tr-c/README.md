# LSM6DS3TR-C (ST) - símbolo, huella LGA-14 y modelo 3D

**Estado: VERIFICADO EN LO ESENCIAL (2026-11-05).** Pendiente de copiar la huella a la librería propia y de confirmar el modelo 3D.

Convención de cada pieza en `hardware/libraries/`: una carpeta por pieza con el símbolo, la huella y el modelo 3D juntos, y este `README.md` con la fuente y el estado de verificación.

## Fuente

Paquete de ST en Ultra Librarian, formato Altium Designer (`altium_nativo/ul_LSM6DS3TR-C.zip`, extraído en `altium_nativo/extraido/Altium/`): `LSM6DS3TR-C.LibPkg`, `.SchLib`, `.PcbLib` y `QFN_S3TR-C_STM.step`. La `.PcbLib` trae tres variantes de la huella (`QFN_S3TR-C_STM`, `-L`, `-M`), cada una con 14 pads. La elegida es `QFN_S3TR-C_STM`, y el símbolo ya apunta a ella.

El `readme.txt` de Ultra Librarian dice que las dimensiones usan el máximo de las tolerancias del fabricante, y que las nominales se redondean al mil más cercano.

KiCad se probó y se descartó (2026-11-05): el importador de Altium no lee la sintaxis de KiCad 10 y la huella quedó vacía.

## Verificado contra `datasheets/lsm6ds3tr-c_datasheet.pdf` (DocID030071 Rev 3)

Los registros se leyeron de la `.PcbLib` y la `.SchLib` con un script, sin abrir Altium.

### Huella `QFN_S3TR-C_STM`

| Punto | Datasheet | Huella | Resultado |
|-------|-----------|--------|-----------|
| Número de pads | 14 | 14 | OK |
| Paso | 0.5 mm | 0.5001 mm (19.69 mil) | OK |
| Distancia entre los centros de los pads 1 y 4 | 1.5 mm | 1.5004 mm | OK |
| Distancia entre los centros de los pads 5 y 7 | 1.0 mm | 1.0002 mm | OK |
| Cuerpo | 2.5 x 3.0 mm | Pads con su borde exterior a ±1.4986 mm (x) y ±1.2446 mm (y) | OK |
| Orden | Pin 1 en la esquina marcada; antihorario | Pad 1 arriba a la izquierda (x -1.2192, y +0.7502); 1-4 bajan por la izquierda, 5-7 van por abajo, 8-11 suben por la derecha, 12-14 van por arriba | OK |
| Tamaño del pad de PCB | Solo el del encapsulado: 0.25 x 0.475 mm (±0.05) | 0.3556 x 0.5588 mm (14 x 22 mil) | Coherente con la regla de TN0018 (+0.1 mm: 0.35 x 0.575), a unos 0.02 mm. La regla viene de un resumen de búsqueda, no del original. |

Los pads de los lados cortos (5-7 y 12-14) tienen el mismo tamaño, con el lado largo en vertical.

### Símbolo `LSM6DS3TR-C` (tabla 2, p.20)

Los 14 pines coinciden en número y nombre: 1 SDO/SA0, 2 SDx, 3 SCx, 4 INT1, 5 VDDIO, 6 GND, 7 GND, 8 VDD, 9 INT2, 10 NC, 11 NC, 12 CS, 13 SCL, 14 SDA. El símbolo enlaza la huella `QFN_S3TR-C_STM`.

Tipos eléctricos en el símbolo de ST: VDDIO, GND y VDD son de alimentación; INT1 e INT2 son Input; el resto son Passive. Según el datasheet, INT1 e INT2 son salidas del sensor. Es un detalle menor: con el ESP32 (E/S) en ese net no da error, pero conviene pasarlos a Output.

## Pendiente

- Copiar la huella `QFN_S3TR-C_STM` a `shared_lib.PcbLib` en `DR-VF-01.LibPkg`, de modo que el símbolo la encuentre al compilar el `.IntLib`.
- Dejar solo la variante elegida de la huella en la librería propia, y reducir el texto `.Designator`, que sale muy grande.
- Modelo 3D (`QFN_S3TR-C_STM.step`): comprobar en Altium su tamaño (2.5 x 3.0 mm) y su posición sobre los pads.
- Apertura de máscara de 0.05 mm, que pide el docente. Con paso de 0.5 mm y pads de 0.36 mm deja una banda de máscara de unos 0.05 mm entre aperturas, que muchos fabricantes no logran.
- `st.com` no responde desde este entorno (timeout con `curl` y WebFetch).
