# Asignación de Pines del Collar DR-VF-01

**Estado:** PROPUESTA v2.4 (2026-11-04), pendiente de revisión del autor. Es el **único mapa de pines** del proyecto: la PCB y la ESP32-S3-DevKitC-1 (N16R8) usan los mismos GPIO para que el firmware sea idéntico en ambas. Cualquier otro mapa en `docs/` o `reports/` debe remitir a este archivo.

**Fuente verificada:** *ESP32-S3-WROOM-1 & WROOM-1U Datasheet v1.8*, Tabla 3-1 (pines del módulo) y Tabla 4-1 (strapping). Las hojas de datos se descargan según `hardware/datasheets/README.md`.

## Reglas del módulo ESP32-S3-WROOM-1U-N16R8

| Regla | Detalle | Fuente |
|-------|---------|--------|
| Solo existen GPIO0-21, GPIO38-48 | El módulo no saca GPIO22 a GPIO34. GPIO22-25 no existen en el chip y GPIO26-32 son la flash | Tabla 3-1 |
| GPIO35, 36, 37 no disponibles | La PSRAM octal (variante R8) los ocupa | Tabla 3-1, nota b |
| GPIO0, 3, 45, 46 son de strapping | Defecto: GPIO0 pull-up, GPIO3 flotante, GPIO45 y GPIO46 pull-down. No conectar circuitos activos a 3, 45, 46 | Tabla 4-1 |
| GPIO19 = USB D-, GPIO20 = USB D+ | Orden inverso al del documento anterior | Tabla 3-1 |
| GPIO15 y GPIO16 = XTAL_32K_P / N | Reservados para el cristal RTC de 32.768 kHz (ver `BOM.md`) | Tabla 3-1 |
| GPIO43 / GPIO44 = UART0 TX / RX | Consola y programación de respaldo: dejar como puntos de prueba | Tabla 3-1 |
| Despertar de deep sleep solo con RTC GPIO (GPIO0-21) | Las fuentes de despertar van en esos pines | Tabla 3-1 |

## Mapa por GPIO

| GPIO | Pin módulo | Señal | Periférico | Notas |
|------|-----------|-------|------------|-------|
| 0 | 27 | BOOT | Botón/pad a GND | Strapping. Pull-up 10 kΩ. A GND = modo descarga |
| 1 | 39 | OPTO_CTRL | VO617A (entrada) | RTC. 330 Ω en serie y pull-down de 100 kΩ para que arranque apagado |
| 2 | 38 | BUZZER_PWM | Zumbador | RTC. PWM de 4 kHz directo, con 100 Ω en serie |
| 3 | 15 | (reservado) | — | Strapping JTAG. No conectar |
| 4 | 4 | IMU_INT1 | LSM6DS3TR-C INT1 | **RTC, despierta de deep sleep** |
| 5 | 5 | LORA_RESET | SX1262 NRESET | RTC |
| 6 | 6 | LORA_DIO1 | SX1262 DIO1 | **RTC, interrupción / despertar** |
| 7 | 7 | BUTTON | Botón de usuario | **RTC, despierta**. 10 kΩ a 3V3, 1 kΩ en serie y 100 nF a GND |
| 8 | 12 | I2C_SDA | LSM6DS3TR-C, MAX17048 | Pull-up 4.7 kΩ |
| 9 | 17 | I2C_SCL | LSM6DS3TR-C, MAX17048 | Pull-up 4.7 kΩ |
| 10 | 18 | LORA_NSS | SX1262 NSS | FSPI CS0. Alto al inicio |
| 11 | 19 | LORA_MOSI | SX1262 MOSI | FSPI D |
| 12 | 20 | LORA_SCK | SX1262 SCK | FSPI CLK |
| 13 | 21 | LORA_MISO | SX1262 MISO | FSPI Q |
| 14 | 22 | LORA_BUSY | SX1262 BUSY | RTC |
| 15 | 8 | (reservado) | XTAL_32K_P | No usar como GPIO si se monta el cristal |
| 16 | 9 | (reservado) | XTAL_32K_N | Igual que GPIO15 |
| 17 | 10 | GNSS_TX | MAX-M10S **RXD** (pin 3) | UART1 TX del ESP32. Cruce TX/RX |
| 18 | 11 | GNSS_RX | MAX-M10S **TXD** (pin 2) | UART1 RX del ESP32 |
| 19 | 13 | USB_D- | USB-C D- | USB nativo |
| 20 | 14 | USB_D+ | USB-C D+ | USB nativo |
| 21 | 23 | GNSS_EXTINT | MAX-M10S EXTINT (pin 5) | Salida del ESP32 hacia una entrada del GNSS (opcional: dejar abierta si no se usa) |
| 35-37 | 28-30 | no disponibles | — | PSRAM octal |
| 38 | 31 | LED_R | LED RGB rojo (cátodo) | Activo en bajo, 750 Ω en serie. En DevKitC-1 v1.1 coincide con el LED integrado |
| 39 | 32 | LED_G | LED RGB verde (cátodo) | Activo en bajo, 470 Ω en serie |
| 40 | 33 | LED_B | LED RGB azul (cátodo) | Activo en bajo, 330 Ω en serie |
| 41 | 34 | CHRG_STAT | CN3791 CHRG | Colector abierto: pull-up 10 kΩ |
| 42 | 35 | LORA_RXEN | Módulo LoRa, pin RX (pin 6, conmutador de antena) | Salida. **POR DECIDIR** |
| 43 | 37 | TXD0 | Punto de prueba / UART TX del módem de Fase 2 | Consola; en Fase 2 la consola pasa a USB CDC. **POR DECIDIR** |
| 44 | 36 | RXD0 | Punto de prueba / UART RX del módem de Fase 2 | Ídem |
| 45, 46 | 26, 16 | (reservados) | — | Strapping. No conectar |
| 47 | 24 | LORA_TXEN | Módulo LoRa, pin TX (pin 7, conmutador de antena) | Salida. Si el conmutador lo gobierna DIO2 (pin 8 unido a pin 7), dejar libre |
| 48 | 25 | RESERVA_F2 | Módem Fase 2 (PWRKEY) | Libre en Fase 1 |

**Señales retiradas del mapa anterior:** `POWER_GOOD` y `ENABLE_CHG` (CN3791). El datasheet del CN3791 (`hardware/datasheets/cn3791_datasheet.pdf`) confirma que no tiene esos pines: sus pines son VG, GND, CHRG, DONE, COM, MPPT, BAT, CSP, VCC y DRV, y CHRG y DONE son salidas de drenador abierto. El TP4056 (carga por USB-C) tiene dos salidas de estado de drenador abierto (pines 6 y 7), pero no queda un GPIO libre para leerlas: ver la sección del TP4056.

