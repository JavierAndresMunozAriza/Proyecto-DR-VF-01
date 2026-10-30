# Lista de Materiales (BOM) - Collar DR-VF-01

**Estado:** REVISIÓN v1.2 (2026-10-30), pendiente de revisión del autor. Estructura reparada y corregida contra la auditoría `odd/tasks/repo-audit-report.md`. Los cambios de contenido están en el historial al final. Lo marcado **POR DECIDIR** no está resuelto: requiere una decisión del autor o una hoja de datos que aún no se ha leído.

Este BOM cubre **solo la PCB final** (ESP32-S3-WROOM-1U-N16R8). La Nordic Thingy:91 X es la prueba de concepto y no forma parte del producto.

## Criterios de selección

1. Componentes con historial comprobado y disponibilidad a largo plazo.
2. Reemplazables en campo.
3. Rango de temperatura de -20 °C a +65 °C para todos los componentes.
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
| 1 | Antena GNSS cerámica (1575 MHz) | ANNMS-420.A | Toko | Digi-Key |

**POR DECIDIR:** comprobar que la ANNMS-420.A es una antena pasiva de 1575.42 MHz y qué plano de masa necesita. El MAX-M10S-00B es la variante estándar con TCXO, LNA y filtro SAW integrados (datasheet, tabla de variantes).

### 1.3 Comunicaciones
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Transceptor LoRa SX1262 (QFN) | SX1262IMLTRT | Semtech | Digi-Key |
| 1 | Antena FPC 2.4 GHz (Wi-Fi/BLE del módulo, vía U.FL) | 2015301200 | Würth Elektronik | Mouser |
| 1 | Conector SMA hembra (antena LoRa) | 6521011000 | Würth Elektronik | Mouser |

**POR DECIDIR, bloqueante:**
- **No hay antena de 915 MHz** en este BOM. La FPC de 2.4 GHz no sirve para LoRa ni para GNSS. Elegir una antena de 915 MHz con su conector (el SMA ya está) y confirmar la ganancia: la hoja de la FPC y `mechanical/ANTENNA_PLACEMENT.md` dan 8 dBi y 2.5 dBi respectivamente.
- **Falta toda la sección RF del SX1262**: cristal de 32 MHz o TCXO de 32 MHz, inductor del DC-DC, conmutador RF controlado por DIO2, red de adaptación y filtro paso bajo. Sin ella el transceptor no funciona. La referencia es el diseño de referencia de Semtech para SX1262 (hoja de datos aún no descargada).
- Si la aplicación no usa Wi-Fi ni BLE, la antena FPC de 2.4 GHz sobra.

### 1.4 Sensores y actuadores
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | IMU 6 ejes LSM6DSOX | LSM6DSOXTR | STMicroelectronics | Digi-Key |
| 1 | Optoacoplador | VO617A-1-E | Vishay | Mouser |
| 1 | Buzzer piezoeléctrico | PKMCS0909E000-R1 | Murata | Digi-Key |
| 1 | Medidor de batería (fuel gauge), I2C | MAX17048 | Analog Devices (Maxim) | Digi-Key |

El MAX17048 lo exigen `docs/REQUIREMENTS.md` y `docs/ARCHITECTURE.md` y no estaba en el BOM. Se añade como componente requerido; el encapsulado exacto (MAX17048G+T10 o equivalente) **POR DECIDIR**.

### 1.5 Gestión de energía
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Regulador LDO 3.3 V, 200 mA | TPS7A0533PDBVR | Texas Instruments | Digi-Key |
| 1 | Cargador solar MPPT CN3791 | CN3791 | Consonance | AliExpress |
| 1 | Protección de batería | BQ2975DRBR | Texas Instruments | Digi-Key |
| 1 | Batería LiPo 3.7 V 1500 mAh | PL063-1500 | PowerStream | Digi-Key |

**POR DECIDIR, bloqueante:** el TPS7A0533 entrega como máximo 200 mA (datasheet TPS7A05 verificado). Los documentos del repo suman cargas de 221 a 394 mA (ESP32-S3, TX LoRa del SX1262 y GNSS en adquisición) y varios asumen un regulador de 500 mA. Hay que elegir entre un regulador de mayor corriente o un convertidor buck-boost (la batería baja de 3.3 V al descargarse, así que un LDO pierde regulación) y recalcular `hardware/POWER_BUDGET.md`. La elección del regulador es de diseño, no se hizo aquí.

**POR DECIDIR:**
- BQ2975 necesita MOSFET externos y resistencia de sensado que no están en el BOM. Confirmar si la batería PL063-1500 ya trae su propia protección, porque entonces el BQ2975 duplica la función.
- CN3791: componentes externos pendientes (inductor, MOSFET, resistencia de sensado, divisores del MPPT, NTC). Una hoja de datos del CN3791 es la fuente (aún no descargada). Cargar por debajo de 0 °C daña la LiPo, de modo que el NTC es necesario.
- El BOM no tiene ruta de carga por USB-C (VBUS al cargador) aunque los documentos dicen que existe.
- Capacidad de batería: 1500 mAh en este BOM, 1200 mAh en `docs/REQUIREMENTS.md`. Panel solar: ver sección 3.

