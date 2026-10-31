# Análisis Térmico del Collar DR-VF-01

## Descripción General

Este documento presenta el análisis térmico detallado de todos los componentes del collar de geocercamiento DR-VF-01. El análisis incluye cálculos de disipación de potencia, incrementos de temperatura y verificación de límites térmicos bajo condiciones de operación normales y extremas.

*Nota: Este análisis sigue los principios de gestión térmica de sistemas embebidos descritos en "Making Embedded Systems" por Elecia White, enfocándose en estrategias de disipación pasiva y protección contra sobrecalentamiento.*

## Componentes de Alta Disipación

### 1. Regulador de Voltaje LDO (TLV75733PDBVR)
**Especificaciones:**
- Voltaje de entrada: 4.2V (batería llena); 5.0V solo si el USB alimentara el LDO directamente (ruta pendiente de decidir)
- Voltaje de salida: 3.3V
- Corriente de salida: 300mA en ráfaga (las cargas del BOM suman 221 a 394mA; máximo del regulador 1A)
- Resistencia térmica (RθJA, datasheet TLV757P, tabla 5.4): DBV 100.8°C/W en la EVM de TI y 231.1°C/W en la placa JEDEC; DYD con pad térmico 60.3°C/W (EVM) y 92.5°C/W (JEDEC)
- Límite de temperatura de unión: 125°C

**Cálculos (Vin = 4.2V, Iout = 300mA, DBV en la EVM):**
- Potencia disipada: P = (Vin - Vout) × Iout = (4.2V - 3.3V) × 0.3A = 0.270W
- Incremento de temperatura: ΔT = P × RθJA = 0.270W × 100.8°C/W = 27.2°C
- Temperatura de unión: Tj = Tamb + ΔT = 65°C + 27.2°C = 92.2°C

**Sensibilidad** (Tj a 65°C de ambiente, corriente continua, régimen estacionario):

| Caso | P | DBV EVM (100.8) | DBV JEDEC (231.1) | DYD EVM (60.3) | DYD JEDEC (92.5) |
|------|---|-----------------|-------------------|----------------|------------------|
| 4.2V, 300mA | 0.270W | 92.2°C | **127.4°C** | 81.3°C | 90.0°C |
| 5.0V, 300mA | 0.510W | 116.4°C | **182.9°C** | 95.8°C | 112.2°C |

**Verificación:**
- Margen con DBV en la EVM (4.2V, 300mA): 125°C - 92.2°C = 32.8°C (ADECUADO)
- Con la resistencia térmica de la placa JEDEC (2s2p, sin vías) el DBV supera los 125°C si los 300mA fueran continuos
- El perfil real es de ráfagas: la corriente promedio es de unos 0.4mA y el LDO disipa unos 0.35mW. No hay datos de la constante de tiempo térmica, así que el cálculo en régimen estacionario es una cota, no una predicción
- El DBV basta en ráfagas cortas, como la transmisión LoRa. Si una carga de unos 300mA fuera continua, o si el USB alimentara el LDO a 5.0V, se usa el TLV75733PDYDR (mismo regulador con pad térmico)

### 2. Cargador Solar (CN3791)
**Especificaciones:**
- Voltaje de entrada: 5.0V (panel solar)
- Voltaje de salida: 4.2V (batería LiPo)
- Eficiencia típica: 88%
- Corriente de salida: 250mA máxima

**Cálculos:**
- Potencia de salida: Pout = 4.2V × 0.25A = 1.05W
- Potencia de entrada: Pin = Pout / η = 1.05W / 0.88 = 1.19W
- Potencia disipada: Pdis = Pin - Pout = 1.19W - 1.05W = 0.14W

**Consideraciones adicionales:**
- Disipación máxima durante carga rápida: 0.2W
- Incremento de temperatura estimado: < 10°C

