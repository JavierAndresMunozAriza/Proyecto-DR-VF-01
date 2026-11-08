# DX-LR30-900M22S (DX-SMART) - símbolo y huella

**Estado: DATOS OFICIALES ENCONTRADOS (2026-11-07).** El paquete del fabricante (`DX-LR30-900Mhz Development & User Information.rar`, que el usuario bajó de su descarga) trae el **manual técnico del módulo V2.0** y archivos de Altium. Los datos de abajo salen del manual (`hardware/datasheets/dx-lr30-900m22s_module_manual.pdf`) y de la huella del fabricante. **Corrección del 2026-11-07:** la separación entre pads (1.27 mm y 5.57 mm) y x = ±7.0 mm coincidían con mi estimación, pero los extremos **no son simétricos** como yo suponía: según la fig. 4 el pad 1 está a 1.0 mm del borde superior y el pad 11 a 2.0 mm del borde inferior. Las coordenadas y de abajo ya están corregidas (+0.5 mm respecto de la versión anterior). **Huella verificada el 2026-11-07** (22 pads de 2.0 x 1.0 mm sin taladro, con las posiciones de la tabla); falta la zona sin cobre, copiarla a `shared_lib`, dibujar el símbolo y comprobarla contra el módulo real cuando llegue.

## Archivos del fabricante (`altium_nativo/`)

| Archivo | Qué es | Problemas |
|---------|--------|-----------|
| `DX-LR20&30_PCB_Footprint_AD.PcbDoc` | Huella de 22 pads dentro de una placa (no es una librería) | Pads con un **taladro chapado de 0.6 mm** y tamaño 1.8 x 0.8 mm, que no coinciden con el dibujo del manual (pads de 2.0 x 1.0 mm sin taladro) |
| `DX-LR20&30_SCH_Part_AD.SchDoc` | Símbolo de 22 pines "LR20/30" de 2 partes, con todos los pines de tipo Passive | Es del LR20 y el LR30 a la vez; el modelo de huella enlazado se llama `PE220-400M22S`, de otra serie; tipos eléctricos sin asignar |

Las posiciones relativas de los pads de la huella del fabricante son las de la tabla de abajo (verificado con `altium_lib_check.py`). Al convertirla en librería, la huella queda **girada 90°** (el pin 1 arriba a la derecha) y con **22 textos de nombres de pin en la capa Bottom Overlay**, que se ven al revés.

## Datos del manual V2.0

| Dato | Valor | Página |
|------|-------|--------|
| Tamaño | 20.0 x 14.0 x 2.3 mm (con la tapa metálica) | 7, 15 |
| Tensión de operación | 1.8 a 3.7 V, **típica 3.3 V; "≥ 3.3 V garantiza la potencia de salida"** | 7, 8 |
| Máximo absoluto de VBAT | **3.7 V** | 12 |
| Corriente | TX 100 mA típica, RX 10 mA, reposo 180 nA | 8 |
| Sensibilidad | −125 dBm (BW 125 kHz, SF 7) | 7 |
| Cristal | 32 MHz **sin compensación de temperatura**: con BW menor de 125 kHz puede haber demasiado desvío de frecuencia | 6 |
| Banda | 850 a 930 MHz | 6, 8 |
| Interfaz de RF | "Stamp hole / IPEX": el pad de borde ANT (pin 21) y un conector IPEX (U.FL) en la tapa del módulo | 7, 16 |
| Nivel de comunicación | 3.3 V; con 5 V hay riesgo de quemar el módulo | 8 |
| Temperatura | −40 a +85 °C | 8 |
| ESD de los pines | HBM 2 kV, CDM 1 kV | 12 |
| Contacto del fabricante | manager@szdx-smart.com, en.szdx-smart.com | 2 |

## Pines (manual p.9 y fig. 5)

