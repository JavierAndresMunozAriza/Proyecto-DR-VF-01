# Presupuesto de Potencia del Collar DR-VF-01

**Estado:** REVISIÓN 2026-10-31. Las corrientes salen de los datasheets de `hardware/datasheets/` (cada componente cita su tabla). Lo que no se pudo verificar está marcado "no verificado" y listado en la sección "Pendiente de verificar".

## Descripción General

Este documento presenta el análisis detallado del presupuesto de potencia para el collar de geocercamiento DR-VF-01. El objetivo es maximizar la autonomía de la batería mientras se mantienen todas las funciones críticas del sistema operativas.

*Nota: Este análisis sigue los principios de gestión de energía de sistemas embebidos descritos en "Making Embedded Systems" por Elecia White, enfocándose en estrategias de bajo consumo y optimización de ciclos de sueño.*

## Estrategia de Gestión de Energía

### Filosofía de Diseño
Siguiendo los principios de Elecia White, nuestra estrategia se basa en:
1. **Minimizar el tiempo activo** - Mantener el sistema en estados de bajo consumo la mayor parte del tiempo
2. **Event-driven architecture** - Activación solo por eventos significativos
3. **Hierarchical power states** - Múltiples niveles de consumo según la actividad

### Estados de Operación
El sistema opera en diferentes estados de potencia según el modelo de máquinas de estados finitos:

| Estado | Descripción | Corriente Promedio | Tiempo Estimado |
|--------|-------------|-------------------|-----------------|
| DEEP_SLEEP | Solo IMU activo | ~124μA | 98.5% del tiempo |
| ACTIVE_GNSS | Posicionamiento activo (solo el MAX-M10S) | ~14mA | 20 segundos por evento |
| ACTIVE_LORA | Transmisión de datos | ~120mA (pico) | 0.5 segundos por transmisión |
| ACTIVE_DISUASION | Mecanismo de disuasión | ~50mA | 5 segundos por activación |

## Componentes del Sistema

### 1. Microcontrolador (módulo ESP32-S3-WROOM-1U-N16R8)
**Arquitectura de Bajo Consumo:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Deep Sleep**: 8μA típica con la memoria y los periféricos RTC encendidos (WROOM-1 datasheet v1.8, tabla 6-7). Con solo la memoria RTC son 7μA. La tabla del módulo no suma consumo de PSRAM en Deep-sleep (su nota de 140μA para la PSRAM octal de 8 MB aplica a Light-sleep)
- **Corriente en Active Mode**: 80mA promedio (CPU a 240MHz), no verificado
- **Corriente en TX Wi-Fi**: 190mA pico, no verificado. El Wi-Fi solo se usa en el modo de configuración

**Estrategias de Optimización:**
- Uso de RTC peripherals durante sleep
- Clock gating para periféricos no utilizados
- Dynamic frequency scaling según carga de trabajo

### 2. Sistema de Posicionamiento (u-blox MAX-M10S)
**Modos de Operación Eficiente** (datasheet MAX-M10S, UBX-20035208):
- **Voltaje de operación**: 3.3V nominal (VCC y V_IO)
- **Corriente en standby por software**: 46μA en V_IO con V_IO de 3.3V más 0.12μA en VCC (tabla 17). Con V_IO de 1.8V son 37μA
- **Corriente en hardware backup**: 28μA con VCC y V_IO apagados y V_BCKP de 3.3V (tabla 17). El diseño actual no tiene un interruptor que apague VCC y V_IO, así que no se usa
- **Corriente en Tracking (modo continuo)**: 9.5mA en VCC más 2.3mA en V_IO con GPS+Galileo+BeiDou B1I, la combinación por defecto (tabla 15, alimentación de 3.0V, 1Hz, 25°C)
- **Corriente en Acquisition**: 11.5mA en VCC más 2.3mA en V_IO, 13.8mA en total (tabla 15). Hay un pico de arranque de hasta 100mA

**Optimización de GNSS:**
- Modo backup para mantenimiento de almanaque
- Aceleración de TTFF mediante datos asistidos
- Sampling adaptativo basado en movimiento

### 3. Sensor de Movimiento (LSM6DSOX)
**Detección Inteligente de Eventos** (datasheet LSM6DSOX DS12814 Rev 4, tabla de características eléctricas):
- **Voltaje de operación**: 3.3V nominal (VDD de 1.71 a 3.6V)
- **Corriente en Power-down**: 3μA
- **Corriente en Low-Power Mode (acelerómetro)**: 4.5μA a 1.6Hz y 9.5μA a 50Hz
- **Corriente en alto rendimiento (acelerómetro y giroscopio)**: 0.55mA

**Wake-up Strategy:**
- Threshold configurable para detección de movimiento significativo
- Duración de ventana para evitar falsos positivos
- Integración con sistema de interrupciones hardware

