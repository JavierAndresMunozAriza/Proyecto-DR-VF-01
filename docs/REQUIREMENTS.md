# Matriz de Requerimientos — Proyecto DR-VF-01
**Sistema Modular de Cerca Virtual para Ganado**  
**Versión:** 1.1  
**Fecha:** 31/10/2026  
**Documento Principal:** [PROJECT_CONTEXT.md](./PROJECT_CONTEXT.md)

---

## 1. Requerimientos Funcionales (RF)

| ID | Descripción | Métrica / Criterio de Aceptación | Fase | Método de Verificación | Trazabilidad |
|----|-------------|-----------------------------------|------|------------------------|--------------|
| **RF-01** | Geolocalización mediante GNSS multiconstelación | Error de posición < 2.5 m (métrica por definir: CEP o RMS; el MAX-M10S da 1.5 m CEP, Tabla 2 de su datasheet) | Fase 1 | Comparación con referencia RTK | `Task_GPSRead` (Core 1), módulo u-blox MAX-M10S (UART1, GPIO17/18), antena GNSS parche pasivo (MPN **POR DECIDIR**) |
| **RF-02** | Evaluación de cerca virtual (algoritmo Point-in-Polygon) | Tiempo de procesamiento < 50 ms | Fase 1 | Medición con osciloscopio en GPIO de salida | `Task_Geofence` (Core 1, Prioridad 4), módulo ESP32-S3-WROOM-1U-N16R8, `firmware/src/geofence/geofence.c` |
| **RF-03** | Acción disuasiva simulada con optoacoplador | Activación de LED indicador al cruzar geocerca | Fase 3 | Verificación visual y medición de corriente | `OPTO_CTRL` (GPIO 1, ver [hardware/pinout.md](../hardware/pinout.md)) + Optoacoplador VO617A; buzzer PKMCS0909E4000-R1 (`BUZZER_PWM`, GPIO2) y LED RGB L-154A4SURK/G5T (GPIO38 a 40) |
| **RF-04** | Telemetría LoRa en banda 915 MHz | Envío de paquetes formateados (Lat, Lon, Battery %, Satellites, Fence Status) | Fase 1 | Captura de paquetes con receptor LoRa | `Task_LoRaTX` (Core 0), módulo LoRa DX-LR30-900M22S (SPI GPIO10 a 13; BUSY 14, DIO1 6, NRESET 5, RXEN 42, TXEN 47), antena LoRa de 8 cm y conector u.FL |
| **RF-05** | Monitoreo de porcentaje de batería | Lectura vía bus I2C con actualización constante | Fase 2 | Comparación con multímetro de precisión | `Task_PowerMgmt` (Core 0), MAX17048 Fuel Gauge (I2C GPIO8/9, dirección 0x36, sobre la batería). Estado de carga: CN3791 CHRG (GPIO41); la salida de estado del TP4056 no tiene GPIO asignado (**POR DECIDIR**) |
| **RF-06** | Despertar desde Deep Sleep por movimiento (*wake-on-motion*) | Por definir (fila añadida el 31/10/2026, sin métrica aprobada) | Fase 2 | Por definir | **Pendiente:** IMU `IMU_INT1` (GPIO4, despierta de Deep Sleep). IMU **POR DECIDIR** (LSM6DSOX o LSM6DS3TR-C del docente) |

## 2. Requerimientos No Funcionales (RNF) y Sistema en Tiempo Real

