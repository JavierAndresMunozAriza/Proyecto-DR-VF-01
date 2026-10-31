# Lista de compras y fuentes - Collar DR-VF-01

**Estado:** BORRADOR (2026-10-31). Complementa a `BOM.md`: el BOM define qué lleva el producto; este archivo define dónde se consigue cada pieza, con qué URL y qué falta por verificar. Los datos de AliExpress salen de capturas de pantalla del 2026-10-30. Los precios están en COP, cambian, y varios incluyen el descuento "Dto. Bienvenida / nuevo comprador", que puede no aplicar a tu cuenta.

**Cómo leer una ficha:** cada pieza tiene un código (`A1`, `A2`...). Úsalo para hablar de ella sin confundir links. "Variante a elegir" es la opción exacta que hay que marcar en la página antes de comprar.

**Estados:** `aceptado` (se compra), `propuesto` (falta que el autor confirme), `rechazado` (con motivo), `del docente` (sin compra), `pendiente` (sin definir).

**Acceso a las URL:** la herramienta de navegación del asistente no puede leer AliExpress (devuelve solo el menú de la página) y no se evade su verificación anticontrol de bots. Las URL se guardan sin parámetros de rastreo y los datos del producto se toman de capturas.

**Regla de verificación:** una URL o una captura prueba qué dice el vendedor, no que la pieza sea original ni que cumpla lo que anuncia. Las descripciones de AliExpress marcadas "Resumen de IA" las genera la plataforma y no se usan como especificación. Lo que no se pueda comprobar con una hoja de datos queda como "no verificado".

## 1. Del docente (sin compra)

Según el autor, el docente tiene estas piezas físicamente (mensaje del docente del 16 de octubre, más su biblioteca `shared_lib.IntLib`). Falta confirmar cantidades.

| Código | Pieza | Referencia | Uso en el proyecto | Notas |
|--------|-------|------------|--------------------|-------|
| D1 | Biblioteca de Altium | `shared_lib.IntLib` | Símbolos y huellas (ver `BOM.md` sección 7) | Solo símbolos y huellas |
| D2 | Cargador Li-ion/LiPo | TP4056 (ESOP-8) | Se usa: carga por USB-C (decidido el 2026-10-31) | Lineal, 4.0 a 8.0 V, hasta 1 A, sin power-path (datasheet verificado); RPROG de 2.4 kΩ para 500 mA |
| D3 | IMU | LSM6DS3TR-C | Prototipo; el BOM usa LSM6DSOX | No es el mismo chip: probar el wake-up por movimiento |
| D4 | Sensor de temperatura | TMP102AIDRLR | Posible corte de carga por temperatura | I2C |
| D5 | Amplificador | TLV9064IDR | Sin uso por ahora | |
| D6 | Transceptor RS-485 | TP8485E-SR | Sin uso | |
| D7 | Monitor de corriente | INA139NA/3K | Sin uso por ahora | |
| D8 | Potenciómetro 10 kΩ | TC33X-2-103E | Sin uso | |
| D9 | Regulador 3 A | DS8570F8 | No se usa hasta revisar su Iq | Un LDO de 3 A suele tener Iq alta; el sleep presupuestado es de unos 95 µA |
| D10 | Pasivos, USB-C 16P, TVS, Schottky, MOSFET, LED RGB | ver biblioteca | Esquemático y prototipo | La biblioteca no trae ESP32-S3-WROOM-1U confirmado, MAX-M10S, LSM6DSOX, MAX17048, módulo LoRa, CN3791, VO617A, buzzer, cristal 32.768 kHz, u.FL ni JST SH |

## 2. AliExpress

Resumen de lo que se compra (detalle en las fichas):

| Código | Pieza | Precio visto | Estado |
|--------|-------|--------------|--------|
| A1 | Módulo LoRa LR30-900M22S | $18.762 | aceptado |
| A2 | Antena LoRa 8 cm con u.FL (10 uds) | $16.562 | aceptado |
| A3 | Kit antena SMA 3 dBi + pigtail (2 uds) | $22.240 | aceptado |
| A4 | Antena 915 MHz 5 dBi SMA con cable | $14.662 | propuesto |
| A5 | MAX17048G+T10 (2 uds) | $6.729 | propuesto |
| A6 | USBLC6-2SC6 (10 uds) | $3.203 | propuesto |
| A7 | Placa cargadora CN3791 6 V | $3.526 | propuesto |
| A8 | Placa MAX-M10S con antena u.FL | $21.389 | propuesto |
| R1 | Antena GNSS activa L1+L5 A-SKY | $25.404 | rechazado |
| R2 | Antena corta GIZONT "10 dBi" | $38.690 | rechazado |
| R3 | Antena GNSS activa 18x18 mm | $9.432 | rechazado |
| R4 | Reguladores TPS7A05xx/TPS7A20xx (5 uds) | $6.418 | rechazado |

