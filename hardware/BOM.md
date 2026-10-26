# Lista de Materiales (BOM) - Collar DR-VF-01

## Descripción General

Este documento presenta la lista completa de materiales necesarios para la fabricación del collar de geocercamiento DR-VF-01. Incluye todos los componentes electrónicos, mecánicos y de embalaje con sus respectivos números de parte del fabricante (MPN) y proveedores recomendados.

## Principios de Selección de Componentes Basados en Elecia White

### Estrategia de Selección
Siguiendo los principios de Elecia White para sistemas embebidos:
1. **Proven components first** - Priorizar componentes con historial comprobado en aplicaciones similares
2. **Long-term availability** - Seleccionar componentes con disponibilidad garantizada por años
3. **Serviceability** - Facilitar reemplazo y reparación de componentes individuales
4. **Cost-effective reliability** - Balance óptimo entre costo y confiabilidad

### Criterios de Evaluación
- **Field reliability data**: Preferir componentes con datos de confiabilidad en campo
- **Temperature ratings**: Todos los componentes certificados para -20°C a +65°C
- **Industrial grade**: Componentes clasificados para aplicaciones industriales
- **Multiple sources**: Al menos dos proveedores para componentes críticos
## Componentes Electrónicos

### 1. Microcontrolador y Memoria
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Microcontrolador ESP32-S3-WROOM-1U-N16R8 | ESP32-S3-WROOM-1U-N16R8 | Espressif | Digi-Key |
| 1 | Flash SPI 128Mbit | GD25Q128CWIGR | GigaDevice | Mouser |
| 1 | SRAM 8Mbit | IS62WVS5128GBLL-10NL | ISSI | Digi-Key |

### 2. Sistema de Posicionamiento
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Módulo GNSS u-blox MAX-M10S | MAX-M10S-00B | u-blox | Avnet |
| 1 | Antena GNSS cerámica | ANNMS-420.A | Toko | Digi-Key |

### 3. Comunicaciones Inalámbricas
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Transceptor LoRa SX1262 | SX1262IMLTRT | Semtech | Digi-Key |
| 1 | Antena FPC 2.4GHz | 2015301200 | Wurth Electronics | Mouser |
| 1 | Conector SMA hembra | 6521011000 | Wurth Electronics | Mouser |

### 4. Sensores y Actuadores
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | IMU 6-axis LSM6DSOX | LSM6DSOXTR | STMicroelectronics | Digi-Key |
| 1 | Optoacoplador VO617A | VO617A-1-E | Vishay | Mouser |
| 1 | Buzzer piezoeléctrico | PKMCS0909E000-R1 | Murata | Digi-Key |

### 5. Gestión de Energía
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Regulador LDO 3.3V 200mA | TPS7A0533PDBVR | Texas Instruments | Digi-Key |
| 1 | Cargador solar MPPT CN3791 | CN3791 | ChipsBase | AliExpress |
| 1 | Protección de batería | BQ2975DRBR | Texas Instruments | Digi-Key |
| 1 | Batería LiPo 3.7V 1500mAh | PL063-1500 | PowerStream | Digi-Key |
### 6. Conectores y Interfaces
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Conector USB-C hembra | 1240-6813-1-ND | Amphenol | Digi-Key |
| 2 | Conector JST PH 2-pin | B2B-PH-K-S(LF)(SN) | JST | Mouser |
| 4 | Conector JST SH 4-pin | B4B-SH-SM4-TB(LF)(SN) | JST | Mouser |
| 1 | Botón tactil 6x6mm | PTS645SL50SMTR92 LFS | C&K | Digi-Key |

### 7. LEDs y Señalización
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | LED RGB difuso | L-154A4SURK/G5T | Kingbright | Digi-Key |
| 3 | LED indicador simple | LTST-C193KGKT | Lite-On | Mouser |

### 8. Componentes Pasivos
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Cristal 40MHz | ECS-.327-12-5PXDN-TR | ECS | Digi-Key |
| 4 | Capacitor cerámico 22pF | C0G2A220J5GACA024 | TDK | Digi-Key |
| 10 | Capacitor cerámico 100nF | CGA2B1X7R1E104K050BC | TDK | Digi-Key |
| 5 | Capacitor cerámico 10μF | C1608X5R1A106K080AE | TDK | Digi-Key |
| 2 | Capacitor tantalio 100μF | TAJA107K016RNJ | AVX | Digi-Key |
## Componentes Mecánicos

### 1. Carcasa y Estructura
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Carcasa principal moldeada | CUSTOM-DRVF01-HOUSING | Fabricación interna | Proveedor local |
| 1 | Tapa posterior moldeada | CUSTOM-DRVF01-BACK | Fabricación interna | Proveedor local |
| 1 | Correa de nylon ajustable | CUSTOM-DRVF01-STRAP | Fabricación interna | Proveedor local |
| 4 | Tornillo M2x6 cabeza allen | 50017102 | PEM | Fastenal |
| 2 | Inserto roscado M2 | PEM-SL2-02 | PEM | Fastenal |

### 2. Protección Ambiental
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Junta tórica EPDM | AS0139 | Apple Rubber | McMaster-Carr |
| 2 | Tapas ciegas USB-C | 1240-6813-1-ND | Amphenol | Digi-Key |
| 1 | Adhesivo acrílico doble cara | 467MP | 3M | Grainger |

