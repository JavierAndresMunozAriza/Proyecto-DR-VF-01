# Ubicación y Diseño de Antenas para Collar DR-VF-01

## Descripción General

Este documento detalla la ubicación, diseño y consideraciones técnicas de las antenas del collar de geocercamiento DR-VF-01. El BOM (`hardware/BOM.md`, secciones 1.2 y 1.3) lista tres antenas, una por banda:

| Antena | Banda | Uso | Pieza en el BOM |
|--------|-------|-----|-----------------|
| LoRa | 915 MHz | Enlace LoRa del módulo DX-LR30-900M22S | Flexible de 8 cm con conector u.FL, sin MPN (listado "IPEX 1.13 868/915 MHz 2 dBi 8 cm") |
| Wi-Fi/BLE | 2.4 GHz | Wi-Fi y BLE del módulo ESP32-S3-WROOM-1U-N16R8 | FPC Würth Elektronik 2015301200, vía u.FL |
| GNSS | 1575.42 MHz (L1) | Receptor MAX-M10S (solo L1) | Parche cerámico pasivo de 18x18x4 mm, MPN POR DECIDIR |

La antena FPC de 2.4 GHz solo sirve al Wi-Fi y al BLE del módulo; no es la antena de LoRa ni la de GNSS. El BOM también lista, solo para pruebas de banco, un conector SMA hembra (Würth 6521011000) y un kit de antena corta SMA de 3 dBi con pigtail u.FL a SMA de 15 cm.

**Estado:** las posiciones y el montaje de las tres antenas están **POR DECIDIR**. Lo marcado "pendiente" no tiene fuente en el repositorio (no hay hoja de datos del Würth 2015301200 ni del parche GNSS) y no es una especificación. La fila de antena GNSS ANNMS-420.A que aún aparece en la tabla del BOM no es una decisión (ver `hardware/BOM.md`, sección 1.2).

## Especificaciones de las Antenas

### 1. Características Técnicas
| Característica | LoRa 915 MHz | Wi-Fi/BLE 2.4 GHz | GNSS L1 |
|----------------|--------------|-------------------|---------|
| Tipo | Monopolo de cuarto de onda, flexible de 8 cm | FPC (flexible PCB) | Parche cerámico pasivo |
| Conexión | Conector u.FL SMD en la PCB (p. ej. Hirose U.FL-R-SMT-1, a verificar) | Conector U.FL del módulo ESP32-S3-WROOM-1U | POR DECIDIR |
| Ganancia | 2 dBi según el listado, sin verificar | Pendiente (sin hoja de datos) | Pendiente (MPN POR DECIDIR) |
| Impedancia | 50 Ω (entrada RF del módulo LoRa según el vendedor) | Pendiente (sin hoja de datos) | Pendiente (MPN POR DECIDIR) |
| Patrón y polarización | Pendiente | Pendiente | Pendiente |

### 2. Dimensiones Físicas
- **LoRa**: longitud 8 cm (BOM). Resto pendiente
- **Wi-Fi/BLE (FPC Würth 2015301200)**: longitud, anchura, grosor y substrato pendientes. Los valores del texto anterior (65.0mm x 15.0mm, 0.1mm, Duroid 5880LCP con constante dieléctrica 2.95, 2.5 dBi) no proceden de la hoja de datos de esta pieza y quedan sin verificar
- **GNSS**: 18mm x 18mm x 4mm (BOM)

### 3. Materiales
- **Antena FPC**: conductor, cubierta y adhesivo pendientes de la hoja de datos de Würth. Los valores del texto anterior (cobre electrolítico 18μm, poliimida 12.5μm, acrílico termoplástico) no tienen fuente y quedan sin verificar
- **Fijación de la antena FPC**: adhesivo acrílico de doble cara 3M 467MP (BOM, sección 2.2)
- **Parche GNSS y antena LoRa**: pendientes (MPN POR DECIDIR y sin MPN)

## Ubicación en la Carcasa

Las posiciones de las tres antenas están **POR DECIDIR**. Las distancias de las secciones 1 a 3 son la propuesta anterior para el FPC de 2.4 GHz, sin fuente, y están pendientes de validar.

### 1. Posicionamiento Óptimo
- **Zona de Instalación**: POR DECIDIR para las tres antenas (propuesta anterior para el FPC: cara superior de la carcasa)
- **Distancia desde bordes**: Mínimo 10mm de cualquier borde (propuesta anterior para el FPC, sin fuente)
- **Orientación**: Paralela al eje longitudinal del collar (propuesta anterior para el FPC, sin fuente)
- **Altura sobre superficie**: Contacto directo con interior de carcasa (propuesta anterior para el FPC, sin fuente)

### 2. Consideraciones de Diseño
- **Separación de metal**: Mínimo 5mm de cualquier componente metálico (propuesta anterior, sin fuente)
- **Plano de masa**: las antenas LoRa y GNSS dependen del plano de masa de la PCB principal; sus dimensiones están pendientes
- **Clearance mecánico**: 2mm libres en todas direcciones (propuesta anterior para el FPC, sin fuente)
- **Flexibilidad**: Radio de curvatura mínimo: 2mm (propuesta anterior para el FPC, sin fuente)