## Conexiones del módulo para el esquemático (41 pines)

**Fuente:** datasheet *ESP32-S3-WROOM-1 & WROOM-1U v1.8*: tabla 3-1 (pines), 4-1 (strapping), 4-3 (arranque), 4-5 (JTAG), 6-2 (alimentación) y figura 9-1 (circuito típico). **Verificado el 2026-11-04:** los pares GPIO a pin físico de este archivo coinciden con la tabla 3-1 (36 GPIO, sin diferencias). La tabla sigue el orden del pin físico, que es como se dibuja el símbolo. `hardware/esp32-s3-wroom-1u_pins.csv` trae las mismas 41 filas para importarlas en Altium, porque la biblioteca del docente no tiene símbolo de este módulo.

| Pin | Nombre | GPIO | Red | Conexión | Fuente |
|-----|--------|------|-----|----------|--------|
| 1 | GND | - | GND | Plano de masa | Tabla 3-1 |
| 2 | 3V3 | - | 3V3 | Salida del TLV75733P; 22 µF y 0.1 µF X7R pegados al pin | Tabla 6-2, figura 9-1 |
| 3 | EN | - | EN | 10 kΩ a 3V3 y 1 µF a GND; punto de prueba. Nunca flotante | Tabla 3-1, capítulo 9 |
| 4 | IO4 | 4 | IMU_INT1 | INT1 del LSM6DS3TR-C (pin 4). RTC, despierta de deep sleep | pinout.md |
| 5 | IO5 | 5 | LORA_RESET | RESET del módulo LoRa (pin 15) | pinout.md |
| 6 | IO6 | 6 | LORA_DIO1 | DIO1 del módulo LoRa (pin 13). RTC, despierta | pinout.md |
| 7 | IO7 | 7 | BUTTON | Botón de usuario a GND; 10 kΩ a 3V3, 1 kΩ en serie y 100 nF a GND. RTC, despierta | pinout.md |
| 8 | IO15 | 15 | XTAL_32K_P | Cristal de 32.768 kHz y su carga | Tabla 3-1 |
| 9 | IO16 | 16 | XTAL_32K_N | Cristal de 32.768 kHz y su carga | Tabla 3-1 |
| 10 | IO17 | 17 | GNSS_TX | RXD del MAX-M10S (pin 3). UART1 TX | pinout.md |
| 11 | IO18 | 18 | GNSS_RX | TXD del MAX-M10S (pin 2). UART1 RX | pinout.md |
| 12 | IO8 | 8 | I2C_SDA | SDA del LSM6DS3TR-C (pin 14) y del MAX17048; pull-up de 4.7 kΩ | pinout.md |
| 13 | IO19 | 19 | USB_D- | D- del USB-C a través del USBLC6; huella de 22 o 33 Ω en serie | Tabla 3-1, guía p.21 |
| 14 | IO20 | 20 | USB_D+ | D+ del USB-C a través del USBLC6; huella de 22 o 33 Ω en serie | Tabla 3-1, guía p.21 |
| 15 | IO3 | 3 | - | Sin conectar (strapping de JTAG; se ignora con los eFuse de fábrica) | Tabla 4-1, tabla 4-5 |
| 16 | IO46 | 46 | - | Sin conectar (strapping; pull-down interno) | Tabla 4-1, tabla 4-3 |
| 17 | IO9 | 9 | I2C_SCL | SCL del LSM6DS3TR-C (pin 13) y del MAX17048; pull-up de 4.7 kΩ | pinout.md |
| 18 | IO10 | 10 | LORA_NSS | NSS del módulo LoRa (pin 19). FSPICS0 | pinout.md |
| 19 | IO11 | 11 | LORA_MOSI | MOSI del módulo LoRa (pin 17). FSPID | pinout.md |
| 20 | IO12 | 12 | LORA_SCK | SCK del módulo LoRa (pin 18). FSPICLK | pinout.md |
| 21 | IO13 | 13 | LORA_MISO | MISO del módulo LoRa (pin 16). FSPIQ | pinout.md |
| 22 | IO14 | 14 | LORA_BUSY | BUSY del módulo LoRa (pin 14) | pinout.md |
| 23 | IO21 | 21 | GNSS_EXTINT | EXTINT del MAX-M10S (pin 5); opcional | pinout.md |
| 24 | IO47 | 47 | LORA_TXEN | Pin TX (7) del conmutador del módulo LoRa. POR DECIDIR | pinout.md |
| 25 | IO48 | 48 | RESERVA_F2 | Reserva de la Fase 2 (PWRKEY del módem) | pinout.md |
| 26 | IO45 | 45 | - | Sin conectar (strapping de VDD_SPI; pull-down interno) | Tabla 4-1, tabla 4-4 |
| 27 | IO0 | 0 | BOOT | 10 kΩ a 3V3; pad o botón a GND para el modo descarga | Tabla 4-1, tabla 4-3 |
| 28 | IO35 | 35 | - | Sin conectar: PSRAM octal del N16R8 | Tabla 3-1, nota b |
| 29 | IO36 | 36 | - | Sin conectar: PSRAM octal del N16R8 | Tabla 3-1, nota b |
| 30 | IO37 | 37 | - | Sin conectar: PSRAM octal del N16R8 | Tabla 3-1, nota b |
| 31 | IO38 | 38 | LED_R | Cátodo del LED rojo por 750 Ω; activo en bajo | pinout.md |
| 32 | IO39 | 39 | LED_G | Cátodo del LED verde por 470 Ω; activo en bajo | pinout.md |
| 33 | IO40 | 40 | LED_B | Cátodo del LED azul por 330 Ω; activo en bajo | pinout.md |
| 34 | IO41 | 41 | CHRG_STAT | CHRG del CN3791 (pin 3); pull-up de 10 kΩ a 3V3 | pinout.md |
| 35 | IO42 | 42 | LORA_RXEN | Pin RX (6) del conmutador del módulo LoRa. POR DECIDIR | pinout.md |
| 36 | RXD0 | 44 | UART0_RX | Punto de prueba (consola) | Tabla 3-1 |
| 37 | TXD0 | 43 | UART0_TX | 499 Ω en serie y punto de prueba (consola) | Tabla 3-1, figura 9-1 |
| 38 | IO2 | 2 | BUZZER_PWM | Zumbador con 100 Ω en serie (PWM de 4 kHz) | pinout.md |
| 39 | IO1 | 1 | OPTO_CTRL | Ánodo del VO617A (pin 1) por 330 Ω; pull-down de 100 kΩ | pinout.md |
| 40 | GND | - | GND | Plano de masa | Tabla 3-1 |
| 41 | EPAD | - | GND | Soldado a GND con vías al plano | Tabla 3-1, capítulo 9 |