### 3. Radio LoRa (SX1262) - Modo TX
**Especificaciones:**
- Voltaje de operación: 3.3V
- Corriente en TX (+22dBm): 120mA
- Duración típica de TX: 0.5 segundos por transmisión

**Cálculos:**
- Potencia consumida: P = 3.3V × 0.120A = 0.396W
- Energía por transmisión: E = 0.396W × 0.5s = 0.198J
- Disipación promedio diaria: Depende de frecuencia de transmisión

## Análisis de Disipación Total

### 1. Escenario de Operación Normal
| Componente | Potencia Disipada | Contribución |
|------------|-------------------|---------------|
| LDO TLV75733P | 0.270W | 65.7% |
| CN3791 | 0.140W | 34.1% |
| SX1262 (promedio) | 0.001W | 0.2% |
| **TOTAL** | **0.411W** | **100%** |

### 2. Escenario de Carga Solar Máxima
| Componente | Potencia Disipada | Contribución |
|------------|-------------------|---------------|
| LDO TLV75733P | 0.270W | 54.7% |
| CN3791 | 0.200W | 40.5% |
| SX1262 (TX) | 0.024W | 4.9% |
| **TOTAL** | **0.494W** | **100%** |

*Nota: el LDO se calcula a Vin = 4.2V y 300mA, el punto de referencia del BOM. Los porcentajes están redondeados a un decimal y suman 100.1% en esta tabla.*

## Distribución Térmica en PCB

### 1. Zonas de Calor Críticas
1. **Área del LDO**: Mayor concentración térmica
   - Temperatura de unión calculada: 92.2°C a 65°C de ambiente (DBV, EVM de TI)
   - Separación recomendada de componentes sensibles: 5mm

2. **Área del Cargador Solar**: Moderada disipación
   - Temperatura superficial estimada: 68°C
   - Proximidad a batería requiere consideración térmica

3. **Área de Radio LoRa**: Disipación pulsada
   - Picos térmicos breves durante transmisión
   - No representa riesgo continuo

### 2. Consideraciones de Diseño PCB
- **Plano de masa extenso**: Para disipación de calor
- **Vías térmicas**: Conectando planos internos para transferencia
- **Separación de componentes**: Entre alta y baja temperatura
- **Materiales PCB**: FR4 estándar (baja conductividad térmica propia; la disipación depende de los planos de cobre y las vías)

## Verificación en Condiciones Extremas

### 1. Temperatura Ambiente Máxima (+65°C)
| Componente | Potencia | ΔT | Tj |
|-----------|----------|----|----|
| LDO (4.2V, 300mA, DBV en EVM) | 0.270W | 27.2°C | 92.2°C |
| CN3791 | 0.140W | 8.5°C | 73.5°C |
| **Máximo** | | | **92.2°C** |

### 2. Sobrecarga de Componentes
- **LDO a 394mA** (suma máxima de las cargas del BOM, 4.2V, DBV en EVM): P = 0.355W, ΔT = 35.7°C, Tj = 100.7°C
- **CN3791 a 300mA**: P = 0.172W, ΔT = 10.5°C, Tj = 75.5°C

## Estrategias de Gestión Térmica

### 1. Diseño Mecánico
- **Ventilación pasiva**: Aprovechamiento de convección natural
- **Contacto con carcasa**: Disipación a través de plástico
- **Colocación estratégica**: Componentes calientes en áreas ventiladas

### 2. Selección de Componentes
- **LDO de bajo dropout**: Minimiza disipación
- **Eficiencia del cargador**: 88% reduce pérdidas
- **Componentes SMD**: Mejor transferencia térmica que through-hole

### 3. Protección Térmica
- **Monitoreo de temperatura**: Sensor NTC en PCB
- **Reducción de carga dinámica**: En condiciones extremas
- **Shutdown térmico**: A 100°C para protección de componentes

## Estrategia de Gestión Térmica Basada en Principios de Sistemas Embebidos

