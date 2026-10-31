# Collar de Geocercamiento DR-VF-01 - Datasheet Ejecutivo

## Descripción General
El DR-VF-01 es un sistema avanzado de geocercamiento para ganado que combina tecnología de posicionamiento GNSS, comunicación LoRa de largo alcance y detección de movimiento inteligente para proporcionar una solución completa de seguimiento y contención de animales en pastoreo.

## Características Principales

### Localización Precisa
- **GNSS de Alta Sensibilidad**: u-blox MAX-M10S (solo L1) con recepción concurrente de hasta cuatro constelaciones (GPS, GLONASS, Galileo, BeiDou) según la configuración; por defecto GPS, Galileo y BeiDou B1I
- **Precisión Horizontal**: < 3 metros RMS (objetivo; sin medir en el collar. La hoja de datos del MAX-M10S indica 1.5 m CEP en condiciones de prueba estáticas)
- **Tiempo de Adquisición**: < 30 segundos en condiciones normales

### Comunicación de Largo Alcance
- **Radio LoRa**: módulo DX-LR30-900M22S (SX1262) con alcance objetivo de más de 5 km en campo abierto (sin medir; el vendedor del módulo indica 8 km de distancia de transmisión abierta, dato de marketing)
- **Frecuencia**: 915MHz (banda ISM americana)
- **Potencia de Transmisión**: +22dBm (máximo del transceptor SX1262)
- **Protocolo Propietario**: Optimizado para bajo consumo y alta fiabilidad

### Detección Inteligente de Movimiento
- **Sensor IMU Integrado**: IMU de 6 ejes para detección de movimiento (LSM6DSOX en el BOM; la elección entre LSM6DSOX y LSM6DS3TR-C está POR DECIDIR)
- **Wake-up Selectivo**: Solo activo cuando el animal se mueve significativamente
- **Umbral Ajustable**: Configurable entre 0.1g y 1.0g

### Eficiencia Energética Revolucionaria
- **Autonomía Extendida**: unos 190 días con batería LiPo de 1200mAh (teórico, calculado con un factor de degradación de 0.85; unos 238 días con 1500mAh; la capacidad de la batería está POR DECIDIR; sin medir)
- **Modo Deep Sleep**: unos 124μA de consumo en reposo del sistema (calculado; sin medir)
- **Carga Solar Integrada**: CN3791 MPPT con panel solar de 6V y al menos 1W (MPN POR DECIDIR)
- **Carga por USB-C**: TP4056 a 500mA (carga de banco); el puerto USB-C no alimenta el sistema
- **Gestión Inteligente de Energía**: Optimización automática según perfil de uso

## Especificaciones Técnicas

### Hardware
| Componente | Especificación |
|------------|---------------|
| Microcontrolador | ESP32-S3-WROOM-1U-N16R8 (16MB Flash, 8MB PSRAM) |
| GNSS | u-blox MAX-M10S (solo L1) |
| Radio LoRa | Módulo DX-LR30-900M22S (SX1262 integrado, SPI, 915MHz) |
| IMU | POR DECIDIR (LSM6DSOX o LSM6DS3TR-C) |
| Regulador | LDO TLV75733PDBVR (3.3V, 1A) |
| Cargador Solar | CN3791 MPPT, solo para el panel solar |
| Cargador USB-C | TP4056, 500mA |
| Medidor de batería | MAX17048 |
| Protección ESD USB | USBLC6-2SC6 en las líneas del USB-C |
| Protección de batería | POR DECIDIR |
| Batería | LiPo 3.7V, 1200mAh o 1500mAh (POR DECIDIR) |
| Panel solar | 6V, al menos 1W (MPN POR DECIDIR) |
| Antenas | LoRa: flexible de 8cm con conector u.FL. Wi-Fi/BLE: FPC de 2.4GHz (Würth 2015301200). GNSS: parche pasivo de 18x18x4mm (MPN POR DECIDIR). Ganancias sin verificar |

### Desempeño
| Parámetro | Valor |
|-----------|-------|
| Rango LoRa | Objetivo > 5km (campo abierto; sin medir) |
| Precisión GNSS | < 3m RMS (objetivo; sin medir) |
| Tiempo de Respuesta PIP | < 50ms (objetivo RF-02; pendiente de medir en la placa) |
| Consumo en Sleep | Unos 124μA (calculado; sin medir) |
| Temperatura de Operación | -20°C a +65°C (límite superior fijado por el módulo ESP32-S3-WROOM-1U-N16R8) |
| Grado de Protección | IP67 (objetivo de diseño; pendiente de ensayo) |

### Dimensiones y Peso
- **Dimensiones**: 120mm x 80mm x 25mm
- **Peso**: 180g (incluyendo batería)
- **Material de Carcasa**: Plástico reforzado con fibra de vidrio
- **Correa**: Nylon resistente UV con hebilla de seguridad

## Funcionalidades Avanzadas

