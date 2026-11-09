"""Automatic checks of the sheet declarations.

1. Structure: every IC pin covered, no single-pin nets, nets shared between sheets are ports or power.
2. Electrical sense: power pins on power nets, no two output pins on one net.
3. Source check: every IC pin is compared with the table row of hardware/pinout.md (and the same rule is
   applied to hardware/pin_connections.csv). A mismatch is an error, an unreadable row is listed for review.
"""
import csv
import os
import re

import model

HERE = os.path.dirname(os.path.abspath(__file__))
PINOUT = os.path.join(HERE, "..", "pinout.md")
CSVF = os.path.join(HERE, "..", "pin_connections.csv")

SECTIONS = {
    "MAX-M10S-00B": "GNSS u-blox MAX-M10S", "LSM6DS3TR-C": "IMU LSM6DS3TR-C",
    "DX-LR30-900M22S": "Módulo LoRa DX-LR30", "TLV75733PDBVR": "Regulador TLV75733PDBVR",
    "MAX17048G+T10": "Medidor de batería MAX17048", "IC_BAT_CHG_4.2V_1A": "Cargador USB-C TP4056",
    "CN3791": "Cargador solar CN3791", "VO617A-1X017T": "Optoacoplador VO617A",
    "LED_RGB_3535_SMD": "LED RGB", "Sw_PUSH_4.5x4.5x5.0xmm": "Botón de usuario",
    "ECS-.327-12.5-34B-TR": "Cristal de 32.768", "PS1240P02BT": "Zumbador",
    "USB_TYPEC_16P": "USB-C (Shou Han", "TVS_5.25V_2CH_SOT_23_6": "USB-C (Shou Han",
    "ESP32-S3-WROOM-1U": "Conexiones del módulo para el esquemático",
}
CSV_COMPONENT = {
    "MAX-M10S-00B": "GNSS u-blox MAX-M10S", "LSM6DS3TR-C": "IMU LSM6DS3TR-C",
    "DX-LR30-900M22S": "Módulo LoRa DX-LR30-900M22S", "TLV75733PDBVR": "Regulador TLV75733PDBVR",
    "MAX17048G+T10": "Medidor de batería MAX17048", "IC_BAT_CHG_4.2V_1A": "Cargador USB-C TP4056",
    "CN3791": "Cargador solar CN3791 y su etapa de potencia", "VO617A-1X017T": "Optoacoplador VO617A",
    "LED_RGB_3535_SMD": "LED RGB NationStar RS-3535MWAM", "Sw_PUSH_4.5x4.5x5.0xmm": "Botón de usuario BZCN TSC015A05026A",
    "ECS-.327-12.5-34B-TR": "Cristal de 32.768 kHz ECS-.327-12.5-34B-TR", "PS1240P02BT": "Zumbador Murata PKMCS0909E4000-R1",
    "USB_TYPEC_16P": "USB-C TYPE-C-3.1-16PIN", "TVS_5.25V_2CH_SOT_23_6": "USBLC6-2SC6",
    "ESP32-S3-WROOM-1U": "ESP32-S3-WROOM-1U-N16R8",
}
PIN_ALIAS = {"IC_BAT_CHG_4.2V_1A": {"Pad": "9"}, "MAX17048G+T10": {"EP": "9"}, "USB_TYPEC_16P": {"Carcasa": "0"}}
# nets that name a point inside a branch: the keyword the source row must contain
KEY = {
    "VG": ["100 nF"], "COM": ["120"], "MPPT": ["Divisor"], "CSP": ["inductor"], "DRV": ["Gate de M1"],
    "PROG": ["1.9"], "OPTO_A": ["GPIO1"], "OPTO_C": ["LED rojo"], "LED_R_K": ["GPIO38"], "LED_G_K": ["GPIO39"],
    "LED_B_K": ["GPIO40"], "BTN_N": ["Nodo N"], "BUZZ_A": ["GPIO2"], "XTAL_32K_P": ["GPIO15"],
    "XTAL_32K_N": ["GPIO16"], "LORA_RXEN_P": ["GPIO42"], "LORA_TXEN_P": ["GPIO47"], "LORA_DIO2": ["0 Ω"],
    "GNSS_RF": ["50 Ω"], "LORA_ANT": ["50 Ω"], "GNSS_SAFEBOOT": ["pad de prueba"],
}
POWER_KEY = {"GND": ["GND", "Pad térmico"], "3V3": ["3V3", "VDDIO"], "VBAT": ["batería", "Nodo de batería", "Unido a IN", "BAT"],
             "VBUS": ["VBUS", "VCC"], "VPANEL": ["panel"], "3V3_GNSS": ["3V3"], "3V3_LORA": ["3V3"]}