### Reglas de conexión

1. **Alimentación:** el pin 2 se alimenta a 3.0 a 3.6 V (máximo absoluto 3.6 V) y el suministro debe dar al menos 0.5 A (tabla 6-2); el TLV75733P entrega 1 A. Un 22 µF y un 0.1 µF X7R van pegados al pin 2 (figura 9-1), más 10 µF a la entrada de alimentación (guía de hardware, p.8 y p.11). Los pines 1, 40 y el EPAD 41 van a GND con vías al plano. Soldar el EPAD no es obligatorio según el datasheet, pero mejora la térmica; con la cantidad justa de pasta.
2. **EN (pin 3):** 10 kΩ a 3V3 y 1 µF a GND (figura 9-1 y guía p.11), nunca flotante. Los umbrales son VIH de 0.75 x VDD y VIL de 0.25 x VDD (tabla 6-3). Con alimentación solar de subida lenta la guía sugiere un supervisor de unos 3.0 V (p.11 y p.12): pendiente.
3. **Strapping:** GPIO0 (pin 27) con 10 kΩ a 3V3 y un pad o botón a GND; a GND al soltar EN entra en descarga. GPIO46 ya está en 0 por defecto, así que para descargar solo hace falta GPIO0 a GND (tabla 4-3). GPIO3, GPIO45 y GPIO46 (pines 15, 26 y 16) quedan sin conectar. GPIO3 se ignora con los eFuse de fábrica (tabla 4-5), y por eso GPIO39 a GPIO42 son GPIO normales.
4. **USB nativo:** D- al pin 13 y D+ al pin 14, pasando por el USBLC6-2SC6; la guía pide huellas en serie de 22 o 33 Ω (se reservan, valor inicial a elegir) y condensadores a tierra sin montar (p.21). VBUS no alimenta al módulo: va solo al TP4056 y al USBLC6.
5. **UART0:** 499 Ω en serie en el pin 37 (TXD0, guía p.16) y puntos de prueba en los pines 36 y 37.
6. **Cristal de 32.768 kHz:** entre los pines 8 y 9 con dos cargas de 16.5 pF por pata (dos 33 pF en serie; ver `BOM.md` 1.8). ESR de 70 kΩ o menos (guía p.14). Sin cristal, GPIO15 y GPIO16 serían GPIO libres.
7. **Sin conectar:** pines 15, 16 y 26 (strapping) y 28 a 30 (PSRAM octal del N16R8). No unir nada a IO35, IO36 ni IO37.
8. **Despertar de deep sleep:** solo los RTC GPIO (GPIO0 a GPIO21) sirven; los pines de despertar del mapa (GPIO4, GPIO6 y GPIO7) cumplen.

### Pendientes del esquemático (sin decidir)

Lista única para todo el mapa de conexiones (actualizada el 2026-11-05). Ninguno lo decidió el asistente.

**Decisiones del autor**
1. **Zumbador:** el PKMCS0909E4000-R1 da unos 52 dB a 1 m y `mechanical/IP67_SPEC.md` pide más de 80 dBA. Cambiar el zumbador o el requisito (`COMPRAS.md` O14).
2. **Electrolítico de entrada del CN3791 (C1):** el de 100 µF 35 V del docente es de agujero pasante (6.3 x 12 mm). Sirve si cabe en la altura de la carcasa; si no, uno SMD por comprar.
3. **Bulk de 3.3 V:** el tantalio de 100 µF (1 o 2) más los cerámicos no pueden pasar de 200 µF a la salida del TLV75733P; hoy suman unos 32.5 µF.
4. **Protección de batería (decidido el 2026-11-05):** la batería lleva protección integrada (PCM) y no hay IC protector en la PCB. Batería provisional: cualquier 1S de 3.7 V con PCM y conector JST PH de 2 pines; la definitiva (una LiPo mejor) se consigue después por el costo de envío (`COMPRAS.md` O6).
5. **Panel solar (O7):** de 6 V y de 1 W a 1.5 W (decidido el 2026-11-05). Falta elegir el modelo; fija los valores finales de R3 y R4 del MPPT y de RCS (O22). Con 1 W la corriente es de 0.17 A y con 1.5 W de 0.25 A, así que un RCS de unos 0.47 Ω cubre todo el rango.
6. **LED de carga del TP4056:** opcional en CHRG, solo con USB.
7. **Supervisor de tensión en EN:** la guía de Espressif lo sugiere con alimentación solar de subida lenta.
8. **Salida del optoacoplador en la Fase 4:** hoy alimenta un LED desde el mismo 3V3; en la Fase 4 va a un conector aislado.
9. **Ubicación de los LED** bajo una ventana sellada (IP67).

**Confirmaciones antes de dibujar o al recibir las piezas**
- **Módulo LoRa:** si RX y TX son entradas del conmutador, si TX va unido a DIO2 por dentro, si trae cristal o TCXO, y cómo se elige entre el pad ANT y el IPEX. Preguntar al vendedor; la PCB ya admite los dos casos con los 0 Ω.
- **Polaridad de los JST:** medir los cables de la batería y del panel comprados y marcar "+" en la serigrafía.
- **Cristal ECS:** su ESR de 70 kΩ máximo es igual al límite del ESP32-S3 (sin margen). Comprobar en el banco que el oscilador del RTC arranca.
- **Pulsador del docente (TSC015):** el datasheet no declara rango de temperatura (el BOM pide -20 a +65 °C); confirmarlo con el fabricante o probarlo en el banco.
- **Condensadores del docente:** leer la hoja del 22 µF de Chinocera (la descripción de la biblioteca dice X5R ±20 %). Los 470 nF son X7R ±10 % según la biblioteca.
- **Línea de 50 Ω:** calcular el ancho con el apilado real del fabricante para la PCB de 0.8 mm.