### A1. Módulo LoRa SX1262, LR30-900M22S (aceptado)
- **Título del listado:** "SX1262 LoRa módulo desarrollador SET DX-LR30 430 ~ 470MHz 868 ~ 915Mhz 22dBm 8km rango 5V UART GPIO ISM banda LoRa módulo RF".
- **Vendedor:** ZC Technology Store. 4.5★, 24 valoraciones, 356 vendidos.
- **Precio visto:** $18.762 (tachado $25.231,38; descuento nuevo comprador de $6.469,38). Máximo 1 unidad por comprador a ese precio. Envío gratis, entrega estimada 12 a 22 de noviembre.
- **Variante a elegir:** **"LR30 900M22S"** (módulo SMD de 20x14x2.3 mm). No elegir la "900M22SP": es una placa con STM32 y UART.
- **Aviso sobre el título:** dice "5V UART", pero eso describe la placa de desarrollo. El módulo es **SPI** y trabaja a 1.8 a 3.7 V, según el folleto del vendedor (`docs/datasheets/SX1262 LoRa módulo desarrollador.pdf`). Ese folleto es marketing, no el datasheet de Semtech.
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
- **URL:** (pendiente)

### A5. MAX17048G+T10 (propuesto)
- **Título del listado:** "2-10 piezas MAX17048G+T10 DFN-8 MAX17048G MAX17048 Chip IC Venta al por mayor".
- **Vendedor:** JINTAIXIN Global Store. 4.7★, más de 3.000 vendidos.
- **Precio visto:** $6.729 por 2 unidades ($3.364,50 cada una; tachado $13.689,90). Envío gratis, entrega estimada 11 a 18 de noviembre.
- **Variante a elegir:** "MAX17048G DFN-8"; número de modelo **2Pcs** (también 5 y 10).
- **Para qué:** medidor de batería (fuel gauge) por I2C, dirección 0x36.
- **Reservas:** el marcaje del chip de la foto no se puede confirmar sin el datasheet (no se pudo descargar). Para prototipo es razonable; para producción, Digi-Key.
- **Al recibir:** debe responder en I2C en 0x36.
- **URL:** (pendiente)

### A6. USBLC6-2SC6, protección ESD USB (propuesto)
- **Título del listado:** "(10 piezas) 100% nuevo Chipset USBLC6-2SC6 UL26 USBLC6-4SC6 UL46 sot23-6".
- **Vendedor:** Shenzhen Hong Ming... (marca SUHMS, "personalizado: sí"). 4.8★, 29 valoraciones, 224 vendidos.
- **Precio visto:** $3.203 por 10 unidades (-10 %, tachado $3.558,89). Envío estándar $9.704,07, entrega estimada 20 de noviembre a 2 de diciembre. 1.854 disponibles.
- **Variante a elegir:** **USBLC6-2SC6**. En la captura el listado traía marcada por defecto la **USBLC6-4SC6** (4 líneas, otro pinout, sin pin de VBUS): no sirve para USB 2.0 D+/D-.
- **Para qué:** protección ESD de D+, D- y VBUS del USB-C.
- **Reservas:** marca no ST. El "manual de usuario" que trae el vendedor es texto genérico sin parámetros: no es un datasheet. Para producción, comprar el ST en Digi-Key o Mouser. Verificar el pinout con el datasheet de ST (no descargado).
- **URL:** (pendiente)

### A7. Placa cargadora solar CN3791 de 6 V (propuesta, para prototipo)
- **Título del listado:** "CN3791 Panel de cargador Solar ajustable MPPT 3,7 V 18650 Placa de cargador de batería de litio 6V/9V/12V PMW protección contra sobretensión".
- **Vendedor:** Realpoy Module Wholesale. 4.9★, 459 valoraciones, más de 5.000 vendidos.
- **Precio visto:** $3.526 (tachado $9.853,54; descuento nuevo comprador de $6.327,54). Envío gratis, entrega estimada 12 a 20 de noviembre.
- **Variante a elegir:** **6V** (también 9V y 12V).
- **Aviso:** es una **placa cargadora**, no un panel solar. El panel de 6 V (al menos 1 W) se compra aparte.
- **Para qué:** probar la carga solar antes de dibujar la PCB.
- **URL:** (pendiente)

