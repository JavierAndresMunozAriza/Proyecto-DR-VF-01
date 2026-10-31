# Secuencia de Arranque del Sistema

## Descripción General

Esta documentación describe la secuencia de arranque del sistema siguiendo los principios de inicialización robusta descritos en "Making Embedded Systems" por Elecia White. El enfoque está en crear un sistema confiable que pueda recuperarse de fallos y mantener operación estable en condiciones de campo.

*Nota: Este proceso de arranque implementa las mejores prácticas de inicialización de sistemas embebidos según Elecia White: verificación sistemática, manejo de errores proactivo y transición controlada a operación normal.*

## 1. Power-On Reset (POR)
- Verificación de voltaje de alimentación estable (>3.0V)
- Inicialización de registros del ESP32-S3
- Configuración de reloj principal (PLL a 240MHz)
- **Principio de Elecia White**: "Always check your assumptions" - Verificación de condiciones básicas antes de proceder

## 2. Inicialización de Periféricos
1. **GPIO Configuration**
   - Configuración de pines según `hardware/pinout.md` (mapa del ESP32-S3; en la Nordic Thingy:91 X lo define su overlay de devicetree)
   - Habilitación de pull-ups/pull-downs
   - Verificación de configuración mediante lectura de estado

2. **I2C Bus Initialization**
   - SDA: GPIO8, SCL: GPIO9 (ver `hardware/pinout.md`)
   - Velocidad: 400kHz estándar
   - Timeout: 100ms
   - **Verificación de bus**: Detección de dispositivos en el bus I2C (MAX17048 en la dirección 0x36 y la IMU)

3. **SPI Bus Initialization**
   - Para comunicación con el módulo LoRa DX-LR30-900M22S (SX1262 integrado)
   - SCK: GPIO12, MISO: GPIO13, MOSI: GPIO11, NSS: GPIO10 (ver `hardware/pinout.md`)
   - Líneas de control del módulo: BUSY GPIO14, DIO1 GPIO6, NRESET GPIO5, RXEN GPIO42, TXEN GPIO47. DIO2 y DIO3 (alimentación del TCXO, si el módulo lo usa): **POR CONFIRMAR** con el vendedor (`hardware/pinout.md`)
   - La antena debe estar conectada antes de energizar el módulo (exigencia de la ficha del vendedor)
   - **Prueba de conexión**: Verificación de comunicación SPI antes de uso

4. **USB nativo y reloj**
   - USB nativo del ESP32-S3: D- GPIO19, D+ GPIO20 (programación; BOOT, GPIO0, a GND = modo descarga, con la batería conectada)
   - PSRAM octal (8 MB) integrada en el encapsulado del chip del módulo N16R8 (no es una memoria externa); GPIO35 a 37 quedan ocupados por ella (`hardware/pinout.md`)
   - Cristal RTC de 32.768 kHz en GPIO15 y GPIO16 (ver `hardware/pinout.md`)

## 3. Inicialización de Sensores
1. **IMU** (POR DECIDIR: LSM6DSOX o LSM6DS3TR-C del docente)
   - Verificación de conexión (WHO_AM_I register; el valor esperado depende de la pieza elegida)
   - Configuración de interrupciones para wake-up (INT1 en GPIO4)
   - Modo low-power habilitado
   - **Prueba funcional**: Validación de lecturas de aceleración y giroscopio

2. **u-blox MAX-M10S GNSS**
   - Baudrate por defecto: 9600 bit/s (el datasheet del MAX-M10S admite de 9600 a 921600 bit/s y no documenta autodetección)
   - Configuración de mensajes UBX-RXM-PMREQ para power saving
   - **Verificación de posicionamiento**: Confirmación de recepción de satélites

3. **Módulo LoRa DX-LR30-900M22S (SX1262)**
   - Frecuencia: 915MHz (banda ISM americana)
   - Potencia: +22dBm (máximo del transceptor SX1262)
   - SF: 12 (para rango extendido)
   - **Prueba de comunicación**: Envío de mensaje de prueba y confirmación de recepción

4. **MAX17048 (medidor de batería)**
   - Comprobación rápida de presencia en I2C (dirección 0x36) y lectura inicial del nivel de carga
   - Está conectado a la batería, no al bus de 3.3 V (`docs/ARCHITECTURE.md`, sección 1.1)

5. **Estado de carga**
   - Solar: salida CHRG del CN3791 en GPIO41 (colector abierto, pull-up de 10 kΩ)
   - USB-C (TP4056): sin GPIO asignado para sus salidas de estado, **POR DECIDIR** (`hardware/pinout.md`)

## 4. Carga de Parámetros de Configuración
- Lectura de coordenadas geográficas desde Flash NVS
- Carga de parámetros de geocerca (polígono)
- Recuperación de última posición conocida
- **Validación de datos**: Verificación de integridad mediante checksums
- **Valores por defecto**: Carga de configuración segura si datos inválidos

## 5. Inicialización del Sistema Operativo
- Creación de tareas RTOS (hilos de Zephyr):
  - `geofence_task` (prioridad alta)
  - `lora_tx_task` (prioridad media)
  - `imu_monitor_task` (prioridad baja)
- Inicialización de colas y semáforos
- Habilitación de watchdog (timeout: 1 segundo)
- **Principio de Elecia White**: "Design for failure" - Watchdog como red de seguridad

## 6. Verificación Final del Sistema
- **Auto-diagnóstico**: Ejecución de pruebas de salud del sistema
- **Calibración**: Ajuste de parámetros según condiciones ambientales
- **Registro de estado**: Almacenamiento de estado inicial en log circular

## 7. Entrada en Modo Operativo
- Transición a estado SLEEP después de 5 segundos de inactividad
- Configuración de las fuentes de wake-up de deep sleep: IMU_INT1 (GPIO4), LORA_DIO1 (GPIO6) y BUTTON (GPIO7)
- Mantener `OPTO_CTRL` (GPIO1) en bajo durante deep sleep con retención de GPIO RTC (`rtc_gpio_hold_en`), además del pull-down externo
- Sistema listo para operación en campo
- **Confirmación de operación**: Indicador LED de sistema listo

## Estrategia de Recuperación de Errores Durante Arranque

### Manejo de Fallos Críticos
Siguiendo el principio de Elecia White de "graceful degradation":
1. **Fallo de sensor**: Operación en modo degradado con funciones limitadas
2. **Fallo de comunicación**: Cambio a métodos alternativos de reporte
3. **Fallo de almacenamiento**: Operación con valores por defecto seguros
4. **Fallo de energía**: Estado de retención crítica de datos importantes

### Registro de Diagnóstico de Arranque
- **Log de inicialización**: Registro detallado de cada paso del arranque
- **Código de estado**: Identificador único para cada condición de fallo
- **Timestamp**: Marca temporal precisa para análisis forense

## Conclusiones

Esta secuencia de arranque robusta asegura que el collar DR-VF-01:
1. Verifica sistemáticamente todos los componentes críticos
2. Maneja graciosamente cualquier fallo detectado
3. Transita de forma controlada a operación normal
4. Mantiene registros detallados para diagnóstico y mantenimiento

El enfoque sigue las recomendaciones de Elecia White para sistemas embebidos confiables, priorizando la verificación, el manejo de errores y la operación estable bajo condiciones de campo adversas.