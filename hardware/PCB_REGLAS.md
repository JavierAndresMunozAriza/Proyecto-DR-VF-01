# Reglas de diseño de la PCB (borrador del 2026-11-07)

Paso 42 de `PLAN_MANANA.md`. Se extrae de las guías de Espressif y u-blox y de los datasheets de `hardware/datasheets/`. Cada valor lleva su fuente, o está marcado **estimación** (cuenta propia, no medida) o **por confirmar**. El fabricante quedó elegido el 2026-11-07 (paso 41): **JLCPCB**, 2 capas, 0.8 mm, acabado **ENIG**. Las reglas de la sección 2 se cargan en Altium para que el DRC avise en vivo.

## 1. Supuestos del apilado

| Dato | Valor | Origen |
|------|-------|--------|
| Capas | 2 (Top y Bottom) | `docs/ARCHITECTURE.md` 133-134 |
| Material y espesor total | FR4, 0.8 mm (por el layout del USB-C de 16 pines) | `docs/ARCHITECTURE.md` 133 |
| Fabricante y acabado | JLCPCB, ENIG | decidido por el usuario el 2026-11-07 |
| Tolerancia del espesor | ±0.1 mm por debajo de 1.0 mm: de 0.7 a 0.9 mm | centro de ayuda de JLCPCB (`jlcpcb.com/capabilities/pcb-capabilities`) |
| Control de impedancia | solo desde 4 capas: en 2 capas no se pide | centro de ayuda de JLCPCB; confirmar al cotizar |
| Cobre | 1 oz (35 µm) | supuesto, por confirmar al pedir |
| Dieléctrico entre capas | 0.8 − 2 x 0.035 = 0.73 mm | estimación |
| εr del FR4 | 4.4 (rango 4.2 a 4.6) | valor típico; por confirmar |
| Plano de masa | vertido de GND en ambas capas; la capa inferior queda como plano de referencia continuo bajo la RF y el USB | `docs/ARCHITECTURE.md` 134 |

## 2. Reglas para cargar en Altium

| Regla | Valor | Estado |
|-------|-------|--------|
| Ancho de pista, señal | 0.2 mm mínimo (JLCPCB admite 0.127 mm en 1-2 capas) | mínimo del fabricante confirmado en su centro de ayuda; 0.2 mm es mi margen |
| Ancho de pista, potencia (3V3, VBAT, GND local) | 0.3 mm mínimo; 0.5 mm en el lazo del CN3791 | estimación (IPC-2221, 1 oz, ΔT = 10 °C: 0.3 mm ≈ 1 A) |
| Separación entre cobre | 0.2 mm mínimo (JLCPCB admite 0.127 mm en 1-2 capas) | mínimo del fabricante confirmado; 0.2 mm es mi margen |
| Vía estándar | taladro 0.3 mm, pad 0.6 mm | 0.3 mm es el mínimo de JLCPCB en 1-2 capas (tolerancia +0.13 / −0.08 mm): la vía queda justo en el mínimo; usar 0.35 mm de taladro donde haya espacio |
| **Línea de 50 Ω (LoRa y GNSS)** | microstrip de **1.35 mm** sobre el plano de la capa inferior: 50.1 Ω con 0.8 mm y εr 4.4; **de 45.8 a 54.0 Ω (≈ ±8 %)** con la tolerancia de espesor de 0.7 a 0.9 mm | **estimación** (Hammerstad-Jensen con corrección por grosor): al no haber impedancia controlada en 2 capas, esa dispersión se acepta para líneas cortas de 868/915 MHz y 1575 MHz; se mide el alcance en la primera placa |
| **Par USB D+/D−** | 90 Ω ±10 %, igual longitud, sin cruzar capas, GND continuo debajo y cobre de GND alrededor | `esp32-s3_hardware_design_guidelines.pdf` 1.4.8 |
| Anchura del par USB en este apilado | pistas de ~1.2 mm con ~0.8 mm de separación daría 90 Ω | **estimación**; no cabe junto a los pads de 0.5 mm del USB-C |
| Máscara y pasta | modo Rule, expansión de máscara de 0.05 mm (la regla del docente según mis notas) | por confirmar contra el proyecto del docente |

**Compromiso del USB:** el USB nativo del ESP32-S3 es de velocidad completa (12 Mb/s). Con el par corto, de igual longitud, junto y con GND continuo debajo, se acepta estrechar las pistas cerca del conector en vez de buscar los 90 Ω exactos. Es una decisión mía, no de la guía de Espressif; hay que comprobarla con la primera placa (enumeración y programación por USB).

Clases de red que se crean en Altium: `PWR` (3V3, VBAT, VCC del CN3791), `RF50` (ANT del LoRa al U.FL, entrada del GNSS al U.FL), `USB` (D+ y D−), `SIG` (el resto).

## 3. Zonificación: qué va lejos de qué

