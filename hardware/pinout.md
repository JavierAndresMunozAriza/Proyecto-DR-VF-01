# Asignación de Pines del Collar DR-VF-01

## Descripción General

Este documento detalla la asignación de pines del microcontrolador ESP32-S3-WROOM-1U-N16R8 utilizado en el collar de geocercamiento DR-VF-01. Incluye conexiones para todos los periféricos, sensores y comunicaciones del sistema.

## Diagrama de Bloques del Sistema

```
ESP32-S3
├── GNSS (u-blox MAX-M10S)
├── IMU (LSM6DSOX)
├── LoRa (SX1262)
├── USB-C (Carga y Configuración)
├── Optoacoplador (VO617A)
├── LEDs Indicadores
├── Botón de Usuario
└── Batería y Cargador Solar
```

## Asignación de Pines del ESP32-S3

### 1. Comunicaciones Seriales

| Pin ESP32 | Función | Periférico | Notas |
|----------|---------|------------|-------|
| GPIO04 | UART1 TX | MAX-M10S GNSS | 9600 baudios |
| GPIO05 | UART1 RX | MAX-M10S GNSS | 9600 baudios |
| GPIO17 | I2C SDA | LSM6DSOX IMU | 400kHz |
| GPIO18 | I2C SCL | LSM6DSOX IMU | 400kHz |
| GPIO07 | SPI MOSI | SX1262 LoRa | 10MHz máximo |
| GPIO06 | SPI MISO | SX1262 LoRa | 10MHz máximo |
| GPIO05 | SPI SCK | SX1262 LoRa | 10MHz máximo |
| GPIO08 | SPI CS | SX1262 LoRa | Activo bajo |
| GPIO09 | DIO1 | SX1262 LoRa | IRQ interrupciones |
| GPIO10 | BUSY | SX1262 LoRa | Señal ocupado |
| GPIO11 | RESET | SX1262 LoRa | Reset activo bajo |

### 2. Interfaces USB y Carga

| Pin ESP32 | Función | Conexión | Notas |
|----------|---------|----------|-------|
| GPIO19 | USB D+ | USB-C | Conector físico |
| GPIO20 | USB D- | USB-C | Conector físico |
| GPIO21 | CHARGE_STAT | CN3791 | Estado de carga |
| GPIO22 | POWER_GOOD | CN3791 | Indicador de alimentación |
| GPIO23 | ENABLE_CHG | CN3791 | Habilitación de carga |

### 3. Control de Actuadores

| Pin ESP32 | Función | Periférico | Notas |
|----------|---------|------------|-------|
| GPIO01 | CONTROL | VO617A Opto | Control de disuasión |
| GPIO02 | BUZZER | Buzzer | Señal PWM para audio |
| GPIO03 | LED_RED | LED RGB | Canal rojo |
| GPIO12 | LED_GREEN | LED RGB | Canal verde |
| GPIO13 | LED_BLUE | LED RGB | Canal azul |

### 4. Entradas Digitales

| Pin ESP32 | Función | Fuente | Notas |
|----------|---------|--------|-------|
| GPIO14 | BUTTON | Botón táctil | Pull-up interno |
| GPIO15 | INT_IMU | LSM6DSOX | Interrupción de movimiento |
| GPIO16 | INT_GNSS | MAX-M10S | Interrupción de posición |

### 5. Alimentación y Referencias

| Pin ESP32 | Función | Valor | Notas |
|----------|---------|-------|-------|
| 3V3 | Alimentación 3.3V | 3.3V | Regulada desde LDO |
| GND | Referencia 0V | 0V | Tierra del sistema |
| VIN | Alimentación principal | 3.7-5.0V | Desde batería o USB |
| EN | Habilitación | 3.3V | Pull-up interno |
## Principios de Diseño Basados en Elecia White

### Organización Clara y Documentada
Siguiendo los principios de Elecia White para sistemas embebidos:
- **Explicit functionality**: Cada pin tiene una función claramente definida
- **Logical grouping**: Agrupación de funciones relacionadas por tipo de interfaz
- **Consistent naming**: Convención de nombres coherente en toda la asignación
- **Traceable connections**: Todas las conexiones están documentadas con referencias

### Protección contra Errores
- **Input protection**: Diodos de protección en líneas susceptibles
- **Current limiting**: Resistencias limitadoras en salidas de corriente
- **Pull-up/pull-down configuration**: Configuración definida para entradas
- **Voltage level matching**: Niveles de señal compatibles entre todos los componentes

### Facilidad de Mantenimiento
- **Accessible test points**: Puntos de prueba disponibles en conexiones críticas
- **Modular connections**: Interfaces independientes para fácil reemplazo
- **Standard interfaces**: Uso de buses estándar (I2C, SPI, UART)
- **Clear labeling**: Identificación clara de funciones en diagramas

## Conexiones de Periféricos

