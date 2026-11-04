# Lista de Materiales (BOM) - Collar DR-VF-01

**Estado:** REVISIÓN v1.9 (2026-11-03), pendiente de revisión del autor. Estructura reparada y corregida contra la auditoría `odd/tasks/repo-audit-report.md`. Los cambios de contenido están en el historial al final. Lo marcado **POR DECIDIR** no está resuelto: requiere una decisión del autor o una hoja de datos que aún no se ha leído.

Dónde comprar cada pieza, con sus URL y su estado: `hardware/COMPRAS.md`.

Este BOM cubre **solo la PCB final** (ESP32-S3-WROOM-1U-N16R8). La Nordic Thingy:91 X es la prueba de concepto y no forma parte del producto.

## Criterios de selección

1. Componentes con historial comprobado y disponibilidad a largo plazo.
2. Reemplazables en campo.
3. Rango de temperatura de -20 °C a +65 °C para todos los componentes, como margen: el rango de operación del producto es de 0 a +65 °C (`docs/REQUIREMENTS.md`, RNF-09). El módulo ESP32-S3-WROOM-1U-N16R8 admite -40 a 65 °C de ambiente (con ECC de la PSRAM activado, 85 °C y 1/16 menos de PSRAM utilizable; datasheet WROOM-1 v1.8): el límite de 65 °C no deja margen.
4. Al menos dos fuentes de suministro para los componentes críticos.

## 1. Componentes electrónicos

### 1.1 Microcontrolador y memoria
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Módulo ESP32-S3-WROOM-1U-N16R8 (16 MB flash quad, 8 MB PSRAM octal, conector U.FL) | ESP32-S3-WROOM-1U-N16R8 | Espressif | Digi-Key |

La flash externa GD25Q128CWIGR y la SRAM externa IS62WVS5128GBLL-10NL **se eliminaron**: el datasheet v1.8 del módulo (Tablas 1-1 y 1-2) ya incluye 16 MB de flash y 8 MB de PSRAM, y el ESP32-S3 no tiene un bus libre para memorias adicionales sin gastar los GPIO que ya faltan.

### 1.2 Posicionamiento
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Módulo GNSS MAX-M10S | MAX-M10S-00B | u-blox | Avnet |
| 1 | Antena GNSS pasiva tipo parche 18x18x4 mm (1575.42 MHz), con cable y conector u.FL (decidido el 2026-11-04) | por decidir (el ANNMS-420.A no se pudo verificar) | por decidir | Digi-Key / Mouser |

**Decidido el 2026-11-04:** el parche va con cable y conector u.FL en la PCB, para que su sintonía no dependa del plano de masa de la placa y la RF propia se reduzca a una línea de 50 Ω corta hasta el conector. **POR DECIDIR:** el MPN. La ANNMS-420.A no se pudo verificar (el código parece corrupto y el fabricante "Toko" es dudoso). El MAX-M10S recibe solo L1 (GPS/QZSS 1575.42 MHz, Galileo E1, BeiDou B1I, GLONASS L1; datasheet), así que **una antena L1+L5 no aporta nada**. Como el módulo ya integra LNA y filtro SAW, se recomienda un **parche cerámico pasivo de 18x18x4 mm a 1575.42 MHz** con datasheet de un fabricante conocido y plano de masa definido en el layout. Una antena activa solo se justifica con cable largo: consume 15-25 mA (el VCC_RF del MAX-M10S entrega como máximo 50 mA). Antena descartada: A-SKY AK521 activa L1+L5 (25 mm, 20 dB de LNA).

### 1.3 Comunicaciones
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Módulo LoRa SX1262 915 MHz, interfaz **SPI**, 20x14x2.3 mm (reemplaza al chip SX1262 suelto SX1262IMLTRT) | DX-LR30-900M22S (variante SMD; no la "SP") | DX-SMART | AliExpress |
| 1 | Antena LoRa 915 MHz flexible de 8 cm con conector u.FL | sin MPN (listado "IPEX 1.13 868/915 MHz 2 dBi 8 cm") | genérico | AliExpress |
| 2 | Conector u.FL SMD para las antenas LoRa y GNSS | p. ej. U.FL-R-SMT-1 (verificar) | Hirose | Digi-Key |
| 1 | Antena FPC 2.4 GHz (Wi-Fi/BLE del módulo, vía U.FL) | 2015301200 | Würth Elektronik | Mouser |
| 1 | Conector SMA hembra (solo pruebas de banco con la antena larga) | 6521011000 | Würth Elektronik | Mouser |
| 1 | Kit de banco: antena corta SMA 3 dBi más pigtail u.FL a SMA de 15 cm | sin MPN (listado AliExpress de 2 uds) | genérico | AliExpress |

