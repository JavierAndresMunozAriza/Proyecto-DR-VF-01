# Arquitectura Técnica — Proyecto DR-VF-01
**Sistema Modular de Cerca Virtual para Ganado**  
**Versión:** 1.0  
**Fecha:** 18/10/2026  
**Documento Principal:** [PROJECT_CONTEXT.md](./PROJECT_CONTEXT.md)

---

## 1. Arquitectura de Alimentación en Dos Niveles

El sistema utiliza una topología distribuida que aprovecha el regulador interno LDO de la placa DevKit:

```text
[Panel Solar 6V] ──► [CN3791 MPPT] ──► [Batería LiPo 3.7V]
                                             │
                                             ├─► [MAX17048 Fuel Gauge]
                                             │
                                             ▼
                                  [Pin VIN ESP32-S3]
                                             │
                                  (LDO Interno 3.3V)
                                             │
                                             ▼
                                  [Bus 3.3V de la PCB]
                                             ├─► GPS u-blox MAX-M10S
                                             ├─► LoRa Core1262
                                             └─► IMU LSM6DSOX
```

- **Línea No Regulada (3.6V - 4.2V):** Conecta la salida de la batería directamente al pin `VIN` del ESP32-S3 y al medidor MAX17048.
- **Línea Regulada (3.3V Estrictos):** Suministrada por el pin `3V3` del ESP32-S3. Entrega hasta 500 mA, cubriendo la demanda máxima del sistema (~224 mA).

**Nota Técnica:** El LDO interno de la DevKitC-1 (AMS1117) tiene una caída de voltaje de ~1V. Con batería a 3.6V, el voltaje regulado podría caer por debajo de 3.3V. Requiere verificación del esquemático de la placa.

## 2. Sustento Teórico del Cargador Solar CN3791

### 2.1 Inductancia del Convertidor Buck (L)
Para un rizado de corriente del 30% (ΔI_L = 0.3 A), voltaje de entrada V_IN = 6.0 V, voltaje de batería V_BAT = 4.2 V:

$$L = \frac{V_{BAT} \cdot (V_{IN} - V_{BAT})}{V_{IN} \cdot f_{sw} \cdot \Delta I_L} = \frac{4.2 \cdot (6.0 - 4.2)}{6.0 \cdot 300,000 \cdot 0.3} = \frac{7.56}{540,000} = 14\ \mu\text{H}$$

**Selección Comercial:** Inductor de potencia SMD de **10 μH a 22 μH** (estándar en el módulo CN3791).

### 2.2 Divisor Tensión MPPT (Fijación a 6V)
El pin `MPPT` compara el voltaje del panel con una referencia interna:

$$V_{MPPT} = V_{ref} \cdot \left(1 + \frac{R_1}{R_2}\right)$$

Con V_MPPT = 6.0 V y asumiendo V_ref = 2.0 V:
$$6.0 = 2.0 \cdot \left(1 + \frac{R_1}{R_2}\right) \implies R_1 = 2 \cdot R_2$$

**Selección Comercial:** R₁ = 200 kΩ, R₂ = 100 kΩ (1% precisión).

**Nota Técnica:** La referencia interna del CN3791 debe verificarse en el datasheet (posiblemente 1.04V en lugar de 2.0V).

### 2.3 Sensado de Corriente (R_CS)
Para limitar la carga rápida a 1 A con referencia de 120 mV:

$$R_{CS} = \frac{0.12\text{ V}}{1.0\text{ A}} = 0.12\ \Omega\ (120\text{ m}\Omega)$$

## 3. Consumo de Corriente y Análisis de Picos

| Módulo | Estado Reposo | Estado Activo | Pico Máximo | Observaciones |
| :--- | :--- | :--- | :--- | :--- |
| **ESP32-S3** | 10 μA (Deep Sleep) | 70 mA (240MHz) | 240 mA (WiFi/BT) | Wi-Fi activo solo en modo de configuración (SoftAP); deshabilitado en operación normal |
| **u-blox MAX-M10S** | 15 μA | 12 mA | 35 mA | Adquisición multiconstelación |
| **Core1262 (SX1262)** | 0.6 μA | 5.3 mA (Rx) | 118 mA (Tx @ +22dBm) | Ráfagas de fracciones de segundo |
| **LSM6DSOX** | 3 μA | 0.55 mA | 1 mA | Sensor de movimiento continuo |
| **MAX17048** | 3 μA | 0.023 mA | 0.05 mA | Medición I2C |
| **Total Sistema** | **< 50 μA** | **~88 mA** | **~224 mA** | **Dentro del margen de 500mA del LDO** |

**Nota sobre escalabilidad a 4G (A7670SA):** Los módulos celulares generan picos de hasta 2.0 A. En implementación futura, se alimentarán directamente de la batería LiPo con capacitor de desacople dedicado de 470 μF.

## 4. Protecciones e Integridad de Señal

