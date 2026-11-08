# PS1240P02BT (TDK) - símbolo, huella y modelo 3D

**Estado: REVISADO (2026-11-07), con una cota sin confirmar contra el datasheet de TDK.** Falta copiar símbolo y huella a `shared_lib` y verificar los archivos guardados.

## Fuente

| Archivo | Origen |
|---------|--------|
| `altium_nativo/SnapEDAsch.SchLib` | SnapMagic (SnapEDA), página del PS1240P02BT, formato Altium; descargado por el usuario. |
| `altium_nativo/SnapEDApcb.PcbLib` | Igual; trae el cuerpo 3D incrustado. |

El usuario bajó el modelo dos veces. Se usó la carpeta que trae el `3d_body` (144 KB). La segunda descarga (104 KB) no tiene modelo 3D, y el `.IntLib` tampoco lo trae y pone una expansión de máscara de 0.1016 mm: **no usar ninguna de las dos**.

## Cómo se verificó

Con `hardware/libraries/tools/altium_lib_check.py` (sin abrir Altium; necesita `olefile` en un venv desechable).

### Símbolo

Dos pines, 1 y 2, de tipo Passive; el zumbador pasivo no tiene polaridad. El designador por defecto es `LS`; en el esquemático se cambia a `BZ?`. El enlace de huella no tiene ruta de archivo (limpio).

### Huella `XDCR_PS1240P02BT` (agujero pasante, cotas en mm)

| Cota | Huella | Referencia | Resultado |
|------|--------|-----------|-----------|
| Separación entre pads | 5.0 (pads en x = ±2.5) | 5 ± 0.5 en el dibujo del anuncio de TZT, que copia la hoja del TDK (imagen borrosa) | Coincide; **falta el dibujo del datasheet de TDK** |
| Pad | 1.28 x 1.28, taladro de 0.85 | pata de 0.65 +0.1 / -0.1 en el mismo dibujo | Holgura de unos 0.2 mm; anillo de 0.215 mm |
| Máscara y pasta | modo Rule, expansión 0.0000 | regla del docente | Sin cambios |
| Texto | ninguno | sin texto literal en la serigrafía | Bien |

Diámetro del cuerpo (12.2 mm) y altura (6.5 o 7.8 mm según la fuente): no se comparó el 3D con esas cotas.

## Pendiente

1. Copiar símbolo y huella a `shared_lib.SchLib` y `shared_lib.PcbLib` y guardar; avisar para verificar los archivos guardados.
2. Al recibir el zumbador (lote TZT `PS1240`, `COMPRAS.md` O14): medir la separación entre patas con calibre y la altura, y comparar con la huella.
3. Bajar el dibujo mecánico del datasheet de TDK (desde la página de DigiKey del PS1240P02BT) y cerrar la cota de 5.0 mm.
4. Medir el nivel de sonido a 1 m y revisar SR-30 (`PLAN_MANANA.md`, "Al recibir las piezas").
