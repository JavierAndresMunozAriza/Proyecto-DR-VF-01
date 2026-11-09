# Hojas del esquemático (guía de dibujo)

Genera las cinco hojas del esquemático con aspecto de Altium a partir de las conexiones declaradas en
`sheets.py` y de los pines reales de `shared_lib.SchLib`. No sustituye al archivo de Altium ni al ERC.

```bash
python3 -m venv /tmp/v && /tmp/v/bin/pip install olefile cairosvg
/tmp/v/bin/python hardware/esquematico/build.py /tmp/esq_out
```

- `sheets.py`: una función por hoja. Cada pin de un circuito integrado lleva una red, `NC` o `Far`.
- `checks.py`: cobertura, redes con un solo pin, puertos sin pareja, tipos eléctricos y comparación pin por
  pin con `hardware/pinout.md` y `hardware/pin_connections.csv`.
- `render.py`, `report.py`: SVG/PNG de cada hoja y el informe HTML.
- Para comparar lo que guardaste en Altium: `python3 hardware/libraries/tools/altium_schdoc_nets.py Esquematico.SchDoc`.
