# Evidencia del Engineering Report de la PoC

Registros crudos de los que salen las cifras del informe. Cada cifra del informe
debe poder rastrearse a uno de estos archivos.

## Cómo se capturó

- Consola de la nRF9151 en la Thingy:91 X por USB (`/dev/ttyACM0`, 115200 baudios), leída desde WSL2.
- Se quitaron los códigos de color ANSI y los retornos de carro. Solo se conservan las líneas
  del log (que empiezan con `[`) y los mensajes de arranque.
- Las marcas de tiempo (`[hh:mm:ss.mmm,uuu]`) son el tiempo desde el último reinicio de la placa,
  no la hora del día.

## Archivos

| Archivo | Qué contiene | Firmware (inferido de la cronología de la sesión) |
|---------|--------------|---------------------------------------------------|
| `step1_simulated_route.log` | Paso 1: ruta simulada sobre el polígono de prueba, con el tiempo de cada `geofence_check()` en microsegundos | Paso 1, anterior al GNSS real |
| `gnss_search_indoor_90s.log` | Primer firmware con GNSS real, 90 s dentro de una casa: 0 a 1 satélites, sin fix. Incluye la versión del firmware del módem | Primer firmware con GNSS (alrededor del commit `41951fd`) |
| `search_indoor_4min.log` | Cuatro minutos de búsqueda dentro de la casa tras añadir el modo demo. Sin desglose por sistema satelital | Commit `6370364` |
| `demo_mode_session.log` | Sesión del modo demo: `SEARCHING -> DEMO`, 42 pasos de la ruta simulada y `DEMO -> SEARCHING` | Commit `6370364` |
| `satellites_by_system_indoor.log` | Satélites rastreados con desglose GPS, QZSS y Galileo, 25 s dentro de la casa | Commit `0b19c71` |
| `geofence_benchmark.log` | Benchmark de `geofence_check()` con polígonos regulares de 4 a 1024 vértices, 3 arranques consecutivos, y el ajuste lineal de la media (calculado aparte con Python) | Compilación `./build.sh bench`, fuentes del commit `31e6787` |
| `host_tests.log` | Salida de `make test/stress/sanitize/cross-check` del motor y de los conjuntos de tests de `alert`, `fence`, `app` y `demo` | Commit `0b19c71` |
| `build_memory.log` | Uso de flash y RAM del build desde cero | Commit `0b19c71` |

Los commits de los registros de la placa se infieren de la hora en que se flasheó cada
compilación durante la sesión de trabajo; no se guardó el hash junto a cada captura.

## Limitaciones de la evidencia (el informe debe declararlas)

- **No existe un log de un fix GNSS real.** Las pruebas en exteriores se hicieron con la placa
  desconectada, sin cable, y solo se observó el LED a simple vista. Todos los logs con GNSS
  son de interiores y no tienen fix.
- Las coordenadas de `demo_mode_session.log` son las del centro **ficticio** del demo
  (4.6097, -74.0817), no una posición real.
- Los tiempos por llamada de `step1_simulated_route.log` y de los demás logs de la placa son
  gruesos: el contador del kernel (`k_cycle_get_32`) corre a 32 768 Hz, y cada lectura es un
  múltiplo de 30.5 µs (214 µs son 7 ticks y 275 µs son 9). Las cifras válidas de tiempo son las
  medias por lote de `geofence_benchmark.log`. Tampoco son una medición con osciloscopio ni
  del ESP32-S3.
- Todo corresponde a una nRF9151 en una Thingy:91 X, no al ESP32-S3 ni al MAX-M10S.
