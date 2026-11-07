# Auditoría: stock del docente frente al BOM, las compras y las librerías

**Fecha:** 2026-11-05. **Documentos auditados:** `hardware/BOM.md` v1.10, `hardware/COMPRAS.md`, `hardware/pinout.md`, y las carpetas de `hardware/libraries/`. **Estado:** informe; no se cambió ningún documento de diseño. Todas las correcciones esperan una decisión del autor (sección 7).

## 1. Regla aplicada

El docente tiene físicamente todo lo que hay en su biblioteca `shared_lib.IntLib`, y lo que publicó en el classroom (`hardware/DOCENTE_CLASSROOM.md`).
1. No se compra una pieza, ni se baja o crea su librería (símbolo, huella y 3D), si el docente ya la tiene en la biblioteca.
2. Excepción: lo que el docente tiene en físico pero solo en el classroom (no en la biblioteca). De eso se baja la librería, pero no se compra.

## 2. Método y límites

1. **Inventario real.** Se leyó el `shared_lib.IntLib` original del docente (no la copia del autor) con `hardware/libraries/tools/altium_lib_check.py`: **71 filas** (70 componentes y la fila `template`), **37 huellas**, y 63 componentes con MPN y código LCSC. `COMPRAS.md` línea 47 dice "61 componentes": está desactualizado.
2. **Lectura propia** de `BOM.md` y `COMPRAS.md` completos.
3. **Revisión independiente:** un subagente de Opus con esfuerzo `xhigh`, sin ver mis conclusiones, volvió a cruzar cada línea contra el inventario.
4. **Verificación de sus afirmaciones.** Lo comprobé contra los archivos. Dos puntos quedaron corregidos o matizados: la línea de `pinout.md` que citó para el 470 nF (no la pude localizar, pero el hecho sí se confirma en `COMPRAS.md`, D10 y D12) y el MPN del tantalio (sección 3.4).
5. **Límites:** las cantidades que tiene el docente de cada pieza no están confirmadas (`COMPRAS.md` línea 47). No hay datasheet local de varias piezas del docente (la lista está en la sección 8). El mensaje del classroom sale de una captura y es breve.

## 3. Hallazgos que cambian una compra o una librería

### 3.1 Pulsador: compra y librería innecesarias (CONFIRMADO; DECIDIDO el 2026-11-05: se usa el del docente)

**Hecho el 2026-11-05:** `BOM.md` v1.11, `COMPRAS.md` (O18 retirado, D13 nuevo, y el PTS645 en la lista de descartados), `pinout.md` v2.6, `pin_connections.csv` y el README de datasheets. La carpeta `hardware/libraries/pts645/` se borró.

| | |
|-|-|
| Qué dice el BOM | Botón táctil 6x6 mm C&K PTS645SL50SMTR92 LFS, Digi-Key (`BOM.md:82`, `COMPRAS.md:35` y `:156`, `pinout.md:347`) |
| Qué tiene el docente | `Sw_PUSH_4.5x4.5x5.0xmm`, BZCN TSC015A05026A (LCSC C2888509), con símbolo de 2 pines, huella `Sw_PUSH_4.5x4.5x5mm` de 4 pads y 3D |
| Evidencia | `BOM.md:209` lista "pulsador 4.5x4.5" como cubierto por el docente. Su datasheet (`hardware/datasheets/bzcn_tsc015.pdf`): 50 mA a 12 V, vida de 100 000 ciclos, terminales 1 y 2 unidos y 3 y 4 unidos (diagrama de circuito), y la huella del docente coincide con su dibujo de soldadura (7.2 x 4.1 mm, pads de 1.35 x 1.1 mm). Ningún documento explica por qué haría falta el de 6x6 |
| Consecuencia | Se bajó la librería del PTS645 (`hardware/libraries/pts645/`) sin necesidad |
| Un dato aún sin confirmar | El datasheet del TSC015 no declara rango de temperatura; el BOM pide -20 a +65 °C (`BOM.md:13`) |

### 3.2 TP4056: no es "solo del classroom"

