# Portal de geocerca (DR-VF-01)

Pagina local para dibujar la cerca de un collar sobre un mapa y enviarla a la
nube. El collar la descarga cuando consulta su shadow y la aplica (ver
`firmware/include/cloud.h` y `docs/poc-thingy91x/PRUEBAS_LTE.md`).

## Uso

```bash
cd tools/fence_portal
export NRFCLOUD_API_KEY=...      # clave REST de nRF Cloud; nunca se sube al repositorio
export NRFCLOUD_DEVICE_ID=...    # ID del dispositivo en nRF Cloud
python3 server.py                # abre http://127.0.0.1:8765
```

Sin las dos variables el portal dibuja y valida, pero no envia. La clave se queda
en el servidor local; el navegador nunca la ve, y el servidor solo escucha en
`127.0.0.1`.

## Que valida

El mismo codec que el collar (`fence_codec.py` es la copia en Python de
`firmware/src/fence_codec/`): entre 3 y 128 vertices, poligono simple (los lados
no se cruzan ni se tocan), area minima de 100 m2, lados de hasta 3.6 km y
latitud hasta 85 grados. Una cerca que el collar rechazaria no se envia.

## Pruebas

```bash
cd tools/fence_portal
python3 -m unittest test_fence_codec test_server
```

`test_fence_codec.py` compara el codec con los vectores
`firmware/src/fence_codec/test_vectors.txt`, los mismos que usa el test en C.

## Sin verificar todavia

- La forma de la respuesta de `GET /v1/devices/{id}` (se lee `state.reported`)
  y la clave de API del portal antiguo de nRF Cloud. Se comprueban con una
  cuenta real.
- La pagina no se ha abierto en un navegador: solo se comprobo la sintaxis del
  JavaScript y que el servidor sirve los archivos.
- Los mosaicos del mapa vienen de openstreetmap.org (necesita internet). El uso
  intensivo no cumple su politica de uso: para el campo, mapas sin conexion.
