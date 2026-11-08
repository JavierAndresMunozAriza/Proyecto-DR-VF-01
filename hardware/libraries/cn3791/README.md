# CN3791 (Consonance) - datos para dibujar el símbolo y la huella

**Estado: HAY UNA LIBRERÍA EN SNAPMAGIC, SIN VERIFICAR.** Corrección del 2026-11-07: antes escribí que no existía; sí hay una en https://www.snapeda.com/parts/CN3791/Consonance/view-part/ (símbolo y huella; no la creó SnapMagic, la sube un usuario, y hay que mapear símbolo y huella tras importarla). LCSC `C154992` solo la trae para EasyEDA. Antes de usarla hay que comprobar con `altium_lib_check.py` que la huella tenga **paso 1.00 mm** y cuerpo de 3.9 x 4.9 mm (no un SSOP de 0.65 mm) y que la numeración de pines sea la de abajo. Si no coincide, se dibuja con los datos de este archivo.

## Aviso sobre el paquete

La hoja lo llama "10 Pin SSOP", pero el dibujo de la p.11 da un paso de **1.000 mm (BSC)**, con cuerpo de 3.9 x 4.9 mm. Eso es un SOP-10 de 150 mil, no un SSOP de paso 0.65 mm. En el generador IPC de Altium no uses SSOP: usa **SOP/SOIC con paso 1.00 mm**.

## Cotas del paquete (hoja `datasheets/cn3791_datasheet.pdf`, p.11, en mm)

| Cota | Mín. | Máx. | Qué es |
|------|------|------|--------|
| A | 1.350 | 1.750 | Alto total |
| A1 | 0.100 | 0.250 | Separación al PCB |
| A2 | 1.350 | 1.550 | Alto del cuerpo |
| b | 0.300 | 0.450 | Ancho de la pata |
| c | 0.170 | 0.250 | Grosor de la pata |
| D | 4.700 | 5.100 | Largo del cuerpo |
| E | 3.800 | 4.000 | Ancho del cuerpo |
| E1 | 5.800 | 6.200 | Ancho entre puntas de las patas |
| e | 1.000 (BSC) | | Paso |
| L | 0.400 | 1.270 | Largo del pie |
| θ | 0° | 8° | Ángulo |

Pasos en Altium: `Tools > IPC Compliant Footprint Wizard`, tipo SOP/SOIC, 10 pines, paso 1.00 mm, y estas cotas (en el wizard van el mínimo y el máximo de E1, D, E, L y b). Nombre sugerido: `SOP-10_3.9x4.9mm_P1.00mm`.

## Pines (hoja p.1 y p.2)

El pin 1 va arriba a la izquierda: pines 1 a 5 bajan por la izquierda y 6 a 10 suben por la derecha.

| Pin | Nombre | Tipo | Función |
|-----|--------|------|---------|
| 1 | VG | Salida | Regulador interno; 100 nF entre VG y VCC |
| 2 | GND | Alimentación | Masa |
| 3 | CHRG | Salida | Drenador abierto, a bajo mientras carga |
| 4 | DONE | Salida | Drenador abierto, a bajo al terminar la carga |
| 5 | COM | Pasivo | Compensación: 220 nF en serie con 120 Ω a GND (ver `pinout.md`) |
| 6 | MPPT | Entrada | Divisor para el punto de máxima potencia (1.205 V) |
| 7 | BAT | Entrada | Lado de la batería de RCS (conexión Kelvin) |
| 8 | CSP | Entrada | Lado del inductor de RCS (conexión Kelvin) |
| 9 | VCC | Alimentación | Entrada del panel |
| 10 | DRV | Salida | Compuerta del MOSFET de canal P (AP2301) |

Designador por defecto: `U?`. Valores máximos absolutos: VCC, VG, DRV, CHRG y DONE hasta 30 V; CSP, BAT, COM y MPPT hasta 6.5 V.

## Pendiente

- Dibujar el símbolo y la huella en `shared_lib` y avisar para verificarlos con `hardware/libraries/tools/altium_lib_check.py`.
- La hoja no trae el patrón de pads recomendado: las cotas de pads (largo, ancho) salen de las reglas del IPC. Verificar con la impresión a escala 1:1 antes de fabricar.
