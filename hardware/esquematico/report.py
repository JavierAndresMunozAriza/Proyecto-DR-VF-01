"""HTML report: sheets as SVG (full view and detail crops), parts, pin tables with their source, nets."""
from html import escape

import checks
import model
import render

DIR_TXT = {"in": "entrada", "out": "salida", "io": "bidireccional"}


def sheet_defs(shs):
    out = ['<svg width="0" height="0" style="position:absolute" aria-hidden="true" focusable="false"><defs>']
    for s in shs:
        W, H = model.SIZES[s.size]
        body = render.render_sheet(s, len(shs))
        out.append('<g id="sheet%d"><rect x="0" y="0" width="%d" height="%d" fill="%s"/>%s</g>' % (s.no, W, H, render.C_PAPER, body))
    out.append("</defs></svg>")
    return "".join(out)


def view(s, region=None, scale=1.0, label=""):
    W, H = model.SIZES[s.size]
    if region:
        x, y, w, h = region
        vb = "%g %g %g %g" % (x, H - (y + h), w, h)
        pw, ph = w, h
    else:
        vb, pw, ph = "0 0 %d %d" % (W, H), W, H
    return ('<svg viewBox="%s" role="img" aria-label="%s" style="aspect-ratio:%g/%g;min-width:%dpx"><use href="#sheet%d"/></svg>'
            % (vb, escape(label), pw, ph, int(pw * scale), s.no))


def part_note(p):
    n = []
    if p.lib in ("Res_1k_0603_1_10W",) and p.value not in ("1k",):
        n.append("copiar la resistencia 0603 y cambiar Value a %s (borrar campos LCSC)" % p.value)
    if p.dnp:
        n.append("sin montar (DNP)")
    if p.lib == "TestPoint":
        n.append(p.note)
    if p.lib == "Cap_100uF_ALU_TH_35V":
        n.append("pin 1 = positivo")
    if p.lib.startswith("D_Schottky"):
        n.append("pin 1 = cátodo, pin 2 = ánodo; cambiar el footprint a SOD_123FL")
    if p.lib.startswith("LED_0805"):
        n.append("pin 1 = ánodo, pin 2 = cátodo")
    if p.lib == "Q_PFET_30V_3A_SOT23":
        n.append("pin 1 = G, 2 = S, 3 = D (ver verificación)")
    if p.value in ("100nF",) and p.lib == "Cap_0.1uF_0603_50V":
        n.append("100 nF X7R (el de la librería es Y5V según BOM.md: comprar X7R)")
    return "; ".join(n)


def sort_key(p):
    pre = p.ref.rstrip("0123456789")
    return (pre, int(p.ref[len(pre):]))


def parts_table(s):
    rows = []
    for p in sorted(model.all_parts(s), key=sort_key):
        rows.append("<tr><td><b>%s</b></td><td><code>%s</code></td><td>%s</td><td>%s</td></tr>" % (
            p.ref, escape(p.lib), escape(p.value), escape(part_note(p))))
    return ('<div class="tbl"><table><thead><tr><th>Ref.</th><th>Símbolo de shared_lib</th><th>Valor</th><th>Nota</th></tr>'
            '</thead><tbody>%s</tbody></table></div>' % "".join(rows))


def pin_tables(s, sections, csv_rows):
    out = []
    for p in sorted(s.parts, key=sort_key):
        if len(p.sym.pins) <= 2:
            continue
        table = checks.rows_for(p.lib, sections, csv_rows)
        rows = []
        for pin in sorted(p.sym.pins, key=lambda q: (len(q["des"]), q["des"])):
            disp = p.pins[pin["des"]]
            net = "sin conectar" if isinstance(disp, model.NC) else (disp.net if isinstance(disp, model.Far) else disp)
            key = pin["des"]
            subs = key.split("/") if p.lib == "USB_TYPEC_16P" else [key]
            srcs = []
            for sub in subs:
                k = sub if sub in table else ("41" if sub.startswith("41_") and "41" in table else sub)
                for text, red, origin, src in table.get(k, []):
                    if origin == "pinout.md" and src and src not in srcs:
                        srcs.append(src)
            rows.append("<tr><td>%s</td><td>%s</td><td><b>%s</b></td><td>%s</td></tr>" % (
                escape(pin["des"]), escape(pin["name"].replace("\\", "")), escape(net), escape("; ".join(srcs) or "—")))
        out.append('<h4>%s %s</h4><div class="tbl"><table><thead><tr><th>Pin</th><th>Nombre</th><th>Red</th><th>Fuente en pinout.md</th>'
                   '</tr></thead><tbody>%s</tbody></table></div>' % (p.ref, escape(p.value or p.lib), "".join(rows)))
    return "".join(out)


