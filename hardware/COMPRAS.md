# Lista de compras y fuentes - Collar DR-VF-01

**Estado:** BORRADOR (2026-11-03). Complementa a `BOM.md`: el BOM define qué lleva el producto; este archivo define dónde se consigue cada pieza, con qué URL y qué falta por verificar. Los datos de AliExpress salen de capturas de pantalla del 2026-10-30. Los precios están en COP, cambian, y varios incluyen el descuento "Dto. Bienvenida / nuevo comprador", que puede no aplicar a tu cuenta.

**Cómo leer una ficha:** cada pieza tiene un código (`A1`, `A2`...). Úsalo para hablar de ella sin confundir links. "Variante a elegir" es la opción exacta que hay que marcar en la página antes de comprar.

**Estados:** `aceptado` (se compra), `propuesto` (falta que el autor confirme), `rechazado` (con motivo), `del docente` (sin compra), `pendiente` (sin definir).

**Acceso a las URL:** la herramienta de navegación del asistente no puede leer AliExpress (devuelve solo el menú de la página) y no se evade su verificación anticontrol de bots. Las URL se guardan sin parámetros de rastreo y los datos del producto se toman de capturas.

**Regla de verificación:** una URL o una captura prueba qué dice el vendedor, no que la pieza sea original ni que cumpla lo que anuncia. Las descripciones de AliExpress marcadas "Resumen de IA" las genera la plataforma y no se usan como especificación. Lo que no se pueda comprobar con una hoja de datos queda como "no verificado".

## 1. Del docente (sin compra)

**Fuente (confirmada por el autor el 2026-11-03):** el docente tiene físicamente todo lo que aparece en su biblioteca de Altium (`shared_lib.IntLib`, 61 componentes con MPN y código LCSC) y lo que publicó en el classroom (mensaje del 16 de octubre). Falta confirmar cantidades.

**Qué se lista aquí:** solo las piezas que el diseño usa y cuyo datasheet confirma que cumplen. Lo que la biblioteca o el classroom traen y el proyecto no usa se omite a propósito.

**Columna Fuente:** `Biblioteca` = la pieza tiene símbolo, huella y código LCSC en `shared_lib.IntLib`. `Classroom` = la nombra el mensaje del docente y no está en la biblioteca, así que su símbolo y huella hay que crearlos o descargarlos.

**Veredicto:** `Aprobado` = el datasheet confirma que cumple en la PCB. `Aprobado con condición` = cumple, y la condición es una tarea de diseño con su fuente.

### 1.1 Se usan en la PCB