# USB tables are keyed by signal name in the "Red" column
USB_RED = {"USBC_DP": "D+", "USBC_DN": "D-", "CC1": "CC1", "CC2": "CC2", "VBUS": "VBUS", "GND": "GND"}


# ------------------------------------------------------------------ parsing
def parse_pinout():
    """{section title: [table]} with table = (header cells, [row cells])."""
    out, cur, table = {}, None, None
    for line in open(PINOUT, encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("#"):
            cur = line.lstrip("# ").strip()
            out.setdefault(cur, [])
            table = None
        elif line.startswith("|") and cur is not None:
            cells = [c.strip() for c in line.strip().strip("|").split("|")]
            if all(set(c) <= set("-: ") for c in cells):
                continue
            if table is None:
                table = (cells, [])
                out[cur].append(table)
            else:
                table[1].append(cells)
        else:
            table = None
    return out


def gpio_map(sections):
    m = {}
    for title, tables in sections.items():
        if title.startswith("Mapa por GPIO"):
            for _, rows in tables:
                for r in rows:
                    sig = r[2].replace("*", "").strip()
                    g = re.match(r"^(\d+)$", r[0])
                    if g:
                        m[sig] = "GPIO" + g.group(1)
    return m


def expand_pins(cell, lib):
    cell = re.sub(r"\(.*?\)", "", cell).strip()
    out = []
    for part in [p.strip() for p in cell.split(",") if p.strip()]:
        r = re.match(r"^(\d+)-(\d+)$", part)
        if r:
            out += [str(i) for i in range(int(r.group(1)), int(r.group(2)) + 1)]
        else:
            out.append(PIN_ALIAS.get(lib, {}).get(part, part.split()[0] if part else part))
    return out


def rows_for(lib, sections, csv_rows):
    """pin -> list of (connection text, red column, origin)."""
    out = {}
    title = SECTIONS.get(lib)
    if title:
        for st, tables in sections.items():
            if title not in st:
                continue
            for header, rows in tables:
                want_usb = lib == "TVS_5.25V_2CH_SOT_23_6"
                if lib in ("USB_TYPEC_16P", "TVS_5.25V_2CH_SOT_23_6") and want_usb != header[0].startswith("Pin del USBLC6"):
                    continue
                if lib == "CN3791" and header[0] != "Pin":
                    continue
                if "Conexión" not in header and lib != "ESP32-S3-WROOM-1U":
                    continue
                ci = header.index("Conexión") if "Conexión" in header else None
                ri = header.index("Red") if "Red" in header else None
                si = header.index("Fuente") if "Fuente" in header else None
                for r in rows:
                    for pin in expand_pins(r[0], lib):
                        out.setdefault(pin, []).append((r[ci] if ci is not None else "", r[ri] if ri is not None else "", "pinout.md",
                                                        r[si] if si is not None else ""))
    comp = CSV_COMPONENT.get(lib)
    if comp:
        for r in csv_rows:
            if r["Component"].startswith(comp):
                for pin in expand_pins(r["Pin"], lib):
                    out.setdefault(pin, []).append((r["Connection"], "", "pin_connections.csv", r.get("Source", "")))
    return out


# ------------------------------------------------------------------ rules
def text_ok(net, text, red, lib, pin, gmap):
    """True/False, or None when the row cannot be judged automatically."""
    t = text
    if lib == "ESP32-S3-WROOM-1U":
        alias = {"USB_DM": "USB_D-", "USB_DP": "USB_D+", "UART0_TXD": "UART0_TX"}
        want = alias.get(net, net)
        red = red.strip() or t.split(":", 1)[0].strip()      # CSV rows read "NET: description"
        if net == "NC":
            return red in ("-", "RESERVA_F2") or "Sin conectar" in t
        return red == want
    if lib in ("USB_TYPEC_16P", "TVS_5.25V_2CH_SOT_23_6") and red:
        if net == "NC":
            return "SBU" in red and "Sin conectar" in t
        if lib == "TVS_5.25V_2CH_SOT_23_6":
            return None
        return red.strip() in (USB_RED.get(net, "?"),) or (net == "GND" and red.strip() == "GND")
    if net == "NC":
        return "Sin conectar" in t or "sin conectar" in t[:40] or "no tiene conexión interna" in t
    if net in model.POWER:
        return any(k in t for k in POWER_KEY[net])
    if net in KEY:
        return any(k in t for k in KEY[net])
    g = gmap.get(net)
    if g:
        return g in t or net in t
    if net in ("USBC_DP", "USBC_DN", "USBLC_DP", "USBLC_DN", "CC1", "CC2", "EN", "BOOT", "UART0_RX", "UART0_TXD", "RESERVA_F2"):
        return None
    return None


def check_sources(sheets, nl):
    sections = parse_pinout()
    gmap = gpio_map(sections)
    gmap.update({"BUTTON": "GPIO7", "CHRG_STAT": "GPIO41"})
    csv_rows = list(csv.DictReader(open(CSVF, encoding="utf-8")))
    ok = bad = unk = missing = 0
    lines = []
    for s in sheets:
        for p in s.parts:
            if p.lib not in SECTIONS or len(p.sym.pins) <= 1:
                continue
            table = rows_for(p.lib, sections, csv_rows)
            for pin, disp in sorted(p.pins.items(), key=lambda kv: (len(kv[0]), kv[0])):
                net = "NC" if isinstance(disp, model.NC) else (disp.net if isinstance(disp, model.Far) else disp)
                subs = pin.split("/") if p.lib == "USB_TYPEC_16P" else [pin]
                for sub in subs:
                    key = sub if sub in table else ("41" if sub.startswith("41_") and "41" in table else sub)
                    cands = table.get(key)
                    if not cands:
                        missing += 1
                        lines.append("SIN FILA  %s %s pin %s -> %s" % (p.ref, p.lib, pin, net))
                        continue
                    for text, red, origin, _src in cands:
                        r = text_ok(net, text, red, p.lib, sub, gmap)
                        if r is True:
                            ok += 1
                        elif r is False:
                            bad += 1
                            lines.append("DIFIERE   %s %s pin %s: spec %s | %s dice: %s %s" % (
                                p.ref, p.lib, pin, net, origin, red, text[:80]))
                        else:
                            unk += 1
                            lines.append("REVISAR   %s %s pin %s -> %s | %s: %s" % (p.ref, p.lib, pin, net, origin, (red + " " + text)[:70]))
    return ok, bad, unk, missing, lines


# ------------------------------------------------------------------ structure and electrical sense
def check_structure(sheets, nl):
    errs = []
    sheet_of_net = {}
    ref_sheet = {}
    for s in sheets:
        for p in model.all_parts(s):
            ref_sheet[p.ref] = s
    for net, pins in nl.nets.items():
        shs = {ref_sheet[r].no for r, _ in pins}
        sheet_of_net[net] = shs
        if net.startswith("~"):
            if len(pins) < 2:
                errs.append("red interna con un solo pin: %s %s" % (net, pins))
            continue
        if net in model.POWER:
            continue
        port_sheets = {s.no for s in sheets if net in s.ports}
        if len(shs) > 1 and not shs <= port_sheets:
            errs.append("red %s aparece en las hojas %s y no es puerto en todas (puertos en %s)" % (net, sorted(shs), sorted(port_sheets)))
        if len(shs) == 1 and net in {n for s in sheets for n in s.ports}:
            if len(pins) < 1 or len(port_sheets) < 2:
                errs.append("puerto %s sin pareja en otra hoja" % net)
        count = len(pins) + (len(port_sheets) if port_sheets else 0)
        if len(pins) == 1 and not port_sheets:
            errs.append("red con un solo pin: %s %s" % (net, pins))
    # port nets must exist on at least two sheets
    all_ports = {}
    for s in sheets:
        for n, d in s.ports.items():
            all_ports.setdefault(n, []).append((s.no, d))
    for n, lst in all_ports.items():
        if len(lst) < 2:
            errs.append("puerto %s existe en una sola hoja: %s" % (n, lst))
        used = [no for no, _ in lst if any(ref_sheet[r].no == no for r, _ in nl.nets.get(n, []))]
        if len(used) != len(lst):
            errs.append("puerto %s declarado en hojas %s pero sin pines en %s" % (n, [no for no, _ in lst], sorted(set(no for no, _ in lst) - set(used))))
    # local names unique across the project: a name used on two sheets must be a port or power (done above)
    return errs, sheet_of_net


def check_electrical(sheets, nl):
    errs = []
    types = {}
    for s in sheets:
        for p in s.parts:
            for pin in p.sym.pins:
                types[(p.ref, pin["des"])] = pin["elec"]
    for net, pins in nl.nets.items():
        outs = [(r, pn) for r, pn in pins if types.get((r, pn)) == "Output"]
        if len(outs) > 1:
            errs.append("varias salidas en %s: %s" % (net, outs))
        for r, pn in pins:
            if types.get((r, pn)) == "Power" and net not in model.POWER and not net.startswith("~") and net not in ("CSP",):
                errs.append("pin de alimentacion %s.%s en red no de alimentacion %s" % (r, pn, net))
    return errs


def check_coverage(sheets):
    errs = []
    for s in sheets:
        for p in s.parts:
            for pin in p.sym.pins:
                if pin["des"] not in p.pins:
                    errs.append("%s %s: pin %s sin disposicion" % (p.ref, p.lib, pin["des"]))
            for k in p.pins:
                try:
                    p.sym.pin(k)
                except KeyError:
                    errs.append("%s %s: el simbolo no tiene el pin %s" % (p.ref, p.lib, k))
    return errs