1. **Desacople en Alta Frecuencia:** Capacitores cerámicos 100 nF (0805) en pines de alimentación de cada módulo
2. **Capacitor Tanque (Bulk):** Capacitor de 10 μF - 100 μF en bus de 3.3V para transitorios LoRa
3. **Resistencias de Pull-Up I2C:** Dos resistencias de 4.7 kΩ en líneas SDA y SCL
4. **Red RC de Reset:** 10 kΩ a 3.3V y 1 μF a GND en pin EN del ESP32-S3
5. **Aislamiento Galvánico para Actuador:** Optoacoplador VO617A con resistencia de 330 Ω (limita corriente GPIO a ~6.6 mA) y pull-down de 10 kΩ

## 5. Especificaciones de Hardware y BOM

### 5.1 Estrategia de Diseño de PCB (Carrier Board)
Diseño de PCB receptora con conectores hembra (Female Headers paso 2.54mm) para:
- Cero fallas por falsos contactos mecánicos
- Intercambio modular de componentes
- Cumplimiento de estándares profesionales Altium

### 5.2 Reglas de Diseño en Altium Designer
- **Capas:** Top/Bottom, FR4, 1.6mm espesor
- **Plano de Masa:** Vertido de cobre GND en ambas capas
- **Pistas Alimentación (VIN, 3V3, BAT):** 0.6 mm (24 mils)
- **Pistas Señal (SPI, I2C, UART):** 0.25 mm (10 mils)

### 5.3 Asignación de Pines ESP32-S3 (PCB final y DevKitC-1 N16R8)

El único mapa de pines vigente es [hardware/pinout.md](../hardware/pinout.md). La PCB final (ESP32-S3-WROOM-1U-N16R8) y la DevKitC-1 (N16R8) usan los mismos GPIO, de modo que el firmware es idéntico en ambas. Resumen:

| Función | Periférico | GPIO |
| :--- | :--- | :--- |
| Bus SPI | LoRa SX1262 | NSS 10, MOSI 11, SCK 12, MISO 13 |
| Control LoRa | SX1262 | BUSY 14, DIO1 6 (despierta), NRESET 5 |
| Bus I2C | LSM6DSOX, MAX17048 | SDA 8, SCL 9 (pull-up 4.7 kΩ) |
| UART1 | GNSS MAX-M10S | TX 17 (hacia RXD), RX 18 (desde TXD) |
| GNSS | MAX-M10S EXTINT | 21 (despierta) |
| IMU | LSM6DSOX INT1 | 4 (despierta de Deep Sleep) |
| Actuador | Optoacoplador VO617A (`OPTO_CTRL`) | 1 |
| Buzzer | PWM | 2 |
| Botón de usuario | Entrada | 7 (despierta) |
| LED RGB | R / G / B | 38 / 39 / 40 |
| Estado de carga | CN3791 CHRG | 41 |
| Módem Fase 2 | UART2 TX / RX, PWRKEY | 47 / 48, 42 |
| Consola | UART0 TX / RX | 43 / 44 (puntos de prueba) |

**Nota Técnica:** El GNSS usa UART1 (GPIO 17/18), no el bus I2C. GPIO 43/44 (UART0) quedan como puntos de prueba para consola. Las reglas del módulo (pines de strapping, USB nativo en GPIO 19/20, PSRAM octal) están en [hardware/pinout.md](../hardware/pinout.md).

### 5.4 Bill of Materials (BOM) — Componentes Clave

| Cantidad | Componente | Descripción | Función |
| :--- | :--- | :--- | :--- |
| 1 | ESP32-S3-DevKitC-1-N16R8 | Placa DevKit Dual-Core 240MHz | Procesador Principal |
| 1 | Waveshare Core1262-HF | Módulo LoRa SX1262 (915 MHz) | Transceptor RF |
| 1 | u-blox MAX-M10S | Módulo Breakout GNSS | Geolocalización |
| 1 | ST LSM6DSOX | Breakout I2C/SPI IMU 6-DOF | Detección de Movimiento |
| 1 | MAX17048 | Breakout I2C Fuel Gauge | Telemetría de Batería |
| 1 | Módulo CN3791 (6V) | Cargador Solar MPPT Buck | Gestión Energética |
| 1 | Batería LiPo 3.7V 1200mAh | Celda 1S con PCM | Fuente de Energía |
| 1 | Panel Solar 6V 1.5W-3W | Monocristalino/Policristalino | Generación Fotovoltaica |
| 1 | Optoacoplador VO617A/PC817 | DIP-4 Through-Hole | Aislamiento del Actuador |
| 1 | LED 3mm Rojo | Through-Hole | Indicador visual de disparo |
| 5 | Capacitores 100 nF | Cerámico SMD 0805 | Desacople de Alta Frecuencia |
| 1 | Capacitor 100 μF/16V | Electrolítico/Tántalo | Capacitor Tanque 3.3V |
| 2 | Resistencias 4.7 kΩ | SMD 0805 | Pull-Ups I2C |
| 1 | Resistencia 330 Ω | Through-Hole/SMD 0805 | Limitadora Optoacoplador |
| 1 | Resistencia 10 kΩ | Through-Hole/SMD 0805 | Pull-Down Salida Optoacoplador |
| N | Headers Hembra/Macho 2.54mm | Tiras de Pinos | Zócalos para Carrier Board |