### 3. Panel Solar
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
## Materiales de Embalaje

### 1. Embalaje Primario
| Cantidad | Descripción | MPN | Fabricante | Proveedor |
|----------|-------------|-----|------------|------------|
| 1 | Caja de cartón corrugado | ULINE-CORRUGATED-BOX | Uline | Uline |
| 1 | Bolsa antiestática | 12"x16" Anti-Stat Bag | Static Solutions | Amazon |
| 1 | Espuma de embalaje | 1/2" Foam Sheet | Uline | Uline |

### 2. Documentación
| Cantidad | Descripción | Referencia | Origen | Notas |
|----------|-------------|------------|--------|-------|
| 1 | Manual de usuario impreso | USER_MANUAL.md | Interno | En español |
| 1 | Guía de inicio rápido | QUICK_START.md | Interno | En español |
| 1 | Tarjeta de garantía | WARRANTY_CARD | Interno | En español |

## Costos Estimados

### 1. Costo por Unidad (USD)
| Categoría | Costo estimado |
## Proveedores Alternativos

### 1. Distribuidores Electrónicos
| Distribuidor | Ventajas | Desventajas |
|--------------|----------|-------------|
| Digi-Key | Stock inmediato, datos técnicos completos | Precios ligeramente más altos |
| Mouser | Amplia selección, envío rápido | Requiere cuenta corporativa |
| Arrow | Precios competitivos | Tiempos de entrega variables |

### 2. Proveedores Mecánicos
| Proveedor | Especialidad | Región |
|-----------|--------------|--------|
| Protolabs | Prototipos rápidos | América del Norte |
| Xometry | Producción a demanda | Global |
| Fictiv | Calidad premium | América del Norte |

## Consideraciones de Sourcing

### 1. Lead Times Típicos
- Componentes estándar: 1-2 semanas
- Componentes personalizados: 4-6 semanas
- Carcasa moldeada: 6-8 semanas (primera tirada)

### 2. MOQ (Cantidad Mínima de Pedido)
- Componentes comunes: 1 unidad
- Componentes personalizados: 100 unidades
- Carcasa moldeada: 500 unidades

### 3. Certificaciones Requeridas
- RoHS compliant para todos los componentes electrónicos
- REACH compliance para materiales mecánicos
- Certificación UL para componentes de potencia

## Historial de Revisiones

| Versión | Fecha | Cambios |
|---------|-------|---------|
| 1.0 | 2024-05-15 | Versión inicial |
| 1.1 | 2024-06-20 | Actualización de precios y MPNs |

## Notas Adicionales

1. Todos los componentes deben cumplir con las especificaciones de temperatura de operación de -20°C a +65°C
2. Los componentes críticos deben tener al menos dos fuentes de suministro
3. La batería debe cumplir con las normativas de transporte IATA para baterías de litio
4. Todos los plásticos deben ser resistentes a UV para aplicaciones exteriores

Esta BOM proporciona una base completa para la fabricación del collar DR-VF-01 con opciones de sourcing flexibles y costos controlados.
|----------|----------------|
| Componentes electrónicos | $24.50 |
| Componentes mecánicos | $8.75 |
| Fabricación de PCB | $3.25 |
| Ensamblaje | $5.50 |
| Embalaje | $1.25 |
| **TOTAL** | **$43.25** |

## Estrategia de Abastecimiento Basada en Principios de Elecia White

### Gestión de Cadena de Suministro
Siguiendo los principios de Elecia White para sistemas embebidos:
- **Multiple sourcing**: Al menos dos proveedores para cada componente crítico
- **Long-term partnerships**: Relaciones establecidas con proveedores confiables
- **Risk assessment**: Evaluación continua de riesgos en la cadena de suministro
- **Quality assurance**: Verificación de calidad en recepción de materiales

### Verificación de Componentes
- **Incoming inspection**: Prueba básica de funcionalidad para componentes críticos
- **Lot traceability**: Registro de lotes para rastreo de calidad
- **Supplier qualification**: Evaluación periódica de proveedores
- **Alternative identification**: Identificación proactiva de componentes sustitutos

Esta BOM proporciona una base completa para la fabricación del collar DR-VF-01 con opciones de sourcing flexibles y costos controlados, siguiendo los principios de diseño robusto de Elecia White.
### 2. Costo por Lote (1000 unidades)
| Categoría | Costo total |
|----------|--------------|
| Componentes electrónicos | $24,500 |
| Componentes mecánicos | $8,750 |
| Fabricación de PCB | $3,250 |
| Ensamblaje | $5,500 |
| Embalaje | $1,250 |
| **TOTAL** | **$43,250** |
|----------|-------------|-----|------------|------------|
| 1 | Panel solar monocristalino 2W | HTJ-2W-D | HiTechGlobal | Amazon Business |
| 1 | Cable solar JST PH 2-pin | CUSTOM-JST-CABLE | Fabricación interna | Local |
| 3 | Resistor 10K 1% | RC0603FR-0710KL | Yageo | Digi-Key |
| 2 | Resistor 100K 1% | RC0603FR-07100KL | Yageo | Digi-Key |