| Pin | Nombre | Función |
|-----|--------|---------|
| 1, 2, 3, 4, 10, 11, 12, 20, 22 | GND | Masa |
| 5 | DIO3 | E/S; alimenta un TCXO externo si lo hay |
| 6 | RXEN | Conmutador de antena (recepción) |
| 7 | TXEN | Conmutador de antena (transmisión) |
| 8 | DIO2 | E/S; control de banda de RF. Puede ir unido a TXEN y no al MCU; si no se usa, queda al aire (p.14) |
| 9 | VCC | Alimentación, 3.3 V típica |
| 13 | DIO1 | E/S |
| 14 | BUSY | Estado |
| 15 | RESET (NRST en el manual) | Reset |
| 16 / 17 / 18 / 19 | MISO / MOSI / SCK / NSS | SPI |
| 21 | ANT | Interfaz de RF |

Los manuales nombran RXEN y TXEN a los pines 6 y 7; el folleto los llamaba RX y TX.

## Símbolo: lista para el Symbol Wizard (`Tools > Symbol Wizard`)

Separada por tabuladores: Número, Nombre, Tipo eléctrico y Lado. Se dejan los 9 pines GND visibles.

```
1	GND	Power	Left
2	GND	Power	Left
3	GND	Power	Left
4	GND	Power	Left
5	DIO3	I/O	Left
6	RXEN	Input	Left
7	TXEN	Input	Left
8	DIO2	I/O	Left
9	VCC	Power	Left
10	GND	Power	Left
11	GND	Power	Left
12	GND	Power	Right
13	DIO1	I/O	Right
14	BUSY	Output	Right
15	RESET	Input	Right
16	MISO	Output	Right
17	MOSI	Input	Right
18	SCK	Input	Right
19	NSS	Input	Right
20	GND	Power	Right
21	ANT	Passive	Right
22	GND	Power	Right
```

Nombre del componente: `DX-LR30-900M22S`. Designador por defecto: `U?`.

## Huella: coordenadas (manual fig. 3 y 4, vista superior de la placa)

Origen en el centro del módulo (14 mm en X y 20 mm en Y), pin 1 arriba a la izquierda y los pads **centrados en el borde** del módulo (la mitad queda fuera).

| Dato | Valor |
|------|-------|
| Pads | 22, de 2.0 mm (hacia afuera y adentro) x 1.0 mm (a lo largo del borde), sin taladro; son los pads del dibujo del manual, fig. 4 |
| Paso | 1.27 mm |
| Hueco entre los pines 3 y 4 y entre 19 y 20 | **5.57 mm de centro a centro** (confirmado en la fig. 4) |
| Extremos | el centro de los pines 1 y 22 está a **1.0 mm** del borde superior del módulo y el de los pines 11 y 12 a **2.0 mm** del borde inferior (fig. 4: 1.0 + 17.0 + 2.0 = 20) |
| Zona bajo el módulo sin cobre | rectángulo de 2.64 x 4.0 mm: x de −2.84 a −0.20 e y de −6.50 a −2.50 (vista superior). Es un componente de la cara inferior del módulo (la zona amarilla de la fig. 5) |

| Pin | x (mm) | y (mm) | | Pin | x (mm) | y (mm) |
|-----|--------|--------|-|-----|--------|--------|
| 1 | −7.0 | +9.00 | | 12 | +7.0 | −8.00 |
| 2 | −7.0 | +7.73 | | 13 | +7.0 | −6.73 |
| 3 | −7.0 | +6.46 | | 14 | +7.0 | −5.46 |
| 4 | −7.0 | +0.89 | | 15 | +7.0 | −4.19 |
| 5 | −7.0 | −0.38 | | 16 | +7.0 | −2.92 |
| 6 | −7.0 | −1.65 | | 17 | +7.0 | −1.65 |
| 7 | −7.0 | −2.92 | | 18 | +7.0 | −0.38 |
| 8 | −7.0 | −4.19 | | 19 | +7.0 | +0.89 |
| 9 | −7.0 | −5.46 | | 20 | +7.0 | +6.46 |
| 10 | −7.0 | −6.73 | | 21 | +7.0 | +7.73 |
| 11 | −7.0 | −8.00 | | 22 | +7.0 | +9.00 |