### 1. Módulo GNSS u-blox MAX-M10S
```
MAX-M10S           ESP32-S3
TX (Pin 12)  ---->  GPIO04 (UART1 TX)
RX (Pin 11)  ---->  GPIO05 (UART1 RX)
VCC (Pin 1)  ---->  3V3
GND (Pin 10) ---->  GND
```

### 2. IMU LSM6DSOX
```
LSM6DSOX         ESP32-S3
SDA (Pin 14) ----> GPIO17 (I2C SDA)
SCL (Pin 13) ----> GPIO18 (I2C SCL)
INT1 (Pin 6) ----> GPIO15 (INT_IMU)
VDD (Pin 10) ----> 3V3
GND (Pin 1)  ----> GND
```

### 3. Transceptor LoRa SX1262
```
SX1262           ESP32-S3
MOSI (Pin 6) ----> GPIO07 (SPI MOSI)
MISO (Pin 7) ----> GPIO06 (SPI MISO)
SCK (Pin 5)  ----> GPIO05 (SPI SCK)
NSS (Pin 4)  ----> GPIO08 (SPI CS)
DIO1 (Pin 11) ----> GPIO09 (DIO1)
BUSY (Pin 12) ----> GPIO10 (BUSY)
RST (Pin 3)  ----> GPIO11 (RESET)
VCC (Pin 10) ----> 3V3
GND (Pin 1)  ----> GND
```

### 4. Optoacoplador VO617A
```
VO617A           ESP32-S3
INPUT+ (Pin 1) ----> GPIO01 (CONTROL)
INPUT- (Pin 2) ----> GND
OUTPUT+ (Pin 4) ----> Disuasión+
OUTPUT- (Pin 3) ----> Disuasión-
```

### 5. LEDs y Botón
```
Componentes      ESP32-S3
LED_RGB_RED  ----> GPIO03 (LED_RED)
LED_RGB_GREEN ----> GPIO12 (LED_GREEN)
LED_RGB_BLUE  ----> GPIO13 (LED_BLUE)
BUTTON_SIGNAL ----> GPIO14 (BUTTON)
BUTTON_GND    ----> GND
```

## Consideraciones de Diseño

### 1. Pull-ups y Pull-downs
- GPIO14 (BUTTON): Pull-up interno activo
- GPIO15 (INT_IMU): Pull-up externo 10KΩ
- GPIO16 (INT_GNSS): Pull-up externo 10KΩ

### 2. Niveles de Señal
- Todos los pines operan a 3.3V lógicos
- Tolerancia de 3.6V máxima en entradas
- Corriente máxima por pin: 12mA

### 3. Configuración Inicial
- GPIO01 (CONTROL): LOW al inicio (disuasión desactivada)
- GPIO08 (SPI CS): HIGH al inicio (SX1262 deseleccionado)
- GPIO09-DIO1: Entrada con pull-down interno

## Estados de Error y Protección

### 1. Protección contra Cortocircuitos
- Fusible PTC en línea de alimentación principal
- Diodos de protección en líneas de comunicación
- Resistencias limitadoras en líneas de control

### 2. Estados de Emergencia
- GPIO01 (CONTROL): Auto-reset a LOW después de 5 segundos
- GPIO08 (SPI CS): Auto-reset a HIGH si no hay respuesta LoRa
- GPIO14 (BUTTON): Detección de pulsación larga para reset

## Programación y Debugging

### 1. Pines de Programación
- GPIO00: BOOT (pull-down para modo programación)
- GPIO04/GPIO05: UART1 también usado para debug
- GPIO19/GPIO20: USB-C para programación y debugging

### 2. Estados de Modo
| Estado | GPIO00 | GPIO01 | GPIO08 | Descripción |
|--------|--------|--------|--------|-------------|
| Normal | Flotante | LOW | HIGH | Operación estándar |
Esta asignación de pines permite una implementación completa y eficiente del collar DR-VF-01 con todas sus funcionalidades, siguiendo los principios de diseño robusto de Elecia White.
| Programación | LOW | LOW | HIGH | Modo flashing |
| Debug | Flotante | LOW | HIGH | Serial debugging |
| Emergencia | LOW | LOW | LOW | Reset forzado |

## Revisión del Diseño

| Versión | Fecha | Cambios |
|---------|-------|---------|
| 1.0 | 2024-05-15 | Diseño inicial |
| 1.1 | 2024-06-20 | Reasignación de pines I2C |

## Notas de Implementación

1. Todos los pines deben tener ruteo directo en PCB
2. Vías de señal deben evitar planos de masa cuando sea posible
3. Pines de alimentación deben tener bypass capacitivo cercano
4. Pines de interrupción deben tener filtrado RC para evitar rebotes
5. Pines SPI deben mantener longitud de trazado igual para integridad de señal

Esta asignación de pines permite una implementación completa y eficiente del collar DR-VF-01 con todas sus funcionalidades.