| Código | Pieza | MPN | Fuente | Uso en la PCB | Veredicto | Evidencia |
|--------|-------|-----|--------|---------------|-----------|-----------|
| D1 | Cargador Li-ion/LiPo | TP4056 (YONGYUTAI, ESOP-8) | Classroom y biblioteca | Carga por USB-C a 500 mA (RNF-05) con RPROG de 2.4 kΩ | Aprobado con condición | I = 1200 V / RPROG (p.2); 2.4 kΩ da 450 a 550 mA; TEMP (pin 1) a GND desactiva el corte por temperatura (p.2). **Condición:** la huella `SOIC_8_3.9mm` de la biblioteca tiene 8 pads y no el pad térmico del ESOP-8, que va a GND (p.1). Hay que hacer la huella con el dibujo de empaque del fabricante, que el datasheet que tenemos no trae. LCSC C20199357 |
| D2 | Conector USB-C hembra, 16 pines | TYPE-C-3.1-16PIN (Shou Han) | Biblioteca | Programación por USB nativo y VBUS hacia el TP4056 | Aprobado (PCB de 0.8 mm) | Solo USB 2.0 (sin pines SuperSpeed); 5 V, 3 A; -25 a +85 °C; pines iguales al símbolo de la biblioteca. El dibujo del fabricante recomienda su layout para una PCB de **0.80 mm**: por eso la PCB pasa a 0.8 mm (decidido el 2026-11-03). Sin clasificación IP: lo sella la carcasa. LCSC C7507405 |
| D3 | Protección ESD USB, 2 líneas | USBLC6-2SC6 (ST, SOT-23-6) | Biblioteca | ESD de D+, D- y VBUS del USB-C | Aprobado | Pinout igual a la huella `SOT_23_6` (1 I/O1, 2 GND, 3 I/O2, 4 I/O2, 5 VBUS, 6 I/O1); VRM 5.25 V; fuga 10 nA típ. (150 nA máx.); 2.5 pF típ. por línea; USB 2.0 hasta 480 Mb/s; ESD nivel 4 de IEC 61000-4-2. Hoja de ST Rev 5 (copia de LCSC): falta bajar la Rev 7 de st.com. LCSC C7519 |
| D4 | IMU | LSM6DS3TR-C (ST) | Classroom | Despertar por movimiento (RF-06), INT1 a GPIO4 | Aprobado | Wake-up enrutable a INT1 (`MD1_CFG.INT1_WU`, p.92); 9 µA a 12.5 Hz en baja potencia y 3 µA en power-down (p.24); 1.71 a 3.6 V; I2C 0x6A o 0x6B; LGA-14 de 2.5 x 3 mm con la misma distribución que el LSM6DSOX salvo los pines 10 y 11 (sin conectar aquí). No tiene MLC. Zephyr no trae trigger de wake-up para esta familia: se configura con la librería oficial de ST (`lsm6ds3tr-c_STdC`). Hay que crear símbolo y huella |
| D5 | MOSFET P (M1 del CN3791) | AP2301 (ALLPOWER, SOT-23) | Biblioteca | Interruptor de entrada del CN3791 | Aprobado | Vds **-20 V** (la biblioteca dice 30 V), Vgs ±12 V, Vth -0.5 a -1.0 V, Rds(on) 57 mΩ típ. a -4.5 V; el CN3791 limita el gate a 8 V bajo VCC (datasheet CN3791 p.7). Sirve mientras la entrada no pase de unos 20 V, que con el panel de 6 V no ocurre. LCSC C385296 |
| D6 | Inductor del CN3791 | CY54-22UH, 22 µH (SHOU HAN) | Biblioteca | Etapa de potencia del CN3791 | Aprobado | Regla del datasheet del CN3791: L > 5·(VCC − VBAT); con 6.05 V y 3.0 V pide 15.3 µH. 1.10 A y 169 mΩ. El de 10 µH no cumple la regla. LCSC C2879694 |
| D7 | LED RGB | RS-3535MWAM (NationStar, 3535 SMD) | Biblioteca | Indicador de estado (RF-03), GPIO38 a 40 | Aprobado con condición | 6 pines independientes (R 4 y 3, G 5 y 2, B 6 y 1); -30 a +85 °C; 3.7 x 3.5 mm. VF: R 1.7 a 2.6 V; G y B 2.4 a 3.4 V. **Condición:** el máximo de G y B (3.4 V) pasa de 3.3 V, así que las resistencias se dimensionan con la curva VF-IF del datasheet; si VF de G o B a la corriente elegida supera 3.0 V a 25 °C, el ánodo se alimenta desde VBAT. Reemplaza al Kingbright L-154A4SURK/G5T, que no tiene datasheet. LCSC C842778 |
| D8 | Conector JST PH de 2 pines | S2B-PH-K-S(LF)(SN) (JST) | Biblioteca | Batería y panel solar | Aprobado | Serie PH, 2 A, -40 a +105 °C, a través de la placa, entrada lateral. El BOM decía B2B-PH-K-S (entrada superior): misma serie y mismas características. LCSC C173752 |
| D9 | Resistencias 0603 | 330 Ω, 1 kΩ, 4.7 kΩ y 10 kΩ | Biblioteca | Series de LED y optoacoplador, pull-ups de I2C, BOOT y CHRG | Aprobado | Son los valores que piden `pinout.md` y `ARCHITECTURE.md` |
| D10 | Condensadores 0603 y 0805 | 1 µF 50 V X5R 0603 (CL10A105KB8NNNC); 10 µF 25 V X5R 0805 (CL21A106KAYNNNE); 10 µF 10 V X5R 0603 (TCC0603X5R106K100CT); 22 µF 16 V 0805 (HGC0805R5226M160NSLJ, Chinocera); 33 pF NP0 50 V 0603 (CC0603JRNPO9BN330, YAGEO) | Biblioteca | RC de EN (1 µF); entrada de alimentación y desacople de 3.3 V (10 µF); 22 µF en el pin 3V3 del módulo (figura 9-1 del datasheet del WROOM-1); 2 x 33 pF en serie por pata en las cargas del cristal de 32.768 kHz | Aprobado con condición | X5R de -55 a +85 °C (Samsung y CCTC); NP0 de ±30 ppm/°C y -55 a +125 °C (YAGEO). El de 10 V solo va en el riel de 3.3 V. **Condición:** el 22 µF de Chinocera es X5R solo por su código (R5); falta leer su hoja antes de darlo por aprobado |

