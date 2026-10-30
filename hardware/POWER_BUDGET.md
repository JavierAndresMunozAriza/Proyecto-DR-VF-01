# Presupuesto de Potencia del Collar DR-VF-01

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
| DEEP_SLEEP | Solo IMU activo | ~190μA | 98.5% del tiempo |
| ACTIVE_GNSS | Posicionamiento activo | ~80mA | 20 segundos por evento |
| ACTIVE_LORA | Transmisión de datos | ~120mA (pico) | 0.5 segundos por transmisión |
| ACTIVE_DISUASION | Mecanismo de disuasión | ~50mA | 5 segundos por activación |

## Componentes del Sistema

### 1. Microcontrolador (ESP32-S3)
**Arquitectura de Bajo Consumo:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Deep Sleep**: 5μA típica (RTC memories retained)
- **Corriente en Active Mode**: 80mA promedio (CPU a 240MHz)
- **Corriente en TX Wi-Fi**: 190mA pico

**Estrategias de Optimización:**
- Uso de RTC peripherals durante sleep
- Clock gating para periféricos no utilizados
- Dynamic frequency scaling según carga de trabajo

### 2. Sistema de Posicionamiento (u-blox MAX-M10S)
**Modos de Operación Eficiente:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Standby**: 25μA
- **Corriente en Tracking**: 45mA promedio
- **Corriente en Acquisition**: 65mA pico

**Optimización de GNSS:**
- Modo backup para mantenimiento de almanaque
- Aceleración de TTFF mediante datos asistidos
- Sampling adaptativo basado en movimiento

### 3. Sensor de Movimiento (LSM6DSOX)
**Detección Inteligente de Eventos:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Sleep Mode**: 2μA
- **Corriente en Low-Power Mode**: 10μA
- **Corriente en Normal Mode**: 650μA

**Wake-up Strategy:**
- Threshold configurable para detección de movimiento significativo
- Duración de ventana para evitar falsos positivos
- Integración con sistema de interrupciones hardware

### 4. Radio LoRa (SX1262)
**Comunicación Eficiente:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Sleep**: 0.7μA
- **Corriente en Standby**: 1.5μA
- **Corriente en TX (+22dBm)**: 120mA pico
- **Corriente en RX**: 5.0mA

**Optimización de RF:**
- Adaptive spreading factor para condiciones de enlace
- Duty cycling para minimizar tiempo de aire
- Acknowledgment implícito mediante protocolo de aplicación

### 5. Optoacoplador (VO617A)
**Control de Disuasión Eficiente:**
- **Voltaje de operación**: 3.3V nominal
- **Corriente en Off**: 1μA
- **Corriente en On**: 50mA máxima

### 6. Cargador Solar (CN3791)
**Gestión Energética Inteligente:**
- **Voltaje de entrada**: 4.5-24V (panel solar)
- **Eficiencia típica**: 88% a 250mA salida
- **Corriente de operación**: 150μA en standby

## Perfiles de Operación

### 1. Estado de Sueño Profundo (SLEEP)
**Descripción**: Estado predominante donde solo el sensor IMU está activo monitoreando movimiento.

| Componente | Estado | Corriente | Duty Cycle | Contribución |
|------------|--------|-----------|------------|--------------|
| ESP32-S3 | Deep Sleep | 5μA | 98.5% | 4.93μA |
| MAX-M10S | Standby | 25μA | 98.5% | 24.63μA |
| LSM6DSOX | Low-Power | 10μA | 98.5% | 9.85μA |
| SX1262 | Sleep | 0.7μA | 98.5% | 0.69μA |
| CN3791 | Standby | 150μA | 100% | 150μA |
| **TOTAL** | | | | **190.09μA** |

### 2. Verificación de Geocerca (GEOFENCE_CHECK)
**Descripción**: Activación por detección de movimiento para verificar posición dentro de geocerca.

| Componente | Estado | Corriente | Duración | Contribución |
|------------|--------|-----------|----------|--------------|
| ESP32-S3 | Active | 80mA | 2s | 160mAs |
| MAX-M10S | Tracking | 45mA | 20s | 900mAs |
| LSM6DSOX | Normal | 650μA | 22s | 14.3mAs |
| SX1262 | Sleep | 1.5μA | 22s | 33μAs |
| CN3791 | Standby | 150μA | 22s | 3.3mAs |
| **TOTAL** | | | | **1077.63mAs** |

### 3. Transmisión LoRa (LORA_TX)
**Descripción**: Envío de coordenadas y estado del sistema cuando se detecta alerta.

| Componente | Estado | Corriente | Duración | Contribución |
|------------|--------|-----------|----------|--------------|
| ESP32-S3 | Active | 80mA | 1s | 80mAs |
| MAX-M10S | Standby | 25μA | 1s | 25μAs |
| LSM6DSOX | Sleep | 10μA | 1s | 10μAs |
| SX1262 | TX | 120mA | 0.5s | 60mAs |
| CN3791 | Standby | 150μA | 1s | 150μAs |
| **TOTAL** | | | | **140.185mAs** |

