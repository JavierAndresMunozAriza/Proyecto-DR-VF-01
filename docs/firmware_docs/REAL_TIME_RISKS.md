# Análisis de Desafíos y Riesgos en Tiempo Real

## Descripción General

Esta documentación analiza los desafíos y riesgos de tiempo real del sistema siguiendo los principios de sistemas embebidos descritos en "Making Embedded Systems" por Elecia White. El enfoque está en identificar, mitigar y gestionar proactivamente los riesgos que podrían afectar el rendimiento en tiempo real del collar DR-VF-01.

*Nota: Este análisis implementa las mejores prácticas de gestión de riesgos en tiempo real según Elecia White: identificación proactiva, mitigación sistemática y monitoreo continuo.*

## 1. Restricción Temporal Crítica: Point-in-Polygon < 50ms

### Desafío
El algoritmo de geocerca debe completar su procesamiento en menos de 50ms (RF-02) para garantizar respuesta inmediata a movimientos del ganado. Ese plazo mide solo el tiempo de procesamiento: la salida de Deep Sleep (~200ms) se cuenta aparte (ver «Timing de Wakeup»).

### Análisis de Riesgo Basado en Principios de Elecia White
Siguiendo el principio de "design for the worst case":
- **Variabilidad en condiciones**: Temperatura, voltaje y envejecimiento de componentes
- **Carga del sistema**: Interferencia de otras tareas críticas
- **Complejidad del polígono**: Número variable de vértices y formas irregulares

### Estrategia de Gestión de Riesgos
Siguiendo los principios de Elecia White:
- **Worst-case analysis**: Análisis de peor caso para cada configuración
- **Graceful degradation**: Degradación controlada bajo condiciones extremas
- **Continuous monitoring**: Monitoreo continuo del tiempo de ejecución

### Soluciones Propuestas (aún no implementadas)

#### A. Optimización del Algoritmo
```c
// Propuesta (el archivo aún no existe): firmware/src/geofence/pip_ray_casting.c
bool pip_optimized(const point_t* p, const polygon_t* poly) {
    // Precondición: Polígono cargado en PSRAM
    // Postcondición: Resultado en < 50ms
    
    int crosses = 0;
    const int vertex_count = poly->num_vertices;
    
    // Vectorización parcial para ESP32-S3 SIMD
    for (int i = 0; i < vertex_count; i++) {
        const point_t* v1 = &poly->vertices[i];
        const point_t* v2 = &poly->vertices[(i+1) % vertex_count];
        
        // Early exit conditions
        if ((v1->y > p->y) == (v2->y > p->y)) continue;
        
        // Reducción de operaciones de punto flotante
        const float vt = (p->y - v1->y) / (v2->y - v1->y);
        if (p->x < v1->x + vt * (v2->x - v1->x)) {
            crosses++;
        }
    }
    
    return (crosses & 1);  // Impar = dentro, par = fuera
}
```

#### B. Uso de Memoria PSRAM
- Precarga del polígono en la PSRAM integrada en el encapsulado del chip (8 MB, octal; módulo N16R8, no es una memoria externa)
- Acceso dual-core simultáneo para paralelismo
- Reducción de latencia de acceso a datos (~10ns vs 150ns)

#### C. Profiling estimado (pendiente de medición en hardware)
| Configuración Polígono | Tiempo Estimado (no medido) | Margen |
|------------------------|---------------|--------|
| 8 vértices             | 12ms          | 38ms   |
| 16 vértices            | 22ms          | 28ms   |
| 32 vértices            | 41ms          | 9ms    |
| 64 vértices            | 78ms          | ❌FAIL |

### Mitigación para Polígonos Grandes
1. Simplificación de Douglas-Peucker durante configuración
2. Subdivisión en cuadrantes para procesamiento paralelo
3. Cache de resultados parciales en SRAM interna

## 2. Interrupción GPIO Wakeup desde la IMU (LSM6DSOX en el ejemplo)

> **POR DECIDIR (IMU):** el BOM lista el LSM6DSOX y el docente tiene el LSM6DS3TR-C. Los ejemplos y notas de registros de esta sección suponen el LSM6DSOX.

### Desafío
Detectar movimiento significativo mientras el sistema está en Deep Sleep requiere configuración precisa del acelerómetro.

### Análisis de Fiabilidad
Según Elecia White, "never trust hardware completely":
- **False positives**: Movimiento no relevante que activa el sistema
- **False negatives**: Movimiento importante no detectado
- **Environmental factors**: Vibraciones ambientales, cambios de temperatura

### Estrategia de Verificación
Implementando el principio de Elecia White de "test everything":
- **Self-testing**: Verificación automática de funcionamiento del sensor
- **Calibration**: Calibración automática según condiciones ambientales
- **Redundancy**: Verificación cruzada con otros sensores cuando disponibles

### Solución Técnica
```c
// Propuesta (el archivo aún no existe): firmware/src/drivers/imu_lsm6dsox.c
int lsm6dsox_configure_wakeup(void) {
    // Configurar threshold de movimiento (0.5g)
    lsm6dso_write_reg(LSM6DSO_WAKE_UP_THS, 0x20);  // 32 LSB = 0.5g
    
    // Configurar duración (16 muestras consecutivas)
    lsm6dso_write_reg(LSM6DSO_WAKE_UP_DUR, 0x10);
    
    // Habilitar interrupt en INT1 pin
    lsm6dso_write_reg(LSM6DSO_INT1_CTRL, 0x20);    // WAKEUP_SRC
    
    // Modo low-power a 26Hz
    lsm6dso_write_reg(LSM6DSO_CTRL1_XL, 0x10);     // 26Hz, 2g range
    
    return 0;
}
```

