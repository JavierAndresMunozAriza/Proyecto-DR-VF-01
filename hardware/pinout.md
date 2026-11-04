# Asignación de Pines del Collar DR-VF-01

**Estado:** PROPUESTA v2.3 (2026-11-04), pendiente de revisión del autor. Es el **único mapa de pines** del proyecto: la PCB y la ESP32-S3-DevKitC-1 (N16R8) usan los mismos GPIO para que el firmware sea idéntico en ambas. Cualquier otro mapa en `docs/` o `reports/` debe remitir a este archivo.

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
| 4 | 4 | IMU_INT1 | LSM6DS3TR-C INT1 | **RTC, despierta de deep sleep** |
| 5 | 5 | LORA_RESET | SX1262 NRESET | RTC |
| 6 | 6 | LORA_DIO1 | SX1262 DIO1 | **RTC, interrupción / despertar** |
| 7 | 7 | BUTTON | Botón de usuario | **RTC, despierta**. Pull-up 10 kΩ + RC antirrebote |
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
| 7 | IO7 | 7 | BUTTON | Botón de usuario a GND; 10 kΩ a 3V3 y RC antirrebote. RTC, despierta | pinout.md |
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
| 31 | IO38 | 38 | LED_R | LED RGB, rojo, con resistencia serie | pinout.md |
| 32 | IO39 | 39 | LED_G | LED RGB, verde, con resistencia serie | pinout.md |
| 33 | IO40 | 40 | LED_B | LED RGB, azul, con resistencia serie | pinout.md |
| 34 | IO41 | 41 | CHRG_STAT | CHRG del CN3791 (pin 3); pull-up de 10 kΩ a 3V3 | pinout.md |
| 35 | IO42 | 42 | LORA_RXEN | Pin RX (6) del conmutador del módulo LoRa. POR DECIDIR | pinout.md |
| 36 | RXD0 | 44 | UART0_RX | Punto de prueba (consola) | Tabla 3-1 |
| 37 | TXD0 | 43 | UART0_TX | 499 Ω en serie y punto de prueba (consola) | Tabla 3-1, figura 9-1 |
| 38 | IO2 | 2 | BUZZER_PWM | Zumbador, por transistor si lo requiere | pinout.md |
| 39 | IO1 | 1 | OPTO_CTRL | Entrada del VO617A con resistencia serie; pull-down de 100 kΩ | pinout.md |
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

- GPIO42 y GPIO47 como RXEN y TXEN del módulo LoRa: dependen de lo que confirme el vendedor sobre DIO2 y el pin TX.
- Las salidas de estado del TP4056 y la salida ALRT del MAX17048 no tienen GPIO asignado.
- Botón o pad de reset en EN y botón de BOOT: se reservan pads.
- Supervisor de tensión en EN para la subida lenta de la alimentación solar.
- Los pines del módulo LoRa salen de un folleto del vendedor, no de un datasheet: confirmarlos antes de dibujar.

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

### IMU LSM6DS3TR-C

SDA y SCL a GPIO8 y GPIO9. INT1 a GPIO4. Para I2C: CS a VDDIO, SDO/SA0 define la dirección (0x6A o 0x6B). Salida INT configurable como push-pull u open-drain: si se deja push-pull no se necesita pull-up externo en GPIO4.

Pines según la Tabla 2 del datasheet del LSM6DS3TR-C: INT1 en el pin 4, SDA en el 14, SCL en el 13, CS (pin 12) a VDDIO para elegir I2C y SDO/SA0 (pin 1) para la dirección. SDx y SCx (pines 2 y 3) van a VDDIO o a GND, y los pines 10 y 11 quedan sin conectar. Los pines 1 a 9 y 12 a 14 coinciden con los del LSM6DSOX.

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

SDA y SCL en el mismo bus I2C que el IMU (GPIO8 y GPIO9). Dirección de 7 bits 0x36 (0110110), sin conflicto con el LSM6DS3TR-C (0x6A o 0x6B). VDD a la batería (2.5 a 4.5 V), no a 3V3. ALRT (drenador abierto): sin GPIO asignado; **POR DECIDIR** si se usa.

### Cargador USB-C TP4056 (pines verificados)

| TP4056 | Pin | Conexión |
|--------|-----|----------|
| TEMP | 1 | A GND (decidido el 2026-11-03: sin corte por frío) |
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
| 2.3 | 2026-11-04 | Sección de conexiones del módulo por pin físico (alimentación, EN, strapping, USB, UART0, cristal y pines sin conectar), verificada contra la tabla 3-1; CSV para crear el símbolo de Altium |
| 2.0 | 2026-10-30 | Rehecho contra el datasheet v1.8 del módulo. GPIO05 ya no se repite. Sin GPIO22-34. USB D+/D- corregidos. GNSS con TX/RX cruzados |