### 1.2 Evaluadas y descartadas (se compra otra pieza)

| Código | Pieza del docente | Motivo | Qué se compra |
|--------|-------------------|--------|---------------|
| D11 | Condensador de 100 nF Walsin 0603F104Z500CT | Es Y5V: pierde 40 % de capacidad a 45 °C y 77 % a 85 °C (hoja de Walsin, p.1). No sirve para el desacople del ESP32-S3 | 100 nF X7R (O21) |
| D12 | Schottky SS34-TD (3 A) | Fuga inversa de hasta 0.5 mA a 25 °C y 40 V; el presupuesto de sleep entero es de 124 µA | Schottky de baja fuga (O20) |
| D13 | Schottky PSBD1DF40V1H (1 A) | La tabla solo da fuga a 40 V. La curva típica marca unos 1.5 µA a 25 °C y unos 80 µA a 75 °C a una décima de la tensión nominal, y el margen de sleep es de 26 µA. Sin un valor garantizado a 4.2 V no se acepta | Schottky de baja fuga (O20) |

### 1.3 Plan B: valores que se arman con piezas del docente

Si el pedido de kits de AliExpress (O19 y O21) no llega, estos valores se arman con series y paralelos. Las resistencias del docente son de 5 % en 0R, 100R, 330R, 1k, 3k3 y 10k (descripción de la biblioteca). Las demás se suponen de 1 % por el sufijo F de su código de fabricante, que no se verificó en un datasheet.

| Necesidad | Se arma con | Resultado | Piezas |
|-----------|-------------|-----------|--------|
| RPROG del TP4056, 2.4 kΩ | 4.7k en paralelo con (4.7k + 191R), todas de 1 % | 2.397 kΩ, 500.7 mA (I = 1200 V / R) | 3 |
| CC1 y CC2 del USB-C, 5.1 kΩ | un 4.7k de 1 % por línea | -7.8 %, dentro de Rd de ±20 % (Microchip AN1914, tabla 1) | 2 |
| Pull-down del optoacoplador, 100 kΩ | 49.9k + 49.9k | 99.8 kΩ | 2 |
| Divisor MPPT del CN3791 (R4 = 100 kΩ, R3 = 402 kΩ) | R4 = 49.9k + 49.9k; R3 = 365k + 33.2k | 99.8 kΩ y 398.2 kΩ, V_MPPT = 1.205 V x (1 + R3/R4) = 6.013 V. El valor final depende del panel (O7) | 4 |
| Serie de UART0 TX, 499 Ω (guía de Espressif, p.16) | 330R + 150R | 480 Ω, -3.8 % (no es crítico) | 2 |
| Serie de USB D+ y D-, 22 o 33 Ω (guía de Espressif, p.21) | tres 100R en paralelo por línea | 33.3 Ω | 6 |
| Cargas del cristal de 32.768 kHz (CL 12.5 pF) | dos 33 pF NP0 en serie por pata | 16.5 pF por pata; carga efectiva de 11.3 a 13.3 pF con 3 a 5 pF parásitos | 4 |
| Condensador del pin COM del CN3791, 220 nF | dos 470 nF en serie | 235 nF; solo si los 470 nF son X7R (su código R7 lo sugiere, sin confirmar) | 2 |

Armar todo así suma unas 25 piezas por soldar. **No se puede armar con piezas del docente:** la resistencia de sensado del CN3791 (la menor del docente es de 84.5 Ω y daría 1.4 mA), el 100 nF X7R y el Schottky de baja fuga. Esas tres se compran en cualquier caso (O20, O21 y O22).


### 1.4 Lo que el docente no tiene (símbolo y huella por crear o descargar en Altium)

ESP32-S3-WROOM-1U-N16R8 (la biblioteca solo trae el ESP32-C3-WROOM-02-N4), MAX-M10S, MAX17048, módulo LoRa, CN3791, VO617A, zumbador, cristal de 32.768 kHz, conector u.FL, JST SH, TLV75733P y la huella ESOP-8 del TP4056. Las piezas se resuelven en las secciones 2 y 3.

## 2. AliExpress

Resumen de lo que se compra (detalle en las fichas):