### Sistema de Disuasión Inteligente
- **Optoacoplador VO617A**: Para activación selectiva de disuasión
- **Feedback Háptico (planificado)**: Vibración controlada para entrenamiento animal; el BOM actual no incluye motor de vibración
- **Programación Personalizable**: Intensidad y duración ajustables

### Configuración Flexible
- **Wi-Fi SoftAP**: Para configuración inicial sin necesidad de infraestructura
- **Interfaz Web Responsiva**: Compatible con dispositivos móviles
- **Geocercas Multipunto (planificado)**: Definición de áreas complejas con múltiples polígonos; el motor actual admite un único polígono
- **Zonas de Exclusión (planificado)**: Áreas peligrosas o restringidas

### Monitoreo y Diagnóstico
- **Registro de Eventos**: Historial completo de movimientos y alertas
- **Diagnóstico Remoto**: Estado del sistema accesible vía LoRa
- **Alertas de Mantenimiento**: Notificaciones de batería baja y mantenimiento requerido

## Aplicaciones

### Ganadería Extensiva
- Seguimiento de reses en grandes extensiones de pasto
- Prevención de pérdida de animales
- Optimización de rotación de pastoreo

### Reservas Naturales
- Monitoreo de fauna sin intervención humana
- Estudios de comportamiento animal
- Prevención de conflicto humano-animal

### Agricultura de Precisión
- Control de acceso a áreas restringidas
- Monitoreo de salud del ganado
- Integración con sistemas de gestión agrícola

## Ventajas Competitivas

### Tecnología Seleccionada
- Componentes industriales seleccionados por fiabilidad
- Validación en condiciones reales de campo (planificada; aún no realizada)
- Certificaciones RF y seguridad eléctrica (objetivo; pendientes de ensayo)

### Facilidad de Uso
- Instalación intuitiva con guía paso a paso
- Interfaz de usuario simplificada
- Soporte técnico especializado

### Escalabilidad
- Arquitectura modular para futuras expansiones
- Compatibilidad con app móvil (futura versión)
- Integración con sistemas de gestión ganadera existentes

## Especificaciones Ambientales

### Condiciones de Operación
- **Temperatura**: -20°C a +65°C ambiente (el módulo ESP32-S3-WROOM-1U-N16R8 está especificado de -40°C a 65°C ambiente; no se declara operación por encima de 65°C)
- **Humedad**: 5% a 95% sin condensación
- **Altitud**: Hasta 5000m sobre el nivel del mar

### Resistencia Ambiental
- **Grado de Protección**: IP67 (protección contra polvo y agua sumergido temporalmente; objetivo de diseño, pendiente de ensayo)
- **Resistencia UV**: Carcasa formulada para exposición solar prolongada
- **Resistencia a Impactos**: IK08 según norma IEC 62262 (objetivo de diseño, sin ensayar)

## Soporte y Garantía

### Garantía
- **Producto**: 2 años contra defectos de fabricación
- **Batería**: 1 año o 500 ciclos de carga
- **Soporte Técnico**: 1 año gratuito con opción de extensión

### Actualizaciones
- **Firmware**: Actualizaciones gratuitas vía OTA (Over-The-Air)
- **Características**: Nuevas funcionalidades añadidas regularmente
- **Compatibilidad**: Mantenimiento de compatibilidad con versiones anteriores

## Información de Pedido

### Modelos Disponibles
| Modelo | Descripción | Precio Sugerido |
|--------|-------------|-----------------|
| DR-VF-01-BAS | Modelo básico con LoRa | $149.99 |
| DR-VF-01-PRO | Modelo profesional con BLE | $179.99 (próximamente) |

### Incluido en el Kit
- Collar DR-VF-01 completamente ensamblado
- Correa de ajuste universal
- Cable de carga USB-C
- Guía de inicio rápido
- Acceso al portal web de configuración

### Opciones Adicionales
- **Paneles Solares Adicionales**: $29.99
- **Correas Extra**: $14.99
- **Kit de Reparación**: $39.99

## Principios de Diseño Inspirados en Elecia White

### Robustez del Sistema
- **Fault tolerance**: El sistema continúa operando bajo condiciones anormales
- **Graceful degradation**: Funcionalidad limitada pero segura bajo fallos
- **Self-diagnostics**: Verificación automática de salud del sistema

### Mantenibilidad
- **Modular architecture**: Componentes intercambiables y actualizables
- **Clear documentation**: Documentación técnica completa y accesible
- **Remote updates**: Capacidad de actualización vía OTA

### Operación Autónoma
- **Low maintenance**: Diseñado para operar sin intervención humana frecuente
- **Energy efficiency**: Optimizado para máxima autonomía
- **Reliable communication**: Protocolos robustos para condiciones de campo

Este producto representa la aplicación práctica de los principios de sistemas embebidos confiables descritos por Elecia White, garantizando operación estable y mantenimiento mínimo en aplicaciones ganaderas extensivas.

---

*Para más información técnica o solicitudes de muestra, contacte a nuestro equipo de ventas.*
