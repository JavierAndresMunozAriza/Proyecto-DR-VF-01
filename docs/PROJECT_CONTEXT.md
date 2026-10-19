# Documento Maestro — Proyecto DR-VF-01
**Sistema Modular de Cerca Virtual para Ganado (*Virtual Fencing System*)**  
**Ubicación de Despliegue:** Zona rural del Departamento del Huila, Colombia  
**Fase Actual:** Desarrollo de Prototipo Funcional (*Carrier Board*) y Firmware Base (Fase 1 & 2)

---

## 1. Visión General y Objetivos del Proyecto

### 1.1 Contexto y Problemática
La ganadería extensiva en regiones rurales de Colombia, como el departamento del Huila, enfrenta altos costos de infraestructura asociados al cercado tradicional (alambre de púas, postes de madera y mantenimiento por vandalismo o deterioro ambiental). El proyecto **DR-VF-01** busca desarrollar un dispositivo embebido portátil (collar) capaz de monitorear la ubicación geográfica del ganado en tiempo real, delimitar áreas de pastoreo mediante geocercas dinámicas (*Geofencing*) y aplicar estímulos disuasivos progresivos para mantener a los animales dentro de los límites preestablecidos.

### 1.2 Justificación Conceptual y Viabilidad
El sistema basa su funcionamiento en la integración de tecnologías de ultra bajo consumo (*Ultra-Low Power*) y transmisión de largo alcance:
1. **Sostenibilidad Energética:** Uso de recarga fotovoltaica asistida por seguimiento del punto de máxima potencia (MPPT) y algoritmos de gestión de energía (*Deep Sleep* reactivo).
2. **Autonomía Operativa:** Comunicación de telemetría a través de LoRa en la banda de $915\text{ MHz}$, eliminando la dependencia inicial de cobertura celular y costos por suscripción de datos en campo.
3. **Robustez y Modularidad:** Arquitectura tipo *Carrier Board* (Placa Madre) que integra módulos comerciales de alta precisión, minimizando el riesgo de fallo en PCB y permitiendo una escalabilidad gradual hacia la etapa de tesis.

### 1.3 Plan de Escalabilidad y Hoja de Ruta
* **Fase 1 (MVP de Escritorio - Semestre Actual):** Integración en protoboard del ESP32-S3, GPS MAX-M10S y LoRa SX1262. Verificación del algoritmo *Point-in-Polygon* y transmisión punto a punto.
* **Fase 2 (Autonomía y Monitoreo de Estado):** Incorporación del gestor solar CN3791, indicador de batería MAX17048 e IMU LSM6DSOX para despertar por movimiento (*Wake-on-Motion*).
* **Fase 3 (Integración de Hardware - Entregable Final de Semestre):** Diseño e impresión de PCB tipo *Carrier Board* en Altium Designer. Ensamble modular sin cables sueltos y prueba de concepto del actuador aislado (Optoacoplador + LED).
* **Fase 4 (Escalabilidad para Tesis):**
  * Integración de módulo celular 4G/LTE (A7670SA) para zonas con cobertura híbrida.
  * Implementación de modelos TinyML en la IMU para clasificación del comportamiento animal (pastoreo, descanso, huida).
  * Generador de pulsos de alto voltaje real con aislamiento de grado industrial y encapsulado con certificación IP67.

---

## 2. Resumen de Requerimientos

Para la matriz completa con trazabilidad, métricas y estado de implementación, ver: [REQUIREMENTS.md](./REQUIREMENTS.md)

### 2.1 Requerimientos Funcionales Principales
* **RF-01:** Geolocalización GNSS con error < 2.5 m
* **RF-02:** Evaluación de geocerca en < 50 ms  
* **RF-03:** Activación de optoacoplador/LED al cruzar límites
* **RF-04:** Telemetría LoRa en 915 MHz
* **RF-05:** Monitoreo continuo de batería vía I2C

### 2.2 Requerimientos No Funcionales Clave
* **RNF-01:** Autonomía de 7 días en oscuridad (1200 mAh)
* **RNF-02:** Procesamiento determinista en FreeRTOS
* **RNF-03:** Consumo < 150 μA en Deep Sleep
* **RNF-04:** Fiabilidad mecánica sin cables de puente

---

## 3. Resumen de Arquitectura Técnica