| Código | Pieza | Precio visto | Estado |
|--------|-------|--------------|--------|
| A1 | Módulo LoRa LR30-900M22S | $18.762 | aceptado |
| A2 | Antena LoRa 8 cm con u.FL (10 uds) | $16.562 | aceptado |
| A3 | Kit antena SMA 3 dBi + pigtail (2 uds) | $22.240 | aceptado |
| A4 | Antena 915 MHz 5 dBi SMA con cable | $14.662 | propuesto |
| A5 | MAX17048G+T10 (2 uds) | $6.729 | aceptado |
| A7 | Placa cargadora CN3791 6 V | $3.526 | aceptado |
| A8 | Placa MAX-M10S con antena u.FL | $21.389 | aceptado |
| R1 | Antena GNSS activa L1+L5 A-SKY | $25.404 | rechazado |
| R2 | Antena corta GIZONT "10 dBi" | $38.690 | rechazado |
| R3 | Antena GNSS activa 18x18 mm | $9.432 | rechazado |
| R4 | Reguladores TPS7A05xx/TPS7A20xx (5 uds) | $6.418 | rechazado |

### A1. Módulo LoRa SX1262, LR30-900M22S (aceptado)
- **Título del listado:** "SX1262 LoRa módulo desarrollador SET DX-LR30 430 ~ 470MHz 868 ~ 915Mhz 22dBm 8km rango 5V UART GPIO ISM banda LoRa módulo RF".
- **Vendedor:** ZC Technology Store. 4.5★, 24 valoraciones, 356 vendidos.
- **Precio visto:** $18.762 (tachado $25.231,38; descuento nuevo comprador de $6.469,38). Máximo 1 unidad por comprador a ese precio. Envío gratis, entrega estimada 12 a 22 de noviembre.
- **Variante a elegir:** **"LR30 900M22S"** (módulo SMD de 20x14x2.3 mm). No elegir la "900M22SP": es una placa con STM32 y UART.
- **Aviso sobre el título:** dice "5V UART", pero eso describe la placa de desarrollo. El módulo es **SPI** y trabaja a 1.8 a 3.7 V, según el folleto del vendedor (`hardware/datasheets/lr30-900m22s_vendor_brochure.pdf`). Ese folleto es marketing, no el datasheet de Semtech.
- **Para qué:** radio LoRa 915 MHz del collar. Pines en `hardware/pinout.md`.
- **Cantidad sugerida:** 2 (collar y receptor, para pruebas de alcance). La oferta limita 1 por comprador: el segundo costaría más o requiere otra cuenta u otro pedido.
- **Antes de comprar o al recibir:** pedir al vendedor para qué sirve DIO2 y si el pin TX va unido a él, y si el oscilador de 32 MHz es cristal o TCXO. Conectar la antena antes de energizar.
- **URL:** (pendiente)

### A2. Antena LoRa de 8 cm con u.FL (aceptado)
- **Título del listado:** "10 Piezas de Antena IPX IPEX 1.13 UF.L de 868MHz 915MHz GSM Integrada de 2DBi y 8CM para LORA LORA32U4II".
- **Vendedor:** Tienda oficial wishiot. 5.0★, 3 valoraciones, 40 vendidos.
- **Precio visto:** $16.562 por 10 unidades (tachado $23.031,38). Envío gratis, entrega estimada 12 a 22 de noviembre.
- **Variante a elegir:** **10pcs** (también hay 20 y 50).
- **Para qué:** candidata de antena del collar: monopolo de cuarto de onda a 915 MHz.
- **Reservas:** el listado dice "868/915" sin separar variantes, y pocas reseñas. Su rendimiento depende del plano de masa de la PCB: probar alcance real.
- **Conector requerido en la PCB:** u.FL SMD (ver `BOM.md` 1.3).
- **URL:** https://es.aliexpress.com/item/1005010805433737.html

### A3. Kit antena SMA 3 dBi con pigtail u.FL a SMA de 15 cm (aceptado, para banco)
- **Título del listado:** "2 uds GSM 868Mhz 915MHz LoRa antena 3DBI con U.FL I-P-E-X-1 a conector SMA para placa Lora ESP32 Lora32u4 II WiFi Lora32 15cm".
- **Vendedor:** XYANT Wireless Store. 4.7★, 81 valoraciones, más de 1.000 vendidos.
- **Precio visto:** $22.240 por 2 unidades (-2 %, tachado $22.858,74; $15.439 por unidad desde 10 unidades). Quedaban 2 en stock.
- **Variante a elegir:** **"Antenna 1 Cable"** (la marcada en la captura, que muestra 2 antenas y 2 cables).
- **Para qué:** pruebas de banco con el módulo LoRa. Es una antena corta con SMA, no la del producto final.
- **Reservas:** los 3 dBi del listado no se pueden verificar.
- **URL:** (pendiente)

