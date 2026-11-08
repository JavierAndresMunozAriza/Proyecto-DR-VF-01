# DX-LR30-900M22S (DX-SMART) - datos para dibujar el símbolo y la huella

**Estado: SIN ARCHIVO DE ALTIUM.** No se encontró una librería descargable de este módulo (2026-11-07). El folleto del vendedor no trae las cotas de los pads: solo el tamaño, el paso y la numeración. Hay que **medir el módulo con calibre al recibirlo** (llega del 13 al 24 de nov) o pedirle el dibujo al vendedor (`en.szdx-smart.com`; el manual de la serie LR20 lista `Manager@szdx-smart.com` para soporte).

## Lo que sí dice el folleto (`SX1262 LoRa módulo desarrollador.pdf`, p.5 y p.11)

| Dato | Valor | Fuente |
|------|-------|--------|
| Tamaño del módulo SMD | 20 x 14 x 2.3 mm | p.5 |
| Paso de los pads | 1.27 mm | p.5 |
| Pines | 22 (el dibujo de la p.11 rotula el último como 23) | p.11 |
| Alimentación | 1.8 a 3.7 V (3.3 V típico) | p.5 y p.11 |
| Impedancia de RF | 50 Ω | p.5 |
| Interfaz | SPI | p.5 |
| Pad extra | cuadro "IPX" arriba a la derecha en el dibujo | p.11 |

Ojo: la tabla de la p.5 habla de 430 a 470 MHz, que es de otra variante de la serie. La que se compra es la `LR30 900M22S` (850 a 930 MHz).

## Pines (p.11)

Los pines van en dos filas, con pads en el borde (con muesca), mirando el módulo por arriba:

| Pin | Nombre | Función | Lado |
|-----|--------|---------|------|
| 1, 2, 3 | GND | Masa | izquierda, arriba |
| 4 | GND | Masa | izquierda |
| 5 | DIO3 | E/S; si el módulo trae TCXO, lo alimenta | izquierda |
| 6 | RX | Control del conmutador de antena | izquierda |
| 7 | TX | Control del conmutador de antena | izquierda |
| 8 | DIO2 | E/S; control de banda de RF | izquierda |
| 9 | VCC | Alimentación | izquierda |
| 10, 11 | GND | Masa | izquierda, abajo |
| 12 | GND | Masa | derecha, abajo |
| 13 | DIO1 | E/S | derecha |
| 14 | BUSY | Estado | derecha |
| 15 | RESET | Reset | derecha |
| 16 | MISO | Salida SPI | derecha |
| 17 | MOSI | Entrada SPI | derecha |
| 18 | SCK | Reloj SPI | derecha |
| 19 | NSS | Selección de chip | derecha |
| 20 | GND | Masa | derecha, arriba |
| 21 | ANT | Interfaz de RF | derecha, arriba |
| 22 | GND | Masa | derecha, arriba |

La tabla del folleto lista como GND los pines 1, 2, 3, 4, 10, 11, 12, 20 y 22.

## Por qué no doy las coordenadas de los pads

El dibujo no está acotado: el paso de 1.27 mm se lee, pero la separación entre el pin 3 y el pin 4 (hay un hueco) y la posición del pad "IPX" no se pueden fijar a partir de la imagen. Una estimación con la imagen daría un error que cuenta en un módulo con pads a 1.27 mm. Por eso la huella se dibuja con el calibre sobre el módulo real.

## Para dibujarla al recibir el módulo

1. Medir: largo y ancho (20 x 14 mm), el centro de cada pad al borde, el hueco entre los pines 3 y 4 y entre 11 y 12/20, y el tamaño de los pads de borde.
2. En Altium, usar el asistente de castellated/edge pads, con paso 1.27 mm y 22 pads.
3. Poner un recuadro de exclusión de cobre bajo la zona de la antena y del pad "IPX" del módulo, según el dibujo del vendedor.
4. Pegar la tabla de pines de `hardware/pinout.md` (sección LoRa) al símbolo.