| ID | Descripción | Métrica / Criterio de Aceptación | Fase | Método de Verificación | Trazabilidad |
|----|-------------|-----------------------------------|------|------------------------|--------------|
| **RNF-01** | Autonomía energética con panel solar de 6 V (potencia y MPN **POR DECIDIR**; el BOM pide al menos 1 W) | 7 días de operación en oscuridad total con batería de 1200 mAh (capacidad **POR DECIDIR**: 1200 o 1500 mAh) | Fase 2 | Prueba de descarga controlada en laboratorio | CN3791 MPPT (solo panel solar), TP4056 (USB-C), batería (capacidad **POR DECIDIR**), LDO TLV75733PDBVR. Estimación teórica: unos 190 días con 1200 mAh y unos 238 días con 1500 mAh (`hardware/POWER_BUDGET.md`). Protección de batería: **POR DECIDIR** |
| **RNF-02** | Procesamiento determinista en Zephyr RTOS | Ejecución sin bloqueo de tareas críticas | Fase 1 | Análisis de latencias con el subsistema de trazado de Zephyr | Arquitectura Dual-Core ESP32-S3 |
| **RNF-03** | Consumo en estado Deep Sleep | < 150 μA total del circuito | Fase 2 | Medición con multímetro en serie con batería | Optimización de modos low-power. Piezas que fijan el piso: LDO TLV75733PDBVR (Iq 25 μA), MAX17048 en hibernación (3 μA), CN3791 en sleep (unos 30 μA desde la batería). Presupuesto: unos 124 μA (123.76 μA), margen de 26 μA y consumos aún pendientes (`hardware/POWER_BUDGET.md`) |
| **RNF-04** | Fiabilidad mecánica sin cables de puente | PCB final sin jumpers o falsos contactos | Fase 3 | Inspección visual y pruebas de vibración | PCB final con el módulo ESP32-S3-WROOM-1U-N16R8 soldado y conectores u.FL (`hardware/BOM.md`); la Carrier Board con zócalos hembra es solo el prototipo del semestre |
| **RNF-05** | Carga por USB-C | 500 mA de corriente de carga (RPROG de 2.4 kΩ); no menos de 3 h para 1500 mAh (2.4 h para 1200 mAh) antes de la cola a tensión constante | Por definir | Medición de corriente de carga | TP4056 (`hardware/BOM.md`). USB-C no alimenta al sistema. Estado de carga legible por el MCU: **pendiente** (sin GPIO asignado). MPN del conector USB-C: **POR DECIDIR** |
| **RNF-06** | Protección de la batería | Por definir (fila añadida el 31/10/2026, sin métrica aprobada) | Por definir | Por definir | **Pendiente:** pieza **POR DECIDIR** (el código BQ2975DRBR del BOM no existe) |
| **RNF-07** | Protección ESD en el puerto USB-C | Por definir (fila añadida el 31/10/2026, sin métrica aprobada) | Por definir | Por definir | USBLC6-2SC6 en D+/D-/VBUS (`hardware/BOM.md`). Fuga y niveles ESD **pendientes de verificar**: falta el datasheet de ST en el repositorio |
| **RNF-08** | Protección IP67 de la carcasa | IP67 (objetivo de diseño; el PRD lo ubica en Fase 2) | Por definir | Por definir | **Pendiente:** sello del USB-C (MPN y tipo **POR DECIDIR**), `mechanical/IP67_SPEC.md` |
| **RNF-09** | Rango de temperatura de operación | Por definir: el BOM usa -20 a +65 °C y el PRD (NF-05) -10 a 50 °C; el módulo N16R8 admite -40 a 65 °C de ambiente | Por definir | Por definir | **Pendiente:** análisis en `hardware/THERMAL.md` |

## 3. Estado de Implementación

- [ ] **RF-01**: Pendiente de implementación
- [ ] **RF-02**: Pendiente de implementación  
- [ ] **RF-03**: Pendiente de implementación
- [ ] **RF-04**: Pendiente de implementación
- [ ] **RF-05**: Pendiente de implementación
- [ ] **RNF-01**: Pendiente de verificación
- [ ] **RNF-02**: Pendiente de verificación
- [ ] **RNF-03**: Pendiente de verificación
- [ ] **RNF-04**: Pendiente de verificación
- [ ] **RF-06**: Pendiente de definición e implementación
- [ ] **RNF-05 a RNF-09**: Pendientes de definición y verificación

---
*Documento generado a partir de la matriz original en PROJECT_CONTEXT.md*