- La biblioteca del docente **sí lo trae**: `IC_BAT_CHG_4.2V_1A`, MPN `TP4056`, fabricante YONGYUTAI, LCSC C20199357, huella `SOIC_8_3.9mm` de 8 pads, con 3D. El mensaje del classroom dice que no aparece en la librería; la biblioteca dice lo contrario.
- `BOM.md:204` lo llama "fuera de la biblioteca" y `BOM.md:211` lo llama "genérico". Los dos están desactualizados. `COMPRAS.md:59` (D1) sí lo dice bien: "Classroom y biblioteca".
- Por eso la excepción del classroom **no aplica**: no corresponde bajar una librería nueva. Lo que falta es el pad térmico de la huella.
- La biblioteca ya trae la huella **`SOIC_8_3.9mm_EP`** (la usa el TPS56528): 8 pads de 0.6 x 1.45 mm con paso de 1.27 mm, pad central de 2.71 x 3.4 mm y 3D. Habría que comprobar si sirve para el ESOP-8 del TP4056, en vez de crear una huella nueva (`COMPRAS.md:59` y `:93`). Para comprobarlo falta el dibujo del paquete del TP4056, que el datasheet local no trae.
- Dos datos más: el datasheet local es de NanJing Top Power (`BOM.md:64`), pero la pieza del docente es de YONGYUTAI. Y el datasheet dice, en su página 1, "the Radiator need connect GND or impending" (conectar a GND o sin conectar; "impending" parece una errata). Los documentos del repo dicen que el pad térmico "va a GND (p.1)" (`COMPRAS.md:59`): es más firme de lo que escribe el datasheet.
- Una observación: todos los pads de `SOIC_8_3.9mm_EP` tienen la máscara y la pasta en modo "none", no en Rule. Hay que mirarlo en Altium antes de reutilizarla.

### 3.3 Diodos Schottky D1 y D2 del CN3791 (O20): compra por revisar

- Se compran 2 Schottky de baja fuga (`BOM.md:116`, `COMPRAS.md:42` y `:158`), porque los del docente "no sirven". El docente tiene `PSBD1DF40V1H` (40 V, 1 A, SOD-123F) y `SS34-TD` (40 V, 3 A, SMA). Cumplen el "40 V o más, 1 A o más" de O20.
- La razón del rechazo: el SS34 tiene fuga de hasta 0.5 mA a 25 °C y 40 V, y del PSBD1DF40V1H solo hay fuga a 40 V (unos 1.5 µA a 25 °C y 80 µA a 75 °C). **Esos datos vienen de PDFs de LCSC que no están en el repo**, y el requisito es la fuga a unos **4.2 V**, no a 40 V. A 4.2 V la fuga real es muy inferior a la de 40 V, pero ese valor no está leído.
- **Acción:** bajar el datasheet del PSBD1DF40V1H (LCSC C110730) y leer su curva de fuga contra tensión inversa. Comprar solo si a 4.2 V y 65 °C no cumple el margen de sueño de 26 µA.

### 3.4 Condensador de tantalio de 100 µF (O25): MPN dudoso y posible duplicado

- Se elige 1 AVX `TAJA107K016RNJ` (`BOM.md:107`, `COMPRAS.md:43` y `:162`) para el bulk de 3.3 V. El docente tiene un electrolítico de aluminio de 100 µF y 35 V (`Cap_100uF_ALU_TH_35V`, LCSC C2960381), ya asignado a C1 del CN3791 (`COMPRAS.md:70`). Su cantidad no está confirmada.
- **El MPN parece inexistente.** En distribuidores aparecen `TAJC107K016RNJ`, `TAJD107K016RNJ`, `TAJE107K016RNJ` y `TAJY107K016RNJ`, pero no `TAJA107K016RNJ` (un solo listado de un comerciante lo menciona y se contradice a sí mismo: habla de 22 µF y de 100 µF). Un tantalio de 100 µF a 16 V no suele existir en el encapsulado A. No encontré el datasheet de AVX. Hay que verificarlo antes de comprar.
- El datasheet del TLV757P (p.4 y siguientes) pide cerámicos X5R o X7R para la salida, de 0.47 µF o más. No dice nada a favor del tantalio.
- Contradicción de cantidad: `BOM.md:107` dice "1 o 2 (POR DECIDIR)" y `COMPRAS.md:43` y `:162` dicen "1 (no 2)", elegido el 2026-11-05.
- **Acción:** decidir primero si el electrolítico del docente sirve como bulk (ESR y vida útil a 65 °C; el docente tiene al menos uno, falta saber cuántos) y, solo si no, buscar un MPN real de 100 µF.

### 3.5 Condensador de 220 nF (O21): lote de 100 piezas para una pieza

