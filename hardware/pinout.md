# Asignación de Pines del Collar DR-VF-01

**Estado:** PROPUESTA v2.2 (2026-10-31), pendiente de revisión del autor. Es el **único mapa de pines** del proyecto: la PCB y la ESP32-S3-DevKitC-1 (N16R8) usan los mismos GPIO para que el firmware sea idéntico en ambas. Cualquier otro mapa en `docs/` o `reports/` debe remitir a este archivo.

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
| 1 | 39 | OPTO_CTRL | VO617A (entrada) | RTC. Resistencia serie y pull-down 100 kΩ para que arranque apagado |
| 2 | 38 | BUZZER_PWM | Buzzer | RTC. PWM, vía transistor si el buzzer lo requiere |
| 3 | 15 | (reservado) | — | Strapping JTAG. No conectar |
| 4 | 4 | IMU_INT1 | LSM6DSOX INT1 | **RTC, despierta de deep sleep** |
| 5 | 5 | LORA_RESET | SX1262 NRESET | RTC |
| 6 | 6 | LORA_DIO1 | SX1262 DIO1 | **RTC, interrupción / despertar** |
| 7 | 7 | BUTTON | Botón de usuario | **RTC, despierta**. Pull-up 10 kΩ + RC antirrebote |
| 8 | 12 | I2C_SDA | LSM6DSOX, MAX17048 | Pull-up 4.7 kΩ |
| 9 | 17 | I2C_SCL | LSM6DSOX, MAX17048 | Pull-up 4.7 kΩ |
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
| 38 | 31 | LED_R | LED RGB rojo | Resistencia serie. En DevKitC-1 v1.1 coincide con el LED integrado |
| 39 | 32 | LED_G | LED RGB verde | Resistencia serie |
| 40 | 33 | LED_B | LED RGB azul | Resistencia serie |
| 41 | 34 | CHRG_STAT | CN3791 CHRG | Colector abierto: pull-up 10 kΩ |
| 42 | 35 | LORA_RXEN | Módulo LoRa, pin RX (pin 6, conmutador de antena) | Salida. **POR DECIDIR** |
| 43 | 37 | TXD0 | Punto de prueba / UART TX del módem de Fase 2 | Consola; en Fase 2 la consola pasa a USB CDC. **POR DECIDIR** |
| 44 | 36 | RXD0 | Punto de prueba / UART RX del módem de Fase 2 | Ídem |
| 45, 46 | 26, 16 | (reservados) | — | Strapping. No conectar |
| 47 | 24 | LORA_TXEN | Módulo LoRa, pin TX (pin 7, conmutador de antena) | Salida. Si el conmutador lo gobierna DIO2 (pin 8 unido a pin 7), dejar libre |
| 48 | 25 | RESERVA_F2 | Módem Fase 2 (PWRKEY) | Libre en Fase 1 |

**Señales retiradas del mapa anterior:** `POWER_GOOD` y `ENABLE_CHG` (CN3791). El datasheet del CN3791 (`hardware/datasheets/cn3791_datasheet.pdf`) confirma que no tiene esos pines: sus pines son VG, GND, CHRG, DONE, COM, MPPT, BAT, CSP, VCC y DRV, y CHRG y DONE son salidas de drenador abierto. El TP4056 (carga por USB-C) tiene dos salidas de estado de drenador abierto (pines 6 y 7), pero no queda un GPIO libre para leerlas: ver la sección del TP4056.

## Conexiones de periféricos

Los números de pin físico de los CI van solo donde se verificaron en la hoja de datos. Para el resto, tomarlos de la hoja de datos al dibujar el esquemático.

### GNSS u-blox MAX-M10S (pines verificados)

| MAX-M10S | Pin | ESP32-S3 |
|----------|-----|----------|
| TXD | 2 | GPIO18 (UART1 RX) |
| RXD | 3 | GPIO17 (UART1 TX) |
| TIMEPULSE | 4 | sin conectar (comparte pin con SAFEBOOT_N; si se usa, resistencia serie de 1 kΩ) |
| EXTINT | 5 | GPIO21 |
| V_BCKP | 6 | batería de respaldo o VCC (ver hoja de datos) |
| VCC | 8 | 3V3, 1.76 a 3.6 V |
| RESET_N | 9 | sin conectar o a un GPIO libre (tiene pull-up interno de 10 kΩ) |

### IMU LSM6DSOX

SDA y SCL a GPIO8 y GPIO9. INT1 a GPIO4. Para I2C: CS a VDDIO, SDO/SA0 define la dirección (0x6A o 0x6B). Salida INT configurable como push-pull u open-drain: si se deja push-pull no se necesita pull-up externo en GPIO4.

Las asignaciones de SDA, SCL e INT1 valen para el LSM6DSOX y para el LSM6DS3TR-C del docente una vez verificadas en el datasheet de cada uno; la elección de IMU es **POR DECIDIR** (`BOM.md`).