La zona sin cobre se calculó a partir de la vista inferior de la fig. 4 (7.2 mm desde el centro de la columna de pads, 3.5 mm desde el borde inferior), reflejada para la vista superior. **Es mi conversión**: comprobarla con el módulo en la mano.

## Cómo hacerla rápido y seguro (corregido tras la primera prueba)

1. Abrir `DX-LR20&30_PCB_Footprint_AD.PcbDoc` (copia en `C:\Users\Usuario\Downloads\DX-LR30_vendor_AD\`), seleccionar el componente y `Design > Make PCB Library`.
2. Nombre `DX-LR30-900M22S` (**sin punto final**), altura 2.3 mm y descripción "LoRa SX1262 module 20x14x2.3 mm, 22 castellated pads, 1.27 mm pitch".
3. **Borrar los 22 textos** de nombres de pin (capa Bottom Overlay): clic derecho en uno > `Find Similar Objects` > `Object Kind = String` > Apply > Delete. Son los que se ven al revés; los números de pin ya los muestran los pads.
4. **Pads en SMD y con la orientación correcta:** seleccionar los 22 pads (`Find Similar Objects`, `Object Kind = Pad`) y poner **Layer = Top Layer**, **Hole = 0**, rotación 0 y el tamaño según la orientación en que esté la huella en ese momento: si los pads van en filas horizontales (la huella recién creada), **X = 1.0 mm e Y = 2.0 mm**; si van en columnas verticales, X = 2.0 mm e Y = 1.0 mm. Comprobar que entre pads quede un hueco de 0.27 mm (si se ven como una barra continua, los pads están al revés y se solapan).
5. **Girar a la orientación estándar** (pin 1 arriba a la izquierda): `Ctrl+A`, `Edit > Move > Rotate Selection`, 90° en sentido antihorario. Si el pin 1 queda abajo a la derecha, deshacer y usar 270°.
6. `Edit > Set Reference > Center`. El origen debe quedar en el **centro del módulo (14 x 20 mm)**. Comprobar: pad 1 en (−7.0, +9.0), pad 11 en (−7.0, −8.0), pad 12 en (+7.0, −8.0) y pad 22 en (+7.0, +9.0).
7. Añadir la marca del pin 1, la marca del IPEX y la zona sin cobre (tabla de arriba), **después** de centrar para no mover el origen.
8. Copiar a `shared_lib.PcbLib` y enlazar al símbolo con el modelo `Any` (sin ruta de archivo). Avisar para verificar con `altium_lib_check.py`.

## Pregunta abierta: ¿por dónde sale la antena?

El manual indica "Stamp hole / IPEX" y la figura 5 muestra el conector IPEX en la tapa del módulo. En el circuito de referencia del fabricante (fig. 2, p.11) el pin **ANT (21) queda sin conexión** y los pines 20 y 22 van a GND, lo que sugiere que la antena se enchufa directamente al IPEX del módulo. **No dice cuál de los dos está activo ni cómo se elige.** Si es el IPEX, la PCB no necesita el U.FL del LoRa ni su línea de 50 Ω (solo el del GNSS). Hay que confirmarlo con el fabricante (manager@szdx-smart.com) o al recibir el módulo, antes del layout. Hasta entonces se conserva el diseño actual y se deja espacio libre sobre el IPEX del módulo.

## Qué medir al recibir el módulo

1. Distancia entre los centros de los pines 3 y 4 (debe ser 5.57 mm), entre los pines 1 y 11 (17.0 mm) y de cada extremo al borde del módulo (1.0 mm arriba y 2.0 mm abajo).
2. Tamaño de un pad de borde y su posición respecto del borde (el manual dice 2.0 x 1.0 mm, centrado en el borde).
3. Posición y tamaño del componente de la cara inferior (zona sin cobre).
4. Por qué conector sale la antena (IPEX o pad ANT): probar con una antena conectada al IPEX y medir.