**Por revisar antes de comprar o crear (auditoría del stock del docente, 2026-11-05; ver `hardware/AUDITORIA_STOCK_DOCENTE.md`)**
- **Schottky D1 y D2 del CN3791 (O20):** leer la fuga a 4.2 V del PSBD1DF40V1H del docente (LCSC C110730) antes de comprar.
- **Bulk de 3.3 V (O25):** el MPN TAJA107K016RNJ parece no existir; valorar el electrolítico de 100 µF del docente.
- **Diodo ESD de la alimentación:** evaluar el LESD5Z3.3T1G del docente.
- **Regulador:** bajar el datasheet del DS8570F8 del classroom (LCSC C5884245) y comparar su corriente en reposo con el presupuesto de sueño.
- **Huella del TP4056:** comparar la `SOIC_8_3.9mm_EP` del docente con el dibujo del ESOP-8.
- **Datasheets** de las piezas del docente que usamos: bajarlos a `hardware/datasheets/` (lista en la sección 8 de la auditoría).
- **Contradicciones** entre `BOM.md` y `COMPRAS.md` (sección 4 de la auditoría): corregirlas al cerrar las decisiones.

## Conexiones de periféricos

Los números de pin físico de los CI van solo donde se verificaron en la hoja de datos. Para el resto, tomarlos de la hoja de datos al dibujar el esquemático.

### GNSS u-blox MAX-M10S (datasheet R08 e integration manual; verificado el 2026-11-04)

18 pads, 10.1 x 9.7 x 2.5 mm. DS = datasheet, IM = integration manual (páginas del PDF).

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1, 10, 12 | GND | GND | DS p9 |
| 2 | TXD | GPIO18 (UART1 RX del ESP32) | DS p9 |
| 3 | RXD | GPIO17 (UART1 TX del ESP32); tiene pull-up interno | DS p9-10 |
| 4 | TIMEPULSE | Sin conectar: nada puede bajarlo al arrancar o el módulo entra en safeboot | IM p29, p90 |
| 5 | EXTINT | GPIO21 (opcional); pull-up interno | IM p29, p47 |
| 6 | V_BCKP | Sin conectar: no hay respaldo independiente | IM p73-74 |
| 7 | V_IO | 3V3, unido a VCC (figura 23 del manual) | IM p73-74 |
| 8 | VCC | 3V3 | IM p73-74 |
| 9 | RESET_N | Sin conectar (pull-up interno de 10 kΩ). Sin condensador a GND | DS p9, p12; IM p28 |
| 11 | RF_IN | Línea de 50 Ω (coplanar con masa en la capa 2, corta y cercada con vías) hasta el u.FL del parche; el bloqueo DC es interno | DS p12; IM p77, p81 |
| 13 | LNA_EN | Sin conectar (antena pasiva); no sirve para otra función | IM p7, p94 |
| 14 | VCC_RF | Sin conectar | IM p8 |
| 15 | VIO_SEL | Sin conectar, que selecciona V_IO de 3.3 V | DS p9; IM p73 |
| 16, 17 | SDA, SCL | Sin conectar (pull-ups internos; se usa UART) | DS p9-10 |
| 18 | SAFEBOOT_N | Sin conectar, con un pad de prueba para recuperar el firmware | DS p9; IM p28 |

- **Alimentación:** la resistencia en serie total del suministro debe ser de 0.2 Ω o menos, así que no lleva ferrita que la supere (IM p73). Pico de 100 mA al arrancar (DS p13). La rampa de V_IO debe estar entre 25 y 35 000 µs/V (DS p11); el TLV757P arranca en unos 550 µs, unos 167 µs/V (cálculo propio), y cumple.
- **Desacople:** el datasheet y el manual no dan valores. Recomendación propia: 100 nF X7R pegado al par VCC y V_IO, con el 10 µF del riel de 3.3 V cerca.
- **Interfaz:** UART e I2C están activos a la vez; no hay pin de selección. UART a 9600 8N1 por defecto; I2C se apaga por firmware con CFG-I2C-ENABLED (DS p14, p16; IM p24, p28).
- **Arranques en caliente:** con el 3.3 V siempre presente se consiguen con el standby por software (UBX-RXM-PMREQ), no con V_BCKP (IM p47).
- **Convivencia con el LoRa:** RF_IN soporta 0 dBm como máximo y su inmunidad a 915 MHz es de -17 dBm; el aislamiento entre antenas en equipos pequeños suele ser de 6 a 20 dB (DS p11; IM p79). Regla de firmware: no transmitir LoRa mientras el GNSS adquiere, o agregar un filtro SAW externo. Separar las antenas al menos 5 mm de otros elementos de RF (IM p80).
- **Corrección:** TIMEPULSE y SAFEBOOT_N son pines distintos (4 y 18), unidos por dentro con 1 kΩ (DS p9, nota 15); la versión anterior decía que compartían pin.

### IMU LSM6DS3TR-C (datasheet DocID030071 Rev 3; verificado el 2026-11-04)

LGA-14, 2.5 x 3.0 mm. Modo I2C.

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1 | SDO/SA0 | GND: dirección 0x6A. No tiene pull-up interno, no puede quedar flotante | p20, p39, p47 |
| 2, 3 | SDx, SCx | GND (modo 1, figura 15) | p20, p45 |
| 4 | INT1 | GPIO4. Push-pull y activo en alto por defecto: sin resistencia | p20, p63 |
| 5 | VDDIO | 3V3 con 100 nF a GND pegado al pin | p20, p45 |
| 6, 7 | GND | GND | p20 |
| 8 | VDD | 3V3 con 100 nF a GND pegado al pin | p45 |
| 9 | INT2 | Sin conectar | p47 |
| 10, 11 | NC | Sin conectar, pero soldados (en el LSM6DSOX son auxiliares; la huella sirve para los dos) | p20 |
| 12 | CS | VDDIO (selecciona I2C) | p38, p47 |
| 13 | SCL | GPIO9 | p20 |
| 14 | SDA | GPIO8 | p20 |

Los pull-ups del bus I2C son los 4.7 kΩ a 3V3 compartidos con el MAX17048 (el datasheet muestra 10 kΩ en su figura, p45). WHO_AM_I lee 0x6A (p49).

### Módulo LoRa DX-LR30-900M22S (folleto del vendedor, `hardware/datasheets/lr30-900m22s_vendor_brochure.pdf`; sin datasheet)

Variante a comprar: **LR30-900M22S** (módulo SMD de 20 x 14 x 2.3 mm, paso de 1.27 mm, 1.8 a 3.7 V). No la "SP", que es una placa con STM32 y UART. **Todo lo de esta tabla sale de material de marketing (folleto p5 y p11) y no está verificado.**