### Módulo LoRa DX-LR30-900M22S (SX1262, SPI; pines tomados del folleto del vendedor, `hardware/datasheets/lr30-900m22s_vendor_brochure.pdf`)

Variante a comprar: **LR30-900M22S** (módulo SMD de 20x14x2.3 mm, 1.8 a 3.7 V). No la "SP", que es una placa con STM32 y UART.

| Módulo | Pin | ESP32-S3 |
|--------|-----|----------|
| NSS | 19 | GPIO10 |
| SCK | 18 | GPIO12 |
| MOSI | 17 | GPIO11 |
| MISO | 16 | GPIO13 |
| RESET | 15 | GPIO5 |
| BUSY | 14 | GPIO14 |
| DIO1 | 13 | GPIO6 |
| RX (conmutador de antena) | 6 | GPIO42 (LORA_RXEN) |
| TX (conmutador de antena) | 7 | GPIO47 (LORA_TXEN) |
| DIO2 | 8 | sin conectar, o unido al pin 7 si DIO2 gobierna el conmutador (`SetDio2AsRfSwitchCtrl`) |
| DIO3 | 5 | alimentación del TCXO externo: sin conectar si el módulo usa cristal |
| VCC | 9 | 3V3 |
| ANT | 21 | línea de 50 ohm hacia el conector u.FL de la PCB |
| GND | 1, 2, 3, 4, 10, 11, 12, 20, 22 | GND |

El módulo trae su propio conector u.FL, pero la antena por defecto sale por el pad ANT (stamp hole). Pendiente de confirmar con el vendedor: si el pin TX va unido a DIO2 por dentro, qué hace DIO2 en la práctica, y si el cristal de 32 MHz es TCXO. La ficha exige conectar la antena antes de energizar. Zephyr usa `semtech,sx1262` con `busy-gpios`, `dio1-gpios`, `tx-enable-gpios`, `rx-enable-gpios`, `dio2-tx-enable` y `dio3-tcxo-voltage`.

### Optoacoplador VO617A

GPIO1 → resistencia serie → LED de entrada. Calcular `R = (3.3 V − V_F) / I_F` con los valores del datasheet del VO617A. El pull-down de 100 kΩ en GPIO1 mantiene la salida apagada durante reset y deep sleep.

### Cristal RTC (opcional pero recomendado)

32.768 kHz entre GPIO15 y GPIO16 con sus dos capacitores de carga. Ver `BOM.md`, sección Pasivos.

### Medidor de batería MAX17048 (datasheet verificado)

SDA y SCL en el mismo bus I2C que el IMU (GPIO8 y GPIO9). Dirección de 7 bits 0x36 (0110110), sin conflicto con el LSM6DSOX (0x6A o 0x6B). VDD a la batería (2.5 a 4.5 V), no a 3V3. ALRT (drenador abierto): sin GPIO asignado; **POR DECIDIR** si se usa.

### Cargador USB-C TP4056 (pines verificados)

| TP4056 | Pin | Conexión |
|--------|-----|----------|
| TEMP | 1 | NTC del paquete de batería; a GND si no se usa |
| PROG | 2 | RPROG de 2.4 kΩ a GND (500 mA) |
| GND | 3 | GND |
| VCC | 4 | VBUS del USB-C, después de la protección ESD (4.0 a 8.0 V) |
| BAT | 5 | batería, después de la protección |
| Salida de carga terminada | 6 | drenador abierto; sin GPIO asignado |
| Salida de carga en curso | 7 | drenador abierto; sin GPIO asignado |
| CE | 8 | nivel alto = operación normal |

Ningún GPIO del módulo queda libre para leer el estado (GPIO42 es LORA_RXEN y todos los GPIO0-21 y 38-48 están asignados). Opciones por decidir: GPIO47 si DIO2 gobierna el conmutador de antena, GPIO48 en la Fase 1, o dejar el estado solo en un LED.

### Cargador solar CN3791 (pines verificados)

CHRG (pin 3) a GPIO41 como CHRG_STAT. DONE (pin 4): la salida de estado que no se use se conecta a GND (datasheet del CN3791). Sin pines de temperatura, habilitación ni power-good.

### USB-C y protección ESD

D+ a GPIO20 y D- a GPIO19 (USB nativo, para programación). CC1 y CC2 con 5.1 kΩ a GND (el collar es dispositivo). USBLC6-2SC6 en D+, D- y VBUS; su pinout se confirma con el datasheet de ST, que está pendiente de bajar. VBUS solo alimenta al TP4056, nunca al LDO.

### Protección de batería

Pieza por identificar: el `BQ2975DRBR` del BOM no existe. Sin pieza no hay pines que asignar.

## Consideraciones de diseño

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
| 2.0 | 2026-10-30 | Rehecho contra el datasheet v1.8 del módulo. GPIO05 ya no se repite. Sin GPIO22-34. USB D+/D- corregidos. GNSS con TX/RX cruzados |
