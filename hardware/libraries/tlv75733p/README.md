# TLV75733PDBVR (Texas Instruments) - símbolo, huella SOT-23-5 y modelo 3D

**Estado: VERIFICADO Y CORREGIDO (2026-11-05).** La huella coincide con el datasheet. El símbolo se corrigió (pin 4 NC añadido y EN pasado a Input) y el archivo guardado lo confirma. Falta copiar símbolo y huella a la librería `DR-VF-01`.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/TLV75733PDBVR.IntLib` | SnapMagic (SnapEDA), página del TLV75733PDBVR, descarga en formato Altium. |

El `.IntLib` trae `TLV75733PDBVR.SchLib` (símbolo), `SOT95P280X145-5N.PcbLib` (huella con cuerpo 3D, nombre IPC-7351) y el modelo STEP incrustado. Las fuentes ya se extrajeron en Altium (`TLV75733PDBVR.LibPkg`).

## Cómo se verificó

Se leyeron los registros del símbolo y de la huella dentro del `.IntLib` con un script (sin abrir Altium) y se compararon con `datasheets/tlv757p_datasheet.pdf`: tabla 4-1 (p.3) y paquete DBV0005A (p.34 a 36).

### Símbolo

| Pin | Datasheet (DBV) | Símbolo | Resultado |
|-----|-----------------|---------|-----------|
| 1 | IN (entrada) | IN, Input | OK |
| 2 | GND | GND, Power | OK |
| 3 | EN (entrada) | EN, **I/O** | Tipo incorrecto: debe ser Input |
| 4 | NC (sin conexión interna) | **No existe** | Falta añadirlo |
| 5 | OUT (salida) | OUT, Output | OK |

El designador por defecto es `U?` y el símbolo enlaza la huella `SOT95P280X145-5N`. Esa huella se renombró a `TLV75733PDBVR` en la librería propia.

### Huella (cotas en mm; datasheet, DBV0005A "Example Board Layout", p.35)

| Cota | Datasheet | Huella | Resultado |
|------|-----------|--------|-----------|
| Número de pads | 5 | 5 | OK |
| Paso | 0.95 | 0.95 | OK |
| Orden | 1-3 a la izquierda de arriba abajo; 4 abajo a la derecha; 5 arriba a la derecha | Igual | OK |
| Tamaño del pad | 1.1 x 0.6 | 1.21 x 0.59 | Distinto, ver abajo |
| Distancia entre los centros de las columnas | 2.6 | 2.51 (x = ±1.255) | Distinto, ver abajo |
| Borde exterior de los pads | ±1.85 | ±1.86 | OK |
| Máscara y pasta | Máscara no definida por la soldadura (preferida), hasta 0.07 mm de margen | Rule en los 5 pads (máscara y pasta) | OK |

El dibujo del datasheet es un *ejemplo*, y TI avisa de que IPC-7351 puede dar diseños alternativos (nota 6). La huella descargada es la nominal de IPC-7351 (`SOT95P280X145-5N`): coincide por fuera con TI y llega 0.1 mm más hacia dentro. Se acepta.

### Modelo 3D

El STEP mide 2.8 mm entre las puntas de los pines, 2.906 mm de largo de cuerpo (2.9 mm típico) y 1.45 mm de alto, que es el máximo del paquete DBV0005A.

## Correcciones hechas en Altium (comprobadas en el archivo guardado, 12:00)

- Pin 4 `NC` añadido al símbolo, tipo Passive: ya coincide con los 5 pads de la huella.
- Pin 3 (EN) cambiado a Input.
- Pines guardados: 1 IN (Input), 2 GND (Power), 3 EN (Input), 4 NC (Passive), 5 OUT (Output).

## Pendiente

- Copiar el símbolo a `shared_lib.SchLib` y la huella `TLV75733PDBVR` a `shared_lib.PcbLib`.
- En el esquemático, el pin 4 se deja sin conectar (el datasheet dice "No internal connection").
