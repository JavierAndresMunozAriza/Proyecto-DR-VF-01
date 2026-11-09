# MAX17048G+T10 (Analog Devices / Maxim) - símbolo, huella TDFN-8 y modelo 3D

**Estado: VERIFICADO; LA HUELLA DIFIERE EN MENOS DE 0.1 MM DEL PATRÓN DE MAXIM (2026-11-05).** Se recomienda ajustar los pads a las cotas de Maxim.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/MAX17048G_T10.IntLib` | SnapMagic (SnapEDA), página del MAX17048G+T10, descarga en formato Altium. El símbolo no lo creó SnapMagic. |
| `adi_docs/tdfn_90-0065.pdf` | Analog Devices, índice de paquetes: patrón de pads "8L TDFN 2x2 mm", doc. 90-0065 rev. E. |
| `adi_docs/tdfn_21-0168.pdf` | Analog Devices, índice de paquetes: dibujo "8L TDFN Exposed Pad, 2x2x0.75 mm", doc. 21-0168 rev. M. |

El `.IntLib` trae `MAX17048G+T10.SchLib` (símbolo), `SON50P200X200X80-9N.PcbLib` (huella con cuerpo 3D, nombre IPC-7351) y el modelo STEP incrustado. Los dos PDF los bajó el usuario con el navegador, porque `analog.com` rechaza las descargas desde este entorno.

## Cómo se verificó

Se leyeron los registros del símbolo y de la huella dentro del `.IntLib` con un script (sin abrir Altium) y se compararon con `datasheets/max17048_datasheet.pdf` (descripción de pines, p.6; paquete T822+3, patrón 90-0065, dibujo 21-0168, p.18) y con los dos PDF de `adi_docs/`.

### Símbolo

Los 9 pines coinciden en número y nombre con el datasheet: 1 CTG, 2 CELL, 3 VDD, 4 GND, 5 ALRT (en el símbolo, `ALERT` con barra encima, activo en bajo), 6 QSTRT, 7 SCL, 8 SDA, EP (pad expuesto, GND), que el símbolo numera como pin 9.

| Pin | Datasheet | Tipo en el símbolo |
|-----|-----------|--------------------|
| 1 CTG | Conectar a GND | Power |
| 2 CELL | Conectar al terminal positivo de la batería (en el MAX17048 no está conectado internamente) | Power |
| 3 VDD | Alimentación y sensado de la batería; filtro de 0.1 µF a GND | Power |
| 4 GND | Tierra, terminal negativo de la batería | Power |
| 5 ALRT | Salida de alerta, drenador abierto, activa en bajo | Output |
| 6 QSTRT | Entrada de arranque rápido; a GND si no se usa | Input |
| 7 SCL | Reloj I2C | Input |
| 8 SDA | Datos I2C, drenador abierto | I/O |
| EP (9) | Pad expuesto, a GND | Power |

El designador por defecto es `U?` y el símbolo enlaza la huella `SON50P200X200X80-9N`.

### Paquete (dibujo 21-0168 rev. M, código T822-3 / T822+3)

El T822+3 sí lleva pad expuesto: la tabla de variaciones del dibujo da D2 = 0.80 ± 0.10 mm y E2 = 1.20 ± 0.10 mm, paso de 0.50 mm y ancho de terminal b = 0.25 ± 0.05 mm. Cuerpo de 2.0 x 2.0 mm (1.9 a 2.1) y altura de 0.70 a 0.80 mm. El índice de paquetes de Analog Devices no lo muestra como "Exposed Pad", pero el dibujo manda.

### Huella frente al patrón 90-0065 rev. E (cotas en mm, tolerancia ±0.02)

| Cota | Patrón de Maxim | Huella descargada | Diferencia |
|------|-----------------|-------------------|------------|
| Paso | 0.50 | 0.50 | 0 |
| Pad de señal, largo (x) | 0.70 | 0.76 | +0.06 |
| Pad de señal, ancho (y) | 0.30 | 0.27 | -0.03 |
| Distancia entre los centros de las dos columnas | 1.98 | 1.97 (x = ±0.985) | -0.01 |
| Pad central, ancho (x) | 0.80 | 0.90 | +0.10 |
| Pad central, alto (y) | 1.38 | 1.30 | -0.08 |
| Orden | 1-4 a la izquierda de arriba abajo; 5-8 a la derecha de abajo arriba | Igual | OK |

El patrón de Maxim cumple IPC-7351A (nota 2 del dibujo). La huella descargada es la nominal de IPC-7351 y queda dentro de 0.1 mm, pero fuera de la tolerancia del dibujo.

**Ajuste recomendado:** pads de señal de 0.70 x 0.30 mm con centros en x = ±0.99 mm (las posiciones en y, ±0.25 y ±0.75 mm, ya coinciden) y pad central de 0.80 x 1.38 mm. En mil: 27.56 x 11.81, ±38.98 y 31.50 x 54.33.

Otros datos de la huella: máscara y pasta de los 8 pads en Rule; el pad central tiene la pasta anulada y una ventana de pasta propia de 0.56 x 0.82 mm (alrededor del 40 % del pad, que seguiría siendo un 40 % con el pad de 0.80 x 1.38 mm). No hay texto suelto en la serigrafía.

### Modelo 3D

El STEP mide 2.0 x 2.0 mm de cuerpo (nominal 2.0 del dibujo) y Altium muestra una altura de 0.8 mm, el máximo de la cota A (0.70 a 0.80 mm).

## Sin verificar

- **Nombre del pin 5.** El datasheet lo llama `ALRT` y el símbolo `ALERT`. Es cosmético.