**Documentos Canónicos:** Para la lista completa de materiales y pinout detallado, ver:
- [hardware/BOM.md](../hardware/BOM.md)
- [hardware/pinout.md](../hardware/pinout.md)

## 6. Arquitectura de Firmware (STR / Zephyr)

El firmware es un único código base sobre **Zephyr RTOS** para dos placas: la prueba de concepto en la **Nordic Thingy:91 X** (nRF9151, nRF Connect SDK, LTE-M/NB-IoT/GNSS) y el producto final en la **PCB con ESP32-S3-WROOM-1U-N16R8** (LoRa SX1262, GNSS externo MAX-M10S, IMU LSM6DSOX). Se comparten `geofence.c`, la FSM y el logging; cada placa aporta su overlay, su `prj.conf` y los adaptadores de GNSS, comunicaciones e IMU.

### 6.1 Distribución de Tareas por Núcleo

```text
               ┌──────────────────────────────────────────┐
               │         ESP32-S3 (Zephyr)                │
               └────────────────────┬─────────────────────┘
                                    │
          ┌──────────────────────────┴──────────────────────────┐
          ▼                                                     ▼
   PRO_CPU (Core 0)                                      APP_CPU (Core 1)
   Gestión de Red y Energía                              Procesamiento y Sensores
   ├─ Task_LoRaTX (Prioridad 3)                          ├─ Task_GPSRead (Prioridad 2)
   ├─ Task_PowerMgmt (Prioridad 1)                       ├─ Task_Geofence (Prioridad 4 - Crítica)
   └─ Queue_Telemetry                                    └─ Task_IMU_Check (Prioridad 1)
```

#### **Core 0 (PRO_CPU) — Mantenimiento, Energía y Comunicaciones:**
1. **`Task_LoRaTX` (Prioridad 3):** Despierta al recibir mensajes en `Queue_Telemetry`, formatea paquetes y transmite vía SPI al SX1262
2. **`Task_PowerMgmt` (Prioridad 1):** Lee batería desde MAX17048 cada 60s, gestiona transición a Deep Sleep

#### **Core 1 (APP_CPU) — Cálculo Determinista y Adquisición:**
1. **`Task_Geofence` (Prioridad 4):** Ejecuta algoritmo Ray-Casting/Point-in-Polygon, activa `OPTO_CTRL` (GPIO 1) si animal fuera de zona
2. **`Task_GPSRead` (Prioridad 2):** Decodifica tramas NMEA/UBX del u-blox MAX-M10S
3. **`Task_IMU_Check` (Prioridad 1):** Configura umbrales de interrupción por movimiento en LSM6DSOX

**Nota:** Los nombres `Task_*` y el reparto por núcleos (válido para el ESP32-S3) se conservan del diseño original. En Zephyr cada tarea es un hilo; la numeración de prioridades usada aquí (4 = más crítica) debe traducirse al esquema de Zephyr, donde un número menor indica mayor prioridad.

### 6.2 Algoritmo de Geocerca (Ray-Casting)

El polígono se almacena en Flash como arreglo de vértices (X_i, Y_i) representando Latitud y Longitud. El algoritmo evalúa si un punto (P_x, P_y) está dentro lanzando un rayo horizontal:

- **Contador Impar:** Punto DENTRO de la cerca (Estado Seguro)
- **Contador Par:** Punto FUERA de la cerca (Activar Estímulo + Alerta LoRa)

## 7. Hallazgos Técnicos y Puntos Abiertos

1. **Regulador LDO:** El AMS1117 de la DevKitC-1 tiene dropout voltage de ~1V. Con batería a 3.6V, el voltaje regulado podría caer por debajo de 3.3V. Requiere verificación del esquemático.
2. **Cálculo de Picos:** La suma de picos individuales (sin Wi-Fi, con el ESP32 a 70 mA) da ~224 mA, valor usado en la tabla. Con pico ESP32 de 240 mA da ~394 mA. Ambos valores caben en los 500 mA del LDO.
3. **Referencia MPPT CN3791:** La referencia interna debe verificarse en datasheet (posiblemente 1.04V en lugar de 2.0V).
4. **Consumo Deep Sleep:** El LDO y chip USB-UART de la DevKit probablemente superan el límite de 150 μA de RNF-03.

---
*Documento generado a partir de las secciones técnicas de PROJECT_CONTEXT.md*