| Pad | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1-4, 10-12, 20, 22 | GND | GND | folleto p11 |
| 5 | DIO3 | Sin conectar. Si el módulo trae TCXO, DIO3 lo alimenta (`SetDIO3AsTCXOCtrl`); se confirma en el arranque leyendo GetDeviceErrors (XOSC_START_ERR) | folleto p11; SX1262 p81-82 |
| 6 | RX (conmutador de antena) | GPIO42 (LORA_RXEN) a través de un 0 Ω | folleto p11 |
| 7 | TX (conmutador de antena) | GPIO47 (LORA_TXEN) a través de un 0 Ω | folleto p11 |
| 8 | DIO2 | Sin conectar, con un 0 Ω **sin montar** hacia el pad 7 | folleto p11; SX1262 p54, p81 |
| 9 | VCC | 3V3 con 10 µF y 100 nF pegados al pad (recomendación propia: el SX1262 pide 118 mA en TX a +22 dBm) | SX1262 p17 |
| 13 | DIO1 | GPIO6 (interrupción, despierta de deep sleep) | folleto p11 |
| 14 | BUSY | GPIO14, sin pull-down: en sleep el chip lo mantiene en alto con 20 kΩ y un pull-down gastaría unos 165 µA | SX1262 p52 |
| 15 | RESET | GPIO5. Pull-up interno de unos 50 kΩ: no forzarlo a bajo en deep sleep (gastaría unos 66 µA) | SX1262 p50, p54 |
| 16 | MISO | GPIO13 | folleto p11 |
| 17 | MOSI | GPIO11 | folleto p11 |
| 18 | SCK | GPIO12 | folleto p11 |
| 19 | NSS | GPIO10. Mantenerlo en alto en deep sleep (retención de GPIO): un flanco de bajada despierta la radio | SX1262 p51 |
| 21 | ANT | Línea de 50 Ω hasta el conector u.FL de la PCB | folleto p9, p11 |

- **Por qué los 0 Ω:** el folleto no dice si RX y TX son entradas del conmutador ni si TX va unido a DIO2 por dentro. Con 0 Ω en los pads 6 y 7, y un 0 Ω sin montar entre DIO2 y TX, la misma PCB sirve para los dos diseños posibles sin choque de salidas. Si DIO2 gobierna el conmutador, se usa `SetDIO2AsRfSwitchCtrl` (SX1262 p81), se quita el 0 Ω del pad 7 y GPIO47 queda libre.
- **Antes de energizar:** conectar la antena (folleto p5).
- **Antena:** el módulo trae también un conector IPEX; por defecto sale por el pad ANT (folleto p9). Cómo se elige una u otra salida no lo dice el folleto: preguntar al vendedor.
- **Niveles:** las entradas del SX1262 aceptan 3.3 V (VIH de 0.7 x VBAT_IO; SX1262 p21).

### Regulador TLV75733PDBVR (datasheet TLV757P; verificado el 2026-11-04)

SOT-23-5 (DBV), sin pad térmico.

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1 | IN | Nodo de batería (VBAT) con 10 µF X5R 25 V a GND (pide 1 µF o más) | p4, p15 |
| 2 | GND | GND | p3 |
| 3 | EN | Unido a IN (sin apagado) | p12 |
| 4 | NC | Sin conectar (no tiene conexión interna) | p3 |
| 5 | OUT | Riel de 3V3 con 1 µF o más a GND | p4, p15 |

La salida admite de 1 a 200 µF en total (p4). Suma actual en el riel de 3.3 V: 22 µF y 0.1 µF del ESP32, 10 µF y 100 nF del LoRa, dos 100 nF del IMU y 100 nF del GNSS, unos 32.5 µF; el bulk de 100 µF de tantalio sigue **POR DECIDIR** y el total no debe pasar de 200 µF. Caída máxima de 425 mV a 1 A (p6). Con el regulador apagado o bajo UVLO, la salida se descarga con unos 120 Ω (p12).

### Medidor de batería MAX17048 (datasheet verificado el 2026-11-04)

TDFN-8 de 2 x 2 mm con pad expuesto.

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1 | CTG | GND | p6 |
| 2 | CELL | Positivo de la batería | p6, p15 |
| 3 | VDD | Positivo de la batería con 0.1 µF a GND | p15 |
| 4 | GND | GND (negativo de la batería) | p6 |
| 5 | ALRT | Sin conectar (no queda GPIO libre; el datasheet lo permite) | p15 |
| 6 | QSTRT | GND | p6 |
| 7 | SCL | GPIO9 | p6 |
| 8 | SDA | GPIO8 | p6 |
| EP | Pad expuesto | GND | p6 |

SDA y SCL soportan hasta 5.5 V y VIH es de 1.4 V, así que el bus de 3.3 V funciona con VDD conectado a la batería (p2). Dirección 0x36 (p16). **Firmware:** poner VRESET en 2.5 V y VRESET.Dis = 1; con el valor por defecto (Dis = 0) la hibernación consume 4 µA y no los 3 µA del presupuesto (p2, p11, p13).

### Cargador USB-C TP4056 (pines verificados el 2026-11-04)

ESOP-8 con pad térmico.

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1 | TEMP | GND (sin corte por temperatura, decisión del 2026-11-03) | p2 |
| 2 | PROG | 2.4 kΩ a GND (500 mA, RNF-05) | p2 |
| 3 | GND | GND | p2 |
| 4 | VCC | VBUS del USB-C después del USBLC6, con 10 µF a GND | p3 |
| 5 | BAT | Nodo de batería con 10 µF a GND | p3 |
| 6 | STDBY | Sin conectar (drenador abierto; no queda GPIO libre) | p3 |
| 7 | CHRG | Sin conectar (drenador abierto). Opción: LED rojo 0805 del docente con 1 kΩ a VCC, que solo enciende con USB | p3 |
| 8 | CE | VCC (como en el circuito típico) | p3 |
| Pad | Térmico | GND, con cobre y vías para disipar | p1 |

La resistencia opcional de 0.2 a 0.5 Ω en serie con la entrada (p3) no se monta. Con USB desconectado el pin BAT consume menos de 2 µA (p2).

### Cargador solar CN3791 y su etapa de potencia (datasheet verificado el 2026-11-04)