- `COMPRAS.md:40` y `:159` compran un lote de 100 piezas de 220 nF X7R para **un** condensador (el del pin COM del CN3791). El plan B (`COMPRAS.md:86`) arma 235 nF con dos 470 nF del docente, "solo si los 470 nF son X7R (su código R7 lo sugiere, sin confirmar)".
- **Esa condición ya se cumple:** la descripción de la biblioteca dice "50V 470nF **X7R** ±10% 0603". Lo mismo con el 22 µF de 16 V (`COMPRAS.md:68`, `pinout.md:138`, `BOM.md:106` dicen "X5R solo por su código"): la biblioteca dice "X5R ±20%".
- El datasheet del CN3791 (p.3) pide "a 220nF capacitor in series with a 120Ω". 235 nF son un 6.8 % más.
- **Acción:** decidir si se acepta 235 nF y se quita el lote de 220 nF del pedido. Es el único motivo de ese segundo lote.

### 3.6 Conector JST SH de 4 pines (O17): comprar 4 sin uso

- `BOM.md:81` compra 4; `COMPRAS.md:36` y `:155` dicen "por definir" y "falta asignarle un uso, o quitarlo". El código `B4B-SH-SM4-TB` no existe en distribuidores (el que existe es `B4B-PH-SM4-TB(LF)(SN)`).
- El docente tiene `H_Male_4x1_TH_100mil` (cabecera de 4 pines) y los JST PH de 2 pines.
- **Acción:** no comprar ninguno hasta tener un uso. Si hace falta un conector de 4 pines, empezar por la cabecera del docente.

### 3.7 Piezas que el texto exige y nadie lista (para no comprarlas por defecto)

| Necesidad | Dónde | Lo que tiene el docente | Estado |
|-----------|-------|-------------------------|--------|
| Diodo ESD en la entrada de alimentación | `BOM.md:122` | `TVS_3V3_1CH_SOD_523` (LESD5Z3.3T1G, unidireccional 3.3 V) | Sin evaluar. Solo se descartó el SMF5.0CA |
| Pads o conector para el cristal RTC y los puntos de prueba | `BOM.md:89`, `pinout.md:432` | Cabeceras de 2 a 7 pines de 2.54 mm | Sin fila en el BOM |
| MOSFET externos del protector de batería (O16) | `BOM.md:71`, `COMPRAS.md:34` y `:154` | `Q_NFET_20V_3.2A_SOT23` (DN2302S) | Sin evaluar. El protector `BQ2975DRBR` además no existe |

### 3.8 Regulador de 3.3 V: aparece una pieza nueva del classroom

- El BOM compra el TLV75733PDBVR (1 A, Iq de 25 µA) y rechaza el `LDO_3V3_1A_FIXED` del docente, que es un AMS1117-3.3 (`BOM.md:212`). Ese rechazo se apoya en datos generales de la familia AMS1117, no en un datasheet del repo.
- El classroom ahora nombra otro regulador, el **DS8570F8** (DSTECH, LCSC C5884245): 3 A, caída de unos 350 mV a 3 A, salida ajustable desde 0.8 V, con salida de "power good", en encapsulados DFN3x3-10L y ESOP-8. No encontré su corriente quiescente ni qué alimentación auxiliar exige (los resultados de búsqueda mencionan un VDD de 3 V). La descarga automática del PDF devolvió una página HTML, no el PDF; no la reintenté.
- **Acción:** bajar el datasheet. Solo habría reemplazo si su corriente en reposo cabe en el presupuesto de sueño de 124 µA (`POWER_BUDGET.md`).

## 4. Contradicciones y datos obsoletos (no cambian una compra, sí la guía del esquemático)

**Estado al 2026-11-07:** todas las filas de esta tabla están corregidas en `BOM.md` v1.16 y en `COMPRAS.md`. Los números de línea son de la versión del 2026-11-05.