Para detalles técnicos completos, cálculos y justificativos, ver: [ARCHITECTURE.md](./ARCHITECTURE.md)

### 3.1 Arquitectura de Alimentación
Sistema en dos niveles: línea no regulada (3.6-4.2V) para VIN ESP32 y MAX17048, y línea regulada (3.3V estrictos) desde LDO interno del ESP32 para módulos periféricos.

### 3.2 Componentes Críticos
* **Procesamiento:** ESP32-S3 Dual-Core 240MHz
* **Comunicación:** LoRa SX1262 (915 MHz)
* **Geolocalización:** u-blox MAX-M10S GNSS
* **Energía:** CN3791 MPPT, MAX17048 Fuel Gauge
* **Detección:** LSM6DSOX IMU 6-DOF

### 3.3 Consumo Eléctrico
* **Reposo:** < 50 μA (Deep Sleep)
* **Activo:** ~88 mA
* **Pico Máximo:** ~221 mA (dentro de los 500 mA del LDO)

### 3.4 Firmware STR/FreeRTOS
Arquitectura dual-core con 5 tareas distribuidas:
* **Core 0:** Gestión de red (LoRa) y energía
* **Core 1:** Procesamiento determinista (geocerca, GPS, IMU)

---

---

## 3. Decisiones de Diseño Electrónico y Justificación Técnica

### 3.1 Arquitectura de Alimentación en Dos Niveles

El sistema utiliza una topología distribuida que aprovecha el regulador interno LDO de la placa DevKit para evitar la adición de etapas de regulación externas redundantes:

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

* **Línea No Regulada ($3.6\text{V} - 4.2\text{V}$):** Conecta la salida de la batería directamente al pin `VIN` del ESP32-S3 y al medidor MAX17048.
* **Línea Regulada ($3.3\text{V}$ Estrictos):** Suministrada por el pin `3V3` del ESP32-S3. Entrega hasta $500\text{ mA}$, cubriendo sobradamente la demanda máxima del sistema ($\sim 221\text{ mA}$).

### 3.2 Sustento Teórico del Cargador Solar CN3791
El CN3791 es un convertidor reductor conmutado (*Buck*) especializado con frecuencia de conmutación de $f_{sw} = 300\text{ kHz}$.

#### **A. Inductancia del Convertidor Buck ($L$)**
Para un rizado de corriente del $30\%$ ($\Delta I_L = 0.3\text{ A}$), voltaje de entrada $V_{IN} = 6.0\text{ V}$, voltaje de batería $V_{BAT} = 4.2\text{ V}$:

$$L = \frac{V_{BAT} \cdot (V_{IN} - V_{BAT})}{V_{IN} \cdot f_{sw} \cdot \Delta I_L} = \frac{4.2 \cdot (6.0 - 4.2)}{6.0 \cdot 300,000 \cdot 0.3} = \frac{7.56}{540,000} = 14\ \mu\text{H}$$

* **Selección Comercial:** Inductor de potencia SMD de **$10\ \mu\text{H}$ a $22\ \mu\text{H}$** (Estándar en el módulo CN3791).

#### **B. Divisor Tensión MPPT (Fijación a 6V)**
El pin `MPPT` compara el voltaje del panel con una referencia interna de $2.0\text{ V}$:

$$V_{MPPT} = 2.0\text{ V} \cdot \left(1 + \frac{R_1}{R_2}\right) \implies 6.0 = 2.0 \cdot \left(1 + \frac{R_1}{R_2}\right) \implies R_1 = 2 \cdot R_2$$

* **Selección Comercial:** $R_1 = 200\text{ k}\Omega$, $R_2 = 100\text{ k}\Omega$ ($1\%$ precisión).

#### **C. Senso de Corriente ($R_{CS}$)**
Para limitar la carga rápida a $1\text{ A}$ con referencia de $120\text{ mV}$:

$$R_{CS} = \frac{0.12\text{ V}}{1.0\text{ A}} = 0.12\ \Omega\ (120\text{ m}\Omega)$$

### 3.3 Consumo de Corriente y Análisis de Picos