def net_table(s, nl):
    refs = {p.ref for p in model.all_parts(s)}
    rows = []
    for net, pins in sorted(nl.nets.items(), key=lambda kv: kv[0].lstrip("~")):
        mine = [(r, pn) for r, pn in pins if r in refs]
        if not mine:
            continue
        kind = "alimentación" if net in model.POWER else ("puerto (%s)" % DIR_TXT[s.ports[net]] if net in s.ports else
                                                           ("interna" if net.startswith("~") else "local"))
        name = "(interna)" if net.startswith("~") else net
        rows.append("<tr><td><b>%s</b></td><td>%s</td><td>%s</td></tr>" % (
            escape(name), kind, escape("  ".join("%s.%s" % rp for rp in mine))))
    return ('<div class="tbl"><table><thead><tr><th>Red</th><th>Tipo</th><th>Pines en esta hoja</th></tr></thead>'
            '<tbody>%s</tbody></table></div>' % "".join(rows))


def ports_table(shs):
    nets = {}
    for s in shs:
        for n, d in s.ports.items():
            nets.setdefault(n, []).append((s.no, d))
    rows = []
    for n, lst in sorted(nets.items()):
        rows.append("<tr><td><b>%s</b></td><td>%s</td></tr>" % (
            escape(n), escape(", ".join("hoja %d (%s)" % (no, DIR_TXT[d]) for no, d in lst))))
    return ('<div class="tbl"><table><thead><tr><th>Red</th><th>Hojas y sentido visto desde cada hoja</th></tr></thead>'
            '<tbody>%s</tbody></table></div>' % "".join(rows))


CSS = """
:root{--bg:#f2f5f5;--surface:#fff;--fg:#14201f;--muted:#56686a;--line:#c9d4d3;--accent:#7a1f1f;--ok:#146c2e;--warn-bg:#fff4e5;--warn-line:#e0a35a;
--sans:"IBM Plex Sans",system-ui,-apple-system,"Segoe UI",sans-serif;--mono:"IBM Plex Mono",ui-monospace,Menlo,Consolas,monospace}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]){--bg:#0f1514;--surface:#172120;--fg:#e5eeec;--muted:#95a8a5;--line:#2b3b39;--accent:#e08a7a;--ok:#5fd38a;--warn-bg:#2a2010;--warn-line:#8a6420;color-scheme:dark}}
:root[data-theme="dark"]{--bg:#0f1514;--surface:#172120;--fg:#e5eeec;--muted:#95a8a5;--line:#2b3b39;--accent:#e08a7a;--ok:#5fd38a;--warn-bg:#2a2010;--warn-line:#8a6420;color-scheme:dark}
body{background:var(--bg);color:var(--fg);font:15px/1.55 var(--sans);padding-block:28px 60px;padding-left:max(16px,env(safe-area-inset-left,0px));padding-right:max(16px,env(safe-area-inset-right,0px))}
main{max-width:1180px;margin:0 auto;display:flex;flex-direction:column;gap:30px}
h1{font-size:27px;line-height:1.2;margin:0 0 6px;text-wrap:balance}h2{font-size:20px;margin:0;text-wrap:balance}h3{font-size:16px;margin:18px 0 6px}h4{font-size:14px;margin:16px 0 6px}
p{margin:0;max-width:75ch}.sub{color:var(--muted)}code{font-family:var(--mono);font-size:12.5px}
nav{display:flex;flex-wrap:wrap;gap:8px}nav a{border:1px solid var(--line);background:var(--surface);color:var(--fg);padding:6px 12px;border-radius:5px;text-decoration:none;font-size:14px}
nav a:focus-visible,summary:focus-visible{outline:2px solid var(--accent);outline-offset:2px}
.card{background:var(--surface);border:1px solid var(--line);border-radius:6px;padding:14px 18px;display:flex;flex-direction:column;gap:8px}
.warn{background:var(--warn-bg);border:1px solid var(--warn-line);border-radius:6px;padding:12px 16px}
ul,ol{margin:0;padding-left:20px;display:flex;flex-direction:column;gap:5px}li{max-width:85ch}
.sheet{display:flex;flex-direction:column;gap:12px;scroll-margin-top:16px}
.scroll{overflow-x:auto;border:1px solid var(--line);border-radius:6px;background:#fbfbea}
.scroll svg{display:block;width:100%;height:auto}
.crops{display:flex;flex-direction:column;gap:14px}.crop h4{margin:0 0 4px;font-size:13.5px;color:var(--muted)}
.tbl{overflow-x:auto;border:1px solid var(--line);border-radius:6px;background:var(--surface)}
table{border-collapse:collapse;width:100%;font-size:13px}th,td{text-align:left;padding:5px 10px;border-bottom:1px solid var(--line);vertical-align:top}
th{background:color-mix(in srgb,var(--line) 40%,transparent);font-weight:600;white-space:nowrap}tr:last-child td{border-bottom:0}
details{border:1px solid var(--line);border-radius:6px;background:var(--surface);padding:8px 14px}summary{cursor:pointer;font-weight:600}
details[open] summary{margin-bottom:8px}.ok{color:var(--ok);font-weight:600}
@media (max-width:640px){body{font-size:14px}}
"""