| Tema | Dónde | Qué dice | Qué debería decir / contra qué choca |
|------|-------|----------|---------------------------------------|
| Optoacoplador | `BOM.md:51` | `VO617A-1-E` | No está en la tabla de pedidos (datasheet p.2). `COMPRAS.md:32` usa `VO617A-1X017T` |
| Protección de batería | `BOM.md:65` y `:71` | `BQ2975DRBR` | `COMPRAS.md:154`: la pieza no existe |
| MAX17048 | `BOM.md:53` y `:57` | Digi-Key, encapsulado por decidir | `COMPRAS.md` A5: MAX17048G+T10 de AliExpress, aceptado |
| Batería | `BOM.md:66` | PL063-1500, Digi-Key | `COMPRAS.md:37` y `:147`: tienda local, 18650 o 603450 |
| Panel solar | `BOM.md:147` y `:150` | Lista HTJ-2W-D y a la vez dice que falta el MPN del panel de 6 V | `COMPRAS.md:38`: por definir |
| Cristal | `BOM.md:120` | "MPN completo POR DECIDIR" | Ya está en `BOM.md:102` y `:230` |
| Pinout del USBLC6 | `BOM.md:87` | Falta verificarlo | `COMPRAS.md:61` (D3) ya dice que coincide con la huella |
| Uso de los JST | `BOM.md:89` | Ninguno tiene uso | `COMPRAS.md:66` (D8) y `pinout.md` los asignan a la batería y al panel |
| Electrolítico de C1 | `BOM.md:72` | "Por comprar o definir" | `COMPRAS.md:70` (D12): es del docente |
| Lista "Faltan además" | `BOM.md:124` | Cuenta como faltantes el pull-down de 100 kΩ y las cargas del cristal | Ya son filas del BOM (`:109` y `:103`) |
| Tamaño de la biblioteca | `COMPRAS.md:47` | 61 componentes | 70 componentes y 1 plantilla; 63 con MPN y LCSC |
| AP2301 | `COMPRAS.md:63`, `BOM.md:210` | "La biblioteca dice 30 V", "MOSFET P de 30 V" | Solo el nombre del componente dice 30 V; su descripción dice "P-CH 20V" |
| 470 nF | `COMPRAS.md` D12 | Se usa en C1 | No está en la tabla D10 ni en el BOM 1.8 |
| Plan B | `COMPRAS.md` 1.2 | Reconstruye los valores del kit | No tiene fila para los 470 Ω del LED verde (con 330 + 150 Ω del docente daría 480 Ω) |
| Fabricante del TP4056 | `BOM.md:64` | NanJing Top Power | `COMPRAS.md:59`: YONGYUTAI, como la biblioteca |
| Origen de los 390 kΩ y 1 kΩ | `BOM.md:110` vs `COMPRAS.md:157` | El BOM los toma del docente | O19 dice que vienen del kit. El docente los tiene los dos |
| Origen de los 2.4 kΩ | `BOM.md:111` vs `COMPRAS.md:157` | "Del kit" | El kit no lo trae: se arma con 3K en paralelo con 12K |
| MPN del 100 nF X7R | `BOM.md:104` vs `COMPRAS.md:159` | "CC0603KRX7R9BB104 YAGEO, lote con MPN nombrado" | El listado "no nombra marca ni MPN" |

## 5. Oportunidades que trae el classroom (no son requisitos)

- **TMP102AIDRLR** (temperatura por I2C): el diseño no tiene corte de carga por frío (`BOM.md:72`) porque el CN3791 no tiene pin de temperatura. Medir la temperatura en la placa o en la batería sería posible, pero el CN3791 tampoco tiene pin de habilitación, así que hacer algo con esa medida necesita un camino de hardware. Es una decisión de diseño aparte.
- **INA139NA/3K**: mediría la corriente de carga solar. El diseño ya lee el porcentaje de batería con el MAX17048.
- **TC33X-2-103E** (potenciómetro de 10 kΩ): podría afinar el divisor MPPT, cuyo valor final depende del panel.
- Sin uso previsto: TLV9064IDR (amplificador) y TP8485E-SR (RS-485).

## 6. Lo revisado y correcto

- **Compradas, sin equivalente del docente:** ESP32-S3-WROOM-1U-N16R8 (la biblioteca solo trae el ESP32-C3-WROOM-02-N4), MAX-M10S, parche GNSS, módulo LoRa y su antena, antena FPC de Würth, conector u.FL, MAX17048, chip CN3791, VO617A, cristal, zumbador, batería y panel.
- **Resistencia de sensado del CN3791 (O22):** la menor resistencia del docente es de 84.5 Ω, demasiado alta.
- **100 nF X7R (O21):** el único 100 nF del docente es Y5V (su descripción dice "Y5V -20%~+80%"); no sirve.
- **Kit de resistencias (O19):** necesario para 3K, 12K, 100 kΩ, 22 Ω, 750 Ω y 470 Ω, que no son piezas sueltas del docente.
- **Tomadas del docente, bien:** USB-C (D2), USBLC6-2SC6 (D3), AP2301 (D5), inductor CY54-47UH (D6), LED RGB RS-3535MWAM (D7), JST PH de 2 pines (D8), resistencias de 0R, 100R, 120R, 330R, 1k, 4.7k y 10k (D9), los condensadores de D10, LED rojo XL-2012SURC (D11) y el electrolítico de 100 µF (D12). Los MPN y las tolerancias coinciden con el inventario.
- **IMU LSM6DS3TR-C:** solo del classroom, así que su librería creada es correcta.
- La aritmética del plan B (2.397 kΩ, 767 Ω, 99.8 kΩ, 398.2 kΩ, 480 Ω, 33.3 Ω y 16.5 pF) está bien.

