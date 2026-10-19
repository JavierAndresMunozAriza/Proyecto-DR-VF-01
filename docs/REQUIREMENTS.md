# Matriz de Requerimientos — Proyecto DR-VF-01
**Sistema Modular de Cerca Virtual para Ganado**  
**Versión:** 1.0  
**Fecha:** 18/10/2026  
**Documento Principal:** [PROJECT_CONTEXT.md](./PROJECT_CONTEXT.md)

---

## 1. Requerimientos Funcionales (RF)

| ID | Descripción | Métrica / Criterio de Aceptación | Fase | Método de Verificación | Trazabilidad |
|----|-------------|-----------------------------------|------|------------------------|--------------|
| **RF-01** | Geolocalización mediante GNSS multiconstelación | Error de posición < 2.5 m | Fase 1 | Comparación con referencia RTK | `Task_GPSRead` (Core 1), Módulo u-blox MAX-M10S |
| **RF-02** | Evaluación de cerca virtual (algoritmo Point-in-Polygon) | Tiempo de procesamiento < 50 ms | Fase 1 | Medición con osciloscopio en GPIO de salida | `Task_Geofence` (Core 1, Prioridad 4) |
| **RF-03** | Acción disuasiva simulada con optoacoplador | Activación de LED indicador al cruzar geocerca | Fase 3 | Verificación visual y medición de corriente | GPIO 5 + Optoacoplador VO617A |
| **RF-04** | Telemetría LoRa en banda 915 MHz | Envío de paquetes formateados (Lat, Lon, Battery %, Satellites, Fence Status) | Fase 1 | Captura de paquetes con receptor LoRa | `Task_LoRaTX` (Core 0), Módulo Core1262 (SX1262) |
| **RF-05** | Monitoreo de porcentaje de batería | Lectura vía bus I2C con actualización constante | Fase 2 | Comparación con multímetro de precisión | `Task_PowerMgmt` (Core 0), MAX17048 Fuel Gauge |

## 2. Requerimientos No Funcionales (RNF) y Sistema en Tiempo Real

| ID | Descripción | Métrica / Criterio de Aceptación | Fase | Método de Verificación | Trazabilidad |
|----|-------------|-----------------------------------|------|------------------------|--------------|
| **RNF-01** | Autonomía energética con panel solar 6V/1.5W | 7 días de operación en oscuridad total con batería 1200 mAh | Fase 2 | Prueba de descarga controlada en laboratorio | CN3791 MPPT + Gestión de energía |
| **RNF-02** | Procesamiento determinista en FreeRTOS | Ejecución sin bloqueo de tareas críticas | Fase 1 | Análisis de latencias con FreeRTOS trace | Arquitectura Dual-Core ESP32-S3 |
| **RNF-03** | Consumo en estado Deep Sleep | < 150 μA total del circuito | Fase 2 | Medición con multímetro en serie con batería | Optimización de modos low-power |
| **RNF-04** | Fiabilidad mecánica sin cables de puente | PCB final sin jumpers o falsos contactos | Fase 3 | Inspección visual y pruebas de vibración | Diseño Carrier Board en Altium |

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

---
*Documento generado a partir de la matriz original en PROJECT_CONTEXT.md*