### 4. Activación de Disuasión (DISUASION)
**Descripción**: Activación del mecanismo de disuasión por violación de geocerca.

| Componente | Estado | Corriente | Duración | Contribución |
|------------|--------|-----------|----------|--------------|
| ESP32-S3 | Active | 80mA | 5s | 400mAs |
| MAX-M10S | Standby | 25μA | 5s | 125μAs |
| LSM6DSOX | Sleep | 10μA | 5s | 50μAs |
| SX1262 | Sleep | 1.5μA | 5s | 7.5μAs |
| VO617A | On | 50mA | 5s | 250mAs |
| CN3791 | Standby | 150μA | 5s | 750μAs |
| **TOTAL** | | | | **650.9325mAs** |

## Cálculo de Consumo Diario

### 1. Suposiciones de Uso
- **Movimiento detectado**: 10 veces por día
- **Transmisiones LoRa**: 20 veces por día (movimiento + reporte periódico)
- **Activaciones de disuasión**: 2 veces por día
- **Verificaciones GNSS periódicas**: 4 veces por día (cada 6 horas)

### 2. Cálculo Detallado
| Actividad | Ocurrencias/día | Consumo por evento | Total diario |
|-----------|----------------|-------------------|--------------|
| SLEEP (base) | 86400s | 190.09μA promedio | 16.42 A·s |
| GEOFENCE_CHECK | 10 | 1077.63mAs | 10.78 A·s |
| LORA_TX | 20 | 140.185mAs | 2.80 A·s |
| DISUASION | 2 | 650.9325mAs | 1.30 A·s |
| **TOTAL** | | | **31.30 A·s** |

### 3. Conversión a Consumo Promedio
- **Consumo diario total**: 31.30 A·s ÷ 3600 s/h ÷ 24 h/día = 0.362 mA promedio
- **Corriente promedio diaria**: 0.362 mA

## Autonomía de Batería

### 1. Cálculo con Batería de 1200mAh
- **Capacidad nominal**: 1200 mAh
- **Factor de degradación**: 0.85 (considerando temperatura y envejecimiento)
- **Capacidad efectiva**: 1200 mAh × 0.85 = 1020 mAh
- **Autonomía teórica**: 1020 mAh ÷ 0.362 mA = 2818 horas ≈ 117 días

### 2. Cálculo con Batería de 1500mAh
- **Capacidad nominal**: 1500 mAh
- **Factor de degradación**: 0.85
- **Capacidad efectiva**: 1500 mAh × 0.85 = 1275 mAh
- **Autonomía teórica**: 1275 mAh ÷ 0.362 mA = 3522 horas ≈ 147 días

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
| Condiciones ideales | 117 días | 147 días |
| Temperatura extrema | 82 días | 103 días |
| 500 ciclos | 94 días | 117 días |
| 1000 ciclos | 76 días | 95 días |

## Carga Solar

### 1. Panel Solar de 2W
- **Voltaje de circuito abierto**: 5.5V
- **Corriente de cortocircuito**: 400mA
- **Eficiencia típica**: 18-20%

### 2. Generación Diaria Promedio
- **Horas de sol efectivas**: 4.5 horas (promedio global)
- **Generación diaria**: 2W × 4.5h = 9Wh
- **Corriente generada**: 9Wh ÷ 3.7V = 2430mAh equivalentes diarios

### 3. Balance Energético
- **Consumo diario**: 0.362 mA × 24h = 8.69 mAh
- **Generación diaria**: 2430 mAh
- **Balance neto**: +2421 mAh (superavit significativo)

## Optimización de Consumo

### 1. Estrategias Previstas
- **Deep Sleep agresivo**: 98.5% del tiempo en modo ultra bajo consumo
- **Despertar selectivo**: Solo por eventos significativos del IMU
- **Gestión inteligente de GNSS**: Solo activo cuando necesario
- **Modulación adaptativa LoRa**: Ajuste de SF según condiciones de enlace

### 2. Mejoras Potenciales
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

El sistema DR-VF-01 está diseñado para operar durante más de 100 días con una sola carga (estimación teórica: 117 días con 1200 mAh y 147 días con 1500 mAh), gracias a:
1. Una estrategia de sueño profundo eficiente (>98% del tiempo)
2. Componentes de bajo consumo seleccionados cuidadosamente
3. Un sistema de carga solar robusto que supera ampliamente el consumo
4. Algoritmos optimizados para minimizar el tiempo activo

Este presupuesto de potencia asegura que el collar pueda operar confiablemente en aplicaciones ganaderas extensivas sin necesidad de mantenimiento frecuente, siguiendo las mejores prácticas de diseño de sistemas embebidos descritas por Elecia White en "Making Embedded Systems".
