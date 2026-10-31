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
* **Fase 1 (MVP de Escritorio - Semestre Actual):** Integración en protoboard del ESP32-S3, GPS MAX-M10S y módulo LoRa DX-LR30-900M22S (SX1262). Verificación del algoritmo *Point-in-Polygon* y transmisión punto a punto.
* **Fase 2 (Autonomía y Monitoreo de Estado):** Incorporación del gestor solar CN3791, el cargador USB-C TP4056, el indicador de batería MAX17048 y una IMU (POR DECIDIR: LSM6DSOX o LSM6DS3TR-C del docente) para despertar por movimiento (*Wake-on-Motion*).
* **Fase 3 (Integración de Hardware - Entregable Final de Semestre):** Diseño e impresión de PCB tipo *Carrier Board* en Altium Designer. Ensamble modular sin cables sueltos y prueba de concepto del actuador aislado (Optoacoplador + LED).
* **Fase 4 (Escalabilidad para Tesis):**
  * Integración de un módulo celular para zonas con cobertura híbrida (**POR DECIDIR:** A7670SA o Quectel BG95-M3).
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
* **RNF-02:** Procesamiento determinista en Zephyr RTOS
* **RNF-03:** Consumo < 150 μA en Deep Sleep
* **RNF-04:** Fiabilidad mecánica sin cables de puente

---

## 3. Resumen de Arquitectura Técnica

Para detalles técnicos completos, cálculos y justificativos, ver: [ARCHITECTURE.md](./ARCHITECTURE.md)

### 3.1 Arquitectura de Alimentación
Sistema en dos niveles: línea no regulada (3.6-4.2V, la batería) y línea regulada (3.3V) para los módulos. En la PCB final la batería la cargan dos cargadores (CN3791 con el panel solar y TP4056 con el USB-C) y alimenta un LDO TLV75733P de 1 A; el USB-C no alimenta al sistema (ver [ARCHITECTURE.md](./ARCHITECTURE.md), sección 1.1). En el prototipo con DevKit se usa el LDO interno de la placa.

### 3.2 Componentes Críticos
* **Procesamiento:** ESP32-S3 Dual-Core 240MHz (PCB final)
* **Objetivos de hardware:** un solo código base Zephyr para dos placas. Prueba de concepto en la Nordic Thingy:91 X (nRF9151, nRF Connect SDK, LTE-M/NB-IoT/GNSS); producto final en la PCB con ESP32-S3-WROOM-1U-N16R8 (módulo LoRa DX-LR30-900M22S con SX1262, GNSS externo MAX-M10S, IMU POR DECIDIR: LSM6DSOX o LSM6DS3TR-C). Se comparten `geofence.c`, la FSM y el logging; cada placa aporta su overlay, su `prj.conf` y los adaptadores de GNSS, comunicaciones e IMU.
* **Comunicación:** módulo LoRa DX-LR30-900M22S (SX1262 integrado, 915 MHz, SPI)
* **Geolocalización:** u-blox MAX-M10S GNSS
* **Energía:** CN3791 MPPT (solo panel solar), TP4056 (carga por USB-C, 500 mA), TLV75733P (LDO de 1 A, solo desde la batería), MAX17048 Fuel Gauge (I2C 0x36, sobre la batería). Protección de batería: **POR DECIDIR** (el código BQ2975DRBR del BOM no existe)
* **Protección USB y antenas:** USBLC6-2SC6 en D+/D-/VBUS. Antenas de LoRa (u.FL, 8 cm), FPC de 2.4 GHz (Würth 2015301200) y parche GNSS pasivo (MPN y posiciones **POR DECIDIR**); ver [hardware/BOM.md](../hardware/BOM.md)
* **Detección:** IMU 6-DOF (**POR DECIDIR:** LSM6DSOX o LSM6DS3TR-C del docente)