> **Nota:** los valores de registro de este ejemplo no están verificados contra la hoja de datos del LSM6DSOX. Según la cabecera de registros de ST (`lsm6dsox_reg.h`), el bit 5 de `INT1_CTRL` es `int1_fifo_full` (el wake-up se enruta a INT1 con `MD1_CFG`), el bit 4 de `WAKE_UP_DUR` es `wake_ths_w` (la duración es `wake_dur`, bits 5-6) y `CTRL1_XL` = `0x10` corresponde a 12.5 Hz, no a 26 Hz.

### Timing de Wakeup
1. IMU detecta movimiento: < 10ms
2. Generación de interrupt: < 1ms
3. ESP32 sale de Deep Sleep: ~200ms
4. Tiempo total hasta PIP: ~211ms (aceptable)

## 3. Manejo de Errores en Tiempo Real

### Principios de Diseño de Tolerancia a Fallos
Según Elecia White, "design for failure":
1. **Fast failure detection** - Detección rápida de condiciones anormales
2. **Graceful degradation** - Degradación controlada sin fallos catastróficos
3. **Recovery mechanisms** - Mecanismos automáticos de recuperación
4. **Fail-safe defaults** - Estados seguros por defecto

### Estrategias de Recuperación Proactiva

#### A. Monitoreo de Salud del Sistema
- Mediciones continuas de voltaje de batería (MAX17048)
- Verificación periódica de sensores
- Detección de condiciones térmicas extremas (el BOM no incluye un sensor de temperatura; **POR DECIDIR**)
- **Principio de Elecia White**: "Monitor everything important"

#### B. Reinicios Controlados
1. Soft Reset: Reinicio de tarea específica
2. Hard Reset: Reinicio del microcontrolador
3. Brown-out Recovery: Recuperación de caída de voltaje
4. **Safety mechanism**: Contador de reinicios para evitar loops

#### C. Logs Circulares en Flash
```c
// Propuesta (el archivo aún no existe): firmware/include/system_log.h
#define LOG_BUFFER_SIZE 1024
typedef struct {
    uint32_t timestamp;
    log_level_t level;
    char message[64];
} log_entry_t;

static log_entry_t log_buffer[LOG_BUFFER_ENTRIES];
static atomic_uint log_index;
```

### Plan de Contingencia

### Estrategia de Respuesta a Incidentes
Siguiendo los principios de Elecia White:
1. **Immediate response** - Respuesta inmediata a condiciones críticas
2. **Safe defaults** - Valores seguros por defecto
3. **Operator notification** - Notificación clara a operadores

### Niveles de Alerta
- **Warning**: Condiciones fuera de rango normal
- **Critical**: Fallos que afectan funcionalidad principal
- **Emergency**: Situaciones que requieren intervención inmediata

### Watchdog Implementation
```c
// Propuesta (el archivo aún no existe): firmware/src/main.c
K_THREAD_DEFINE(geofence_task, STACK_SIZE, geofence_thread, 
                NULL, NULL, NULL, PRIORITY, 0, 0);

void geofence_thread(void) {
    // Activar watchdog con timeout de 1 segundo
    k_watchdog_init(&wdt, 1000);
    
    while(1) {
        k_watchdog_feed(&wdt);
        
        // Ejecutar PIP algorithm
        pip_result_t result = pip_execute();
        
        // Verificar tiempo de ejecución
        if (get_execution_time() > 50) {
            LOG_WARNING("PIP execution time exceeded 50ms");
            // Registrar evento y continuar
        }
        
        k_sleep(K_SECONDS(1));
    }
}
```

## 4. Estrategias de Validación y Verificación

### Principios de Validación de Elecia White
1. **Test in realistic conditions** - Pruebas en condiciones reales
2. **Measure, don't guess** - Medición empírica en lugar de suposiciones
3. **Validate edge cases** - Verificación de condiciones límite
4. **Continuous improvement** - Mejora continua basada en datos

### A. Pruebas en Nordic Thingy:91 X (prueba de concepto)
- Ejecutar algoritmo PIP con coordenadas mock
- Medir tiempo de ejecución con stopwatch de osciloscopio
- Verificar precisión contra implementación de referencia

### B. Simulación de Condiciones Extremas
- Temperatura: -20°C a 65°C (el módulo ESP32-S3-WROOM-1U-N16R8 admite -40 a 65 °C de ambiente: en 65 °C no queda margen; con ECC de PSRAM llega a 85 °C y se pierde 1/16 de la PSRAM, datasheet del módulo)
- Vibración: Simulación de movimiento animal
- Interferencia: Señales LoRa concurrentes

### C. Métricas de Desempeño
| Métrica | Objetivo | Resultado | Estado |
|---------|----------|------|--------|
| Tiempo PIP | < 50ms | No medido | Pendiente |
| Consumo Sleep | < 150 μA (RNF-03; presupuesto: unos 124 μA) | No medido | Pendiente |
| Tiempo Wakeup | < 300ms | No medido (estimación de diseño: ~211ms, ver «Timing de Wakeup») | Pendiente |
| Precisión GNSS | < 3m | No medido | Pendiente |

## Conclusiones

Este análisis exhaustivo de riesgos en tiempo real asegura que el collar DR-VF-01:

1. **Identifica proactivamente** todos los riesgos críticos de tiempo real
2. **Mitiga sistemáticamente** cada riesgo con soluciones específicas
3. **Monitorea continuamente** el desempeño en condiciones operativas
4. **Recupera automáticamente** de condiciones anormales sin intervención humana

La implementación sigue las mejores prácticas de Elecia White para sistemas embebidos robustos, garantizando operación confiable bajo todas las condiciones previstas.