El chip SX1262 suelto se sustituye por un módulo: ya integra el cristal de 32 MHz, el conmutador de antena y la adaptación, lo que cierra el bloqueante de la sección RF. Confirmado en el material del vendedor (`docs/datasheets/SX1262 LoRa módulo desarrollador.pdf`): interfaz **SPI**, SX1262 original, 1.8 a 3.7 V, 0 a +22 dBm, 850 a 930 MHz, pines NSS, SCK, MOSI, MISO, RESET, BUSY, DIO1, DIO2, DIO3, RX, TX y ANT (tabla en `hardware/pinout.md`). Es material de marketing, no el datasheet de Semtech. Para comprar, elegir la variante **LR30 900M22S**; la "900M22SP" es una placa con STM32 y UART.

Pendiente de confirmar con el vendedor: para qué sirve DIO2 y si el pin TX va unido a él por dentro (de eso depende si GPIO47 hace falta), y si el oscilador de 32 MHz es cristal o TCXO. La ficha exige conectar la antena antes de energizar. La antena por defecto sale por el pad ANT; la PCB necesita una línea de 50 ohm hasta el conector u.FL.

**POR DECIDIR:** la antena de 8 cm es un monopolo de cuarto de onda y su rendimiento depende del plano de masa de la PCB. Probar alcance real. Para bancada, el kit SMA de 3 dBi con pigtail es suficiente. Conector de la antena LoRa: el módulo trae una pastilla IPX y una pata ANT; confirmar con el vendedor cómo se selecciona y mantener solo una opción (el conector u.FL de la PCB o el IPX del módulo).

### 1.4 Sensores y actuadores
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | IMU 6 ejes LSM6DS3TR-C (decidido el 2026-11-03) | LSM6DS3TR-C | STMicroelectronics | Docente (classroom) |
| 1 | Optoacoplador | VO617A-1-E | Vishay | Mouser |
| 1 | Buzzer piezoeléctrico | PKMCS0909E4000-R1 (corregido el 2026-10-31: el código anterior PKMCS0909E000-R1 no aparece en distribuidores) | Murata | Digi-Key |
| 1 | Medidor de batería (fuel gauge), I2C | MAX17048 | Analog Devices (Maxim) | Digi-Key |

**IMU (decidido el 2026-11-03):** LSM6DS3TR-C del docente. Cumple RF-06: el wake-up se enruta a INT1 (`MD1_CFG.INT1_WU`, datasheet DocID030071 p.92), consume 9 µA a 12.5 Hz en baja potencia y 3 µA en power-down (p.24) y alimenta de 1.71 a 3.6 V. No tiene MLC, así que la clasificación de conducta dentro del sensor queda fuera de la Fase 1. Si la Fase 4 la pide, el LSM6DSOX comparte la huella LGA-14 de 2.5 x 3 mm, con los pines 10 y 11 sin conectar. Zephyr no tiene un driver con el nombre del LSM6DS3TR-C, y los drivers de la familia (`st,lsm6dsl` y `st,lsm6dso`) no implementan el trigger de wake-up: el wake-up se configura con la librería oficial de ST (`lsm6ds3tr-c_STdC`, en `modules/hal/st/sensor/stmemsc`).

El MAX17048 lo exigen `docs/REQUIREMENTS.md` y `docs/ARCHITECTURE.md` y no estaba en el BOM. Se añade como componente requerido; el encapsulado exacto (MAX17048G+T10 o equivalente) **POR DECIDIR**.

