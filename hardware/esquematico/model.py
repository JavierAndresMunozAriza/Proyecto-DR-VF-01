"""Schematic model: parts, nets, sheets and the netlist derived from them.

Every connection is declared once (pin -> net, or part chains between nets). The netlist is
derived from that declaration, so the drawing and the checks cannot disagree.
"""
import symlib

POWER = {"GND": "GND", "3V3": "3V3", "VBAT": "VBAT", "VBUS": "VBUS", "VPANEL": "VPANEL",
         "3V3_GNSS": "3V3_GNSS", "3V3_LORA": "3V3_LORA"}   # the last two sit behind a 0 ohm link: draw them as Power Ports


class NC:
    def __init__(self, note=""):
        self.note = note


class Far:
    """Plain net on an IC pin with a longer stub (avoids clashes between tall symbols on adjacent pins)."""

    def __init__(self, net, ext=30):
        self.net, self.ext = net, ext


class Ser:
    """Series parts on an IC pin, ending on a named net."""

    def __init__(self, parts, net):
        self.parts, self.net = parts, net


class Part:
    def __init__(self, lib, value="", ref=None, pins=None, at=None, rot=0, first=None, note="", dnp=False):
        self.lib, self.value, self.ref, self.pins = lib, value, ref, pins
        self.at, self.rot, self.first, self.note, self.dnp = at, rot, first, note, dnp
        self.sheet = None
        self.sym = None

    def prefix(self):
        return (self.sym.designator["text"] if self.sym and self.sym.designator else "?").rstrip("?")


class Par:
    """Parts in parallel inside a branch."""

    def __init__(self, parts):
        self.parts = parts


class Branch:
    """A chain of parts between two nets. orient 'v': a on top, b below. 'h': a left, b right."""

    def __init__(self, a, parts, b, at, orient="v", title=""):
        self.a, self.parts, self.b, self.at, self.orient, self.title = a, parts, b, at, orient, title


class Tp:
    """Test point on a net. `at` is the point where the net wire meets the test-point pin."""

    def __init__(self, net, at, dir="down", ref=None):
        self.net, self.at, self.dir, self.ref = net, at, dir, ref
        self.part = None


class Sheet:
    def __init__(self, no, title, size="A4", ports=None, notes=None, texts=None):
        self.no, self.title, self.size = no, title, size
        self.ports = ports or {}      # net -> 'in' | 'out' | 'io'
        self.notes = notes or []
        self.texts = texts or []      # (x, y, text, size) free text on the drawing
        self.parts = []               # ICs and loose parts with `pins`
        self.branches = []
        self.tps = []
        self.titles = []              # (x, y, text) block titles
        self.regions = []             # (title, x, y, w, h) detail crops, sheet coordinates (y up)

    def add(self, part):
        part.sheet = self
        self.parts.append(part)
        return part


SIZES = {"A4": (1169, 827), "A3": (1654, 1169)}


def all_parts(sheet):
    out = list(sheet.parts)
    for br in sheet.branches:
        for el in br.parts:
            out += el.parts if isinstance(el, Par) else [el]
    for p in list(sheet.parts):
        for disp in (p.pins or {}).values():
            if isinstance(disp, Ser):
                out += disp.parts
    for tp in sheet.tps:
        out.append(tp.part)
    return out


def bind(sheets, symbols):
    """Attach symbols, create test-point parts, assign designators."""
    for s in sheets:
        for tp in s.tps:
            tp.part = Part("TestPoint", value="", ref=tp.ref, note="punto de prueba " + tp.net)
            tp.part.sheet = s
        for br in s.branches:
            for el in br.parts:
                for p in (el.parts if isinstance(el, Par) else [el]):
                    p.sheet = s
        for p in list(s.parts):
            for disp in (p.pins or {}).values():
                if isinstance(disp, Ser):
                    for q in disp.parts:
                        q.sheet = s
    fixed, counters = set(), {}
    for s in sheets:
        for p in all_parts(s):
            if p.lib not in symbols:
                raise SystemExit("symbol not in library: %s" % p.lib)
            p.sym = symbols[p.lib]
            if p.ref:
                if p.ref in fixed:
                    raise SystemExit("duplicate designator %s" % p.ref)
                fixed.add(p.ref)
    # automatic numbers skip the explicit ones, so adding explicit high numbers never renumbers existing parts
    for s in sheets:
        for p in all_parts(s):
            if not p.ref:
                pre = p.prefix()
                n = counters.get(pre, 0) + 1
                while "%s%d" % (pre, n) in fixed:
                    n += 1
                counters[pre] = n
                p.ref = "%s%d" % (pre, n)


def two_pin_order(part):
    """(first, second) pin designators of a two-pin part, honouring `first` (a role like 'A')."""
    des = sorted(p["des"] for p in part.sym.pins)
    if len(des) == 1:
        return des[0], None
    if len(des) != 2:
        raise SystemExit("not a two-pin symbol: %s" % part.lib)
    first = "1" if "1" in des else des[0]
    if part.first:
        roles = symlib.ROLES.get(part.lib)
        if not roles or part.first not in roles:
            raise SystemExit("%s: no role %r known" % (part.lib, part.first))
        first = roles[part.first]
    second = [d for d in des if d != first][0]
    return first, second


class Netlist:
    def __init__(self):
        self.nets = {}      # name -> [(ref, pin)]
        self.nc = {}        # (ref, pin) -> note
        self.anon = 0

    def add(self, net, ref, pin):
        self.nets.setdefault(net, []).append((ref, pin))

    def new_anon(self, hint):
        self.anon += 1
        return "~%s%d" % (hint, self.anon)


def build_netlist(sheets):
    nl = Netlist()
    for s in sheets:
        for p in s.parts:
            if not p.pins:
                raise SystemExit("%s %s has no pin table" % (p.lib, p.ref))
            for pin, disp in p.pins.items():
                if isinstance(disp, NC):
                    nl.nc[(p.ref, pin)] = disp.note or "sin conectar"
                elif isinstance(disp, (str, Far)):
                    nl.add(disp if isinstance(disp, str) else disp.net, p.ref, pin)
                elif isinstance(disp, Ser):
                    prev = nl.new_anon("%s_%s_" % (p.ref, pin))
                    nl.add(prev, p.ref, pin)
                    for i, q in enumerate(disp.parts):
                        f, sec = two_pin_order(q)
                        nl.add(prev, q.ref, f)
                        nxt = disp.net if i == len(disp.parts) - 1 else nl.new_anon("%s_" % q.ref)
                        nl.add(nxt, q.ref, sec)
                        prev = nxt
                else:
                    raise SystemExit("bad disposition on %s.%s" % (p.ref, pin))
        for br in s.branches:
            prev = br.a
            for i, el in enumerate(br.parts):
                last = i == len(br.parts) - 1
                nxt = br.b if last else nl.new_anon("br")
                if nxt is None:
                    nxt = nl.new_anon("open")
                for q in (el.parts if isinstance(el, Par) else [el]):
                    f, sec = two_pin_order(q)
                    nl.add(prev, q.ref, f)
                    if sec:
                        nl.add(nxt, q.ref, sec)
                prev = nxt
        for tp in s.tps:
            nl.add(tp.net, tp.part.ref, "1")
    return nl
