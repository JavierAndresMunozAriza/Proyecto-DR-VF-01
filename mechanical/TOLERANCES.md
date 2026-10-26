# Tolerancias Mecánicas del Collar DR-VF-01

## Descripción General

Este documento establece las tolerancias dimensionales y geométricas para todos los componentes mecánicos del collar de geocercamiento DR-VF-01. Las tolerancias están definidas para garantizar el ensamble preciso, el sellado IP67 y la durabilidad estructural bajo condiciones de operación ganadera.

## Tolerancias Dimensionales Generales

### 1. Clases de Tolerancia
- **Clase A (Crítica)**: ±0.05mm (elementos de sellado y alineación)
- **Clase B (Importante)**: ±0.1mm (interacciones funcionales)
- **Clase C (General)**: ±0.2mm (dimensiones no críticas)

### 2. Unidades de Medida
Todas las dimensiones están expresadas en milímetros (mm) a menos que se indique lo contrario.

## Carcasa Principal

### 1. Dimensiones Exteriores
| Dimensión | Nominal | Tolerancia | Clase |
|-----------|---------|------------|-------|
| Longitud | 120.0 | ±0.2 | C |
| Anchura | 80.0 | ±0.2 | C |
| Altura | 25.0 | ±0.2 | C |
| Grosor pared | 2.5 | ±0.1 | B |

### 2. Superficies de Sellado
| Característica | Tolerancia | Notas |
|----------------|------------|-------|
| Planitud cara superior | 0.05 | Clase A |
| Paralelismo tapa-carcasa | 0.03 | Clase A |
| Concentricidad ejes | 0.02 | Clase A |
| Rugosidad superficial | Ra 1.6 max | ISO 4287 |

### 3. Ranuras y Cavidades
| Elemento | Tolerancia ancho | Tolerancia profundidad |
|----------|------------------|------------------------|
| Ranura junta tórica | ±0.03 | ±0.02 |
| Cavidad PCB | ±0.1 | ±0.05 |
| Alojamiento USB-C | ±0.05 | ±0.03 |

## Componentes Electrónicos

### 1. PCB Principal
| Dimensión | Nominal | Tolerancia | Clase |
|-----------|---------|------------|-------|
| Longitud | 75.0 | ±0.1 | B |
| Anchura | 50.0 | ±0.1 | B |
| Grosor | 1.6 | ±0.05 | C |
| Posición taladros montaje | ±0.05 | A |

### 2. Conector USB-C
| Característica | Tolerancia | Notas |
|----------------|------------|-------|
| Posición eje X | ±0.1 | B |
| Posición eje Y | ±0.1 | B |
| Profundidad inserción | ±0.05 | A |
| Ángulo de montaje | ±0.5° | B |

### 3. Botón de Accionamiento
| Dimensión | Nominal | Tolerancia | Clase |
|-----------|---------|------------|-------|
| Diámetro exterior | 8.0 | ±0.05 | A |
| Recorrido útil | 0.3 | ±0.02 | A |
| Fuerza actuación | 150g | ±20g | B |
## Elementos de Fijación

### 1. Tornillos Autofresantes M2
| Característica | Tolerancia | Notas |
|----------------|------------|-------|
| Diámetro nominal | M2 | ISO 7046 |
| Longitud útil | ±0.1 | C |
| Torque apriete | 0.7 N·m ±0.1 | Especificación crítica |

### 2. Agujeros de Montaje
| Tipo | Tolerancia diámetro | Tolerancia posición |
|------|---------------------|---------------------|
| Roscado interno | ±0.05 | ±0.03 |
| Taladro pasante | ±0.03 | ±0.02 |

## Juntas y Sellos

### 1. Junta Tórica (EPDM)
| Dimensión | Tolerancia | Norma |
## Correa de Ajuste

### 1. Dimensiones Nominales
| Elemento | Nominal | Tolerancia | Clase |
|----------|---------|------------|-------|
| Longitud total | 500 | ±5 | C |
| Anchura | 25 | ±1 | B |
| Grosor | 3 | ±0.2 | C |

### 2. Hebilla de Seguridad
| Dimensión | Tolerancia | Notas |
|-----------|------------|-------|
| Diámetro pasador | ±0.1 | B |
| Longitud útil | ±0.5 | C |
| Resistencia tracción | >500N | Test |