| Módulo | Estado Reposo | Estado Activo | Pico Máximo | Observaciones |
| :--- | :--- | :--- | :--- | :--- |
| **ESP32-S3** | $10\ \mu\text{A}$ (*Deep Sleep*) | $70\text{ mA}$ (240MHz) | $240\text{ mA}$ (WiFi/BT) | WiFi deshabilitado en firmware |
| **u-blox MAX-M10S** | $15\ \mu\text{A}$ | $12\text{ mA}$ | $35\text{ mA}$ | Adquisición multiconstelación |
| **Core1262 (SX1262)** | $0.6\ \mu\text{A}$ | $5.3\text{ mA}$ (Rx) | $118\text{ mA}$ (Tx @ +22dBm) | Ráfagas de raras fracciones de segundo |
| **LSM6DSOX** | $3\ \mu\text{A}$ | $0.55\text{ mA}$ | $1\text{ mA}$ | Sensor de movimiento continuo |
| **MAX17048** | $3\ \mu\text{A}$ | $0.023\text{ mA}$ | $0.05\text{ mA}$ | Medición I2C |
| **Total Sistema** | **$< 50\ \mu\text{A}$** | **$\sim 88\text{ mA}$** | **$\sim 221\text{ mA}$** | **Completamente dentro del margen de 500mA del LDO** |

*Nota sobre la escalabilidad a 4G (A7670SA):* Los módulos de telefonía celular generan picos de hasta $2.0\text{ A}$. Al implementarlo en la tesis, se alimentará directamente de los bornes de la batería LiPo utilizando un capacitor de desacople dedicado de $470\ \mu\text{F}$.

### 3.4 Protecciones e Integridad de Señal
1. **Desacople en Alta Frecuencia:** Capacitores cerámicos $100\text{ nF}$ (0805) en los pines de alimentación de cada módulo individual.
2. **Capacitor Tanque (Bulk):** Capacitor de $10\ \mu\text{F} - 100\ \mu\text{F}$ en el bus de $3.3\text{V}$ para absorber transitorios durante la transmisión LoRa.
3. **Resistencias de Pull-Up I2C:** Dos resistencias de $4.7\text{ k}\Omega$ en las líneas `SDA` y `SCL`.
4. **Red RC de Reset:** $10\text{ k}\Omega$ a $3.3\text{V}$ y $1\ \mu\text{F}$ a $GND$ en el pin `EN` del ESP32-S3 para prevenir arranques erráticos.
5. **Aislamiento Galvánico para Actuador:** Optoacoplador VO617A accionado con una resistencia de entrada de $330\ \Omega$ (limitando la corriente del GPIO a $\sim 6.6\text{ mA}$) y una resistencia *pull-down* de $10\text{ k}\Omega$ a la salida.

---

## 4. Especificaciones del Hardware (Altium Designer) y BOM

### 4.1 Estrategia de Diseño de la PCB (*Carrier Board*)
Para la entrega de final de semestre no se soldarán CI microscópicos directos, sino que se diseñará una PCB receptora (*Motherboard*) con **tiras de conectores hembra (Female Headers paso 2.54mm)**. Esto garantiza:
* Cero fallas por falsos contactos mecánicos de protoboard.
* Posibilidad de intercambiar módulos en caso de falla.
* Cumplimiento con estándares de diseño profesional en Altium (planos de masa, ancho de pistas calculados, serigrafía clara).

### 4.2 Reglas de Diseño en Altium Designer
* **Capa Top / Bottom:** PCB de 2 capas (FR4, $1.6\text{mm}$ espesor).
* **Plano de Masa:** Vertido de cobre para `GND` en ambas capas (*GND Copper Pour*).
* **Ancho de Pistas de Alimentación (`VIN`, `3V3`, `BAT`):** $0.6\text{ mm}$ (24 mils) para soportar picos de corriente sin caída de tensión.
* **Ancho de Pistas de Señal (SPI, I2C, UART):** $0.25\text{ mm}$ (10 mils).

### 4.3 Asignación de Pines e Interfaces (Pinout ESP32-S3)