### 1.5 Gestión de energía
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Regulador LDO 3.3 V, **1 A**, Iq 25 µA, SOT-23-5 (reemplaza al TPS7A0533, de 200 mA) | TLV75733PDBVR | Texas Instruments | Digi-Key |
| 1 | Cargador solar MPPT CN3791 | CN3791 | Consonance | AliExpress |
| 1 | Cargador lineal para la carga por USB-C (1 A máx.; se limita a 500 mA con RPROG de 2.4 kΩ) | TP4056 (ESOP-8, con pad térmico a GND; la huella `SOIC_8_3.9mm` de la biblioteca no lo tiene) | NanJing Top Power | Docente (prototipo); producción: distribuidor por definir |
| 1 | Protección de batería | BQ2975DRBR | Texas Instruments | Digi-Key |
| 1 | Batería LiPo 3.7 V 1500 mAh | PL063-1500 | PowerStream | Digi-Key |

**Regulador (decidido el 2026-10-30):** el TPS7A0533 entrega 200 mA como máximo y las cargas del repo suman 224 a 394 mA, así que se sustituye por el TLV757P de 3.3 V. Datasheet (`TLV757P`, verificado): 1 A, corriente quiescente de 25 µA típicos, entrada de 1.45 a 5.5 V, caída máxima de 425 mV a 1 A, descarga activa de la salida. Referencia exacta: **TLV75733PDBVR** (SOT-23-5, carrete de 3000). También existen TLV75733PDRVR (WSON-6) y TLV75733PDYDR (SOT-23-5 con pad térmico). Para pocas unidades, el sufijo T es el de 250 piezas según la nomenclatura del datasheet; comprobar la existencia en el distribuidor. Con entrada de 4.2 V y 300 mA disipa unos 0.27 W. La resistencia térmica del DBV es 100.8 °C/W en la placa de evaluación de TI y 231.1 °C/W en la placa JEDEC, de modo que en régimen continuo el DYD con pad térmico (60.3 °C/W) es más seguro; en ráfagas cortas, como el TX de LoRa, el DBV basta. El TLV755P (TLV75533PDBVR) es de solo 500 mA. Hay que recalcular `hardware/POWER_BUDGET.md` con una Iq de 25 µA en lugar de 1 µA.

**POR DECIDIR:**
- BQ2975 necesita MOSFET externos y resistencia de sensado que no están en el BOM. Confirmar si la batería PL063-1500 ya trae su propia protección, porque entonces el BQ2975 duplica la función.
- CN3791: componentes externos pendientes (inductor, MOSFET de paso, resistencia de sensado, divisor del MPPT, diodos D1 y D2). Fuente: `hardware/datasheets/cn3791_datasheet.pdf`. El divisor para 6 V es R3 = 402 kΩ y R4 = 100 kΩ (referencia de 1.205 V; ver `docs/ARCHITECTURE.md`, sección 2.2). **El CN3791 no tiene pin de temperatura ni de habilitación** (sus pines son VG, GND, CHRG, DONE, COM, MPPT, BAT, CSP, VCC y DRV), así que no admite un NTC directo. Cargar por debajo de 0 °C daña la LiPo. **Decidido el 2026-11-03:** el collar se usa solo en zonas sin heladas (por debajo de unos 2.300 m; el IDEAM ubica las heladas entre 2.300 y 2.900 m) y no lleva corte de carga por frío. El pin TEMP (pin 1) del TP4056 va a GND, que desactiva esa función (datasheet del TP4056, p.2). Restricción de uso documentada: no usar el collar donde haya heladas.
- Carga por USB-C (decidida el 2026-10-31): VBUS al TP4056 y de ahí a la batería, con RPROG de 2.4 kΩ para limitar a 500 mA. El USB-C no alimenta al LDO. El CN3791 queda solo para el panel solar porque con el divisor de 6 V no inicia carga desde 5 V. Pendiente: sumar al presupuesto de sleep la corriente del TP4056 sin USB (el datasheet da 2.5 µA típicos en standby con entrada presente; sin entrada no está tabulada) y decidir si se lee su salida de estado.
- Batería: la PL063-1500 es difícil de conseguir y el envío es caro. Con 0.223 mA promedio, 1200 mAh ya da 191 días, así que no hace falta 1500. Opciones: LiPo 603450 de unos 1200 mAh con protección, LiPo 103450 de 1800-2000 mAh, o celda 18650 protegida (de marca verificable, con lengüetas soldadas o conductores: un portapilas de resortes es poco fiable con la vibración y el sellado IP67 del collar). Revisar la polaridad del conector JST contra la PCB. Si la celda trae protección propia, el BQ2975 sobra. Eléctricamente todas son iguales (1S, 3.7 V nominal, 4.2 V cargada): se conectan igual al cargador, así que la elección puede esperar. Lo que cambia es la capacidad, el tamaño, el conector y la polaridad. **POR DECIDIR.** Panel solar: ver sección 2.3.