## Tolerancias Geométricas (GD&T)

### 1. Posicionamiento Relativo
| Control | Tolerancia | Datum reference |
## Materiales y Acabados

### 1. Carcasa Principal (PC+GF20%)
| Propiedad | Valor objetivo | Tolerancia |
|-----------|----------------|------------|
| Dureza Shore D | 75 | ±5 |
| Resistencia tracción | 120 MPa | ±10 |
| Módulo elasticidad | 3.5 GPa | ±0.3 |

### 2. Recubrimientos
| Tipo | Espesor | Tolerancia |
|------|---------|------------|
| Recubrimiento UV | 25 μm | ±5 |
| Tratamiento superficial | 1-3 μm | ±0.5 |

## Procedimientos de Verificación

### 1. Equipos de Medición
- Calibrador digital (±0.01mm)
- Micrómetro (±0.001mm)
- Rugosímetro (±0.01μm Ra)
- Proyector de perfil (±0.005mm)

### 2. Frecuencia de Control
- **Pre-producción**: 100% de piezas
- **Producción**: Muestra estadística (AQL 0.65)
- **Validación periódica**: Cada lote de 1000 unidades

## Documentación de Cambios

### 1. Historial de Revisiones
| Versión | Fecha | Cambios principales |
|---------|-------|-------------------|
| 1.0 | 2024-05-15 | Versión inicial |
| 1.1 | 2024-06-20 | Ajuste tolerancia USB-C |

### 2. Procedimiento de Modificación
Cualquier cambio en estas tolerancias requiere:
1. Aprobación del ingeniero principal
2. Verificación de impacto en IP67
3. Actualización de documentación asociada
4. Validación en prototipo funcional

Estas tolerancias aseguran la interoperabilidad, durabilidad y calidad del producto final, manteniendo los estándares exigidos para equipos electrónicos expuestos a condiciones ambientales severas.
|---------|------------|-----------------|
| Posición PCB | 0.1 | A-B-C |
| Perpendicularidad USB | 0.05 | A |
| Paralelismo tapa | 0.03 | B |

### 2. Forma y Orientación
| Control | Tolerancia | Aplicación |
|---------|------------|------------|
| Planitud | 0.05 | Superficies sellado |
| Circularidad | 0.03 | Agujeros montaje |
| Cilindricidad | 0.02 | Ejes de simetría |
|-----------|------------|-------|
| Diámetro interior | ±0.05 | ISO 3601 |
| Sección transversal | ±0.02 | ISO 3601 |
| Compresión recomendada | 15-30% | Interna |

### 2. Superficies de Contacto
| Parámetro | Valor máximo | Notas |
|-----------|--------------|-------|
| Rugosidad (Ra) | 1.6 μm | ISO 4287 |
| Ondulación (Wt) | 4.0 μm | ISO 4287 |
| Desviación planitud | 0.05 mm | Clase A |

## Antena FPC

### 1. Dimensiones Físicas
| Dimensión | Nominal | Tolerancia | Clase |
|-----------|---------|------------|-------|
| Longitud total | 65.0 | ±0.5 | C |
| Anchura útil | 15.0 | ±0.2 | B |
| Grosor dieléctrico | 0.1 | ±0.01 | A |

### 2. Puntos de Soldadura
| Característica | Tolerancia | Notas |
|----------------|------------|-------|
| Posición pads | ±0.05 | A |
| Dimensiones pad | ±0.02 | A |
| Coplanaridad | 0.075 | IPC-A-610 |
## Estrategia de Verificación Basada en Principios de Elecia White

### Verificación Sistemática
Siguiendo el principio de Elecia White de "measure everything important":
- **Critical features first**: Verificación prioritaria de elementos Clase A
- **Statistical sampling**: Muestreo representativo para elementos Clase B y C
- **Traceable measurements**: Registro completo de todas las mediciones

### Gestión de Calidad Proactiva
- **Prevention over correction**: Diseño que minimiza necesidad de ajustes
- **Early detection**: Identificación temprana de desviaciones del proceso
- **Continuous improvement**: Actualización de tolerancias basada en datos reales

Estas tolerancias aseguran la interoperabilidad, durabilidad y calidad del producto final, manteniendo los estándares exigidos para equipos electrónicos expuestos a condiciones ambientales severas, siguiendo los principios de diseño robusto de Elecia White.