### 3. Integración Mecánica
- **Fijación**: Adhesivo acrílico de doble cara (3M 467MP) para el FPC
- **Protección**: Cubierta de poliimida adicional (propuesta anterior, sin fuente)
- **Sellado**: Compatible con requerimientos IP67 (objetivo de diseño); el paso de los cables u.FL por la carcasa está pendiente (ver `mechanical/IP67_SPEC.md`)
- **Durabilidad**: rango de temperatura del producto -20°C a +65°C; la resistencia de la antena y del adhesivo a ciclos térmicos está pendiente de verificar

### 4. Antena LoRa 915 MHz (8 cm, u.FL)
- **Tipo**: monopolo de cuarto de onda. Su rendimiento depende del plano de masa de la PCB (BOM, sección 1.3)
- **Posición, orientación y recorrido del cable**: POR DECIDIR
- **Montaje**: la antena debe estar conectada antes de energizar el módulo LoRa (material del vendedor)
- **Validación**: probar el alcance real; el alcance no está medido

### 5. Antena GNSS (parche pasivo 18x18x4 mm)
- **Elección**: el MAX-M10S ya integra LNA y filtro SAW, por eso el BOM recomienda un parche pasivo; una antena activa solo se justifica con cable largo (BOM, sección 1.2)
- **Plano de masa y zona libre**: POR DECIDIR (definir en el layout; sus dimensiones no tienen fuente hasta elegir el MPN)
- **Posición y orientación en la carcasa**: POR DECIDIR

### 6. Línea de 50 Ω y Conectores u.FL
- **Módulo LoRa**: la antena por defecto sale por el pad ANT (pin 21); la PCB necesita una línea de 50 Ω hasta el conector u.FL (`hardware/pinout.md`). Ancho, longitud y dieléctrico de la pista: pendientes (dependen del apilado de la PCB, no definido)
- **Conector u.FL del módulo LoRa frente al pad ANT**: pendiente de confirmar con el vendedor
- **Módulo ESP32-S3-WROOM-1U**: el FPC de 2.4 GHz se conecta al conector U.FL del módulo
- **Pistas SPI**: lejos de la antena del módulo (`hardware/pinout.md`)

### 7. Kit de Banco SMA
- Conector SMA hembra (Würth 6521011000) y kit de antena corta SMA de 3 dBi con pigtail u.FL a SMA de 15 cm (los 3 dBi del listado no se pueden verificar)
- Solo para pruebas de banco; no forma parte del producto

### 8. Aislamiento entre Antenas
- Separación entre las antenas de 915 MHz, 1575 MHz y 2.4 GHz: POR DECIDIR (sin fuente; pendiente de simulación o medición)

## Diseño Electromagnético

### 1. Simulación y Optimización
- **Software**: CST Studio Suite 2023 (propuesto)
- **Modelo 3D**: Incluye carcasa plástica y componentes internos
- **Frecuencia objetivo**: 2400-2500MHz para el FPC Wi-Fi/BLE; 915MHz (LoRa) y 1575.42MHz (GNSS) pendientes de definir
- **S11 objetivo**: <-10dB en banda de operación (objetivo sin fuente)

### 2. Parámetros de Rendimiento
Objetivos para la antena Wi-Fi/BLE de 2.4 GHz, sin fuente en el repositorio y pendientes de validar. Los de LoRa y GNSS están pendientes.

| Parámetro | Valor objetivo | Método de medición |
|-----------|----------------|-------------------|
| S11 (Return Loss) | <-10dB | VNA Agilent E5071C |
| Ganancia | >2.0 dBi | Cámara anecoica IEEE 149 |
| Eficiencia radiación | >85% | Balanza de ganancia |
| Ancho de banda | >80MHz | -10dB puntos de retorno |

### 3. Efectos Ambientales
- **Humedad**: Variación <5% en parámetros hasta 95% HR
- **Temperatura**: Estabilidad ±2% entre -20°C y +65°C
- **Flexión**: Resistencia a 10,000 ciclos de flexión ±10°

## Integración con PCB Principal

### 1. Puntos de Conexión
- **LoRa**: conector u.FL SMD en la PCB (BOM), con línea de 50 Ω desde el pad ANT del módulo
- **Wi-Fi/BLE**: conector U.FL del módulo ESP32-S3-WROOM-1U; no hay soldadura directa de la antena a la PCB
- **GNSS**: POR DECIDIR (el punto de alimentación y el plano de masa dependen del MPN)
- **Posición relativa y tolerancia de soldadura de los conectores**: pendiente
- Se retiraron los pads de soldadura directa del texto anterior (feed de 1.5mm x 1.5mm y dos pads de tierra de 1.0mm x 1.0mm): el BOM usa conectores u.FL

