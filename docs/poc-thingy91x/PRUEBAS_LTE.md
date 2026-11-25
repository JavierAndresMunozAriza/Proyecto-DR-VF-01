# Pruebas de campo LTE-M: geocerca desde la nube (Thingy:91 X)

Estado: **plan de pruebas, nada medido todavia** (2026-11-25). Cada cifra de
este documento marcada como "propuesta" es una decision por tomar, no un
resultado.

## Para que sirven

Las pruebas con el Thingy:91 X y su SIM contestan las preguntas que fijan el
diseno de la placa completa (Fase 2: ESP32-S3 + modem celular), que se manda a
fabricar a **finales de mayo de 2027**:

| Pregunta | Decide |
|---|---|
| Hay LTE-M, NB-IoT o solo 4G Cat-1 en la finca? | Modem: BG95-M3 (LTE-M/NB-IoT + respaldo 2G) o A7670SA (Cat-1) |
| Que tan fuerte es la senal (RSRP) en el potrero? | Antena celular, su ubicacion y la huella del conector U.FL |
| Cuanto tarda el collar en recibir una cerca nueva? | Intervalo de consulta y modo de ahorro (PSM, eDRX) |
| Cuanto consume en PSM, con GNSS y con la radio? | Tamano de la bateria y del panel solar (`hardware/POWER_BUDGET.md`) |
| Cuantos datos y mensajes por dia usa? | Plan de datos de la SIM y plan de nRF Cloud |
| Conviven bien el GNSS y el LTE? | Si el GNSS tarda mas en dar un fix con el LTE activo |

## Preparacion

1. **SIM.** El Thingy:91 X trae una SIM de Onomondo o de Wireless Logic con datos
   gratis (fuente: documentacion de Nordic). **Sin verificar:** cuantos datos
   incluye, hasta cuando, y si cubre Colombia. Mira el mapa de cobertura del
   proveedor y anota el resultado aqui antes de salir.
2. **Cuenta de nRF Cloud.** Crea la cuenta, agrega el dispositivo con el
   *attestation token* que el firmware escribe en el log la primera vez
   (`/dev/ttyACM0`, 115200 baudios), y guarda la clave REST y el ID del
   dispositivo como variables de entorno del portal (nunca en el repositorio).
3. **Firmware.** Desde `firmware/`: `./build.sh cloud` y `./build.sh flash-cloud`.
   El log de la aplicacion sigue en `/dev/ttyACM0`. Antes de la campana,
   en casa: (a) reclama y aprovisiona el dispositivo una vez (el primer
   aprovisionamiento apaga la radio y obliga a reiniciar el GNSS; ese camino
   nunca corrio en la placa); (b) carga una vez `./build.sh` + `./build.sh flash`
   (la aplicacion sin nube, ya dividida en modulos) y repite el demo del boton
   y la prueba al aire libre, porque esa version no se ha probado en la placa.
4. **Portal.** `tools/fence_portal/server.py` (ver su `README.md`).
5. **Medidor de corriente** (por ejemplo un PPK2): sin el, las pruebas de
   energia quedan **pendientes**; no se estima.

## Pruebas

Anota cada resultado en una copia de la tabla, con fecha, lugar (sin
coordenadas en el repositorio) y version del firmware (`git rev-parse --short
HEAD`). Los registros con ubicacion real van en
`reports/engineering_report_poc/evidence/private/`, que git ignora.

| ID | Que se mide | Como | Resultado |
|---|---|---|---|
| L1 | Cobertura: operador, tecnologia (LTE-M o NB-IoT), banda y RSRP en 5 o mas puntos del potrero | Linea `Link: ...` del log, que sale en cada consulta (cada 120 s); espera dos lineas por punto | pendiente |
| L2 | Tiempo de arranque a conectado, primera vez (con aprovisionamiento) y despues | Marcas de tiempo del log | pendiente |
| L3 | Latencia de la cerca: del boton "Enviar" a "New fence applied" | Hora del portal y del log, con consulta cada 120 s | pendiente |
| L4 | La cerca sobrevive a un reinicio y a un corte de energia | Reiniciar, y desconectar la bateria mientras llega una cerca; revisar `Stored fence loaded` | pendiente |
| L5 | El collar rechaza una cerca invalida enviada por fuera del portal | `PATCH` manual con un blob con CRC malo; revisar el log y `reported.fence_status` | pendiente |
| L6 | Telemetria al cruzar el borde: llega un mensaje por cruce, no mas de uno cada 30 s | Caminar por la linea de la cerca con el portal abierto | pendiente |
| L7 | Mensajes y bytes por dia | Panel de nRF Cloud y estadisticas del modem (comando por verificar) | pendiente |
| L8 | Tiempo hasta el primer fix GNSS con el LTE conectado, frente al de solo GNSS (de `Engineering Report` rev. 1.0) | `./build.sh flash-cloud` frente a `./build.sh flash`, mismo lugar | pendiente |
| L9 | Corriente media en reposo (PSM) y durante una consulta | PPK2, solo si hay medidor | pendiente |

## De los resultados a la placa

Umbrales **propuestos**, para que el usuario los acepte o cambie antes de la
campana:

| Resultado | Decision de hardware |
|---|---|
| LTE-M con RSRP de -110 dBm o mejor en 90 % o mas de los puntos (L1) | BG95-M3 |
| Solo 4G Cat-1 (sin LTE-M ni NB-IoT) | A7670SA, y revisar el presupuesto de energia: no entra en PSM tan profundo |
| RSRP peor que -110 dBm en mas de 10 % de los puntos | Antena externa de mejor ganancia o ubicacion distinta del collar |
| Latencia de cerca (L3) mayor que el intervalo de consulta mas 30 s | Revisar el firmware antes de elegir el intervalo del producto |
| Corriente media en PSM (L9) que no cumple RNF-01 (7 dias en oscuridad con 1200 mAh) | Mas bateria o panel; actualizar `POWER_BUDGET.md` |

Los resultados se llevan a `docs/design-reviews/` y al presupuesto de energia
antes de tocar el esquematico de la revision 2 (marzo y abril de 2027).

## Limitaciones conocidas de la prueba

- El Thingy:91 X usa el modem nRF9151 y nRF Cloud. La placa final usara otro
  modem y probablemente otro servicio en la nube. El **formato de la cerca** y
  el de la telemetria no cambian; el adaptador de la nube si (`cloud.h`).
- En PSM el collar solo se entera de una cerca nueva cuando consulta: la nube
  no puede empujarla.
- Los mosaicos del mapa del portal necesitan internet.