```text
[ESP32-S3 DevKitC-1 (N16R8)]
 ├─ Bus SPI (LoRa SX1262)
 │   ├─ GPIO 11 ──► MOSI
 │   ├─ GPIO 12 ──► MISO
 │   ├─ GPIO 13 ──► SCK
 │   ├─ GPIO 10 ──► CS (NSS)
 │   ├─ GPIO 14 ──► DIO1
 │   ├─ GPIO 9  ──► BUSY
 │   └─ GPIO 21 ──► RST
 ├─ Bus I2C (IMU, Fuel Gauge, GPS)
 │   ├─ GPIO 1  ──► SDA (con Pull-up 4.7kΩ)
 │   └─ GPIO 2  ──► SCL (con Pull-up 4.7kΩ)
 ├─ UART (GPS Backup)
 │   ├─ GPIO 43 ──► TXD (hacia RXD GPS)
 │   └─ GPIO 44 ──► RXD (desde TXD GPS)
 └─ GPIOs Especiales
     ├─ GPIO 4  ──► Interruptor INT1 IMU (Wake-up)
     └─ GPIO 5  ──► Ánodo Optoacoplador VO617A (vía R 330Ω)
```

### 4.4 Bill of Materials (BOM) — Componentes Clave

| Cantidad | Componente / Referencia | Descripción / Encapsulado | Función |
| :--- | :--- | :--- | :--- |
| 1 | ESP32-S3-DevKitC-1-N16R8 | Placa DevKit Dual-Core 240MHz | Procesador Principal |
| 1 | Waveshare Core1262-HF | Módulo LoRa SX1262 (915 MHz, IPEX) | Transceptor RF |
| 1 | u-blox MAX-M10S | Módulo Breakout GNSS con Antena | Geolocalización |
| 1 | ST LSM6DSOX | Breakout I2C/SPI IMU 6-DOF | Detección de Movimiento |
| 1 | MAX17048 | Breakout I2C Fuel Gauge | Telemetría de Batería |
| 1 | Módulo CN3791 (Versión 6V) | Cargador Solar MPPT Buck | Gestión Energética |
| 1 | Batería LiPo 3.7V 1200mAh | Celda 1S con PCM y JST-PH 2.0mm | Fuente de Energía |
| 1 | Panel Solar 6V 1.5W - 3W | Monocristalino / Policristalino | Generación Fotovoltaica |
| 1 | Optoacoplador VO617A / PC817 | DIP-4 Through-Hole | Aislamiento del Actuador |
| 1 | LED 3mm Rojo | Through-Hole | Indicador visual de disparo |
| 5 | Capacitores $100\text{ nF}$ | Cerámico SMD 0805 | Desacople de Alta Frecuencia |
| 1 | Capacitor $100\ \mu\text{F} / 16\text{V}$ | Electrolítico / Tántalo | Capacitor Tanque $3.3\text{V}$ |
| 2 | Resistencias $4.7\text{ k}\Omega$ | SMD 0805 | Pull-Ups I2C |
| 1 | Resistencia $330\ \Omega$ | Through-Hole / SMD 0805 | Limitadora Optoacoplador |
| 1 | Resistencia $10\text{ k}\Omega$ | Through-Hole / SMD 0805 | Pull-Down Salida Optoacoplador |
| N | Headers Hembra / Macho 2.54mm | Tiras de Pinos | Zócalos para Carrier Board |

---

## 5. Arquitectura Planeada del Firmware (STR / FreeRTOS)

El firmware se diseñará bajo un enfoque de **Sistema en Tiempo Real (STR)** utilizando las capacidades Dual-Core del ESP32-S3 bajo FreeRTOS:

