# Definición del MVP de Firmware

# Definición del MVP de Firmware

## Descripción General

Esta documentación define el Producto Mínimo Viable (MVP) del firmware siguiendo los principios de desarrollo de sistemas embebidos descritos en "Making Embedded Systems" por Elecia White. El enfoque está en crear una implementación funcional que demuestre capacidades críticas mientras mantiene simplicidad y robustez.

*Nota: Este MVP implementa las mejores prácticas de desarrollo incremental según Elecia White: funcionalidad esencial primero, arquitectura escalable y validación sistemática.*
## Objetivo del MVP
Crear una implementación mínima funcional que demuestre las capacidades críticas del sistema de geocercamiento para el semestre, incluyendo detección de movimiento, verificación de geocerca y comunicación LoRa.

## Componentes del MVP

### 1. Sistema de Detección de Movimiento
**Hardware**: LSM6DSOX IMU
**Funcionalidad**:
- Detección de movimiento mediante interrupciones
- Configuración de threshold ajustable (0.1g - 1.0g)
- Wake-up del sistema desde Deep Sleep

**Implementación**:
```c
// firmware/src/drivers/imu_lsm6dsox.c
int imu_init_movement_detection(float threshold_g) {
    // Configurar acelerómetro para detección de movimiento
    lsm6dsox_write_reg(LSM6DSO_WAKE_UP_THS, (uint8_t)(threshold_g * 64));
    lsm6dsox_write_reg(LSM6DSO_WAKE_UP_DUR, 0x10);  // 16 muestras consecutivas
    
    // Habilitar interrupción en INT1
    lsm6dsox_write_reg(LSM6DSO_INT1_CTRL, 0x20);
    
    return 0;
}
```

### 2. Algoritmo de Geocerca (Point-in-Polygon)
**Requisitos**:
- Ejecución en < 50ms
- Soporte para polígonos de hasta 32 vértices
- Implementación en C puro portable

**API**:
```c
// firmware/include/geofence.h
typedef struct {
    double lat;
    double lon;
} geopoint_t;

typedef struct {
    geopoint_t* vertices;
    int num_vertices;
} geofence_t;

/**
 * @brief Determina si un punto está dentro de una geocerca
 * @param point Coordenadas a verificar
 * @param fence Polígono definido por vértices
 * @return true si está dentro, false si está fuera
 */
bool is_point_in_geofence(const geopoint_t* point, const geofence_t* fence);
```

### 3. Comunicación LoRa
**Configuración**:
- Frecuencia: 915MHz (ISM band)
- Spreading Factor: 12 (máximo rango)
- Bandwidth: 125kHz
- Potencia: +22dBm

**Funcionalidad**:
- Transmisión de coordenadas cuando se detecta movimiento fuera de geocerca
- Recepción de comandos de configuración
- Envío de mensajes de estado del sistema

**Implementación**:
```c
// firmware/src/tasks/lora_task.c
int lora_send_geofence_alert(geopoint_t* location) {
    uint8_t payload[20];
    // Codificar coordenadas en payload
    encode_gps_coordinates(payload, location);
    
    // Enviar mensaje LoRa
    return sx1262_transmit(payload, sizeof(payload));
}
```

### 4. Gestión de Energía
**Modos de Operación**:
- Deep Sleep: < 5μA
- Active Mode: Variable según periféricos activos
- Standby GNSS: 25μA

**Estrategia**:
- ESP32 en Deep Sleep la mayor parte del tiempo
- IMU en modo low-power monitor
- GNSS activo solo durante verificaciones
- LoRa en modo sleep entre transmisiones

### 5. Configuración Inicial
**Método**: Wi-Fi SoftAP + Servidor Web
**Funcionalidad**:
- Punto de acceso WiFi para configuración inicial
- Interfaz web para definir geocerca
- Almacenamiento en Flash NVS
- Actualización de parámetros sin necesidad de programador

**Componentes Web**:
- Página de configuración de geocerca (coordenadas)
- Ajuste de sensibilidad de movimiento
- Visualización de estado del sistema
- Registro de eventos recientes

## Arquitectura del MVP