### 4. Radio LoRa (módulo DX-LR30-900M22S con SX1262)
**Comunicación Eficiente** (datasheet SX1261/2 Rev 1.2, tabla de consumo; el módulo no tiene datasheet con corrientes):
- **Voltaje de operación**: 3.3V nominal (VBAT de 1.8 a 3.7V)
- **Corriente en Sleep**: 160nA con arranque en frío y 600nA con la configuración retenida. Se usa 0.6μA
- **Corriente en Standby (STDBY_RC)**: 0.6mA
- **Corriente en TX (+22dBm)**: 120mA pico
- **Corriente en RX**: 5.0mA, no verificado
- El consumo del módulo completo (conmutador de antena, oscilador) **no está verificado**

**Optimización de RF:**
- Adaptive spreading factor para condiciones de enlace
- Duty cycling para minimizar tiempo de aire
- Acknowledgment implícito mediante protocolo de aplicación

### 5. Optoacoplador (VO617A)
**Control de Disuasión Eficiente:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Off**: 1μA, no verificado
- **Corriente en On**: 50mA máxima, no verificado

### 6. Cargador Solar (CN3791)
**Gestión Energética Inteligente** (datasheet CN3791 Rev 1.0):
- **Voltaje de entrada**: 4.5-28V; con el divisor de 6V solo inicia carga por encima de unos 6.2V
- **Eficiencia típica**: 88% a 250mA salida, no verificado
- **Corriente desde la batería en sleep** (sin entrada, de noche): unos 30μA (9μA en los pines BAT y CSP más unos 21μA por el MOSFET de paso). No incluye la fuga del diodo de recirculación D2, que depende de la pieza elegida
- **Corriente de operación** (desde el panel, sin conmutar): 1.0mA típica. No sale de la batería

### 7. Regulador LDO (TLV75733P)
**Alimentación de 3.3V desde la batería** (datasheet TLV757P, SBVS322C):
- **Voltaje de entrada**: 1.45 a 5.5V
- **Corriente máxima de salida**: 1A
- **Corriente quiescente (Iq)**: 25μA típica, siempre presente. El TPS7A0533 anterior tenía 1μA pero solo entrega 200mA, menos que las cargas del sistema (224 a 394mA)

### 8. Medidor de Batería (MAX17048)
**Fuel gauge por I2C** (datasheet MAX17048 Rev 7, p. 1):
- **Voltaje de operación**: VDD de 2.5 a 4.5V (se conecta a la batería)
- **Corriente en hibernación**: 3μA. Entra solo cuando detecta un estado de baja corriente
- **Corriente activa**: 23μA

### 9. Cargador USB-C (TP4056)
**Carga lineal de 500mA** (datasheet TP4056):
- **Voltaje de entrada**: 4.0 a 8.0V (USB-C de 5V)
- **Corriente desde la batería en standby**: 2.5μA típica y 6μA máxima (tabla de características). Sin USB conectado no está tabulada; se usa el mismo valor
- Solo carga con USB conectado (carga de banco)

## Perfiles de Operación

### 1. Estado de Sueño Profundo (SLEEP)
**Descripción**: Estado predominante donde solo el sensor IMU está activo monitoreando movimiento.

| Componente | Estado | Corriente | Duty Cycle | Contribución |
|------------|--------|-----------|------------|--------------|
| ESP32-S3 | Deep Sleep | 8μA | 98.5% | 7.88μA |
| MAX-M10S | Standby por software | 46.12μA | 98.5% | 45.43μA |
| LSM6DSOX | Low-Power | 9.5μA | 98.5% | 9.36μA |
| SX1262 | Sleep | 0.6μA | 98.5% | 0.59μA |
| CN3791 | Sleep (sin entrada) | 30μA | 100% | 30μA |
| TLV75733P | Iq | 25μA | 100% | 25μA |
| MAX17048 | Hibernación | 3μA | 100% | 3μA |
| TP4056 | Standby | 2.5μA | 100% | 2.5μA |
| **TOTAL** | | | | **123.76μA** |

El total cumple RNF-03 (< 150μA) con 26μA de margen. El margen es corto: faltan por sumar los consumos de la sección "Pendiente de verificar".

### 2. Verificación de Geocerca (GEOFENCE_CHECK)
**Descripción**: Activación por detección de movimiento para verificar posición dentro de geocerca.