### A8. Placa MAX-M10S con antena u.FL (propuesta, para banco)
- **Título del listado:** "U-BLOX MAX-M10S-00B-01 de 10.ª Generación, Módulo de Posicionamiento Beidou GPS GLONASS, Placa de Desarrollo, Placa Base, Antena".
- **Vendedor:** Shenzhen Zhumengxin E... 4.9★, 12 valoraciones, 350 vendidos. 32.518 disponibles.
- **Precio visto:** $21.389. Envío estándar $14.743,57, entrega estimada 11 a 21 de noviembre.
- **Variante a elegir:** la captura muestra "**MAX-M10S Board**" (3 opciones en total). **Comprobar cuál incluye la antena u.FL**: el listado muestra fotos con antena, pero la opción marcada puede ser solo la placa.
- **Para qué:** probar el GNSS con la DevKit del ESP32-S3 antes de la PCB. No es el componente del BOM final.
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
- **Motivo:** el TPS7A05 entrega 200 mA como máximo (`hardware/datasheets/tps7a05_datasheet.pdf`) y las cargas del repo suman 221 a 394 mA; el BOM v1.4 ya lo sustituyó por el TLV75733PDBVR de 1 A (O1). El "0.5 A" del título y del resumen de IA de la página no coincide con el datasheet del TPS7A05. Además es un "Original" sin trazabilidad, y para un regulador de potencia conviene Digi-Key o Mouser.

## 3. Otras fuentes (Digi-Key, Mouser, tiendas locales)

Para cada pieza: qué buscar exactamente y qué comprobar.

| Código | Pieza | Qué buscar | Dónde | Qué comprobar | Estado |
|--------|-------|------------|-------|---------------|--------|
| O1 | Regulador LDO 3.3 V, 1 A | **TLV75733PDBVR** (SOT-23-5, carrete de 3000); alternativa TLV75733PDYDR (con pad térmico) | Digi-Key / Mouser | Datasheet TLV757P: 1 A, Iq 25 µA, entrada de 1.45 a 5.5 V. Para pocas unidades, sufijo T (250 uds) si existe | propuesto |
| O2 | Módulo ESP32-S3 | **ESP32-S3-WROOM-1U-N16R8** | Digi-Key / Mouser | Es la variante con U.FL. En "artículos similares" de una captura apareció un listado de AliExpress de este módulo a $11.123; no se evaluó | pendiente |
| O3 | Parche GNSS pasivo | 18x18x4 mm, 1575.42 MHz, con datasheet de fabricante conocido | Digi-Key / Mouser | Plano de masa requerido por el fabricante | pendiente |
| O4 | Conector u.FL SMD | p. ej. Hirose U.FL-R-SMT-1 | Digi-Key | Huella y 50 ohm | pendiente |
| O5 | USB-C de 16 pines | candidato Amphenol GSB1C41110SSHR | Digi-Key | Que su huella coincida con `CON_USB_TYPEC_16P` de la biblioteca | pendiente |
| O6 | Batería 1S Li-ion/LiPo | 18650 protegida de marca, o LiPo 603450 de unos 1200 mAh con protección | tienda local | Polaridad del JST, protección, capacidad real | pendiente |
| O7 | Panel solar de 6 V, al menos 1 W | por definir | por definir | Voltaje en circuito abierto compatible con el CN3791 de 6 V | pendiente |
| O8 | Cristal 32.768 kHz, CL 12.5 pF | serie ECS-.327 | Digi-Key | MPN completo y capacitores de carga (unos 18 a 22 pF) | pendiente |

## 4. Qué comprobar al recibir

- **A1, módulo LoRa:** conectar la antena antes de energizar.
- **A5, MAX17048:** debe responder en I2C en la dirección 0x36.
- **A6, USBLC6:** que sea la variante de 2 líneas y que el pinout coincida con la huella.
- **A8, MAX-M10S:** leer `UBX-MON-VER` en u-center.
- **O6, batería:** polaridad del conector JST contra la PCB, y que traiga protección.