### A4. Antena 915 MHz 5 dBi SMA con cable de 10 cm (propuesta, solo banco)
- **Título del listado:** "Antena Omnidireccional de 868MHz 915MHz 5DBi SMA Macho con Cable de Extensión SMA Hembra a IP-EX de 10cm para LoRa para Arduino".
- **Vendedor:** Science Education Store. 4.6★, 11 valoraciones, 334 vendidos.
- **Precio visto:** $14.662 (tachado $44.023,71; descuento nuevo comprador de $29.361,71). Envío gratis, entrega estimada 12 a 22 de noviembre.
- **Variante a elegir:** **"915 antenna cable"** (la marcada en la captura; hay también variantes de 868 MHz).
- **Para qué:** pruebas de alcance con buena ganancia. Es larga (unos 20 cm): no cabe en el collar.
- **Estado:** propuesto. Ninguna decisión la acepta y hace lo mismo que la A3 (aceptada) con más ganancia: decidir si se compra.
- **URL:** (pendiente)

### A5. MAX17048G+T10 (aceptado)
- **Título del listado:** "2-10 piezas MAX17048G+T10 DFN-8 MAX17048G MAX17048 Chip IC Venta al por mayor".
- **Vendedor:** JINTAIXIN Global Store. 4.7★, más de 3.000 vendidos.
- **Precio visto:** $6.729 por 2 unidades ($3.364,50 cada una; tachado $13.689,90). Envío gratis, entrega estimada 11 a 18 de noviembre.
- **Variante a elegir:** "MAX17048G DFN-8"; número de modelo **2Pcs** (también 5 y 10).
- **Para qué:** medidor de batería (fuel gauge) por I2C, dirección 0x36.
- **Por qué se acepta:** RF-05 exige el porcentaje de batería por I2C y el BOM lo incluye. El datasheet (`hardware/datasheets/max17048_datasheet.pdf`) confirma TDFN de 8 pines, VDD de 2.5 a 4.5 V, 3 µA en hibernación, 23 µA activo y dirección I2C 0x36.
- **Reservas:** el marcaje del chip de la foto no se puede confirmar desde la foto. Para prototipo es razonable; para producción, Digi-Key.
- **Al recibir:** debe responder en I2C en 0x36.
- **URL:** (pendiente)

### A7. Placa cargadora solar CN3791 de 6 V (aceptada, para prototipo)
- **Título del listado:** "CN3791 Panel de cargador Solar ajustable MPPT 3,7 V 18650 Placa de cargador de batería de litio 6V/9V/12V PMW protección contra sobretensión".
- **Vendedor:** Realpoy Module Wholesale. 4.9★, 459 valoraciones, más de 5.000 vendidos.
- **Precio visto:** $3.526 (tachado $9.853,54; descuento nuevo comprador de $6.327,54). Envío gratis, entrega estimada 12 a 20 de noviembre.
- **Variante a elegir:** **6V** (también 9V y 12V).
- **Aviso:** es una **placa cargadora**, no un panel solar. El panel de 6 V (al menos 1 W) se compra aparte.
- **Para qué:** probar la carga solar antes de dibujar la PCB.
- **Por qué se acepta:** el CN3791 de 6 V coincide con el panel de 6 V de RNF-01 y con la decisión del 2026-10-30. Al recibir: medir desde qué tensión de entrada empieza a cargar (con 5 V de USB no debe cargar) y su consumo desde la batería sin entrada; la placa trae LEDs y otras piezas que el chip solo no tiene.
- **URL:** (pendiente)

### A8. Placa MAX-M10S con antena u.FL (aceptada, para banco)
- **Título del listado:** "U-BLOX MAX-M10S-00B-01 de 10.ª Generación, Módulo de Posicionamiento Beidou GPS GLONASS, Placa de Desarrollo, Placa Base, Antena".
- **Vendedor:** Shenzhen Zhumengxin E... 4.9★, 12 valoraciones, 350 vendidos. 32.518 disponibles.
- **Precio visto:** $21.389. Envío estándar $14.743,57, entrega estimada 11 a 21 de noviembre.
- **Variante a elegir:** la captura muestra "**MAX-M10S Board**" (3 opciones en total). **Comprobar cuál incluye la antena u.FL**: el listado muestra fotos con antena, pero la opción marcada puede ser solo la placa.
- **Para qué:** probar el GNSS con la DevKit del ESP32-S3 antes de la PCB. No es el componente del BOM final.
- **Por qué se acepta:** RF-01 exige GNSS multiconstelación con el MAX-M10S. La referencia del listado, MAX-M10S-00B-01, es la versión de producción en masa de la MAX-M10S-00B del BOM (tabla de tipos del datasheet de u-blox).
- **Al recibir:** leer `UBX-MON-VER` en u-center y comparar con el datasheet del MAX-M10S.
- **URL:** (pendiente)