### 3.3 Consumo Eléctrico
* **Reposo:** unos 124 μA en Deep Sleep (calculado en `hardware/POWER_BUDGET.md`; requisito RNF-03: < 150 μA)
* **Activo:** ~88 mA
* **Pico Máximo:** ~224 mA (dentro de los 500 mA del LDO de la DevKit; la PCB final usa el TLV75733P de 1 A)
* **Autonomía:** requisito de 7 días en oscuridad (RNF-01); estimación teórica de unos 190 días con 1200 mAh (`hardware/POWER_BUDGET.md`)

### 3.4 Firmware STR/Zephyr
Arquitectura dual-core (ESP32-S3) con 5 tareas distribuidas:
* **Core 0:** Gestión de red (LoRa) y energía
* **Core 1:** Procesamiento determinista (geocerca, GPS, IMU)

---

## 3. Decisiones de Diseño Electrónico y Justificación Técnica

### 3.1 Arquitectura de Alimentación en Dos Niveles

En la PCB final la batería es el nodo central: el CN3791 la carga desde el panel solar, el TP4056 desde el USB-C, y un LDO TLV75733P de 1 A genera los 3.3 V (diagrama y justificación en [ARCHITECTURE.md](./ARCHITECTURE.md), sección 1.1). El prototipo con DevKit aprovecha el regulador interno LDO de la placa para evitar la adición de etapas de regulación externas redundantes:

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
                                             ├─► Módulo LoRa DX-LR30-900M22S
                                             └─► IMU (POR DECIDIR)