### 1.6 Conectores e interfaces
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Conector USB-C hembra, 16 pines SMD ángulo recto | TYPE-C-3.1-16PIN | Shou Han | Docente (biblioteca; LCSC C7507405) |
| 2 | Conector JST PH 2 pines, entrada lateral | S2B-PH-K-S(LF)(SN) | JST | Docente (biblioteca; LCSC C173752) |
| 4 | Conector JST SH 4 pines | B4B-SH-SM4-TB(LF)(SN) | JST | Mouser |
| 1 | Botón táctil 6x6 mm | PTS645SL50SMTR92 LFS | C&K | Digi-Key |
| 1 | Protección ESD USB 2.0 de 2 líneas, SOT23-6 (D+/D- y VBUS) | USBLC6-2SC6 | STMicroelectronics | Docente (biblioteca; LCSC C7519) |
| 2 | Resistencia CC1/CC2 5.1 kΩ (USB-C como dispositivo): ver la tabla 1.8 | — | — | — |


**Nota sobre USBLC6:** el docente tiene el USBLC6-2SC6 de ST (biblioteca `TVS_5.25V_2CH_SOT_23_6`, SOT-23-6), así que no se compra en AliExpress. Para D+/D- se necesita la variante **-2SC6**, no la -4SC6 (4 líneas, otro pinout y sin pin de VBUS). Falta bajar el datasheet de ST y verificar que el pinout coincida con la huella de la biblioteca.

**Conector USB-C (decidido el 2026-11-03):** se usa el de la biblioteca del docente, `CON_USB_TYPEC_16P` = Shou Han TYPE-C-3.1-16PIN (LCSC C7507405, huella `CON_USB_C_16P_RA`). Se retira el código `1240-6813-1-ND` (no verificable) y el candidato Amphenol. **POR DECIDIR:** faltan fusible/PTC del USB-C y las resistencias en serie de D+/D- que recomienda Espressif. Falta un conector o pads para el cristal RTC y los puntos de prueba de `hardware/pinout.md`. Ninguno de los conectores JST tiene un uso documentado: asignar uso al dibujar el esquemático.

### 1.7 LEDs
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | LED RGB SMD 3535 (6 pines independientes) | RS-3535MWAM | NationStar | Docente (biblioteca; LCSC C842778) |

Decidido el 2026-11-03: el LED RGB del docente reemplaza al Kingbright L-154A4SURK/G5T, que no tiene datasheet con ese código, y se quitan los 3 LED LTST-C193KGKT, porque `pinout.md` solo asigna el LED RGB (GPIO38 a 40). Condición de diseño: el máximo de VF de G y B (3.4 V) pasa de los 3.3 V del riel; se dimensionan las resistencias con la curva VF-IF del datasheet y, si VF de G o B a la corriente elegida supera 3.0 V a 25 °C, el ánodo se alimenta desde VBAT.