### R1. Antena GNSS activa L1+L5 A-SKY (rechazada)
- **Título:** "Antena Activa de Cerámica GNSS GPS de Doble Frecuencia L1 y L5, de Alta Precisión, Ultrapequeña, para UAV e IoT". Vendedor: Tienda oficial GNSS A-S... 4.5★, 8 valoraciones, 161 vendidos. $25.403,85 ($19.298,37 por unidad desde 10). Variante AK521.
- **Motivo:** el MAX-M10S solo recibe L1 (datasheet). La antena es activa (20 dB de LNA, 15 a 25 mA), ocupa 25 mm y el módulo ya trae LNA y filtro SAW.

### R2. Antena corta GIZONT "10 dBi" (rechazada)
- **Título:** "Antena GIZONT LoRa 868MHz 863-870MHz 906MHz 915MHz, Antena Corta con Conector U.FL a SMA para Placa Lora ESP32Lora32u4II". Vendedor: High Quality Antenna St... 3.7★, 3 valoraciones, 24 vendidos. $38.690. Variante GT-588 915MHz.
- **Motivo:** 10 dBi no es creíble en una antena de 5 cm, y tiene pocas reseñas.

### R3. Antena GNSS activa de 18x18 mm (rechazada)
- **Título:** "Antena GPS activa 1575,42 MHz 18x18mm antena GNSS de cerámica de alta ganancia para rastreador de mascotas DIY posicionamiento preciso GPS activo". Vendedor: Tienda oficial BDLXGNSS. 5.0★, 20 valoraciones, 244 vendidos. $9.432 (tachado $16.916,36).
- **Motivo:** sin datasheet ni número de parte. El módulo ya trae LNA, así que se prefiere un parche pasivo.