```text
               ┌──────────────────────────────────────────┐
               │         ESP32-S3 (FreeRTOS)              │
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

### 5.1 Distribución de Tareas por Núcleo

#### **Core 0 (PRO_CPU) — Mantenimiento, Energía y Comunicaciones:**
1. **`Task_LoRaTX` (Prioridad Alta - 3):** Despierta al recibir un mensaje en la cola `Queue_Telemetry`, formatea el paquete y lo transmite vía SPI al SX1262.
2. **`Task_PowerMgmt` (Prioridad Baja - 1):** Lee la batería desde el MAX17048 cada 60 segundos y gestiona la transición a *Deep Sleep* si la IMU no detecta movimiento durante más de 5 minutos.

#### **Core 1 (APP_CPU) — Cálculo Determinista y Adquisición:**
1. **`Task_Geofence` (Prioridad Máxima - 4):** Ejecuta el algoritmo *Ray-Casting / Point-in-Polygon* al recibir nuevas coordenadas. Si el animal está fuera de la zona, activa inmediatamente el GPIO 5 (Optoacoplador) sin esperar ciclo de transmisión.
2. **`Task_GPSRead` (Prioridad Media - 2):** Decodifica tramas NMEA/UBX del u-blox MAX-M10S y actualiza la estructura de posición global.
3. **`Task_IMU_Check` (Prioridad Baja - 1):** Configura los umbrales de interrupción por movimiento en la IMU LSM6DSOX.

### 5.2 Algoritmo de Geocerca (*Ray-Casting*)
El polígono de la cerca virtual se almacena en la memoria Flash como un arreglo de vértices $(X_i, Y_i)$ representando Latitud y Longitud. El algoritmo evalúa si un punto $(P_x, P_y)$ está dentro lanzando un rayo horizontal e incrementando un contador cada vez que cruza un segmento del polígono:

* **Contador Impar:** El punto está **DENTRO** de la cerca (Estado Seguro).
* **Contador Par:** El punto está **FUERA** de la cerca (Activar Estímulo + Alerta LoRa).

---

## 4. Avances Completados y Próximos Pasos

### 4.1 Hitos Alcanzados (Aprobados)
- [x] Selección y validación del hardware base (ESP32-S3, Core1262, MAX-M10S, LSM6DSOX, CN3791)
- [x] Modelado teórico del sistema de alimentación fotovoltaica y cálculo de componentes Buck para el CN3791 (L=14 μH, R_CS=0.12 Ω, Divisor 6V)
- [x] Definición del esquema de distribución de voltajes y protecciones (Capacitores de desacople, pull-ups I2C, optoaislamiento)
- [x] Formulación de la arquitectura de la PCB tipo *Carrier Board* para la entrega final de semestre

### 4.2 Próximos Pasos Inmediatos
1. **Paso 1:** Configurar la estructura de archivos en el repositorio de GitHub
2. **Paso 2 (Fase 1 Firmware):** Crear el código base en PlatformIO/Arduino IDE que inicialice el bus SPI con el LoRa SX1262 y capture tramas GPS
3. **Paso 3 (Esquemático Altium):** Dibujar el esquemático de la *Carrier Board* conectando los zócalos de los módulos con las redes de alimentación y señales definidas

---

## 5. Estructura del Repositorio

```text
DR-VF-01/
├── docs/
│   ├── PROJECT_CONTEXT.md          <-- Este documento ejecutivo
│   ├── REQUIREMENTS.md             <-- Matriz de requerimientos
│   ├── ARCHITECTURE.md             <-- Arquitectura técnica detallada
│   ├── schematics/                 <-- Diagramas en PDF y bloques
│   └── math_models/                <-- Sustento matemático
├── firmware/
│   ├── include/                    <-- Librerías y definiciones de pines
│   ├── src/
│   │   ├── main.cpp                <-- Inicialización FreeRTOS y Tareas
│   │   ├── geofence.cpp            <-- Algoritmo Ray-Casting
│   │   ├── lora_management.cpp     <-- Drivers SX1262
│   │   └── power_management.cpp    <-- Gestión Sleep y MAX17048
│   └── platformio.ini
└── hardware/
    ├── altium_project/             <-- Proyecto Altium Designer (Carrier Board)
    ├── gerber/                     <-- Archivos para fabricación de PCB
    ├── BOM.md                      <-- Listado de materiales
    └── pinout.md                   <-- Asignación de pines
```

## 6. Índice de Documentos

| Documento | Propósito | Contenido Principal |
|-----------|-----------|---------------------|
| **[PROJECT_CONTEXT.md](./PROJECT_CONTEXT.md)** | Resumen ejecutivo | Visión, hoja de ruta, avances y estructura general |
| **[REQUIREMENTS.md](./REQUIREMENTS.md)** | Especificaciones | Matriz de RF/RNF con trazabilidad y estado |
| **[ARCHITECTURE.md](./ARCHITECTURE.md)** | Diseño técnico | Alimentación, cálculos, consumo, hardware, firmware |
| **[hardware/BOM.md](../hardware/BOM.md)** | Componentes | Lista completa de materiales para compra |
| **[hardware/pinout.md](../hardware/pinout.md)** | Interconexión | Asignación detallada de pines y interfaces |

---
*Documento actualizado como resumen ejecutivo del proyecto DR-VF-01*