## 7. Librerías creadas o por crear

| Librería | ¿La tiene el docente? | Veredicto |
|----------|-----------------------|-----------|
| LSM6DS3TR-C, ESP32-S3-WROOM-1U, MAX-M10S, TLV75733PDBVR, MAX17048, VO617A, cristal ECS, conector U.FL | No | Correctas |
| **PTS645 (pulsador)** | **Sí, el TSC015 equivalente** | **Innecesaria.** Se puede borrar `hardware/libraries/pts645/` (incluye un zip de SamacSys que tampoco debe subirse a un repositorio público por su licencia) |
| **Huella ESOP-8 del TP4056** | Sí: símbolo y una huella parecida (`SOIC_8_3.9mm_EP`) | No bajar nada; comprobar la huella existente (sección 3.2) |
| JST SH de 4 pines | No, pero sin uso asignado | No crear hasta que tenga uso |
| CN3791 y módulo LoRa | No | Pendientes, sin archivo de Altium encontrado |

## 8. Datasheets que faltan en `hardware/datasheets/`

Las piezas del docente que el diseño usa y cuyo datasheet **no está** en la carpeta (el README dice que algunas se consultaron en línea): USB-C Shou Han TYPE-C-3.1-16PIN, USBLC6-2SC6, AP2301, CY54-47UH, RS-3535MWAM, XL-2012SURC, el electrolítico HRK de 100 µF, los condensadores (Samsung, CCTC, Chinocera, YAGEO) y las resistencias (FOJAN, Yageo). Además faltan los datasheets de las piezas compradas: zumbador Murata, conector Hirose, antena de Würth, y los de lo que está por elegir (Schottky, protector de batería, parche GNSS, resistencia de sensado). Los PDF no se suben a git (`.gitignore`, regla `*.pdf`), así que solo existen en este equipo.

**Actualizado el 2026-11-07:** ya están en la carpeta los datasheets del USB-C, el USBLC6-2SC6 (Rev 5), el AP2301, el CY54, el RS-3535MWAM, el XL-2012SURC, el electrolítico HRK (hoja de la serie CD110), el LESD5Z3.3T1G y los Chinocera (ver `hardware/datasheets/README.md`). Siguen faltando los de Samsung, CCTC, YAGEO y las resistencias (opcionales) y la Rev 7 del USBLC6.

## 9. Decisiones que necesito del autor (con mi recomendación)

| # | Decisión | Recomendación |
|---|----------|---------------|
| 1 | Pulsador: ¿el del docente (TSC015, 4.5 x 4.5 mm) o el PTS645 de 6x6? | **DECIDIDO (2026-11-05): el del docente.** Documentos actualizados y `libraries/pts645/` borrada |
| 2 | Schottky O20 | **DECIDIDO (2026-11-06):** se usan los PSBD1DF40V1H del docente y no se compra el O20 (`PLAN_MANANA.md`, paso 17) |
| 3 | 220 nF (O21) | **DECIDIDO (2026-11-05):** se quita el lote y se usan 2 x 470 nF X7R del docente en serie (235 nF). Documentos actualizados (BOM v1.12, COMPRAS, pinout v2.7, CSV). Falta confirmar que el docente tenga al menos 3 de 470 nF |
| 4 | Tantalio O25 | Verificar el MPN. Valorar primero el electrolítico del docente como bulk. **Sigue abierto** (paso 26 del plan): el código TAJA107K016RNJ parece no existir |
| 5 | JST SH de 4 pines (O17) | **DECIDIDO (2026-11-05):** se quita del BOM y de las compras (sin uso). Si más adelante hace falta uno de 4 pines, empezar por la cabecera del docente |
| 6 | Diodo ESD de la alimentación y MOSFET del protector | **Revisado el 2026-11-07:** el LESD5Z3.3T1G cumple (VRWM 3.3 V, fuga máxima de 1 µA a 25 °C). El DN2302S ya no hace falta: el protector se retiró el 2026-11-05 |
| 7 | Regulador | **DECIDIDO (2026-11-06):** se queda el TLV75733P; el DS8570F8 consume 0.6 mA (`PLAN_MANANA.md`, paso 18) |
| 8 | Huella del TP4056 | **DECIDIDO (2026-11-06):** sirve la `SOIC_8_3.9mm_EP` con el pad 9 a GND, y ya está en la biblioteca (`PLAN_MANANA.md`, paso 19) |
| 9 | Limpieza de la sección 4 | **HECHO (2026-11-07)** |