### 1.6 Conectores e interfaces
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Conector USB-C hembra | 1240-6813-1-ND (código Digi-Key, no MPN) | Amphenol | Digi-Key |
| 2 | Conector JST PH 2 pines | B2B-PH-K-S(LF)(SN) | JST | Mouser |
| 4 | Conector JST SH 4 pines | B4B-SH-SM4-TB(LF)(SN) | JST | Mouser |
| 1 | Botón táctil 6x6 mm | PTS645SL50SMTR92 LFS | C&K | Digi-Key |

**POR DECIDIR:** el código del USB-C es un número de Digi-Key; obtener el MPN real del fabricante. Faltan resistencias CC (5.1 kΩ x2), protección ESD y fusible/PTC del USB-C. Falta un conector o pads para el cristal RTC y los puntos de prueba de `hardware/pinout.md`. Ninguno de los conectores JST tiene un uso documentado: asignar uso al dibujar el esquemático.

### 1.7 LEDs
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | LED RGB difuso | L-154A4SURK/G5T | Kingbright | Digi-Key |
| 3 | LED indicador simple | LTST-C193KGKT | Lite-On | Mouser |

### 1.8 Pasivos
| Cant. | Descripción | MPN | Fabricante | Proveedor |
|-------|-------------|-----|------------|-----------|
| 1 | Cristal **32.768 kHz** (RTC del ESP32-S3), CL 12.5 pF | ECS-.327-12.5-... (prefijo ECS-.327 = 32.768 kHz) | ECS | Digi-Key |
| 4 | Capacitor cerámico C0G 22 pF | C0G2A220J5GACA024 | TDK | Digi-Key |
| 10 | Capacitor cerámico 100 nF | CGA2B1X7R1E104K050BC | TDK | Digi-Key |
| 5 | Capacitor cerámico 10 µF | C1608X5R1A106K080AE | TDK | Digi-Key |
| 2 | Capacitor de tantalio 100 µF | TAJA107K016RNJ | AVX | Digi-Key |
| 3 | Resistor 10 kΩ 1 % | RC0603FR-0710KL | Yageo | Digi-Key |
| 2 | Resistor 100 kΩ 1 % | RC0603FR-07100KL | Yageo | Digi-Key |

**Corrección:** la fila antes decía "Cristal 40 MHz" con el código ECS-.327-12-5PXDN-TR. El prefijo ECS-.327 corresponde a un cristal de 32.768 kHz y el módulo ya incluye su cristal de 40 MHz (datasheet v1.8, sección de características). El MPN completo **POR DECIDIR**: el texto anterior estaba corrupto.

**Faltan en el BOM** (se añaden al dibujar el esquemático, sus valores salen de `hardware/pinout.md`): 2 resistencias de 4.7 kΩ (pull-up I2C), 1 de 100 kΩ (pull-down del optoacoplador), resistencias en serie de los LED, resistencia en serie del optoacoplador (la ARCHITECTURE usa 330 Ω), y los capacitores de carga del cristal de 32.768 kHz (a partir de CL: unos 18-22 pF cada uno).

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

**POR DECIDIR:** el voltaje del panel no está en este BOM. `docs/ARCHITECTURE.md` usa 6 V / 1.5 W y el diagrama del informe un "MPPT CN3791 6V". El CN3791 necesita un panel de voltaje coherente con su divisor MPPT: fijar un solo valor.

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
| Waveshare Core1262-HF | ARCHITECTURE.md | Módulo de prototipo, no va en la PCB |
| ESP32-S3-DevKitC-1 y AMS1117 | ARCHITECTURE.md | Placa de prototipo, no va en la PCB |
| PC817 | ARCHITECTURE.md | Optoacoplador de prototipo; la PCB usa el VO617A |
| A7670SA y BG95-M3 | ARCHITECTURE.md / informe | Se contradicen como módem de Fase 2. **POR DECIDIR** |
| Nordic Thingy:91 X | README, docs | Prueba de concepto, fuera de la PCB |

## Historial de revisiones

| Versión | Fecha | Cambios |
|---------|-------|---------|
| 1.0 | 2024-05-15 | Versión inicial |
| 1.1 | 2024-06-20 | Actualización de precios y MPN |
| 1.2 | 2026-10-30 | Estructura reparada (tabla solar, resistores y costos estaban desordenados). Eliminadas flash y SRAM externas (redundantes con el módulo). Cristal 40 MHz corregido a 32.768 kHz. MAX17048 añadido. Puntos bloqueantes marcados POR DECIDIR |