### 1.8 Pasivos
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Cristal **32.768 kHz** (RTC del ESP32-S3), CL 12.5 pF | ECS-.327-12.5-... (prefijo ECS-.327 = 32.768 kHz) | ECS | Digi-Key |
| 4 | Capacitor cerámico NP0 33 pF, dos en serie por pata del cristal (16.5 pF por pata: con CL 12.5 pF y 3 a 5 pF parásitos hacen falta 15 a 19 pF por pata; los 22 pF anteriores daban 14 a 16 pF de carga, alta) | CC0603JRNPO9BN330 | YAGEO | Docente (biblioteca) |
| 5 | Capacitor cerámico 100 nF X7R, 50 V (1 en el pin 3V3 del módulo, 1 entre VG y VCC del CN3791; el resto para IMU, MAX17048 y repuesto: el número por pin sale al dibujar el esquemático) | CC0603KRX7R9BB104 | YAGEO | AliExpress (lote con MPN nombrado, `COMPRAS.md` O21) |
| 5 | Capacitor cerámico 10 µF X5R (entrada de alimentación y desacople de 3.3 V) | CL21A106KAYNNNE (25 V, 0805) y TCC0603X5R106K100CT (10 V, 0603, solo en el riel de 3.3 V) | Samsung y CCTC | Docente (biblioteca) |
| 1 | Capacitor cerámico 22 µF 16 V, 0805 (pin 3V3 del módulo, figura 9-1 del datasheet del WROOM-1; falta leer la hoja de Chinocera) | HGC0805R5226M160NSLJ | Chinocera | Docente (biblioteca) |
| 1 o 2 (**POR DECIDIR**) | Capacitor de tantalio 100 µF (bulk de 3.3 V para los transitorios del LoRa) | TAJA107K016RNJ | AVX | Digi-Key |
| 4 | Resistor 10 kΩ (pull-ups de EN, BOOT, botón y CHRG; 5 % del docente) | FRC0603J103TS | FOJAN | Docente (biblioteca) |
| 2 | Resistor 100 kΩ 1 % (pull-down del optoacoplador y R4 del divisor MPPT) | por definir (serie RC0603FR-07) | Yageo | AliExpress (kit, `COMPRAS.md` O19); plan B: 49.9k + 49.9k del docente |
| 2 | Resistor 390 kΩ y 10 kΩ en serie (R3 del divisor MPPT del CN3791: 400 kΩ, V_MPPT de 6.025 V; el valor final depende del panel, `COMPRAS.md` O7) | RC0603FR-07390KL y FRC0603J103TS | Yageo y FOJAN | Docente (biblioteca) |
| 1 | Resistor 2.4 kΩ 1 % (RPROG del TP4056, 500 mA) | por definir (serie RC0603FR-07) | Yageo | AliExpress (kit, `COMPRAS.md` O19); plan B: 4.7k ‖ (4.7k + 191R) del docente |
| 2 | Resistor 5.1 kΩ 1 % (CC1 y CC2 del USB-C como dispositivo) | por definir (serie RC0603FR-07) | Yageo | AliExpress (kit, `COMPRAS.md` O19); plan B: un 4.7 kΩ de 1 % del docente por línea (-7.8 %, dentro de Rd de ±20 %, Microchip AN1914 tabla 1) |
| 1 | Resistor 120 Ω y condensador 220 nF X7R en serie (red del pin COM del CN3791, datasheet p.3 y p.7) | FRC0603F1200TS y 220 nF X7R | FOJAN | Docente (120 Ω); 220 nF en `COMPRAS.md` O21 |
| 2 | Resistor 22 o 33 Ω (serie de USB D+ y D-, guía de Espressif p.21) | por definir | por definir | AliExpress (kit, O19); plan B: tres 100R en paralelo por línea |
| 1 | Resistor de sensado del CN3791, 1 %, RCS = 0.12 V / corriente de carga | por definir (`COMPRAS.md` O22) | por definir | Mouser o Digi-Key, o AliExpress |
| 1 | Diodo Schottky de baja fuga (D2 del CN3791; D1 es opcional) | por definir (`COMPRAS.md` O20) | por definir | Digi-Key |

**Cubierto por el docente** (`hardware/COMPRAS.md` D9 y D10): resistencias 0603 de 330 Ω, 1 kΩ, 4.7 kΩ y 10 kΩ, y condensadores de 1 µF, 10 µF (25 V y 10 V), 22 µF de 16 V y 33 pF NP0. El 100 nF de la biblioteca es Y5V y no sirve (D11). El resto se compra (`COMPRAS.md` O19 a O22): kit de resistencias 0603 1 %, lote de condensadores X7R, resistencia de sensado y Schottky de baja fuga. Si el pedido de AliExpress falla, `COMPRAS.md` 1.3 arma los valores con piezas del docente.