SSOP de 10 pines con paso de 1.00 mm y sin pad expuesto (p11); no usar una huella MSOP de 0.5 mm.

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1 | VG | 100 nF X7R a VCC (C2) | p2, p3 |
| 2 | GND | GND, con cobre amplio (es el disipador) | p3, p10 |
| 3 | CHRG | GPIO41 (CHRG_STAT) con 10 kΩ a 3V3; drenador abierto hasta 30 V | p3-4 |
| 4 | DONE | GND (salida de estado que no se usa) | p7 |
| 5 | COM | 120 Ω en serie con 235 nF X7R (dos 470 nF X7R del docente en serie) a GND (R2 y C4); su masa vuelve al pin GND | p2, p10 |
| 6 | MPPT | Divisor: R3 de VCC a MPPT = 390 kΩ + 10 kΩ; R4 de MPPT a GND = 100 kΩ. V_MPPT = 1.205 x (1 + 400/100) = 6.025 V | p6 |
| 7 | BAT | Lado de la batería de RCS, con conexión Kelvin | p10 |
| 8 | CSP | Lado del inductor de RCS, con conexión Kelvin | p10 |
| 9 | VCC | Positivo del panel, con C1 | p2 |
| 10 | DRV | Gate de M1 | p2 |

**Etapa de potencia (figura 1, p2):** positivo del panel → VCC y source de M1 (AP2301, gate a DRV) → drain de M1 al ánodo de D1 → cátodo de D1 al nodo de conmutación → inductor L → CSP → RCS → BAT y nodo de batería. D2 va del nodo de conmutación (cátodo) a GND (ánodo).

| Parte | Valor | Por qué | Fuente |
|-------|-------|---------|--------|
| M1 | AP2301 del docente | Canal P de nivel lógico; Vgs de ±12 V frente al recorte de 8 V del CN3791; Vds de -20 V | p7-8; hoja del AP2301 |
| D1 | Schottky de baja fuga (O20), **montado** | Sin D1, de noche la batería alimenta el divisor MPPT (unos 8 µA) y el panel, y pierde unos 21 µA por el diodo del MOSFET. Con D1 solo queda su fuga | p9 |
| D2 | Schottky de baja fuga (O20) | Diodo de rueda libre; de noche queda en inversa a VBAT | p9 |
| L | **47 µH**, CY54-47UH del docente (0.71 A, 355 mΩ) | La regla L > 5 x (VCC - VBAT) µH pide más de 25 µH con 8 V de panel en circuito abierto y 3.0 V de batería; el de 22 µH quedaba al límite. Rizado de unos 0.10 A a 6 V y 3.7 V (cálculo propio); sirve hasta unos 0.6 A de carga | p8 |
| RCS | 0.12 V / I de carga (O22); con un panel de 1 W, unos 0.25 A, sale de unos 0.47 Ω en 0805 | ICH = 120 mV / RCS, ±8 % | p4, p6 |
| C1 | Electrolítico más 10 µF X5R 25 V más 470 nF | El datasheet pide los tres en paralelo. El electrolítico de 100 µF 35 V del docente es de agujero pasante (6.3 x 12 mm): sirve si cabe en altura; si no, uno SMD por comprar (**POR DECIDIR**) | p8 |
| C2 | 100 nF X7R de VG a VCC | | p2 |
| C3 | Dos 10 µF X5R 25 V en BAT | El datasheet pide electrolítico más cerámico; solo cerámico es válido con más capacidad, porque la batería siempre está conectada | p8 |
| R2 y C4 | 120 Ω y 235 nF X7R (dos 470 nF en serie; el datasheet pide 220 nF) | Red de compensación de COM | p2 |
| R3 y R4 | 390 kΩ + 10 kΩ y 100 kΩ | MPPT de 6.025 V; la carga arranca con más de 6.15 V en la entrada. El valor final depende de la tensión de máxima potencia del panel (O7) | p5, p6 |

**Layout (p9-10):** lazo de diodos, MOSFET, inductor y C1 lo más corto posible, con el positivo de C1 junto al source de M1; masas de C3 y D2 al cobre de masa de C1 antes de ir a la masa del sistema; masa analógica y de potencia separadas hasta un punto; RCS junto a la salida del inductor, con CSP y BAT como par apretado (Kelvin). No conectar la batería con el cargador funcionando (p7-8).

### Optoacoplador VO617A (datasheet Rev 2.8; verificado el 2026-11-04)

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 1 | Ánodo | GPIO1 (OPTO_CTRL) a través de 330 Ω; pull-down de 100 kΩ de GPIO1 a GND, antes de la resistencia | p1 |
| 2 | Cátodo | GND | p1 |
| 3 | Emisor | GND | p1 |
| 4 | Colector | Cátodo de un LED rojo 0805 del docente; ánodo del LED a 3V3 a través de 1 kΩ | p1 |

- **Entrada:** VF de unos 1.1 V a 5 mA (figura 5, p6; la tabla solo da VF a 60 mA). Con 330 Ω: (3.3 - 1.1) / 330 = 6.7 mA típico; en el peor caso (VOH de 2.64 V y VF de 1.2 V) 4.4 mA. Máximo de 60 mA.
- **Salida (RF-03, indicador):** IC = (3.3 - 2.0 - 0.4) / 1 kΩ = 0.9 mA. Con el bin -1 (CTR de 40 a 80 %) y 0.85 de factor a 65 °C, la corriente mínima disponible es 0.40 x 6.7 mA x 0.85 = 2.3 mA, así que el transistor satura (p2, p6).
- **Aislamiento:** con el LED alimentado desde el mismo 3V3 la salida no queda aislada. Basta para la simulación de la Fase 3. En la Fase 4 el colector y el emisor van a un conector hacia el generador aislado.
- **Código de pedido:** el sufijo "-1-E" del BOM no aparece en la tabla de pedidos del datasheet (p2); la versión SMD del bin -1 es VO617A-1X017T.

### Zumbador Murata PKMCS0909E4000-R1 (hoja de Murata del 2025-03-11)

Dos terminales SMD sin polaridad marcada (piezo).

| Terminal | Conexión | Fuente |
|----------|----------|--------|
| 1 | GPIO2 (BUZZER_PWM) a través de 100 Ω | hoja de Murata p1-3 |
| 2 | GND | hoja de Murata p1 |

- Excitación externa con onda cuadrada de 4 kHz. Una salida de 0 a 3.3 V equivale a ±1.65 V, algo más que los ±1.5 V con los que Murata mide 72 dB típicos a 10 cm (p2). Con 100 Ω el pico de corriente es de 33 mA, por debajo de los 40 mA del GPIO, y la media es de unos 0.12 mA (cálculo propio con unos 9 nF, valor de un distribuidor sin verificar).
- **Requisito incumplido:** `mechanical/IP67_SPEC.md` pide más de 80 dBA a 1 m. Con 72 dB a 10 cm este zumbador da unos 52 dB a 1 m, y unos 70 dB incluso a su máximo de ±12.5 V. **POR DECIDIR:** cambiar el zumbador o el requisito.