| Elemento | Regla de distancia | Fuente |
|----------|--------------------|--------|
| Componentes de RF del GNSS | **5 mm como mínimo** a cualquier otro componente de RF; lejos de lo digital | `max-m10s_integration_manual.pdf` 4.4 |
| Parche GNSS y su U.FL | del lado opuesto al conector del Wi-Fi del módulo y al LoRa; la interferencia fuera de banda del Wi-Fi/BT y del LoRa puede degradar el fix | misma guía, 4.2 |
| MAX-M10S | lejos de fuentes de calor (TP4056, CN3791, inductor) y sin pistas de otras señales debajo del módulo, en las dos capas | misma guía, 4.4 |
| Cristal de 32.768 kHz | lejos de pistas digitales y de inductores; sin vías en sus dos pistas; las cargas de 33 pF junto a sus pads | `esp32-s3_hardware_design_guidelines.pdf` 1.4.3 (es la guía del cristal de 40 MHz del chip; se aplica por analogía) |
| Etapa de potencia solar (CN3791, AP2301, D1, D2, L, C1) | agrupada, lo más lejos posible del GNSS y del cristal | `pinout.md` y `cn3791_datasheet.pdf` p9-10 |

Orden de ubicación sugerido: 1) conectores y módulos con antena (ESP32, LoRa, GNSS); 2) etapa de potencia solar; 3) regulador y cargador USB; 4) desacople y pasivos.

## 4. Reglas por bloque

**ESP32-S3-WROOM-1U.** Es la versión con conector U.FL: no hay antena impresa. La guía que pide recortar la placa y 15 mm de despeje en la caja es para el módulo con antena PCB (`esp32-s3_hardware_design_guidelines.pdf` 1.4.7) y no aplica; sí aplica poner abundante cobre de GND y vías de GND densas cerca del conector de RF. 22 µF y 0.1 µF en el pin 3V3 del módulo; el RC de EN junto al pin y EN nunca flotante; huecos para R y C en serie en las pistas USB, cerca del módulo.

**USB-C.** Par diferencial corto, ver la sección 2. Vías de retorno a GND en cada cambio de capa, si hay alguno.

**GNSS (MAX-M10S).** Masa continua bajo el módulo, vías de GND abundantes y sin tramos sueltos (stubs) en los planos de masa: si no se pueden evitar, terminan en una vía a la masa de referencia. Línea de RF corta, de 50 Ω, rodeada de vías de GND. Resistencia en serie de la alimentación VCC de 0.2 Ω o menos (`max-m10s_integration_manual.pdf` 4.1.1). Huella con cobre y máscara del mismo tamaño y pasta en forma de T (4.4.1).

**LoRa (DX-LR30-900M22S).** Del pin ANT (21) al U.FL, con la línea de 50 Ω y vías de GND a los lados; masa continua bajo los pads de masa del módulo (folleto y `mechanical/ANTENNA_PLACEMENT.md` 72). Antena conectada antes de energizar.

**Regulador TLV75733P.** CIN y COUT lo más cerca posible, planos de cobre en sus conexiones y vías térmicas alrededor (`tlv757p_datasheet.pdf` 7.4.1). El 22 µF adicional va junto al LoRa y deja una huella 1206 sin montar (`pinout.md` 125).

**TP4056.** El pad térmico a GND con una zona de cobre amplia y vías hacia la capa inferior: es el disipador (`tp4056_yongyutai_datasheet.pdf`, sección térmica, y `pinout.md`).

**CN3791.** Lazo diodos-MOSFET-inductor-C1 lo más corto posible, con el positivo de C1 junto al source de M1; masas de C3 y D2 al cobre de C1 antes de ir a la masa del sistema; RCS junto a la salida del inductor con CSP y BAT como par (conexión Kelvin); no conectar la batería con el cargador funcionando (`cn3791_datasheet.pdf` p9-10, resumido en `pinout.md`).

## 5. Puntos abiertos

1. **Fabricante cerrado (JLCPCB, ENIG, 2 capas, 0.8 mm).** Al hacer el pedido, comprobar en la página: el espesor de 0.8 mm, el acabado ENIG, el precio, el plazo de fabricación y el **envío a Colombia con los impuestos de importación** (no encontré datos). El espesor del dieléctrico y el εr reales no los publica el fabricante en 2 capas: la línea de 50 Ω queda con la dispersión de la sección 2.
1b. **Fecha límite del pedido:** la PCB se espera a inicios de diciembre; hay que fijarla al conocer el plazo real de entrega, con el esquemático, el layout y las librerías (CN3791, LoRa, 0.47 Ω, huella 1206) terminados antes.
2. **Tamaño de la PCB y de la caja:** sin definir (ver `PLAN_MANANA.md`).
3. **Comprobaciones en la primera placa:** enumeración y programación por USB, fix del GNSS con el parche sobre el plano de cobre, rizado del riel de 3.3 V en el TX y alcance del LoRa.
4. Las reglas de máscara y de separación del docente se verifican contra las huellas ya guardadas en `shared_lib` antes de fijar el DRC.