### R4. Reguladores TPS7A05xx/TPS7A20xx de Digalai (rechazado)
- **Título:** "Original TPS7A05285PDBVR TPS7A05xx/20xxPDBVR TPS7A2015PDBVR regulador LDO de ruido ultrabajo 1,2-6V 0.5A circuito integrado SOT-23-". Vendedor: Digalai Store, 4.9★, más de 2.000 vendidos. $6.418 por 5 uds (tachado $12.887,38). Variantes: TPS7A2055, TPS7A2032, TPS7A0533, TPS7A0530, TPS7A05285 y TPS7A2015; la preseleccionada es TPS7A2055PDBVR, no la TPS7A0533. URL: no registrada.
- **Motivo:** el TPS7A05 entrega 200 mA como máximo (datasheet de TI, https://www.ti.com/lit/ds/symlink/tps7a05.pdf) y las cargas del repo suman 224 a 394 mA; el BOM v1.4 ya lo sustituyó por el TLV75733PDBVR de 1 A (O1). El "0.5 A" del título y del resumen de IA de la página no coincide con el datasheet del TPS7A05. Además es un "Original" sin trazabilidad, y para un regulador de potencia conviene Digi-Key o Mouser.

## 3. Otras fuentes (Digi-Key, Mouser, tiendas locales)

Para cada pieza: qué buscar exactamente y qué comprobar.

| Código | Pieza | Qué buscar | Dónde | Qué comprobar | Estado |
|--------|-------|------------|-------|---------------|--------|
| O1 | Regulador LDO 3.3 V, 1 A | **TLV75733PDBVR** (SOT-23-5, carrete de 3000); alternativa TLV75733PDYDR (con pad térmico) | Digi-Key / Mouser | Datasheet TLV757P: 1 A, Iq 25 µA, entrada de 1.45 a 5.5 V. Para pocas unidades, sufijo T (250 uds) si existe | aceptado |
| O2 | Módulo ESP32-S3 | **ESP32-S3-WROOM-1U-N16R8** | Digi-Key / Mouser | Es la variante con U.FL. En "artículos similares" de una captura apareció un listado de AliExpress de este módulo a $11.123; no se evaluó | pendiente |
| O3 | Parche GNSS pasivo con cable y u.FL (decidido el 2026-11-04) | 18x18x4 mm, 1575.42 MHz, pasivo (sin LNA), con cable corto y conector u.FL o IPEX, con datasheet de fabricante conocido | Digi-Key / Mouser, o AliExpress si el listado trae datasheet | Plano de masa que exige el fabricante del parche; que sea de 1575.42 MHz y pasivo. Falta elegir el MPN | pendiente |
| O4 | Conector u.FL SMD (2 en la PCB: LoRa y GNSS) | p. ej. Hirose U.FL-R-SMT-1 | Digi-Key | Huella y 50 ohm | pendiente |
| O6 | Batería 1S Li-ion/LiPo | 18650 protegida de marca, o LiPo 603450 de unos 1200 mAh con protección | tienda local | Polaridad del JST, protección, capacidad real | pendiente |
| O7 | Panel solar de 6 V, al menos 1 W | por definir | por definir | Voltaje en circuito abierto compatible con el CN3791 de 6 V | pendiente |
| O8 | Cristal 32.768 kHz, CL 12.5 pF | serie ECS-.327 | Digi-Key | MPN completo y cargas de 15 a 19 pF por pata (CL = C/2 + parásita; con 22 pF la carga queda en 14 a 16 pF, alta). Ver `BOM.md` 1.8 y la sección 1.3 | pendiente |
| O9 | Antena FPC 2.4 GHz (Wi-Fi/BLE del módulo) | **2015301200** | Mouser | Ganancia y huella; datasheet de Würth pendiente (la descarga automática responde 403) | pendiente |
| O10 | Conector SMA hembra (solo banco) | **6521011000** | Mouser | Huella; datasheet de Würth pendiente | pendiente |
| O11 | Módulo GNSS MAX-M10S | **MAX-M10S-00B** (MAX-M10S-00B-01 es la versión de producción; LGA de 18 pads, 10.1 x 9.7 x 2.5 mm) | Avnet / Digi-Key | Módulo **confirmado como suficiente solo** (datasheet R08, 2026-01-30, en producción masiva): trae TCXO, LNA, filtro SAW y bloqueo DC en RF_IN, así que no necesita cristal ni etapa RF externa. Falta: línea de 50 Ω hasta la antena y V_BCKP alimentado para arranques en caliente. A8 es una placa de desarrollo, no el componente del BOM final | aceptado, falta fuente |
| O13 | Optoacoplador | **VO617A-1-E** | Mouser | CTR y bins en `hardware/datasheets/vo617a_datasheet.pdf` | pendiente |
| O14 | Zumbador | **PKMCS0909E4000-R1** (el BOM decía PKMCS0909E000-R1, que no aparece en distribuidores) | Digi-Key | Un listado anota una fecha de última compra (31-mar-2027): comprobar disponibilidad | pendiente |
| O16 | Protección de batería 1S con MOSFET externos | **por identificar** (el BQ2975DRBR del BOM no existe) | Digi-Key | Elegir la pieza y sus MOSFET y resistencia de sensado | pendiente |
| O17 | Conector de 4 pines | el B4B-SH-SM4-TB del BOM no aparece en distribuidores; existe B4B-PH-SM4-TB(LF)(SN) | Mouser | Los conectores de 2 pines son del docente (D8). Este de 4 pines no tiene uso documentado: asignarlo al dibujar el esquemático, o quitarlo del BOM | pendiente |
| O18 | Pulsador 6x6 mm | PTS645SL50SMTR92 LFS | Digi-Key | Huella; datasheet de C&K pendiente | pendiente |
| O19 | Kit de resistencias 0603 1 % (E24) | Buscar "0603 1% resistor kit" de 0 Ω a 10 MΩ (unas 100 a 660 piezas). Valores que se usan: 2.4 kΩ (RPROG, RNF-05), 5.1 kΩ x2 (CC1 y CC2), 100 kΩ, 390 kΩ + 10 kΩ (R3 del divisor MPPT: 400 kΩ, V_MPPT de 6.036 V) y 22 o 33 Ω x2 (series de USB) | AliExpress (con tu pedido; ver 3.1) | El listado debe decir 1 %. Al recibir, medir dos o tres valores con el multímetro. Los valores finales del CN3791 salen al dibujarlo (`ARCHITECTURE.md` 2.2) | aceptado (2026-11-03) |
| O20 | Diodos Schottky de baja fuga (D1 y D2 del CN3791) | 40 V o más, 1 A o más, con fuga inversa **garantizada** a unos 4.2 V y a 65 °C | Digi-Key | De noche D1 y D2 quedan en inversa a unos 4.2 V (datasheet CN3791, p.9) y esa fuga sale de la batería: el presupuesto de sleep es de 124 µA. Elegir el MPN con ese dato en su datasheet | pendiente |
| O21 | Condensadores 0603 X7R y C0G | 100 nF X7R, 50 V (p. ej. YAGEO CC0603KRX7R9BB104, de la misma familia que el 1 nF CC0603KRX7R9BB102 de la biblioteca), 220 nF X7R (COM del CN3791) y, si no se usan los 33 pF del docente en serie, 15 o 18 pF C0G | AliExpress (con tu pedido; ver 3.1) | El listado debe nombrar el MPN y decir X7R o C0G. El multímetro mide la capacidad pero no el dieléctrico: un lote dudoso se prueba con un solo condensador antes de usarlo en la PCB. Reemplaza al 100 nF Y5V del docente (D11) | aceptado (2026-11-03) |
| O22 | Resistencia de sensado del CN3791 | 1 % con RCS = 0.12 V / I. Un panel de 1 W entrega unos 0.17 A, así que la corriente de carga sale de unos 0.2 a 0.5 A y RCS de 0.24 a 0.6 Ω. Potencia P = 0.12 V x I: 0805 hasta 0.5 A y 1206 desde 0.5 A | Mouser o Digi-Key si el envío a Colombia que se ve al pagar es menor a unos USD 20; si no, AliExpress con el MPN nombrado | Falta fijar la corriente de carga (depende del panel, O7) y con ella el valor | pendiente |

### 3.1 Dónde comprar los pasivos (consultado el 2026-11-03; precios solo orientativos)

| Ruta | Qué se vio | Costo de envío a Colombia | Cuándo conviene |
|------|------------|---------------------------|-----------------|
| AliExpress | Kit de 100 resistencias 0603 1 % a $3.610; kit de 660 piezas 0603/0805/1206 1 % a $12.280; 50 condensadores 0805 100 nF X7R a $1.117. Hay reportes de condensadores falsos o mal etiquetados | No confirmado en esas páginas; tus capturas de A1 a A8 muestran envío gratis en casi todo | Kits y pasivos baratos, junto con tu pedido |
| LCSC | El comprador paga los impuestos (FCA o EXW). Sin promoción de envío gratis para Sudamérica | No se publica; se calcula al pagar | Solo en un pedido grande |
| Digi-Key y Mouser | Piezas de marca con datasheet. Desde EE. UU. no pagarían arancel ni IVA bajo USD 200 (comunicado 158 de 2021 de la DIAN, que exige menos de 6 unidades; sin confirmar para 10 a 20 piezas de un valor) | No encontrado; se ve al pagar. Si pasa de unos USD 20, sale más caro que las piezas | Resistencia de sensado y Schottky, si el envío es bajo |
| Electronilab (Bogotá) | Resistencias 0805 1 % a COP 50 c/u en múltiplos de 10; entrega de 1 a 5 días; envío gratis desde COP 300.000. **No tiene 0603** ni condensadores SMD | Bajo COP 300.000 no se indica | No sirve para los 0603 de la biblioteca |
| Sigma Electrónica | La búsqueda de "0603" no devolvió productos | No consultado | No sirve |

Impuestos (DIAN): sin arancel hasta USD 200 FOB. Desde China se paga IVA de 19 % (lectura propia de las fuentes). El decreto 1474 de 2025, que bajaba el límite a USD 50, fue tumbado (sentencia C-079 de 2026). Didácticas Electrónicas, Vistronica y Mercado Libre no se pudieron consultar (error 403).

## 4. Qué comprobar al recibir

- **A1, módulo LoRa:** conectar la antena antes de energizar.
- **A5, MAX17048:** debe responder en I2C en la dirección 0x36.
- **A8, MAX-M10S:** leer `UBX-MON-VER` en u-center.
- **O6, batería:** polaridad del conector JST contra la PCB, y que traiga protección.
- **A7, placa CN3791:** a qué tensión empieza a cargar y su consumo desde la batería sin entrada.
- **O14, zumbador:** disponibilidad (aviso de última compra).
- **O19, kit de resistencias:** medir dos o tres valores con el multímetro y confirmar que sean de 1 %.
- **O21, condensadores:** medir la capacidad de unos cuantos; el dieléctrico (X7R o C0G) no se puede medir, así que un lote dudoso se prueba antes de usarlo en la PCB.