### LED RGB NationStar RS-3535MWAM (spec B2; verificado el 2026-11-04)

6 pines independientes; el pin 1 está en la esquina marcada "A". Ánodos a 3V3 y cátodos al GPIO a través de la resistencia: **activo en bajo**.

| Pin | Nombre | Conexión | Fuente |
|-----|--------|----------|--------|
| 4 | R+ | 3V3 | p6 |
| 3 | R- | GPIO38 (LED_R) a través de 750 Ω | p6 |
| 5 | G+ | 3V3 | p6 |
| 2 | G- | GPIO39 (LED_G) a través de 470 Ω | p6 |
| 6 | B+ | 3V3 | p6 |
| 1 | B- | GPIO40 (LED_B) a través de 330 Ω | p6 |

- **Cálculo a unos 2 mA por color**, con la VF de la curva VF-IF a 25 °C (p3): rojo 1.83 V, verde 2.35 V, azul 2.62 V a 2 mA. R = (3.3 - VF - 0.04) / 2 mA da 715, 455 y 320 Ω, redondeados a 750, 470 y 330 Ω. Los 750 y 470 Ω salen del kit (O19) y el 330 Ω es del docente.
- **Condición de la sección 1 de `COMPRAS.md` (D7) resuelta:** la VF de verde y azul a la corriente elegida queda por debajo de 3.0 V, así que el ánodo va a 3V3 y no hace falta VBAT. Con un coeficiente típico de InGaN, a 0 °C quedan 1.6 a 1.8 mA (estimación: el datasheet no da el coeficiente).
- El firmware enciende cada color con el GPIO en bajo.

### Botón de usuario BZCN TSC015A05026A (del docente, 4.5 x 4.5 mm)

| Terminal | Conexión | Fuente |
|----------|----------|--------|
| 1 (los dos pads con el nombre 1) | GND | regla de cableado; datasheet de BZCN p1 (circuito: terminales 1 y 2 unidos) |
| 2 (los dos pads con el nombre 2) | Nodo N: 10 kΩ a 3V3; N a GPIO7 a través de 1 kΩ; 100 nF X7R de GPIO7 a GND | cálculo propio; datasheet de BZCN p1 (terminales 3 y 4 unidos) |

- El símbolo del docente tiene 2 pines y la huella 4 pads: los dos pads llamados 1 son los terminales 1 y 2 del datasheet (siempre unidos) y los dos llamados 2 son los terminales 3 y 4 (siempre unidos). Al pulsar se conecta una pareja con la otra.
- Corriente nominal de 50 mA a 12 V DC (datasheet de BZCN p1); esta línea maneja unos 0.33 mA a 3.3 V. Fuerza de 260 gf ±50. El datasheet no declara rango de temperatura.
- Antirrebote: al soltar, τ = 11 kΩ x 100 nF = 1.1 ms y llega a VIH en 1.5 ms; al pulsar, τ = 100 µs. Agregar unos 20 ms de antirrebote por firmware. Sin consumo en sleep con el botón suelto.
- EN y GPIO0 llevan además pads de prueba para reset y modo descarga (ver la sección del módulo).

### Cristal de 32.768 kHz ECS-.327-12.5-34B-TR (ECX-31B)

| Pad | Conexión | Fuente |
|-----|----------|--------|
| 1 | GPIO15 (pin 8 del módulo) y su carga a GND | p1 (PAD CONNECTIONS: In/Out) |
| 2 | GPIO16 (pin 9 del módulo) y su carga a GND | p1 (PAD CONNECTIONS: Out/In) |

- Carga de cada pata: dos 33 pF NP0 del docente en serie (16.5 pF). CL = 16.5 / 2 + 3 a 5 pF parásitos = 11.3 a 13.3 pF, frente a 12.5 pF.
- Sin resistencia de polarización (guía de Espressif p14). ESR de 70 kΩ como máximo (datasheet de ECS p1), igual al límite de 70 kΩ del ESP32-S3 (guía de Espressif p11): cumple, sin margen.
- El cristal no tiene polaridad: los dos pads son intercambiables. La huella sugerida es de dos pads de 1.1 x 1.9 mm separados 1.4 mm (p1). La potencia de excitación máxima es de 1.0 µW (no comparada con la del ESP32-S3).

### USB-C (Shou Han TYPE-C-3.1-16PIN) y protección ESD (USBLC6-2SC6)

| Pin del USB-C | Red | Conexión | Fuente |
|---------------|-----|----------|--------|
| A4, B9, A9, B4 | VBUS | Pin 5 del USBLC6 y pin 4 (VCC) del TP4056; nunca al LDO | hoja de Shou Han p1 |
| A1, B12, A12, B1 | GND | GND | hoja de Shou Han p1 |
| A6, B6 | D+ | Unidos junto al conector (cada tramo de unión de menos de 3.5 mm) y al pin 1 del USBLC6 | ST AN4871 p7 |
| A7, B7 | D- | Unidos igual y al pin 3 del USBLC6 | ST AN4871 p7 |
| A5 | CC1 | 5.1 kΩ a GND, propia | ST AN4871 |
| B5 | CC2 | 5.1 kΩ a GND, propia (no compartir una resistencia entre CC1 y CC2) | ST AN4871 |
| A8, B8 | SBU1, SBU2 | Sin conectar | práctica habitual (sin guía oficial) |
| Carcasa (4 patas) | GND | Directa a GND; huella opcional de 1 MΩ en paralelo con 4.7 nF | práctica habitual (sin guía oficial) |

| Pin del USBLC6 | Conexión | Fuente |
|----------------|----------|--------|
| 1 (I/O1) | D+ desde el conector | ST p1, p5 |
| 6 (I/O1) | D+ hacia el módulo: 22 Ω en serie y pin 14 (GPIO20) | ST p5, p8; Espressif p21 |
| 3 (I/O2) | D- desde el conector | ST p1, p5 |
| 4 (I/O2) | D- hacia el módulo: 22 Ω en serie y pin 13 (GPIO19) | ST p5, p8; Espressif p21 |
| 5 (VBUS) | VBUS, sin tramo muerto | ST p5 |
| 2 (GND) | GND con pistas cortas al plano | ST p5 |

