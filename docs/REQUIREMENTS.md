# Matriz de Requerimientos — Proyecto DR-VF-01
**Sistema Modular de Cerca Virtual para Ganado**  
**Versión:** 1.2  
**Fecha:** 20/11/2026  
**Documento Principal:** [PROJECT_CONTEXT.md](./PROJECT_CONTEXT.md)

---

## 1. Requerimientos Funcionales (RF)

| ID | Descripción | Métrica / Criterio de Aceptación | Fase | Método de Verificación | Trazabilidad |
|----|-------------|-----------------------------------|------|------------------------|--------------|
| **RF-01** | Geolocalización mediante GNSS multiconstelación | Error de posición < 2.5 m (métrica por definir: CEP o RMS; el MAX-M10S da 1.5 m CEP, Tabla 2 de su datasheet) | Fase 1 | Comparación con referencia RTK | `Task_GPSRead` (Core 1), módulo u-blox MAX-M10S (UART1, GPIO17/18), antena GNSS parche pasivo (MPN **POR DECIDIR**) |
| **RF-02** | Evaluación de cerca virtual (algoritmo Point-in-Polygon) | Tiempo de procesamiento < 50 ms | Fase 1 | Medición con osciloscopio en GPIO de salida | `Task_Geofence` (Core 1, Prioridad 4), módulo ESP32-S3-WROOM-1U-N16R2, `firmware/src/geofence/geofence.c` |
| **RF-03** | Acción disuasiva simulada con optoacoplador | Activación de LED indicador al cruzar geocerca | Fase 3 | Verificación visual y medición de corriente | `OPTO_CTRL` (GPIO 1, ver [hardware/pinout.md](../hardware/pinout.md)) + Optoacoplador VO617A; buzzer PKMCS0909E4000-R1 (`BUZZER_PWM`, GPIO2) y LED RGB RS-3535MWAM, y LED rojo 0805 en la salida del optoacoplador (GPIO38 a 40) |
| **RF-04** | Telemetría LoRa en banda 915 MHz | Envío de paquetes formateados (Lat, Lon, Battery %, Satellites, Fence Status) | Fase 1 | Captura de paquetes con receptor LoRa | `Task_LoRaTX` (Core 0), módulo LoRa DX-LR30-900M22S (SPI GPIO10 a 13; BUSY 14, DIO1 6, NRESET 5, RXEN 42, TXEN 47), antena LoRa de 8 cm y conector u.FL |
| **RF-05** | Monitoreo de porcentaje de batería | Lectura vía bus I2C con actualización constante | Fase 2 | Comparación con multímetro de precisión | `Task_PowerMgmt` (Core 0), MAX17048 Fuel Gauge (I2C GPIO8/9, dirección 0x36, sobre la batería). Estado de carga: CN3791 CHRG (GPIO41); la salida de estado del TP4056 no tiene GPIO asignado (**POR DECIDIR**) |
| **RF-06** | Despertar desde Deep Sleep por movimiento (*wake-on-motion*) | Por definir (fila añadida el 31/10/2026, sin métrica aprobada) | Fase 2 | Por definir | **Pendiente:** IMU `IMU_INT1` (GPIO4, despierta de Deep Sleep). IMU LSM6DS3TR-C del docente |
| **RF-07** | Actualización remota de la geocerca por red celular (**propuesto el 2026-11-25, sin aprobar**) | Una cerca dibujada en el portal llega al collar sin que el usuario esté cerca, se valida, se guarda en flash y se aplica; latencia máxima igual al intervalo de consulta más 30 s (propuesta); una cerca inválida se rechaza y se informa | Fase 2 (placa con módem) | Pruebas L3 a L5 de `docs/poc-thingy91x/PRUEBAS_LTE.md` | `fence_codec`, `fence_manager`, `cloud_sync`, `tools/fence_portal/`, formato en [PROTOCOLO_CERCA.md](firmware_docs/PROTOCOLO_CERCA.md) |

## 2. Requerimientos No Funcionales (RNF) y Sistema en Tiempo Real