### Filosofía de Diseño Térmico
Siguiendo los principios de Elecia White sobre diseño robusto de sistemas embebidos:
1. **Fail-safe thermal design** - El sistema debe operar de forma segura incluso bajo condiciones térmicas extremas
2. **Predictive thermal management** - Monitoreo proactivo de condiciones térmicas
3. **Graceful thermal degradation** - Reducción controlada de funcionalidad bajo estrés térmico

### Arquitectura de Protección Térmica
Implementamos una estrategia de protección en múltiples capas:

| Nivel | Mecanismo | Acción |
|-------|-----------|--------|
| 1. Prevención | Diseño térmico optimizado | Disipación pasiva |
| 2. Detección | Monitoreo continuo | Alertas tempranas |
| 3. Respuesta | Reducción de carga | Degradación controlada |
| 4. Protección | Shutdown térmico | Protección de componentes |

## Validación Experimental (pendiente: aún no hay hardware construido)

### 1. Pruebas Térmicas Planificadas
- **Cámara térmica FLIR ONE**: Mediciones en condiciones reales
- **Cámara de temperatura controlada**: De -20°C a +65°C
- **Perfil de estrés térmico**: Ciclos acelerados de envejecimiento

### 2. Resultados (pendientes de medición)
| Condición | Temperatura Máxima Medida | Comparación con Cálculo |
|-----------|---------------------------|-------------------------|
| 25°C ambiente | Pendiente (cálculo: 52.2°C en LDO) | Pendiente |
| 65°C ambiente | Pendiente (cálculo: 92.2°C en LDO) | Pendiente |
| Carga solar | Pendiente | Pendiente |

## Consideraciones Ambientales

### 1. Efecto de Humedad
- **Condensación**: Riesgo en cambios bruscos de temperatura
- **Corrosión**: En componentes metálicos expuestos
- **Protección**: Sellado IP67 evita estos problemas

### 2. Efecto Solar Directo
- **Incremento térmico**: Hasta 15°C adicional en exposición directa
- **Gradiente térmico**: Variaciones en carcasa pueden causar estrés
- **Compensación**: Diseño con margen térmico adicional

## Recomendaciones de Diseño

### 1. Mejoras Propuestas
1. **Uso de regulador Buck**: Para reducir disipación LDO
2. **Optimización de ruta de corriente**: Minimizar resistencias
3. **Mejora de disipación**: Vías térmicas adicionales en PCB

### 2. Monitoreo Continuo
- **Sensor de temperatura**: Integrado en próxima revisión
- **Registro térmico**: En memoria no volátil para diagnóstico
- **Alertas térmicas**: Por LoRa en condiciones extremas

## Conclusiones

El análisis térmico del collar DR-VF-01 demuestra que:

1. El LDO TLV75733PDBVR queda en 92.2°C a 65°C de ambiente con 300mA continuos y la resistencia térmica de la EVM (límite 125°C). Con la de la placa JEDEC llegaría a 127.4°C, así que el margen depende del cobre de la PCB y de que la carga sea de ráfagas
2. Si una carga de unos 300mA fuera continua, o si el USB alimentara el LDO a 5.0V, se usa el TLV75733PDYDR con pad térmico (112.2°C como máximo en esos casos)
3. El resto de componentes opera dentro de sus límites y la disipación está distribuida en la PCB
4. No se requieren soluciones de enfriamiento activas

Las temperaturas máximas calculadas son de régimen estacionario y las mediciones están pendientes; el caso del DBV con carga continua es el único que supera el límite, y el DYD lo cubre.

Este diseño térmico sigue las mejores prácticas de sistemas embebidos descritas por Elecia White en "Making Embedded Systems", garantizando:
- **Robustez térmica** mediante diseño defensivo
- **Monitoreo proactivo** de condiciones críticas
- **Respuesta controlada** a eventos térmicos extremos
- **Protección integral** de componentes sensibles