- Enrutado de paso (entra por un pin y sale por el opuesto) y el USBLC6 lo más cerca posible del conector (ST p5-6). Las resistencias de 22 Ω van del lado del ESP32, cerca del módulo (ST p8; Espressif p21); salen del kit (O19).
- Con el plan B de piezas del docente, CC1 y CC2 llevan un 4.7 kΩ cada una (ver `COMPRAS.md` 1.2).

### Conectores de batería y panel (JST S2B-PH-K-S)

| Pin | Batería | Panel solar | Fuente |
|-----|---------|-------------|--------|
| 1 | Negativo (GND) | Negativo (GND) | elección de diseño |
| 2 | Positivo (VBAT: protección, TP4056 BAT, CN3791 BAT por RCS, MAX17048 y TLV75733P IN) | Positivo (VCC del CN3791) | elección de diseño |

- JST no define polaridad y los fabricantes de baterías usan las dos. **Antes de soldar, medir los cables de la batería y del panel que se compren** y marcar "+" en la serigrafía. Usar el mismo orden en los dos conectores.
- 2 A, 100 V, -40 a +105 °C, para PCB de 0.8 a 1.6 mm (jst_b2b-ph-k-s.pdf p1).

### Conectores u.FL (Hirose U.FL-R-SMT-1), LoRa y GNSS

| Pad | Conexión | Fuente |
|-----|----------|--------|
| Central (señal) | Línea de 50 Ω desde el pad ANT del LoRa o desde RF_IN del MAX-M10S | catálogo de Hirose p2-3 |
| Dos laterales | GND, con vías al plano | catálogo de Hirose p3 |

- Sin pistas bajo el cuerpo del conector (p3). 50 Ω, de DC a 6 GHz, 30 ciclos de conexión.
- Ancho de la línea de 50 Ω: depende del apilado del fabricante. Como referencia, una microstrip sobre FR4 de 0.8 mm (εr 4.5) da unos 1.5 mm por la fórmula de Hammerstad (cálculo aproximado); se prefiere coplanar con masa y vías junto a los pads de masa. Confirmar con la calculadora del fabricante.

### Protección de batería

Pieza por identificar: el `BQ2975DRBR` del BOM no existe (O16). Recomendación: comprar una batería con circuito de protección integrado (PCM), así no hace falta protección en la PCB. **POR DECIDIR** con la compra de la batería (O6).

## Consideraciones de diseño

**Reglas de firmware que salen del mapa de conexiones (2026-11-04):**
- LoRa en deep sleep: NSS (GPIO10) retenido en alto, RESET (GPIO5) sin forzar a bajo y BUSY (GPIO14) sin pull-down; si no, se pierden de 66 a 165 µA.
- MAX17048: VRESET en 2.5 V y VRESET.Dis = 1, para 3 µA en hibernación.
- No transmitir LoRa mientras el GNSS adquiere (RF_IN soporta 0 dBm como máximo).
- LED RGB activo en bajo; zumbador con PWM de 4 kHz; antirrebote de unos 20 ms en el botón.
- IMU: XL_HM_MODE = 1 para el modo de 9 µA (ver `docs/firmware_docs/REAL_TIME_RISKS.md`).

1. Niveles lógicos de 3.3 V en todos los pines.
2. Fuentes de despertar de deep sleep: IMU_INT1 (GPIO4), LORA_DIO1 (GPIO6), BUTTON (GPIO7). EXTINT es una entrada del GNSS y no despierta al ESP32.
3. Mantener `OPTO_CTRL` en bajo durante deep sleep con la retención de GPIO RTC (`rtc_gpio_hold_en`) además del pull-down externo.
4. Pines SPI con longitudes de pista similares y lejos de la antena del módulo.
5. Puntos de prueba en TXD0, RXD0, EN, GPIO0, 3V3 y GND.

## Modos de arranque

| Modo | GPIO0 | Descripción |
|------|-------|-------------|
| Normal | Alto (pull-up) | Arranca desde flash |
| Descarga | Bajo al soltar EN | Programación por USB nativo (GPIO19/20) o UART0 |

## Revisión

| Versión | Fecha | Cambios |
|---------|-------|---------|
| 1.0-1.1 | 2024 | Diseño inicial (con GPIO inexistentes y duplicados) |
| 2.1 | 2026-10-30 | Pines del módulo LoRa DX-LR30 verificados (RXEN/TXEN en GPIO42/47). Fase 2 pasa a UART0 (GPIO43/44) con PWRKEY en GPIO48. Corregido: EXTINT es entrada del GNSS, no fuente de despertar |
| 2.2 | 2026-10-31 | Secciones de MAX17048, TP4056, CN3791, USB-C y protección de batería; corregido el candidato de GPIO del TP4056 (GPIO42 es LORA_RXEN); nota de IMU; folleto del módulo LoRa movido a `hardware/datasheets/` |
| 2.3 | 2026-11-04 | Sección de conexiones del módulo por pin físico (alimentación, EN, strapping, USB, UART0, cristal y pines sin conectar), verificada contra la tabla 3-1; CSV para crear el símbolo de Altium |
| 2.4 | 2026-11-04 | Conexiones pin por pin de todos los componentes aceptados, verificadas con sus datasheets: MAX-M10S, LSM6DS3TR-C, LoRa (folleto), TLV75733P, MAX17048, TP4056, CN3791 con su etapa de potencia (inductor de 47 µH, D1 montado), VO617A, zumbador, LED RGB activo en bajo, botón, cristal, USB-C con USBLC6, JST y u.FL. CSV conjunto en `hardware/pin_connections.csv`. Lista única de pendientes |
| 2.5 | 2026-11-05 | Cristal: ECS-.327-12.5-34B-TR (ECX-31B de 2 pads) en lugar del ECX-306X de 4 pads; el datasheet que había en el repo era de otro encapsulado. CSV actualizado |
| 2.6 | 2026-11-05 | Pulsador: BZCN TSC015A05026A del docente (4.5 x 4.5 mm; terminales 1-2 y 3-4 unidos) en lugar del C&K PTS645; CSV actualizado |
| 2.7 | 2026-11-05 | Red COM del CN3791 con dos 470 nF X7R del docente en serie (235 nF) en lugar de un 220 nF comprado; CSV actualizado |
| 2.8 | 2026-11-05 | Lista de pendientes actualizada: batería provisional con protección integrada (sin IC protector), panel de 6 V de 1 a 1.5 W y lista de lo que se revisa antes de comprar |
| 2.0 | 2026-10-30 | Rehecho contra el datasheet v1.8 del módulo. GPIO05 ya no se repite. Sin GPIO22-34. USB D+/D- corregidos. GNSS con TX/RX cruzados |