| ID | Descripción | Métrica / Criterio de Aceptación | Fase | Método de Verificación | Trazabilidad |
|----|-------------|-----------------------------------|------|------------------------|--------------|
| **RNF-01** | Autonomía energética con panel solar de 6 V, de 1 a 1.5 W (prueba: panel de 6 V y 220 mA; panel definitivo **POR DECIDIR**) | 7 días de operación en oscuridad total con batería de 1200 mAh (capacidad definitiva **POR DECIDIR**: 1200 o 1500 mAh) | Fase 2 | Prueba de descarga controlada en laboratorio | CN3791 MPPT (solo panel solar), TP4056 (USB-C), batería (prueba: 603450 de 1200 mAh), LDO TLV75733PDBVR. Estimación teórica: unos 190 días con 1200 mAh y unos 238 días con 1500 mAh (`hardware/POWER_BUDGET.md`). Protección de batería: integrada en la celda (PCM), decidido el 2026-11-05 |
| **RNF-02** | Procesamiento determinista en Zephyr RTOS | Ejecución sin bloqueo de tareas críticas | Fase 1 | Análisis de latencias con el subsistema de trazado de Zephyr | Arquitectura Dual-Core ESP32-S3 |
| **RNF-03** | Consumo en estado Deep Sleep | < 150 μA total del circuito | Fase 2 | Medición con multímetro en serie con batería | Optimización de modos low-power. Piezas que fijan el piso: LDO TLV75733PDBVR (Iq 25 μA), MAX17048 en hibernación (3 μA), CN3791 en sleep (unos 30 μA desde la batería). Presupuesto: unos 124 μA (123.76 μA), margen de 26 μA y consumos aún pendientes (`hardware/POWER_BUDGET.md`) |
| **RNF-04** | Fiabilidad mecánica sin cables de puente | PCB final sin jumpers o falsos contactos | Fase 3 | Inspección visual y pruebas de vibración | PCB final con el módulo ESP32-S3-WROOM-1U-N16R2 soldado y conectores u.FL (`hardware/BOM.md`); la Carrier Board con zócalos hembra es solo el prototipo del semestre |
| **RNF-05** | Carga por USB-C | 500 mA de corriente de carga (RPROG de 1.9 kΩ); no menos de 3 h para 1500 mAh (2.4 h para 1200 mAh) antes de la cola a tensión constante | Por definir | Medición de corriente de carga | TP4056 (`hardware/BOM.md`). USB-C no alimenta al sistema. Estado de carga legible por el MCU: **pendiente** (sin GPIO asignado). MPN del conector USB-C: **POR DECIDIR** |
| **RNF-06** | Protección de la batería | Por definir (fila añadida el 31/10/2026, sin métrica aprobada) | Por definir | Por definir | **Decidido el 2026-11-05:** la celda trae protección integrada (PCM: sobrecarga, sobredescarga y cortocircuito) y la PCB no lleva IC protector; el BQ2975DRBR se retiró porque no existe. La métrica sigue por definir |
| **RNF-07** | Protección ESD en el puerto USB-C | Por definir (fila añadida el 31/10/2026, sin métrica aprobada) | Por definir | Por definir | USBLC6-2SC6 en D+/D-/VBUS (`hardware/BOM.md`). Fuga y niveles ESD **pendientes de verificar**: falta el datasheet de ST en el repositorio |
| **RNF-08** | Protección IP67 de la carcasa | IP67 (objetivo de diseño; el PRD lo ubica en Fase 2) | Por definir | Por definir | **Pendiente:** sello del USB-C (MPN y tipo **POR DECIDIR**), `mechanical/IP67_SPEC.md` |
| **RNF-09** | Rango de temperatura de operación | 0 a +65 °C (decidido el 2026-11-04), límite ambiente del módulo dentro de la carcasa; NF-05 usa 0 a 50 °C para la temperatura del aire en campo. Los componentes del BOM se eligen para -20 a +65 °C como margen y el módulo N16R2 admite -40 a 85 °C de ambiente (decidido el 2026-11-26 en lugar del N16R8, que admite 65 °C) | Por definir | Por definir | **Pendiente:** análisis en `hardware/THERMAL.md`. Alcance decidido el 2026-11-03: uso solo en zonas sin heladas (por debajo de unos 2.300 m). La carga de la batería no tiene corte por frío, por eso el rango empieza en 0 °C |

## 3. Estado de Verificación

Estado de cada requerimiento según la prueba de concepto (PoC) en la Nordic Thingy:91 X (nRF9151), no en la PCB final. El detalle, los criterios `PoC-01` a `PoC-11`, la metodología y la evidencia están en el *Engineering Report* (`reports/engineering_report_poc/`, secciones de resultados y de cumplimiento). Medido el 2026-11-20.

| ID | Estado en la PoC | Plataforma | Resultado |
|----|------------------|------------|-----------|
| **RF-01** | No cumple en estas condiciones | nRF9151, cielo parcial | CEP50 de 3.5 a 7.4 m frente a 2.5 m. Es precisión (dispersión), no exactitud absoluta; el CEP no está definido (CEP50 o RMS). Falta cielo abierto y un punto de referencia |
| **RF-02** | Parcial | nRF9151 | 350 µs con 4 vértices y 61.4 ms con 1024; llega a 50 ms cerca de los 830 vértices. Falta medirlo en el ESP32-S3 con osciloscopio |
| **RF-03** | Parcial | nRF9151 | Solo la alerta visual con el LED RGB; no hay optoacoplador, buzzer ni medición de corriente |
| **RF-04** | No cubierto | — | La PoC no usa LoRa |
| **RF-05** | No cubierto | — | La PoC no usa el MAX17048 |
| **RF-06** | No cubierto | — | La PoC no usa la IMU ni el sueño profundo |
| **RF-07** | Parcial (propuesto) | Pruebas de host | El código del codec, del gestor, de la telemetría y del portal pasa sus pruebas en el PC y el firmware con la nube compila (`./build.sh cloud`). No se ha probado en la placa ni con una cuenta de nube |
| **RNF-01** a **RNF-04** | No verificado | — | Autonomía, determinismo en Zephyr (sin traza de latencias), consumo en sueño profundo y fiabilidad mecánica no se midieron |
| **RNF-05** a **RNF-09** | No verificado | — | Siguen pendientes de definición y verificación |

Además, el SRD pide GNSS a más de 1 Hz (SR-10): la nRF9151 entrega 1 Hz, así que no se cumple en la PoC; el requisito apunta al MAX-M10S.

---
*Documento generado a partir de la matriz original en PROJECT_CONTEXT.md*