**Corrección:** la fila antes decía "Cristal 40 MHz" con el código ECS-.327-12-5PXDN-TR. El prefijo ECS-.327 corresponde a un cristal de 32.768 kHz y el módulo ya incluye su cristal de 40 MHz (datasheet v1.8, sección de características). El MPN completo **POR DECIDIR**: el texto anterior estaba corrupto.

**Según la guía de diseño y el datasheet del módulo de Espressif** (`hardware/datasheets/`): RC de reset en EN con R = 10 kΩ y C = 1 µF (EN no puede quedar flotante; con alimentación solar de subida lenta la guía sugiere un supervisor de unos 3.0 V, p.11 y p.12), diodo ESD y 10 µF o más en la entrada de alimentación, 22 µF y 0.1 µF en el pin 3V3 del módulo (figura 9-1 del datasheet del WROOM-1), resistencia de 499 Ω en el TX de UART0 (plan B: 330R + 150R) y resistencia en serie más condensador a tierra en SCK del SPI (sin valor en la guía). El TLV75733P necesita 1 µF o más a la entrada y a la salida (datasheet p.3 y p.4) y su salida admite 1 a 200 µF en total, así que el bulk de 3.3 V (tantalio de 100 µF más los cerámicos) debe sumar menos de 200 µF: **POR DECIDIR** al conocer la corriente de TX del módulo LoRa. El CN3791 pide además los condensadores de entrada y salida de su datasheet (p.8).

**Faltan además** (se añaden al dibujar el esquemático, sus valores salen de `hardware/pinout.md`): 2 resistencias de 4.7 kΩ (pull-up I2C), 1 de 100 kΩ (pull-down del optoacoplador), resistencias en serie de los LED, resistencia en serie del optoacoplador (la ARCHITECTURE usa 330 Ω), y las cargas del cristal de 32.768 kHz (dos 33 pF en serie por pata, ver la tabla).

## 2. Componentes mecánicos

### 2.1 Carcasa y estructura
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Carcasa principal moldeada | CUSTOM-DRVF01-HOUSING | Fabricación interna | Proveedor local |
| 1 | Tapa posterior moldeada | CUSTOM-DRVF01-BACK | Fabricación interna | Proveedor local |
| 1 | Correa de nylon ajustable | CUSTOM-DRVF01-STRAP | Fabricación interna | Proveedor local |
| 4 | Tornillo M2x6 cabeza allen | 50017102 | PEM | Fastenal |
| 2 | Inserto roscado M2 | PEM-SL2-02 | PEM | Fastenal |

### 2.2 Protección ambiental
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Junta tórica EPDM | AS0139 | Apple Rubber | McMaster-Carr |
| 2 | Tapas ciegas USB-C | **POR DECIDIR** (antes repetía el código del conector USB-C) | — | — |
| 1 | Adhesivo acrílico doble cara | 467MP | 3M | Grainger |

### 2.3 Panel solar
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Panel solar monocristalino 2 W | HTJ-2W-D | HiTechGlobal | Amazon Business |
| 1 | Cable solar JST PH 2 pines | CUSTOM-JST-CABLE | Fabricación interna | Local |

**Decidido el 2026-10-30:** panel de **6 V nominal** (el CN3791 en su versión de 6 V; para prototipo, el módulo CN3791 de 6 V de AliExpress, que es una placa cargadora, no un panel). Falta el MPN de un panel de 6 V y al menos 1 W; el HTJ-2W-D no declara su voltaje. Antes decía: `docs/ARCHITECTURE.md` usa 6 V / 1.5 W y el diagrama del informe un "MPPT CN3791 6V". El CN3791 necesita un panel de voltaje coherente con su divisor MPPT: fijar un solo valor.

## 3. Materiales de embalaje

| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Caja de cartón corrugado | ULINE-CORRUGATED-BOX | Uline | Uline |
| 1 | Bolsa antiestática | 12"x16" Anti-Stat Bag | Static Solutions | Amazon |
| 1 | Espuma de embalaje | 1/2" Foam Sheet | Uline | Uline |