```

* **Línea No Regulada ($3.6\text{V} - 4.2\text{V}$):** Conecta la salida de la batería directamente al pin `VIN` del ESP32-S3 y al medidor MAX17048.
* **Línea Regulada ($3.3\text{V}$ Estrictos):** Suministrada por el pin `3V3` del ESP32-S3. Entrega hasta $500\text{ mA}$, cubriendo sobradamente la demanda máxima del sistema ($\sim 224\text{ mA}$).

### 3.2 Sustento Teórico del Cargador Solar CN3791
El CN3791 es un convertidor reductor conmutado (*Buck*) especializado con frecuencia de conmutación de $f_{sw} = 300\text{ kHz}$.

#### **A. Inductancia del Convertidor Buck ($L$)**
Para un rizado de corriente del $30\%$ ($\Delta I_L = 0.3\text{ A}$), voltaje de entrada $V_{IN} = 6.0\text{ V}$, voltaje de batería $V_{BAT} = 4.2\text{ V}$:

$$L = \frac{V_{BAT} \cdot (V_{IN} - V_{BAT})}{V_{IN} \cdot f_{sw} \cdot \Delta I_L} = \frac{4.2 \cdot (6.0 - 4.2)}{6.0 \cdot 300,000 \cdot 0.3} = \frac{7.56}{540,000} = 14\ \mu\text{H}$$

* **Selección Comercial:** Inductor de potencia SMD de **$10\ \mu\text{H}$ a $22\ \mu\text{H}$** (Estándar en el módulo CN3791).

#### **B. Divisor Tensión MPPT (Fijación a 6V)**
El pin `MPPT` se regula a $1.205\text{ V}$ (datasheet del CN3791, ver [ARCHITECTURE.md](./ARCHITECTURE.md)):

$$V_{MPPT} = 1.205\text{ V} \cdot \left(1 + \frac{R_3}{R_4}\right) \implies 6.0 = 1.205 \cdot \left(1 + \frac{R_3}{R_4}\right) \implies \frac{R_3}{R_4} = 3.98$$

* **Selección Comercial:** $R_3 = 402\text{ k}\Omega$, $R_4 = 100\text{ k}\Omega$ ($1\%$ precisión), que da $V_{MPPT} = 6.05\text{ V}$. La carga solo inicia con más de $6.17\text{ V}$ en la entrada, así que un USB de $5\text{ V}$ no puede usar este cargador: la carga por USB-C va por el TP4056.

#### **C. Sensado de Corriente ($R_{CS}$)**
Para limitar la carga rápida a $1\text{ A}$ con referencia de $120\text{ mV}$:

$$R_{CS} = \frac{0.12\text{ V}}{1.0\text{ A}} = 0.12\ \Omega\ (120\text{ m}\Omega)$$

### 3.3 Consumo de Corriente y Análisis de Picos

| Módulo | Estado Reposo | Estado Activo | Pico Máximo | Observaciones |
| :--- | :--- | :--- | :--- | :--- |
| **ESP32-S3** | $8\ \mu\text{A}$ (*Deep Sleep*) | $70\text{ mA}$ (240MHz) | $240\text{ mA}$ (WiFi/BT) | Wi-Fi activo solo en modo de configuración (SoftAP); deshabilitado en operación normal. Reposo: datasheet del módulo WROOM-1, Tabla 6-7 |
| **u-blox MAX-M10S** | $46\ \mu\text{A}$ (standby por software) | $12\text{ mA}$ | $35\text{ mA}$ | Adquisición multiconstelación. Reposo con V_IO a $3.3\text{ V}$ (datasheet del MAX-M10S) |
| **DX-LR30-900M22S (SX1262)** | $0.6\ \mu\text{A}$ | $5.3\text{ mA}$ (Rx) | $118\text{ mA}$ (Tx @ +22dBm) | Ráfagas de raras fracciones de segundo. Valores del chip SX1262: el módulo no tiene tabla de corrientes |
| **IMU (POR DECIDIR; cifras del LSM6DSOX)** | $9.5\ \mu\text{A}$ | $0.55\text{ mA}$ | $1\text{ mA}$ | Sensor de movimiento continuo |
| **MAX17048** | $3\ \mu\text{A}$ (hibernación) | $0.023\text{ mA}$ | $0.05\text{ mA}$ | Medición I2C; conectado a la batería |
| **CN3791** | $30\ \mu\text{A}$ (sin entrada, desde la batería) | — | — | Solo carga desde el panel solar |
| **TLV75733P (LDO)** | $25\ \mu\text{A}$ (Iq) | — | — | Solo alimentado desde la batería |
| **TP4056** | $2.5\ \mu\text{A}$ (standby) | — | — | Solo carga por USB-C |
| **Total Sistema** | **$\sim 124\ \mu\text{A}$ ($123.76\ \mu\text{A}$)** | **$\sim 88\text{ mA}$** | **$\sim 224\text{ mA}$** | **Reposo calculado en `hardware/POWER_BUDGET.md`: cumple RNF-03 ($< 150\ \mu\text{A}$) con $26\ \mu\text{A}$ de margen. Faltan por sumar el USBLC6-2SC6, la protección de batería (POR DECIDIR) y los extras del módulo LoRa. Los picos caben en los 500 mA del LDO de la DevKit (prototipo) y en el 1 A del TLV75733P (PCB final)** |

*Nota sobre la escalabilidad celular (módem de Fase 2, **POR DECIDIR:** A7670SA o BG95-M3):* Los módulos de telefonía celular generan picos de hasta $2.0\text{ A}$ (cifra sin verificar; depende del módem que se elija). Al implementarlo en la tesis, se alimentará directamente de los bornes de la batería LiPo utilizando un capacitor de desacople dedicado de $470\ \mu\text{F}$.

### 3.4 Protecciones e Integridad de Señal
1. **Desacople en Alta Frecuencia:** Capacitores cerámicos $100\text{ nF}$ (0805) en los pines de alimentación de cada módulo individual.
2. **Capacitor Tanque (Bulk):** Capacitor de $10\ \mu\text{F} - 100\ \mu\text{F}$ en el bus de $3.3\text{V}$ para absorber transitorios durante la transmisión LoRa.
3. **Resistencias de Pull-Up I2C:** Dos resistencias de $4.7\text{ k}\Omega$ en las líneas `SDA` y `SCL`.
4. **Red RC de Reset:** $10\text{ k}\Omega$ a $3.3\text{V}$ y $1\ \mu\text{F}$ a $GND$ en el pin `EN` del ESP32-S3 para prevenir arranques erráticos.
5. **Aislamiento Galvánico para Actuador:** Optoacoplador VO617A accionado con una resistencia serie de entrada ($330\ \Omega$ provisional, limitando la corriente del GPIO a $\sim 6.6\text{ mA}$; el valor final se calcula con el datasheet, ver [hardware/pinout.md](../hardware/pinout.md)) y una resistencia *pull-down* de $100\text{ k}\Omega$ en `GPIO1` (lado de entrada, según `hardware/pinout.md` y `hardware/BOM.md`).

---

## 4. Especificaciones del Hardware (Altium Designer) y BOM

### 4.1 Estrategia de Diseño de la PCB (*Carrier Board*)
Para la entrega de final de semestre no se soldarán CI microscópicos directos, sino que se diseñará una PCB receptora (*Motherboard*) con **tiras de conectores hembra (Female Headers paso 2.54mm)**. Esto garantiza:
* Cero fallas por falsos contactos mecánicos de protoboard.
* Posibilidad de intercambiar módulos en caso de falla.
* Cumplimiento con estándares de diseño profesional en Altium (planos de masa, ancho de pistas calculados, serigrafía clara).

**Alcance:** los conectores hembra aplican solo al prototipo del semestre. La PCB final lleva soldado el módulo ESP32-S3-WROOM-1U-N16R8 (`hardware/BOM.md`, sección 1.1) y cumple RNF-04 sin cables de puente.

### 4.2 Reglas de Diseño en Altium Designer
* **Capa Top / Bottom:** PCB de 2 capas (FR4, $1.6\text{mm}$ espesor).
* **Plano de Masa:** Vertido de cobre para `GND` en ambas capas (*GND Copper Pour*).
* **Ancho de Pistas de Alimentación (`VIN`, `3V3`, `BAT`):** $0.6\text{ mm}$ (24 mils) para soportar picos de corriente sin caída de tensión.
* **Ancho de Pistas de Señal (SPI, I2C, UART):** $0.25\text{ mm}$ (10 mils).

### 4.3 Asignación de Pines e Interfaces (Pinout ESP32-S3)

El único mapa de pines vigente es [hardware/pinout.md](../hardware/pinout.md). La PCB final y la ESP32-S3-DevKitC-1 (N16R8) usan los mismos GPIO. Resumen de los pines esenciales:

| Función | Periférico | GPIO |
| :--- | :--- | :--- |
| Bus SPI | Módulo LoRa DX-LR30-900M22S (SX1262) | NSS 10, MOSI 11, SCK 12, MISO 13 (BUSY 14, DIO1 6, NRESET 5, RXEN 42, TXEN 47) |
| Bus I2C | IMU (POR DECIDIR), MAX17048 (0x36) | SDA 8, SCL 9 (pull-up 4.7 kΩ) |
| UART1 | GNSS MAX-M10S | TX 17, RX 18 |
| IMU | INT1 (wake-up; LSM6DSOX o LSM6DS3TR-C, POR DECIDIR) | 4 |
| Actuador | Optoacoplador VO617A (`OPTO_CTRL`) | 1 |
| Módem Fase 2 (**POR DECIDIR:** A7670SA o BG95-M3) | UART TX / RX, PWRKEY | 43 / 44, 48 |

### 4.4 BOM de prototipo — Componentes Clave

Lista del prototipo con DevKit. La lista canónica de la PCB final (TLV75733PDBVR, TP4056, USBLC6-2SC6, antenas, conectores) es [hardware/BOM.md](../hardware/BOM.md).

| Cantidad | Componente / Referencia | Descripción / Encapsulado | Función |
| :--- | :--- | :--- | :--- |
| 1 | ESP32-S3-DevKitC-1-N16R8 | Placa DevKit Dual-Core 240MHz | Procesador Principal |
| 1 | DX-LR30-900M22S | Módulo LoRa SX1262 (915 MHz, SPI, 1.8 a 3.7 V) | Transceptor RF |
| 1 | u-blox MAX-M10S | Módulo Breakout GNSS con Antena | Geolocalización |
| 1 | IMU 6-DOF (POR DECIDIR: ST LSM6DSOX o LSM6DS3TR-C) | Breakout I2C/SPI | Detección de Movimiento |
| 1 | MAX17048 | Breakout I2C Fuel Gauge | Telemetría de Batería |
| 1 | Módulo CN3791 (Versión 6V) | Cargador Solar MPPT Buck | Gestión Energética |
| 1 | Batería LiPo 3.7V (capacidad POR DECIDIR: 1200 o 1500 mAh) | Celda 1S con JST-PH 2.0mm | Fuente de Energía |
| 1 | Panel Solar 6V, al menos 1 W (MPN POR DECIDIR) | Monocristalino / Policristalino | Generación Fotovoltaica |
| 1 | Optoacoplador VO617A / PC817 | DIP-4 Through-Hole | Aislamiento del Actuador |
| 1 | LED 3mm Rojo | Through-Hole | Indicador visual de disparo |
| 5 | Capacitores $100\text{ nF}$ | Cerámico SMD 0805 | Desacople de Alta Frecuencia |
| 1 | Capacitor $100\ \mu\text{F} / 16\text{V}$ | Electrolítico / Tántalo | Capacitor Tanque $3.3\text{V}$ |
| 2 | Resistencias $4.7\text{ k}\Omega$ | SMD 0805 | Pull-Ups I2C |
| 1 | Resistencia $330\ \Omega$ | Through-Hole / SMD 0805 | Limitadora Optoacoplador |
| 1 | Resistencia $100\text{ k}\Omega$ | Through-Hole / SMD 0805 | Pull-Down de GPIO1 (entrada del optoacoplador) |
| N | Headers Hembra / Macho 2.54mm | Tiras de Pinos | Zócalos para Carrier Board |

---

## 5. Arquitectura Planeada del Firmware (STR / Zephyr)

El firmware se diseñará bajo un enfoque de **Sistema en Tiempo Real (STR)** sobre **Zephyr RTOS**, con un único código base para la prueba de concepto (Nordic Thingy:91 X) y el producto final (ESP32-S3). El reparto siguiente aprovecha las capacidades Dual-Core del ESP32-S3:

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

### 5.1 Distribución de Tareas por Núcleo

#### **Core 0 (PRO_CPU) — Mantenimiento, Energía y Comunicaciones:**
1. **`Task_LoRaTX` (Prioridad Alta - 3):** Despierta al recibir un mensaje en la cola `Queue_Telemetry`, formatea el paquete y lo transmite vía SPI al SX1262.
2. **`Task_PowerMgmt` (Prioridad Baja - 1):** Lee la batería desde el MAX17048 cada 60 segundos y gestiona la transición a *Deep Sleep* si la IMU no detecta movimiento durante más de 5 minutos.

#### **Core 1 (APP_CPU) — Cálculo Determinista y Adquisición:**
1. **`Task_Geofence` (Prioridad Máxima - 4):** Ejecuta el algoritmo *Ray-Casting / Point-in-Polygon* al recibir nuevas coordenadas. Si el animal está fuera de la zona, activa inmediatamente `OPTO_CTRL` (GPIO 1, Optoacoplador) sin esperar ciclo de transmisión.
2. **`Task_GPSRead` (Prioridad Media - 2):** Decodifica tramas NMEA/UBX del u-blox MAX-M10S y actualiza la estructura de posición global.
3. **`Task_IMU_Check` (Prioridad Baja - 1):** Configura los umbrales de interrupción por movimiento en la IMU (POR DECIDIR: LSM6DSOX o LSM6DS3TR-C).

**Nota:** Los nombres `Task_*` se conservan del diseño original; en Zephyr cada tarea es un hilo. La numeración de prioridades usada aquí (4 = más crítica) debe traducirse al esquema de Zephyr, donde un número menor indica mayor prioridad.

### 5.2 Algoritmo de Geocerca (*Ray-Casting*)
El polígono de la cerca virtual se almacena en la memoria Flash como un arreglo de vértices $(X_i, Y_i)$ representando Latitud y Longitud. El algoritmo evalúa si un punto $(P_x, P_y)$ está dentro lanzando un rayo horizontal e incrementando un contador cada vez que cruza un segmento del polígono:

* **Contador Impar:** El punto está **DENTRO** de la cerca (Estado Seguro).
* **Contador Par:** El punto está **FUERA** de la cerca (Activar Estímulo + Alerta LoRa).

---

## 4. Avances Completados y Próximos Pasos

### 4.1 Hitos Alcanzados (Aprobados)
- [x] Selección y validación del hardware base (ESP32-S3, módulo LoRa DX-LR30-900M22S, MAX-M10S, CN3791; la IMU queda POR DECIDIR)
- [x] Modelado teórico del sistema de alimentación fotovoltaica y cálculo de componentes Buck para el CN3791 (L=14 μH, R_CS=0.12 Ω, Divisor 6V)
- [x] Definición del esquema de distribución de voltajes y protecciones (Capacitores de desacople, pull-ups I2C, optoaislamiento)
- [x] Formulación de la arquitectura de la PCB tipo *Carrier Board* para la entrega final de semestre

### 4.2 Próximos Pasos Inmediatos
1. **Paso 1:** Configurar la estructura de archivos en el repositorio de GitHub
2. **Paso 2 (Fase 1 Firmware):** Crear el código base Zephyr (un solo código para Thingy:91 X y ESP32-S3) que inicialice el bus SPI con el LoRa SX1262 y capture tramas GPS
3. **Paso 3 (Esquemático Altium):** Dibujar el esquemático de la *Carrier Board* conectando los zócalos de los módulos con las redes de alimentación y señales definidas

---

## 5. Estructura del Repositorio

```text
DR-VF-01/
├── docs/
│   ├── PROJECT_CONTEXT.md          <-- Este documento ejecutivo
│   ├── REQUIREMENTS.md             <-- Matriz de requerimientos
│   ├── ARCHITECTURE.md             <-- Arquitectura técnica detallada
│   ├── firmware_docs/              <-- Secuencia de arranque, FSM, MVP y riesgos de tiempo real
│   ├── comercial_docs/             <-- Datasheet y manual de usuario
│   └── DR-VF-01_Memoria_Tecnica.pdf <-- Informe técnico compilado
├── firmware/
│   ├── include/geofence.h          <-- API del motor de geocerca
│   ├── src/geofence/               <-- Motor Ray-Casting (geofence.c), Makefile y pruebas en host
│   └── (planeado)                  <-- main.c, CMakeLists.txt, prj.conf y boards/ (aplicación Zephyr compartida; overlay y prj.conf por placa)
├── hardware/
│   ├── BOM.md                      <-- Listado de materiales
│   ├── pinout.md                   <-- Asignación de pines
│   ├── POWER_BUDGET.md, THERMAL.md <-- Presupuesto de potencia y análisis térmico
│   ├── datasheets/                 <-- Índice de hojas de datos
│   └── (planeado)                  <-- altium_project/ (Carrier Board) y gerber/ (fabricación de PCB)
├── mechanical/                     <-- Especificaciones mecánicas (sellado, tolerancias, antena)
└── reports/                        <-- Fuentes LaTeX del informe técnico
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