| Componente | Estado | Corriente | Duración | Contribución |
|------------|--------|-----------|----------|--------------|
| ESP32-S3 | Active | 80mA | 2s | 160mAs |
| MAX-M10S | Acquisition/Tracking | 13.8mA | 20s | 276mAs |
| LSM6DSOX | Alto rendimiento | 550μA | 22s | 12.1mAs |
| SX1262 | Sleep | 0.6μA | 22s | 13.2μAs |
| CN3791 | Sleep (sin entrada) | 30μA | 22s | 0.66mAs |
| TLV75733P | Iq | 25μA | 22s | 0.55mAs |
| MAX17048 | Activo | 23μA | 22s | 0.506mAs |
| TP4056 | Standby | 2.5μA | 22s | 0.055mAs |
| **TOTAL** | | | | **449.88mAs** |

### 3. Transmisión LoRa (LORA_TX)
**Descripción**: Envío de coordenadas y estado del sistema cuando se detecta alerta.

| Componente | Estado | Corriente | Duración | Contribución |
|------------|--------|-----------|----------|--------------|
| ESP32-S3 | Active | 80mA | 1s | 80mAs |
| MAX-M10S | Standby | 46.12μA | 1s | 46.12μAs |
| LSM6DSOX | Low-Power | 9.5μA | 1s | 9.5μAs |
| SX1262 | TX | 120mA | 0.5s | 60mAs |
| CN3791 | Sleep (sin entrada) | 30μA | 1s | 30μAs |
| TLV75733P | Iq | 25μA | 1s | 25μAs |
| MAX17048 | Activo | 23μA | 1s | 23μAs |
| TP4056 | Standby | 2.5μA | 1s | 2.5μAs |
| **TOTAL** | | | | **140.14mAs** |

### 4. Activación de Disuasión (DISUASION)
**Descripción**: Activación del mecanismo de disuasión por violación de geocerca.

| Componente | Estado | Corriente | Duración | Contribución |
|------------|--------|-----------|----------|--------------|
| ESP32-S3 | Active | 80mA | 5s | 400mAs |
| MAX-M10S | Standby | 46.12μA | 5s | 230.6μAs |
| LSM6DSOX | Low-Power | 9.5μA | 5s | 47.5μAs |
| SX1262 | Sleep | 0.6μA | 5s | 3μAs |
| VO617A | On | 50mA | 5s | 250mAs |
| CN3791 | Sleep (sin entrada) | 30μA | 5s | 150μAs |
| TLV75733P | Iq | 25μA | 5s | 125μAs |
| MAX17048 | Activo | 23μA | 5s | 115μAs |
| TP4056 | Standby | 2.5μA | 5s | 12.5μAs |
| **TOTAL** | | | | **650.68mAs** |

## Cálculo de Consumo Diario

### 1. Suposiciones de Uso
- **Movimiento detectado**: 10 veces por día
- **Transmisiones LoRa**: 20 veces por día (movimiento + reporte periódico)
- **Activaciones de disuasión**: 2 veces por día
- **Verificaciones GNSS periódicas**: 4 veces por día (cada 6 horas)

### 2. Cálculo Detallado
| Actividad | Ocurrencias/día | Consumo por evento | Total diario |
|-----------|----------------|-------------------|--------------|
| SLEEP (base) | 86400s | 123.76μA promedio | 10.693 A·s |
| GEOFENCE_CHECK | 10 | 449.88mAs | 4.499 A·s |
| LORA_TX | 20 | 140.14mAs | 2.803 A·s |
| DISUASION | 2 | 650.68mAs | 1.301 A·s |
| **TOTAL** | | | **19.296 A·s** |

### 3. Conversión a Consumo Promedio
- **Consumo diario total**: 19.296 A·s ÷ 3600 s/h ÷ 24 h/día = 0.223 mA promedio
- **Corriente promedio diaria**: 0.223 mA

## Autonomía de Batería

### 1. Cálculo con Batería de 1200mAh
- **Capacidad nominal**: 1200 mAh
- **Factor de degradación**: 0.85 (considerando temperatura y envejecimiento)
- **Capacidad efectiva**: 1200 mAh × 0.85 = 1020 mAh
- **Autonomía teórica**: 1020 mAh ÷ 0.223 mA = 4574 horas ≈ 191 días

### 2. Cálculo con Batería de 1500mAh
- **Capacidad nominal**: 1500 mAh
- **Factor de degradación**: 0.85
- **Capacidad efectiva**: 1500 mAh × 0.85 = 1275 mAh
- **Autonomía teórica**: 1275 mAh ÷ 0.223 mA = 5717 horas ≈ 238 días

## Consideraciones Ambientales

### 1. Efecto de la Temperatura
- **A -20°C**: Reducción de capacidad del 30%
- **A +45°C**: Reducción de capacidad del 15%
- **A +65°C**: Reducción de capacidad del 25%

### 2. Efecto del Envejecimiento
- **Después de 500 ciclos**: Reducción de capacidad del 20%
- **Después de 1000 ciclos**: Reducción de capacidad del 35%