def build(shs, nl, summary_html, verifier_html, sections, csv_rows):
    nav = "".join('<a href="#h%d">%d. %s</a>' % (s.no, s.no, escape(s.title)) for s in shs)
    body = []
    for s in shs:
        crops = "".join('<div class="crop"><h4>%s</h4><div class="scroll">%s</div></div>' % (
            escape(t), view(s, (x, y, w, h), 1.6, t)) for t, x, y, w, h in s.regions)
        notes = "".join("<li>%s</li>" % escape(t[2]) for t in s.texts)
        body.append(
            '<section class="sheet" id="h%d"><h2>Hoja %d: %s <span class="sub">(%s)</span></h2>'
            '<div class="scroll">%s</div>'
            '<h3>Detalles ampliados</h3><div class="crops">%s</div>%s'
            '<details><summary>Lista de piezas</summary>%s</details>'
            '<details><summary>Tablas de pines de los circuitos integrados</summary>%s</details>'
            '<details><summary>Lista de redes de la hoja</summary>%s</details></section>' % (
                s.no, s.no, escape(s.title), s.size, view(s, None, 0.78, "Hoja %d completa" % s.no), crops,
                ('<div class="card"><b>Notas de esta hoja</b><ul>%s</ul></div>' % notes) if notes else "",
                parts_table(s), pin_tables(s, sections, csv_rows), net_table(s, nl)))
    return """<title>Hojas del esquemático DR-VF-01</title>
<meta name="claude:viewport-fit-sides" content="cover">
<link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;600&family=IBM+Plex+Sans:wght@400;600&display=swap">
<style>%s</style>
%s
<main>
<header><h1>Esquemático DR-VF-01, hoja por hoja</h1>
<p class="sub">Cinco hojas con el aspecto de Altium, sacadas de <code>hardware/pinout.md</code> y de los pines reales de <code>shared_lib</code>. Son una guía para imitar: no son el archivo de Altium.</p></header>
<nav aria-label="Hojas">%s</nav>
<section class="card"><h2>Antes de empezar</h2><ol>
<li>Una hoja por bloque, dentro del mismo proyecto <code>DR-VF-01.PrjPcb</code>: <code>01</code> la que ya tienes (<code>Esquematico.SchDoc</code>), y añade <code>02</code>, <code>03</code>, <code>04</code> y <code>05</code> con <code>File &gt; New &gt; Schematic</code>. Hojas A3 para la 01, 02 y 03, A4 para la 04 y 05.</li>
<li><code>Project &gt; Project Options &gt; Options &gt; Net Identifier Scope = Global</code>. Así una etiqueta o puerto con el mismo nombre es la misma red en todas las hojas, y las etiquetas que ya pusiste (<code>USB_DP</code>, <code>USB_DM</code>) siguen valiendo.</li>
<li><b>Designadores:</b> escríbelos como en la imagen (R1, C3...). Así la lista de redes que saco de tu archivo se compara pieza por pieza.</li>
<li><b>Estilo de dibujo:</b> cada pin sale a una etiqueta de red (texto rojo) o a un puerto de alimentación. No hace falta tirar cables largos: dos etiquetas con el mismo nombre son la misma red.</li>
<li>Rojo con cruz = <em>No ERC</em> (pin sin conectar). Los puertos con forma de flecha unen hojas: pon su tipo E/S en <code>Unspecified</code> para que el ERC no marque falsos conflictos; la flecha del dibujo solo indica el sentido.</li>
<li>Los valores que no están en la librería (5.1k, 1.8k, 22, 0.47, 3k, 100k) se arman copiando una resistencia 0603 y cambiando <em>Value</em>, <em>Comment</em>, <em>Manufacturer Part Number</em>, <em>Manufacturer</em> y <em>ComponentDescription</em>, y borrando los campos LCSC. Siempre en el esquemático, nunca en la PCB.</li></ol></section>
<section class="warn"><p><b>Cambios del 2026-11-09 tras las auditorías MCU y POWER</b> (versión 5 de estas hojas). <b>Hoja 01:</b> J2 pasa a la red <code>BATT_J2</code> y entre J2 y VBAT va un AP2301 (<code>Q2</code>: drenador a J2, fuente a VBAT, puerta a GND); un N-FET <code>Q3</code> (drenador a MPPT, fuente a GND, puerta a <code>CHG_INHIBIT</code> con <code>R32</code> de 100 kΩ a GND); R11 pasa a 100 kΩ; STDBY y CHRG del TP4056 a GND; C1 de 10 µF X7R; <code>C31</code> y <code>C32</code> sin montar en D+ y D-; el TLV75733P sale a <code>3V3_LDO</code> y un 0 Ω (<code>R33</code>) lo une a 3V3; pad <code>TP8</code> en VBAT. <b>Hoja 02:</b> IO21 = <code>CHG_INHIBIT</code> (antes GNSS_EXTINT) e IO48 = <code>GNSS_RESET</code> (antes sin conectar). <b>Hoja 03:</b> EXTINT del GNSS sin conectar; RESET_N por <code>R34</code> (0 Ω); puentes <code>R35</code> y <code>R36</code> (0 Ω) hacia <code>3V3_GNSS</code> y <code>3V3_LORA</code>; <code>R37</code> pull-up de NSS; <code>R38</code> y <code>R39</code> pull-downs de RXEN y TXEN; <code>R40</code> y <code>C33</code> en SCK; pads <code>TP9</code> y <code>TP10</code> en el UART del GNSS. <b>Hoja 04:</b> pull-ups de I2C de 3.3 kΩ y pad <code>TP11</code>. Las piezas que ya habías dibujado conservan su designador; las nuevas llevan números altos (R32 a R40, C31 a C33, Q2, Q3, TP8 a TP11).</p></section>
<section class="warn"><p><b>Cambio en tu hoja 01 ya dibujada:</b> R5 y R6 del PROG del TP4056 ya no son 3k y 12k en paralelo. Ahora van <b>en serie</b>: R5 = 1.8k y R6 = 100 (1.9 kΩ, unos 500 mA con el chip de Yongyutai). Edita sus valores y quita el cable que las ponía en paralelo.</p><p style="margin-top:8px"><b>Pendiente en tu librería:</b> en <code>shared_lib.PcbLib</code>, el footprint del ESP32-S3 tiene 12 vías térmicas (<code>41_10</code> a <code>41_21</code>) que el símbolo no tiene: ponles <b>Designator = <code>41_1</code></b> o quedan sin red, flotando. El footprint de <code>D_Schottky_40V_1A_SOD_123F</code> ya apunta a <code>SOD_123FL</code>. <b>Decididos el 2026-11-08 y sin dibujar en esta versión:</b> fusible PTC en el USB-C, diodo ESD LESD5Z3.3T1G en el riel de 3.3 V y supervisor de tensión en EN (se revisan tras medir la primera placa); el LED de carga del TP4056 sigue opcional. Antena del LoRa (pad ANT o IPEX) sin confirmar con el vendedor.</p></section>
<section class="card"><h2>Redes que cruzan entre hojas</h2>%s<p class="sub">Alimentación global (potencia): GND, 3V3, VBAT, VBUS y VPANEL, con símbolo de alimentación en todas las hojas. En la hoja 03, <code>3V3_GNSS</code> y <code>3V3_LORA</code> también son <b>Power Port</b> (están detrás de un puente de 0 Ω): sin un Power Port, el ERC marca los pines de alimentación del GNSS y del LoRa como "sin fuente".</p></section>
%s
%s
%s
</main>""" % (CSS, sheet_defs(shs), nav, ports_table(shs), "\n".join(body), summary_html, verifier_html)
