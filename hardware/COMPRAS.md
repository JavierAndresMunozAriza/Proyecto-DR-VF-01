# Lista de compras y fuentes - Collar DR-VF-01

**Estado:** BORRADOR (2026-10-30). Complementa a `BOM.md`: el BOM define qué lleva el producto; este archivo define dónde se consigue cada pieza, cuál es su URL y qué falta por verificar. Los precios son los vistos en pantalla en COP y cambian: la fecha de cada captura es 2026-10-30.

**Estados:** `aceptado` (se compra), `propuesto` (falta que el autor confirme), `rechazado` (con motivo), `del docente` (sin compra), `pendiente` (sin definir).

**Acceso a las URL:** la herramienta de navegación del asistente no puede leer AliExpress (devuelve solo el menú de la página) y no se evade su verificación anticontrol de bots. Las URL se guardan sin parámetros de rastreo y los datos del producto se toman de capturas.

**Regla de verificación:** una URL o una captura prueba qué dice el vendedor, no que la pieza sea original ni que cumpla lo que anuncia. Lo que no se pueda comprobar con una hoja de datos queda como "no verificado".

## 1. Del docente (sin compra)

Según el autor, el docente tiene estas piezas físicamente. Falta confirmar cantidades.

| Pieza | Referencia | Uso en el proyecto | Estado |
|-------|------------|--------------------|--------|
| Biblioteca de Altium | `shared_lib.IntLib` | Símbolos y huellas (ver `BOM.md` sección 7) | del docente |
| Cargador Li-ion | TP4056 (ESOP-8) | Posible carga por USB; sin power-path | del docente |
| IMU | LSM6DS3TR-C | Prototipo; el BOM usa LSM6DSOX | del docente |
| Sensor de temperatura | TMP102AIDRLR | Posible corte de carga por temperatura | del docente |
| Amplificador | TLV9064IDR | Sin uso por ahora | del docente |
| Transceptor RS-485 | TP8485E-SR | Sin uso | del docente |
| Monitor de corriente | INA139NA/3K | Sin uso por ahora | del docente |
| Potenciómetro 10 kΩ | TC33X-2-103E | Sin uso | del docente |
| Regulador 3 A | DS8570F8 | No se usa hasta revisar su Iq | del docente |
| Pasivos, USB-C 16P, TVS, Schottky, MOSFET, LED RGB | ver biblioteca | Esquemático y prototipo | del docente |

## 2. AliExpress

| Pieza | Modelo | Vendedor | Precio visto | URL | Estado | Notas de verificación |
|-------|--------|----------|--------------|-----|--------|-----------------------|
| Módulo LoRa SX1262 | LR30-900M22S (variante SMD, no "SP") | ZC Technology Store | $18.762 | (pendiente) | aceptado | SPI confirmado en el folleto. Confirmar con el vendedor: DIO2/TX y cristal vs TCXO |
| Antena LoRa 915 MHz, u.FL, 8 cm | "IPEX 1.13 868/915 MHz 2 dBi" | wishiot | $16.562 (10 uds) | https://es.aliexpress.com/item/1005010805433737.html | aceptado | Probar alcance real; depende de la masa de la PCB |
| Kit antena SMA 3 dBi + pigtail u.FL a SMA 15 cm | "GSM 868/915 3 dBi" | XYANT Wireless Store | $22.240 (2 uds) | (pendiente) | aceptado | Para banco |
| Antena 915 MHz 5 dBi SMA con cable 10 cm | "Omnidireccional 868/915 5 dBi" | Science Education Store | $14.662 | (pendiente) | propuesto | Larga: solo banco |
| Módulo MAX17048 | MAX17048G+T10 (DFN-8) | JINTAIXIN Global Store | $6.729 (2 uds) | (pendiente) | propuesto | Verificar dirección I2C 0x36 al recibir. Producción: Digi-Key |
| Protección ESD USB | USBLC6-2SC6 (elegir la variante **-2SC6**, no la -4SC6) | Shenzhen Hong Ming (marca SUHMS) | $3.203 (10 uds) | (pendiente) | propuesto | Marca no ST; solo prototipo |
| Cargador solar CN3791 (placa) | CN3791 MPPT, versión 6 V | Realpoy Module Wholesale | $3.526 | (pendiente) | propuesto | Es un módulo cargador, no un panel |
| Módulo GNSS MAX-M10S con antena u.FL | "MAX-M10S-00B-01" | Shenzhen Zhumengxin | $21.389 | (pendiente) | propuesto | Verificar con `UBX-MON-VER` en u-center |
| Antena GNSS activa L1+L5 | A-SKY AK521 | A-SKY | $25.404 | (no aplica) | rechazado | El MAX-M10S solo recibe L1; antena activa de 25 mm que consume 15 a 25 mA |
| Antena corta "10 dBi" | GIZONT 915 MHz | High Quality Antenna Store | $38.690 | (no aplica) | rechazado | 10 dBi no es creíble en 5 cm; 3 reseñas |
| Antena GNSS activa 18x18 mm | "Active GPS Antenna 1575.42 MHz" | BDLXGNSS | $9.432 | (no aplica) | rechazado | Sin datasheet; el módulo ya trae LNA; se prefiere parche pasivo |

## 3. Otras fuentes (Digi-Key, Mouser, tiendas locales)

| Pieza | Referencia | Dónde | URL | Estado |
|-------|------------|-------|-----|--------|
| Regulador LDO 3.3 V, 1 A | TLV75733PDBVR (o PDYDR) | Digi-Key / Mouser | (pendiente) | propuesto |
| Módulo ESP32-S3 | ESP32-S3-WROOM-1U-N16R8 | Digi-Key / Mouser / AliExpress | (pendiente) | pendiente |
| Parche GNSS pasivo 18x18x4 mm, 1575.42 MHz | por definir | Digi-Key / Mouser | (pendiente) | pendiente |
| Conector u.FL SMD | por definir (p. ej. Hirose U.FL-R-SMT-1) | Digi-Key | (pendiente) | pendiente |
| USB-C de 16 pines | por definir (candidato: Amphenol GSB1C41110SSHR) | Digi-Key | (pendiente) | pendiente |
| Batería 1S Li-ion/LiPo | 18650 protegida o LiPo 603450 | tienda local | (pendiente) | pendiente |
| Panel solar de 6 V, al menos 1 W | por definir | por definir | (pendiente) | pendiente |
| Cristal 32.768 kHz, CL 12.5 pF | por definir (serie ECS-.327) | Digi-Key | (pendiente) | pendiente |

## 4. Qué comprobar al recibir

- **MAX-M10S:** leer `UBX-MON-VER` en u-center y comparar con el datasheet.
- **MAX17048:** debe responder en I2C en la dirección 0x36.
- **Módulo LoRa:** conectar la antena antes de energizar.
- **Batería:** polaridad del conector JST contra la PCB, y que traiga protección.
- **USBLC6:** que sea la variante de 2 líneas y que el pinout coincida con la huella.