### 3. Autonomía Ajustada
| Condición | Autonomía 1200mAh | Autonomía 1500mAh |
|-----------|-------------------|-------------------|
| Condiciones ideales | 191 días | 238 días |
| Temperatura extrema | 133 días | 167 días |
| 500 ciclos | 152 días | 191 días |
| 1000 ciclos | 124 días | 155 días |

## Carga Solar

### 1. Panel Solar de 6V (modelo pendiente)
- **Tensión nominal**: 6V con una potencia de al menos 1W (BOM sección 2.3). El MPN está por decidir
- **Condición con el CN3791**: el divisor de 6V solo permite iniciar la carga con más de unos 6.2V en la entrada. El panel debe tener una tensión de circuito abierto claramente mayor y su punto de máxima potencia cerca de 6V
- **Eficiencia del cargador**: 88%, no verificado

### 2. Generación Diaria Promedio
- **Horas de sol efectivas**: 4.5 horas (promedio global), suposición
- **Energía diaria del panel**: 1W × 4.5h = 4.5Wh
- **Carga equivalente en la batería**: 4.5Wh ÷ 3.7V × 0.88 = 1070mAh por día

### 3. Balance Energético
- **Consumo diario**: 0.223 mA × 24h = 5.35 mAh
- **Generación diaria**: 1070 mAh
- **Balance neto**: +1065 mAh (superávit significativo con sol; de noche o con el panel cubierto manda la batería)

## Pendiente de verificar

Consumos que todavía no están en las tablas, porque no hay datasheet verificado en el repo o la pieza no está definida:
- **USBLC6-2SC6**: corriente de fuga de la protección ESD (datasheet de ST pendiente de bajar).
- **Protección de batería**: el código `BQ2975DRBR` del BOM no existe; sin pieza definida no hay corriente quiescente.
- **Módulo DX-LR30-900M22S completo**: no tiene datasheet con corrientes; se usan las del SX1262.
- **Diodo D2 del CN3791**: la fuga de la pieza que se elija se suma a los 30μA.
- **Zumbador y LEDs**: sin consumo en sleep si sus transistores o resistencias no dejan fuga; falta confirmarlo en el esquemático.
- **ESP32-S3 en Active (80mA) y RX del SX1262 (5mA)**: valores heredados sin fuente.
- **TP4056 sin USB**: el datasheet solo tabula el standby con entrada presente.

## Optimización de Consumo

### 1. Estrategias Previstas
- **Deep Sleep agresivo**: 98.5% del tiempo en modo ultra bajo consumo
- **Despertar selectivo**: Solo por eventos significativos del IMU
- **Gestión inteligente de GNSS**: Solo activo cuando necesario
- **Modulación adaptativa LoRa**: Ajuste de SF según condiciones de enlace

### 2. Mejoras Potenciales
- **Apagar VCC y V_IO del MAX-M10S** con un interruptor de carga y usar el hardware backup (28μA en lugar de 46μA en standby), con el costo de un arranque más lento
- **Implementación de BLE 5.0**: Para configuración más eficiente
- **Optimización de algoritmo PIP**: Reducción de tiempo de cálculo
- **Uso de coprocesador**: Para funciones de detección de movimiento

## Análisis de Fiabilidad y Tolerancia a Fallos

### Estrategia de Recuperación de Energía
Siguiendo los principios de sistemas tolerantes a fallos de Elecia White:
1. **Brown-out detection** - Monitoreo continuo del estado de carga
2. **Graceful degradation** - Reducción de funcionalidad bajo baja energía
3. **Predictive maintenance** - Alertas proactivas de estado crítico

### Monitoreo de Salud del Sistema
- Voltage monitoring para detección de problemas de alimentación
- Temperature compensation para precisión en condiciones extremas
- Self-diagnostics para identificación de componentes degradados

## Conclusiones

El sistema DR-VF-01 está diseñado para operar durante más de 100 días con una sola carga (estimación teórica: 191 días con 1200 mAh y 238 días con 1500 mAh), gracias a:
1. Una estrategia de sueño profundo eficiente (>98% del tiempo) con un consumo de 123.76μA, por debajo del límite de 150μA de RNF-03
2. Componentes de bajo consumo seleccionados cuidadosamente
3. Un sistema de carga solar robusto que supera ampliamente el consumo
4. Algoritmos optimizados para minimizar el tiempo activo

El margen frente a RNF-03 es de 26μA y los consumos de la sección "Pendiente de verificar" pueden consumirlo. El mayor aporte del sleep es el MAX-M10S en standby (45μA), seguido del CN3791 (30μA) y el LDO (25μA).

Este presupuesto de potencia asegura que el collar pueda operar confiablemente en aplicaciones ganaderas extensivas sin necesidad de mantenimiento frecuente, siguiendo las mejores prácticas de diseño de sistemas embebidos descritas por Elecia White en "Making Embedded Systems".