Documentación impresa incluida: `docs/comercial_docs/USER_MANUAL.md`, guía de inicio rápido y tarjeta de garantía (contenido interno, en español).

## 4. Costos estimados

**Las cifras no están respaldadas por una lista de precios y no incluyen los cambios de esta revisión (memorias eliminadas, componentes añadidos). Recalcular cuando se cierren los puntos POR DECIDIR.** El informe `.tex` habla de un lote de 100 unidades y este BOM de 1000: unificar.

| Categoría | Por unidad (USD) | Lote de 1000 (USD) |
|-----------|-----------------|--------------------|
| Componentes electrónicos | 24.50 | 24,500 |
| Componentes mecánicos | 8.75 | 8,750 |
| Fabricación de PCB | 3.25 | 3,250 |
| Ensamblaje | 5.50 | 5,500 |
| Embalaje | 1.25 | 1,250 |
| **Total** | **43.25** | **43,250** |

## 5. Abastecimiento

| Distribuidor | Ventajas | Desventajas |
|--------------|----------|-------------|
| Digi-Key | Stock inmediato, datos técnicos completos | Precios algo más altos |
| Mouser | Amplia selección, envío rápido | Puede requerir cuenta corporativa |
| Arrow | Precios competitivos | Plazos variables |

| Proveedor mecánico | Especialidad | Región |
|--------------------|--------------|--------|
| Protolabs | Prototipos rápidos | América del Norte |
| Xometry | Producción a demanda | Global |
| Fictiv | Calidad premium | América del Norte |

Plazos típicos: componentes estándar 1-2 semanas, personalizados 4-6, carcasa moldeada 6-8. MOQ: comunes 1 unidad, personalizados 100, carcasa 500. Requisitos: RoHS en todo lo electrónico, REACH en lo mecánico, UL en componentes de potencia, y la batería debe cumplir las normas IATA para baterías de litio. Plásticos resistentes a UV.

## 6. Componentes en los documentos y fuera de este BOM

| Componente | Dónde aparece | Estado |
|------------|---------------|--------|
| Waveshare Core1262-HF | ARCHITECTURE.md | Sustituido por el módulo DX-LR30-900M22S (decisión del 2026-10-30); si algún documento aún lo nombra, la mención es obsoleta |
| ESP32-S3-DevKitC-1 y AMS1117 | ARCHITECTURE.md | Placa de prototipo, no va en la PCB |
| PC817 | ARCHITECTURE.md | Optoacoplador de prototipo; la PCB usa el VO617A |
| A7670SA y BG95-M3 | ARCHITECTURE.md / informe | Se contradicen como módem de Fase 2. **POR DECIDIR** |
| Nordic Thingy:91 X | README, docs | Prueba de concepto, fuera de la PCB |
| Placas de desarrollo de MAX17048, LSM6DS3TR-C, MAX-M10S y CN3791, LED de 3 mm y cabeceras de 2.54 mm | ARCHITECTURE.md | Solo prototipo, no van en la PCB |
| Antena de 5 dBi SMA y placa MAX-M10S con u.FL | COMPRAS.md (A4, A8) | Solo banco, no van en el producto |

## 7. Biblioteca Altium del docente (`shared_lib.IntLib`)

Según el autor (confirmado el 2026-11-03), el docente tiene físicamente todo lo de su biblioteca y lo que publicó en el classroom; la lista detallada con MPN y código LCSC está en `hardware/COMPRAS.md` sección 1. Del classroom, y fuera de la biblioteca, el diseño usa el TP4056 (carga por USB-C, decidido el 2026-10-31) y el LSM6DS3TR-C (IMU, decidido el 2026-11-03). Las demás piezas del classroom no tienen uso en este diseño. Para dibujar el esquemático, la biblioteca sirve así:

| Cubre | Entradas de la biblioteca |
|-------|---------------------------|
| Pasivos | capacitores 0603/0805 (100 nF, 1 µF, 10 µF, 22 µF, 33 pF, 470 nF), resistencias 0603 (4k7, 10k, 330R, 1k y otras), ferrita 0603, inductores de 2.2 a 47 µH |
| Conectores | USB-C de 16 pines, JST 2 mm, bornes de 2 y 3 posiciones, pulsador 4.5x4.5 |
| Protección y potencia | TVS 5.25 V de 2 canales SOT23-6 (USB), TVS de 3.3, 5 y 12 V, diodos Schottky de 40 V (1 y 3 A), MOSFET P de 30 V y N de 20 V |
| Otros | LED RGB 3535, LED 0805, cargador Li-ion genérico `IC_BAT_CHG_4.2V_1A`, convertidores buck/boost genéricos |
| LDO `LDO_3V3_1A_FIXED` | Por su descripción (1 A, 60 dB a 120 Hz) parece de la familia LM1117/AMS1117: dropout de 1 V o más y corriente quiescente de mA. No sirve con la LiPo (necesitaría unos 4.4 V de entrada) ni con el presupuesto de sleep. Su MPN es AMS1117-3.3 (UMW, LCSC C347222): no se usa |

No cubre y hay que crear o descargar: huella del ESP32-S3-WROOM-1U (la biblioteca solo trae el ESP32-C3-WROOM-02-N4, otro módulo; el datasheet indica que la 1U no lleva zona de exclusión de antena), MAX-M10S, LSM6DS3TR-C, MAX17048, el módulo LoRa, CN3791, VO617A, buzzer, cristal de 32.768 kHz, conector u.FL y JST SH.

## Historial de revisiones

| Versión | Fecha | Cambios |
|---------|-------|---------|
| 1.0 | 2024-05-15 | Versión inicial |
| 1.1 | 2024-06-20 | Actualización de precios y MPN |
| 1.2 | 2026-10-30 | Estructura reparada (tabla solar, resistores y costos estaban desordenados). Eliminadas flash y SRAM externas (redundantes con el módulo). Cristal 40 MHz corregido a 32.768 kHz. MAX17048 añadido. Puntos bloqueantes marcados POR DECIDIR |
| 1.3 | 2026-10-30 | SX1262 suelto reemplazado por módulo SPI, antena LoRa u.FL de 8 cm, USBLC6-2SC6, parche GNSS pasivo recomendado, opciones de batería, requisitos de Espressif para EN y alimentación |
| 1.4 | 2026-10-30 | Módulo LoRa DX-LR30-900M22S confirmado (SPI), regulador TLV75733PDBVR de 1 A, panel de 6 V, IMU del docente, notas de USB-C y USBLC6 |
| 1.5 | 2026-10-31 | Carga por USB-C con TP4056 (del docente) y CN3791 solo para el panel; divisor MPPT recalculado con la referencia real de 1.205 V; CN3791 sin pin de temperatura; presupuesto de sleep de 124 µA (123.76 µA) y autonomía de 191 días (1200 mAh) |
| 1.6 | 2026-11-03 | El docente tiene todo lo de su biblioteca y del classroom: conector USB-C (Shou Han TYPE-C-3.1-16PIN) y USBLC6-2SC6 pasan a "Docente"; se retiran el código Digi-Key del USB-C y la compra del USBLC6 en AliExpress |
| 1.7 | 2026-11-03 | Piezas del docente verificadas contra sus datasheets: IMU LSM6DS3TR-C (sin MLC), LED RGB RS-3535MWAM, JST S2B-PH-K-S, TP4056 con huella ESOP-8; sin corte de carga por frío (uso solo en zonas sin heladas); PCB de 0.8 mm por el layout del USB-C; se quitan los LED LTST |
| 1.8 | 2026-11-04 | Pasivos: cargas del cristal de 33 pF en serie (los 22 pF daban carga alta), 22 µF y 0.1 µF del módulo, 100 nF X7R en lugar de 10 de Y5V, red COM y condensadores del CN3791, límite de 200 µF del TLV75733P, resistencia de sensado y Schottky; ruta de compra con kits de AliExpress y plan B con piezas del docente |
| 1.9 | 2026-11-04 | Antena GNSS con cable y conector u.FL (dos u.FL SMD en la PCB: LoRa y GNSS); se quita la mención de una pieza del docente sin uso |