### Capas de Software
1. **Aplicación**: Tareas de geocerca y comunicación
2. **Middleware**: Algoritmos de procesamiento y lógica de negocio
3. **HAL**: Abstracción de hardware para portabilidad
4. **RTOS**: Zephyr RTOS para gestión de tareas. Un solo código base para la prueba de concepto (Nordic Thingy:91 X) y el producto final (ESP32-S3); cada placa aporta su overlay, su `prj.conf` y sus adaptadores de GNSS, comunicaciones e IMU
5. **Drivers**: Controladores específicos de hardware

### Tareas RTOS Principales
```c
// firmware/src/main.c
K_THREAD_DEFINE(imu_monitor, STACK_SIZE, imu_monitor_thread, 
                NULL, NULL, NULL, IMU_PRIORITY, 0, 0);

K_THREAD_DEFINE(geofence_check, STACK_SIZE, geofence_thread,
                NULL, NULL, NULL, GEOFENCE_PRIORITY, 0, 0);

K_THREAD_DEFINE(lora_handler, STACK_SIZE, lora_thread,
                NULL, NULL, NULL, LORA_PRIORITY, 0, 0);
```

## Validación del MVP

### Pruebas Unitarias
1. **Algoritmo PIP**: Verificar precisión con polígonos conocidos
2. **Detección IMU**: Confirmar sensibilidad ajustable
3. **Comunicación LoRa**: Medir rango y confiabilidad
4. **Consumo Energético**: Validar con multímetro de bancada

### Métricas de Éxito
| Métrica | Objetivo | Método de Medición |
|---------|----------|-------------------|
| Tiempo de respuesta PIP | < 50ms | Osciloscopio |
| Consumo en Sleep | < 5μA | Multímetro HP 34401A |
| Rango LoRa | > 5km | Pruebas en campo abierto |
| Autonomía | > 60 días | Cálculo basado en perfil de uso |
| Precisión GNSS | < 3m | Comparación con GPS comercial |

### Entregables del MVP
1. Firmware Zephyr funcional para el ESP32-S3 (placa final), con el mismo código base verificable en la Nordic Thingy:91 X (prueba de concepto)
2. Documentación de configuración inicial
3. Guía de pruebas y validación
4. Código fuente con comentarios explicativos
5. Demo funcional para presentación de semestre

## Objetivo del MVP
Crear una implementación mínima funcional que demuestre las capacidades críticas del sistema de geocercamiento para el semestre, incluyendo detección de movimiento, verificación de geocerca y comunicación LoRa.

### Principios de Diseño del MVP según Elecia White
1. **Start simple** - Comenzar con funcionalidad esencial y bien implementada
2. **Build incrementally** - Construir capas de complejidad de forma controlada
3. **Test early and often** - Validar cada componente antes de añadir más complejidad
4. **Design for growth** - Crear arquitectura que pueda expandirse fácilmente
## Estrategia de Desarrollo Iterativo

### Fases de Implementación
1. **Fase 1 - Core Functionality**: Detección de movimiento + algoritmo PIP básico
2. **Fase 2 - Communication Layer**: Integración LoRa y mensajes básicos
3. **Fase 3 - Energy Management**: Optimización de consumo y estados de sueño
4. **Fase 4 - User Interface**: Configuración web y experiencia de usuario
5. **Principio de Elecia White**: "Build incrementally" - Progreso medible en cada fase

## Conclusiones

Esta definición del MVP proporciona una base sólida para demostrar las capacidades críticas del sistema mientras mantiene el alcance manejable para el semestre actual, siguiendo los principios de desarrollo robusto de Elecia White:

1. **Enfoque en lo esencial** - Solo funcionalidad crítica y bien implementada
2. **Arquitectura escalable** - Base para expansión futura sin reescritura
3. **Validación sistemática** - Pruebas completas en cada fase de desarrollo
4. **Documentación integral** - Registro claro de decisiones y diseño

El MVP asegura un progreso medible hacia los objetivos del proyecto mientras minimiza riesgos técnicos y mantiene la calidad del código.
## Roadmap hacia Versión Comercial
1. **Fase 1 (Semestre Actual)**: MVP funcional con LoRa
2. **Fase 2 (Próximo Semestre)**: Integración BLE 5.0 para app móvil
3. **Fase 3 (Optimización)**: Mejoras de eficiencia energética
4. **Fase 4 (Producción)**: Certificaciones y escalamiento

Esta definición del MVP proporciona una base sólida para demostrar las capacidades críticas del sistema mientras mantiene el alcance manejable para el semestre actual.