### 2. Impedancia de Transmisión
- **Línea de 50 Ω**: del pad ANT del módulo LoRa al conector u.FL. Ancho, grosor del dieléctrico y longitud: pendientes hasta definir el apilado de la PCB
- **Soldadura**: Aleación SAC305 (valor del texto anterior, sin fuente)

### 3. Verificación Eléctrica
Criterios sin fuente en el repositorio, pendientes de validar; aplican a cada conexión u.FL.
- **Medición S-parameters**: VNA antes de ensamble final
- **Prueba de continuidad**: Multímetro (<1Ω conexión)
- **Aislamiento**: >100MΩ entre feed y ground
- **Resistencia mecánica**: >5N fuerza de tracción

## Procedimiento de Instalación

Aplica a la antena FPC Wi-Fi/BLE. El montaje de las antenas LoRa y GNSS está POR DECIDIR.

### 1. Preparación de Superficie
1. Limpiar área de adherencia con isopropanol al 99%
2. Abridar superficie con lija fina (400 grit) si necesario
3. Aplicar adhesivo de activación (3M Scotch-Weld PR100)
4. Dejar secar 10 minutos antes de colocar antena

### 2. Colocación Precisa
1. Alinear marcadores de referencia en antena y carcasa
2. Colocar antena comenzando desde extremo fijo
3. Eliminar burbujas de aire con tarjeta plástica
4. Presionar firmemente durante 30 segundos

### 3. Verificación Post-Instalación
1. Medir impedancia con analizador vectorial
2. Verificar patrón de radiación en zona libre
3. Confirmar S11 < -10dB en 2.4GHz
4. Registrar parámetros en hoja de ensamble

## Consideraciones de Certificación

### 1. Compatibilidad Electromagnética
- **Emisión (objetivo)**: FCC Part 15 Class B; pendiente de ensayo
- **Inmunidad (objetivo)**: resistir un campo electromagnético de 3V/m; pendiente de ensayo
- **Armónicos**: < -30dBc en frecuencias fundamentales

### 2. Certificaciones RF
- **CE Marking**: Directiva RED 2014/53/EU
- **FCC ID**: Pendiente de asignación
- **IC RSS (objetivo)**: RSS-Gen y RSS-247; pendiente de evaluación

### 3. Seguridad Eléctrica
- **Voltajes presentes en el producto**: 3.3V en la lógica, 4.2V máximo en la batería, 5V en VBUS del USB-C y 6V nominales en la entrada del panel solar
- **Corriente de fuga**: <100μA bajo todas condiciones
- **Aislamiento**: >500V DC entre circuitos y carcasa

## Mantenimiento y Reparación

### 1. Diagnóstico de Problemas
- **Indicadores de fallo**: Pérdida de cobertura, aumento de errores LoRa
- **Herramientas de diagnóstico**: VNA portátil, analizador de espectro
- **Pruebas comunes**: Medición de S11, verificación de conexiones

### 2. Procedimiento de Reemplazo
1. Calentar área con pistola de aire caliente (150°C)
2. Retirar cuidadosamente con herramienta de precisión
3. Limpiar residuos de adhesivo con isopropanol
4. Instalar nueva antena siguiendo procedimiento estándar

### 3. Calidad del Reemplazo
- Verificar impedancia antes de cerrar carcasa
- Confirmar cobertura en prueba de campo cercano
- Documentar nuevo número de serie en registro de mantenimiento

## Anexos

### 1. Diagramas Técnicos
- Patrón de radiación 2D y 3D
- Diagrama de impedancia vs frecuencia
- Layout de puntos de conexión

### 2. Datos de Materiales
- Hoja de datos de la antena Würth 2015301200 (pendiente; no está en el repositorio)
- Hoja de datos del parche GNSS (MPN POR DECIDIR)
- Especificaciones del adhesivo 3M 467MP
- Certificados de conformidad de materiales

## Principios de Diseño Basados en Elecia White

### Optimización de Rendimiento
Siguiendo los principios de Elecia White para sistemas embebidos:
- **Function-first design**: Priorizar funcionalidad crítica sobre características adicionales
- **Environmental awareness**: Considerar todas las condiciones operativas esperadas
- **Measurement-driven optimization**: Basar decisiones en datos empíricos de prueba

### Verificación Sistemática
- **Test everything critical**: Validar todos los aspectos que afectan el rendimiento RF
- **Document all findings**: Registrar resultados de pruebas para referencia futura
- **Verify in realistic conditions**: Probar bajo condiciones similares a uso real

### Mantenimiento Práctico
- **Design for serviceability**: Facilitar diagnóstico y reparación sin herramientas especiales
- **Enable field repair**: Permitir reemplazo de componentes sin equipo de laboratorio
- **Provide clear procedures**: Instrucciones detalladas para técnicos de mantenimiento

Esta documentación recoge las tres antenas del BOM (LoRa, Wi-Fi/BLE y GNSS). Las posiciones y dimensiones marcadas POR DECIDIR o pendientes se completan cuando se cierren los puntos abiertos, siguiendo los principios de diseño robusto de Elecia White.
