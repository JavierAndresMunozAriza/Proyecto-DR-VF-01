# Protocolo de la geocerca (formato y flujo)

Estado: implementado y probado en el PC; **sin probar en la placa ni con una
cuenta de nube** (2026-11-25). Fuente de verdad del formato:
`firmware/include/fence_codec.h`. Copia en Python: `tools/fence_portal/fence_codec.py`.
Los dos codecs se comprueban contra los mismos vectores
(`firmware/src/fence_codec/test_vectors.txt`).

## Por que un formato propio

El mismo blob viaja por cualquier enlace (nube hoy, BLE o serie despues) y lo
leen la Thingy:91 X y la placa con ESP32-S3. Asi la aplicacion, el portal y la
validacion no cambian al cambiar de placa o de servicio en la nube; solo cambia
el adaptador del enlace (`firmware/include/cloud.h`).

## Blob

Enteros en little endian. Para n vertices el tamano es `20 + 4n` bytes (532
con 128 vertices).

| Offset | Tamano | Campo |
|---|---|---|
| 0 | 1 | version del formato (1) |
| 1 | 1 | banderas (0, reservado) |
| 2 | 2 | numero de vertices n |
| 4 | 4 | id de la cerca |
| 8 | 4 | version de la cerca (mayor = mas nueva) |
| 12 | 4 | latitud del primer vertice, int32, 1e-7 grados |
| 16 | 4 | longitud del primer vertice, int32, 1e-7 grados |
| 20 | 4 por cada vertice siguiente | delta de latitud y de longitud, int16 cada uno, en pasos de 1e-6 grados (unos 11 cm) |
| 20 + 4(n-1) | 4 | CRC-32 (IEEE 802.3) de todos los bytes anteriores |

El delta se calcula desde el vertice anterior *ya decodificado*, asi el error
no se acumula: cada vertice queda a menos de 5.5 cm del pedido. Un lado puede
medir hasta 3.6 km. El primer vertice no se repite al final.

## Que acepta el collar

Entre 3 y 128 vertices; latitud hasta 85 grados y longitud dentro de
-180 a 180 (el motor no soporta el antimeridiano); poligono simple (ningun lado
cruza, toca ni se superpone con otro, sin lados de largo cero); area minima de
100 m2; version distinta de 0 (el 0 queda para las cercas que crea el propio
dispositivo). El orden de las comprobaciones esta en `fence_codec_decode()`.

| Codigo | Significado |
|---|---|
| 0 | aplicada |
| -1 a -10 | `fence_codec_status`: argumento, tamano, CRC, formato, numero de vertices, rango, no simple, area, lado largo (solo codificar), buffer |
| -21 | version 0 reservada |
| -22 | fallo del almacen (flash) |
| -23 | el motor de geocerca la rechazo |

Una descarga repetida de la cerca que ya esta activa (mismo id, misma version y
mismo contenido) se informa como aplicada y no cambia nada. No hay regla de
"version mayor": gana el estado que pide la nube, asi un reloj mal puesto en el
portal o una version enorme no bloquean las cercas siguientes.

## Flujo por la nube (nRF Cloud)

```
Portal --PATCH desired.fence (base64 del blob)--> nRF Cloud
Collar cada CONFIG_DRVF_CLOUD_POLL_INTERVAL_S (120 s por defecto):
   shadow get con delta --> hay "fence"? --> decodifica base64
   --> fence_manager_apply: valida, GUARDA en flash, y recien entonces reemplaza
   --> PATCH reported {fence: <el mismo texto>, fence_version, fence_status}
```

- El collar repite en `reported.fence` exactamente el texto que recibio: asi el
  *delta* de la proxima consulta queda vacio y la cerca no se descarga otra vez.
  Tambien lo hace con una cerca que rechazo, para no descargar una mala sin fin;
  `fence_status` dice por que.
- La nube no puede empujar la cerca: el collar la recibe cuando consulta. Con
  PSM el tiempo maximo es el intervalo de consulta.
- El portal pone como version la hora en segundos: solo identifica y ordena
  las cercas a simple vista; el collar no la usa para rechazar.
- Si el flash falla al guardar, el collar no informa nada y vuelve a intentarlo
  en la siguiente consulta. Un valor de `desired.fence` que no se puede
  decodificar (base64 invalido, vacio) se informa con el codigo -4 cuando es
  texto base64 y no cuenta como error de enlace.
- Durante el demo del boton el collar no aplica cercas nuevas; las toma despues.

## Telemetria

El JSON lo arma `telemetry_format_json()` y el adaptador lo envuelve para
nRF Cloud. Un reporte sale solo cuando cambia el estado, dentro o fuera, o la
version de la cerca, mas un latido (`CONFIG_DRVF_TELEMETRY_HEARTBEAT_S`, 900 s),
y nunca con menos de `CONFIG_DRVF_TELEMETRY_MIN_INTERVAL_S` (30 s) entre dos.
Campos: `state`, `fence_id`, `fence_version`, y si existen `inside`, `dist_m`,
`lat`, `lon`, `fix_age_s`, `rsrp_dbm`, `band`, `operator`. En el demo no se
envia nada (las posiciones son simuladas).

## Limites conocidos

- La cuota del plan gratuito de nRF Cloud (cifra de terceros, sin verificar:
  3000 mensajes al mes) obliga a mantener la telemetria por cambio de estado.
- El primer aprovisionamiento apaga la radio y detiene la sesion GNSS; la
  aplicacion la reinicia (`gnss_restart()`), pero ese camino nunca corrio en la
  placa. Conviene reclamar el dispositivo antes de la campana de campo.
- Los reportes de telemetria que esperan en la cola mas que el intervalo del
  latido se descartan al reconectar, para que la nube no los muestre como
  actuales.
- No hay autenticacion de la cerca mas alla de la cuenta de nube: quien puede
  escribir el shadow puede poner una cerca valida.
