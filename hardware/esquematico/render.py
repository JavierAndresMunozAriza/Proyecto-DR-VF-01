"""SVG renderer with the look of an Altium schematic sheet (A4/A3, 10 mil units, y up)."""
from xml.sax.saxutils import escape

import model
import symlib

C_PAPER = "#fbfbea"
C_GRID = "#ebebd2"
C_WIRE = "#00007f"
C_NET = "#800000"
C_TXT = "#000080"
C_BODY = "#800000"
C_PIN = "#1a1a1a"
FS_NET, FS_REF, FS_PIN, FS_NUM, FS_TTL = 9.0, 9.0, 7.6, 6.6, 12.0
CW = 0.56  # average character width / font size


def tw(text, fs=FS_NET):
    return len(text) * fs * CW


def rotp(x, y, r):
    for _ in range(r % 4):
        x, y = -y, x
    return x, y


DIRS = {0: (1, 0), 1: (0, 1), 2: (-1, 0), 3: (0, -1)}


class Canvas:
    def __init__(self, W, H):
        self.W, self.H = W, H
        self.layers = {k: [] for k in ("grid", "wire", "part", "text", "top")}

    def Y(self, y):
        return self.H - y

    def add(self, layer, s):
        self.layers[layer].append(s)

    def line(self, pts, color=C_WIRE, w=1.0, layer="wire", dash=None, fill="none"):
        p = " ".join("%.2f,%.2f" % (x, self.Y(y)) for x, y in pts)
        d = ' stroke-dasharray="%s"' % dash if dash else ""
        self.add(layer, '<polyline points="%s" fill="%s" stroke="%s" stroke-width="%.2f" stroke-linejoin="round" '
                        'stroke-linecap="round"%s/>' % (p, fill, color, w, d))

    def polygon(self, pts, fill, color, w=0.8, layer="part"):
        p = " ".join("%.2f,%.2f" % (x, self.Y(y)) for x, y in pts)
        self.add(layer, '<polygon points="%s" fill="%s" stroke="%s" stroke-width="%.2f" stroke-linejoin="round"/>'
                 % (p, fill, color, w))

    def rect(self, x1, y1, x2, y2, fill, color, w=0.8, rx=0, layer="part"):
        x, y = min(x1, x2), min(y1, y2)
        self.add(layer, '<rect x="%.2f" y="%.2f" width="%.2f" height="%.2f" rx="%.2f" fill="%s" stroke="%s" '
                        'stroke-width="%.2f"/>' % (x, self.Y(max(y1, y2)), abs(x2 - x1), abs(y2 - y1), rx, fill, color, w))

    def dot(self, x, y, r=2.0, color=C_WIRE):
        self.add("top", '<circle cx="%.2f" cy="%.2f" r="%.2f" fill="%s"/>' % (x, self.Y(y), r, color))

    def text(self, x, y, s, fs=FS_NET, color=C_NET, anchor="start", weight="normal", rot=0, over=False, layer="text"):
        deco = ' text-decoration="overline"' if over else ""
        tr = ' transform="rotate(%d %.2f %.2f)"' % (-rot, x, self.Y(y)) if rot else ""
        self.add(layer, '<text x="%.2f" y="%.2f" font-size="%.1f" fill="%s" text-anchor="%s" font-weight="%s" '
                        'font-family="Arial, Helvetica, sans-serif"%s%s>%s</text>'
                 % (x, self.Y(y), fs, color, anchor, weight, deco, tr, escape(s)))

    def nc_cross(self, x, y):
        s = 3.2
        self.line([(x - s, y - s), (x + s, y + s)], "#e00000", 1.3, "top")
        self.line([(x - s, y + s), (x + s, y - s)], "#e00000", 1.3, "top")


class Placed:
    def __init__(self, part, ox, oy, rot):
        self.part, self.sym, self.ox, self.oy, self.rot = part, part.sym, ox, oy, rot

    def pt(self, x, y):
        rx, ry = rotp(x, y, self.rot)
        return self.ox + rx, self.oy + ry

    def hot(self, des):
        p = self.sym.pin(des)
        return self.pt(p["hx"], p["hy"])

    def out_dir(self, des):
        p = self.sym.pin(des)
        return rotp(*DIRS[p["rot"]], self.rot)

    def extent(self):
        """Half extent of the body (graphics only) across and along, for text offsets."""
        xs, ys = [], []
        for pr in self.sym.prims:
            for x, y in pr.get("pts", []):
                px, py = self.pt(x, y)
                xs.append(px)
                ys.append(py)
        if not xs:
            return (self.ox, self.oy, self.ox, self.oy)
        return (min(xs), min(ys), max(xs), max(ys))


def draw_symbol(cv, pl, show_pin_text=True, name_override=None):
    sym = pl.sym
    for pr in sym.prims:
        k = pr["kind"]
        col = symlib.bgr(pr.get("color"), C_BODY)
        if k == "rect":
            (x1, y1), (x2, y2) = [pl.pt(*p) for p in pr["pts"]]
            fill = symlib.bgr(pr.get("fill"), "#ffffb0") if pr.get("solid") else "none"
            cv.rect(x1, y1, x2, y2, fill, col, 1.0, rx=pr.get("rx", 0) if pr.get("round") else 0)
        elif k == "poly":
            cv.line([pl.pt(*p) for p in pr["pts"]], col, 1.0, "part")
        elif k == "polygon":
            cv.polygon([pl.pt(*p) for p in pr["pts"]], symlib.bgr(pr.get("fill"), "#ffffb0"), col)
        elif k == "arc":
            cx, cy = pl.pt(*pr["pts"][0])
            r = pr["radius"]
            a0, a1 = pr["a0"] + 90 * pl.rot, pr["a1"] + 90 * pl.rot
            import math
            full = abs((a1 - a0) % 360) < 1e-6
            if full:
                cv.add("part", '<circle cx="%.2f" cy="%.2f" r="%.2f" fill="none" stroke="%s" stroke-width="1"/>'
                       % (cx, cv.Y(cy), r, col))
            else:
                sx, sy = cx + r * math.cos(math.radians(a0)), cy + r * math.sin(math.radians(a0))
                ex, ey = cx + r * math.cos(math.radians(a1)), cy + r * math.sin(math.radians(a1))
                large = 1 if (a1 - a0) % 360 > 180 else 0
                cv.add("part", '<path d="M %.2f %.2f A %.2f %.2f 0 %d 0 %.2f %.2f" fill="none" stroke="%s" '
                               'stroke-width="1"/>' % (sx, cv.Y(sy), r, r, large, ex, cv.Y(ey), col))
        elif k == "label":
            x, y = pl.pt(*pr["pts"][0])
            cv.text(x, y, pr["text"], 7, "#000000", layer="part")
    for p in sym.pins:
        bx, by = pl.pt(p["x"], p["y"])
        hx, hy = pl.pt(p["hx"], p["hy"])
        cv.line([(bx, by), (hx, hy)], C_PIN, 1.0, "part")
        side = (p["rot"] + pl.rot) % 4
        if not show_pin_text:
            continue
        name = p["name"].replace("\\", "") if p["show_name"] else ""
        over = "\\" in p["name"]
        if name:
            if side == 0:
                cv.text(bx - 3, by - 2.6, name, FS_PIN, C_PIN, "end", over=over, layer="part")
            elif side == 2:
                cv.text(bx + 3, by - 2.6, name, FS_PIN, C_PIN, "start", over=over, layer="part")
            elif side == 1:
                cv.text(bx + 2.6, by - 3, name, FS_PIN, C_PIN, "end", rot=90, over=over, layer="part")
            else:
                cv.text(bx + 2.6, by + 3, name, FS_PIN, C_PIN, "start", rot=90, over=over, layer="part")
        if p["show_num"]:
            if side == 0:
                cv.text(bx + 2, by + 1.6, p["des"], FS_NUM, "#000000", "start", layer="part")
            elif side == 2:
                cv.text(bx - 2, by + 1.6, p["des"], FS_NUM, "#000000", "end", layer="part")
            else:
                cv.text(bx + 2, by + (2 if side == 1 else -8), p["des"], FS_NUM, "#000000", "start", layer="part")


def ref_texts(cv, pl, ref, value, dnp=False, side="right"):
    """Designator and value around a two-pin part. side 'up'/'down' = one compact line on that side."""
    f, s = model.two_pin_order(pl.part)
    x0, y0, x1, y1 = pl.extent()
    hf = pl.hot(f)
    hs = pl.hot(s) if s else hf
    cx, cy = (hf[0] + hs[0]) / 2, (hf[1] + hs[1]) / 2
    val = value + (" DNP" if dnp else "")
    if side in ("up", "down") and abs(hf[1] - hs[1]) < 1e-6:
        off = max(y1 - cy, 5) + 3.5
        cv.text(cx, cy + off if side == "up" else cy - off - 6, "%s %s" % (ref, val), FS_REF, C_TXT, "middle")
    elif abs(hf[1] - hs[1]) < 1e-6:        # horizontal
        up = max(y1 - cy, 5) + 3.5
        cv.text(cx, cy + up, ref, FS_REF, C_TXT, "middle")
        cv.text(cx, cy - up - 6, val, FS_REF, C_TXT, "middle")
    else:                                 # vertical
        right = max(x1 - cx, 5) + 4
        if side == "left":
            cv.text(cx - right, cy + 1.5, ref, FS_REF, C_TXT, "end")
            cv.text(cx - right, cy - 8, val, FS_REF, C_TXT, "end")
        else:
            cv.text(cx + right, cy + 1.5, ref, FS_REF, C_TXT, "start")
            cv.text(cx + right, cy - 8, val, FS_REF, C_TXT, "start")


def orient_two_pin(part, A, d):
    """Placed part whose first pin hot end sits at A and whose second hot end lies along d."""
    f, s = model.two_pin_order(part)
    sym = part.sym
    pf, ps = sym.pin(f), sym.pin(s)
    vx, vy = ps["hx"] - pf["hx"], ps["hy"] - pf["hy"]
    for r in range(4):
        rx, ry = rotp(vx, vy, r)
        n = (rx * rx + ry * ry) ** 0.5
        if abs(rx / n - d[0]) < 1e-6 and abs(ry / n - d[1]) < 1e-6:
            fx, fy = rotp(pf["hx"], pf["hy"], r)
            return Placed(part, A[0] - fx, A[1] - fy, r)
    raise SystemExit("cannot orient %s along %s" % (part.lib, d))


def part_len(part):
    f, s = model.two_pin_order(part)
    pf, ps = part.sym.pin(f), part.sym.pin(s)
    return ((ps["hx"] - pf["hx"]) ** 2 + (ps["hy"] - pf["hy"]) ** 2) ** 0.5


def draw_two_pin(cv, part, A, d, texts=True, side="right"):
    pl = orient_two_pin(part, A, d)
    draw_symbol(cv, pl, show_pin_text=False)
    if texts:
        ref_texts(cv, pl, part.ref, part.value, part.dnp, side)
    f, s = model.two_pin_order(part)
    return pl.hot(s)


# ---------------------------------------------------------------------------- terminations
def terminal(cv, sheet, net, P, d, force_label=False):
    """Wire from P along d ending in a power port, a sheet port or a net label."""
    dx, dy = d
    nx, ny = -dy, dx
    if net in model.POWER:
        Q = (P[0] + dx * 10, P[1] + dy * 10)
        cv.line([P, Q], C_WIRE, 1.0)
        if net == "GND":
            for i, hw in enumerate((7, 4.5, 2)):
                o = i * 3
                a = (Q[0] + dx * o + nx * hw, Q[1] + dy * o + ny * hw)
                b = (Q[0] + dx * o - nx * hw, Q[1] + dy * o - ny * hw)
                cv.line([a, b], C_NET, 1.2)
            tx, ty = Q[0] + dx * 11, Q[1] + dy * 11
            _beyond(cv, "GND", Q, d, 7)
        else:
            a = (Q[0] + nx * 6, Q[1] + ny * 6)
            b = (Q[0] - nx * 6, Q[1] - ny * 6)
            cv.line([a, b], C_NET, 1.4)
            _beyond(cv, net, Q, d, 3)
        return Q
    if net in sheet.ports and not force_label:
        kind = sheet.ports[net]
        Q = (P[0] + dx * 6, P[1] + dy * 6)
        cv.line([P, Q], C_WIRE, 1.0)
        w = tw(net, 7.8) * 1.2 + 14
        h = 4.6
        tip = 4.6
        # local polygon along +x then rotate to d
        if kind == "in":
            pts = [(0, 0), (tip, h), (w, h), (w, -h), (tip, -h)]
        elif kind == "out":
            pts = [(0, h), (w - tip, h), (w, 0), (w - tip, -h), (0, -h)]
        else:
            pts = [(0, 0), (tip, h), (w - tip, h), (w, 0), (w - tip, -h), (tip, -h)]
        def tr(p):
            if dx == 1:
                return (Q[0] + p[0], Q[1] + p[1])
            if dx == -1:
                return (Q[0] - p[0], Q[1] - p[1])
            raise SystemExit("ports only horizontally")
        cv.polygon([tr(p) for p in pts], "#ffffd0", C_NET, 0.9)
        cx = tr((w / 2, 0))
        cv.text(cx[0], cx[1] - 2.7, net, 7.8, C_NET, "middle")
        return Q
    L = max(12, tw(net) + 6)
    Q = (P[0] + dx * L, P[1] + dy * L)
    cv.line([P, Q], C_WIRE, 1.0)
    if dx == 1:
        cv.text(P[0] + 2, P[1] + 2.2, net, FS_NET, C_NET, "start")
    elif dx == -1:
        cv.text(P[0] - 2, P[1] + 2.2, net, FS_NET, C_NET, "end")
    else:
        cv.text(P[0] + 3, (P[1] + Q[1]) / 2 - 3, net, FS_NET, C_NET, "start")
    return Q


def _beyond(cv, text, Q, d, gap):
    dx, dy = d
    if dy == -1:
        cv.text(Q[0], Q[1] - 10 - gap, text, FS_NET, C_NET, "middle")
    elif dy == 1:
        cv.text(Q[0], Q[1] + gap + 3, text, FS_NET, C_NET, "middle")
    elif dx == 1:
        cv.text(Q[0] + gap + (8 if text == "GND" else 2), Q[1] - 3, text, FS_NET, C_NET, "start")
    else:
        cv.text(Q[0] - gap - (8 if text == "GND" else 2), Q[1] - 3, text, FS_NET, C_NET, "end")


# ---------------------------------------------------------------------------- ICs
def draw_ic(cv, sheet, part):
    ox, oy = part.at
    pl = Placed(part, ox, oy, 0)
    draw_symbol(cv, pl)
    sym = part.sym
    bx0, by0, bx1, by1 = pl.extent()
    dx_, dy_ = bx0, by1 + 4
    cv.text(dx_, dy_ + 2, part.ref, FS_REF, C_TXT, "start", weight="bold")
    text = part.value or (sym.comment["text"] if sym.comment else "")
    if text:
        cv.text(dx_ + tw(part.ref, FS_REF) * 1.25 + 8, dy_ + 2, text, FS_REF, C_TXT, "start")
    # bundles of adjacent power pins on the same side
    pins = sorted(sym.pins, key=lambda p: (p["rot"], -p["hy"], p["hx"]))
    done = set()
    side_groups = {}
    for p in pins:
        disp = part.pins.get(p["des"])
        if isinstance(disp, str) and p["rot"] in (0, 2):
            side_groups.setdefault((p["rot"], disp), []).append(p)
    for (rot, net), grp in side_groups.items():
        grp.sort(key=lambda p: -p["hy"])
        run = [grp[0]]
        runs = []
        for p in grp[1:]:
            if abs(run[-1]["hy"] - p["hy"] - 10) < 1e-6 and abs(run[-1]["hx"] - p["hx"]) < 1e-6:
                run.append(p)
            else:
                runs.append(run)
                run = [p]
        runs.append(run)
        for run in runs:
            if len(run) < 2:
                continue
            d = DIRS[rot]
            ends = []
            for p in run:
                h = pl.pt(p["hx"], p["hy"])
                e = (h[0] + d[0] * 10, h[1])
                cv.line([h, e], C_WIRE, 1.0)
                ends.append(e)
                done.add(p["des"])
            cv.line([ends[0], ends[-1]], C_WIRE, 1.0)
            for e in ends[1:-1]:
                cv.dot(*e)
            mid = ends[len(ends) // 2]
            cv.dot(*mid)
            terminal(cv, sheet, net, mid, d)
    seen = set()
    for p in pins:
        if p["des"] in done:
            continue
        key = (round(p["hx"]), round(p["hy"]), repr(vars(part.pins.get(p["des"])) if hasattr(part.pins.get(p["des"]), "__dict__")
                                                      else part.pins.get(p["des"])))
        if key in seen:
            continue
        seen.add(key)
        if p["des"] not in part.pins:
            raise SystemExit("%s %s: pin %s has no disposition" % (part.ref, part.lib, p["des"]))
        disp = part.pins[p["des"]]
        h = pl.pt(p["hx"], p["hy"])
        d = DIRS[p["rot"]]
        if isinstance(disp, model.NC):
            cv.nc_cross(*h)
        elif isinstance(disp, str):
            terminal(cv, sheet, disp, h, d)
        elif isinstance(disp, model.Far):
            e = (h[0] + d[0] * disp.ext, h[1] + d[1] * disp.ext)
            cv.line([h, e], C_WIRE, 1.0)
            terminal(cv, sheet, disp.net, e, d)
        elif isinstance(disp, model.Ser):
            cur = h
            cv.line([cur, (cur[0] + d[0] * 6, cur[1] + d[1] * 6)], C_WIRE, 1.0)
            cur = (cur[0] + d[0] * 6, cur[1] + d[1] * 6)
            for i, q in enumerate(disp.parts):
                end = draw_two_pin(cv, q, cur, d)
                if i < len(disp.parts) - 1:
                    nxt = (end[0] + d[0] * 6, end[1] + d[1] * 6)
                    cv.line([end, nxt], C_WIRE, 1.0)
                    cur = nxt
                else:
                    cur = end
            terminal(cv, sheet, disp.net, cur, d)
        else:
            raise SystemExit("bad disposition")


# ---------------------------------------------------------------------------- branches
def draw_branch(cv, sheet, br):
    x, y = br.at
    d = (0, -1) if br.orient == "v" else (1, 0)
    if br.a is not None:
        back = (-d[0], -d[1])
        terminal(cv, sheet, br.a, (x, y), back, force_label=(br.orient == "v"))
    cur = (x, y)
    n = len(br.parts)
    for i, el in enumerate(br.parts):
        if isinstance(el, model.Par) and br.orient == "h":
            x0 = cur[0] + 6
            cv.line([cur, (x0, cur[1])], C_WIRE, 1.0)
            ends = []
            for off, q in zip((13, -13), el.parts):
                a = (x0 + 4, cur[1] + off)
                cv.line([(x0, cur[1] + off), a], C_WIRE, 1.0)
                end = draw_two_pin(cv, q, a, d, side="up" if off > 0 else "down")
                ends.append(end)
            x1 = ends[0][0] + 4
            for off, e in zip((13, -13), ends):
                cv.line([e, (x1, e[1])], C_WIRE, 1.0)
            cv.line([(x0, cur[1] + 13), (x0, cur[1] - 13)], C_WIRE, 1.0)
            cv.line([(x1, cur[1] + 13), (x1, cur[1] - 13)], C_WIRE, 1.0)
            cv.dot(x0, cur[1])
            cv.dot(x1, cur[1])
            cur = (x1, cur[1])
        elif isinstance(el, model.Par):
            offs = (-13, 13)
            top = cur
            ends = []
            for off, q in zip(offs, el.parts):
                a = (top[0] + off, top[1] - 5)
                cv.line([top, (top[0] + off, top[1]), a], C_WIRE, 1.0)
                end = draw_two_pin(cv, q, a, d, side="left" if off < 0 else "right")
                ends.append(end)
            bot = (top[0], ends[0][1] - 5)
            for off, e in zip(offs, ends):
                cv.line([e, (e[0], bot[1]), (top[0], bot[1])], C_WIRE, 1.0)
            cv.dot(top[0], top[1])
            cv.dot(bot[0], bot[1])
            cur = bot
        else:
            end = draw_two_pin(cv, el, cur, d)
            cur = end
        if i < n - 1:
            nxt = (cur[0] + d[0] * 6, cur[1] + d[1] * 6)
            cv.line([cur, nxt], C_WIRE, 1.0)
            cur = nxt
    if br.b is not None:
        terminal(cv, sheet, br.b, cur, d, force_label=(br.orient == "v"))


def draw_tp(cv, sheet, tp):
    x, y = tp.at
    d = (0, -1) if tp.dir == "down" else (0, 1) if tp.dir == "up" else (1, 0)
    back = (-d[0], -d[1])
    # net wire arrives from the opposite side; the test-point pin runs along d
    cur = tp.part.sym.pin("1")
    pl = None
    for r in range(4):
        rx, ry = rotp(cur["x"] - cur["hx"], cur["y"] - cur["hy"], r)
        n = (rx * rx + ry * ry) ** 0.5
        if abs(rx / n - d[0]) < 1e-6 and abs(ry / n - d[1]) < 1e-6:
            fx, fy = rotp(cur["hx"], cur["hy"], r)
            pl = Placed(tp.part, x - fx, y - fy, r)
            break
    draw_symbol(cv, pl, show_pin_text=False)
    end = pl.pt(cur["x"], cur["y"])
    cv.text(end[0] + (4 if d[0] == 0 else 0), end[1] - 8 if d[1] == -1 else end[1] + 2, tp.part.ref, FS_REF, C_TXT,
            "start")
    cv.dot(x, y)
    terminal(cv, sheet, tp.net, (x, y), back, force_label=True)


# ---------------------------------------------------------------------------- sheet frame
def frame(cv, sheet, total):
    W, H = cv.W, cv.H
    for gx in range(0, int(W) + 1, 10):
        cv.add("grid", '<line x1="%d" y1="0" x2="%d" y2="%d" stroke="%s" stroke-width="0.3"/>' % (gx, gx, H, C_GRID))
    for gy in range(0, int(H) + 1, 10):
        cv.add("grid", '<line x1="0" y1="%d" x2="%d" y2="%d" stroke="%s" stroke-width="0.3"/>' % (cv.Y(gy), W, cv.Y(gy), C_GRID))
    m = 20
    cv.rect(m, m, W - m, H - m, "none", "#000000", 1.4, layer="top")
    cv.rect(m + 8, m + 8, W - m - 8, H - m - 8, "none", "#000000", 0.6, layer="top")
    cols = 4 if sheet.size == "A4" else 6
    rows = 4
    cw = (W - 2 * m) / cols
    rh = (H - 2 * m) / rows
    for i in range(cols):
        cx = m + cw * i
        if i:
            cv.line([(cx, H - m), (cx, H - m - 8)], "#000000", 0.6, "top")
            cv.line([(cx, m), (cx, m + 8)], "#000000", 0.6, "top")
        cv.text(cx + cw / 2, H - m - 4.5, str(i + 1), 7, "#000000", "middle", layer="top")
    for j in range(rows):
        cy = H - m - rh * j
        if j:
            cv.line([(m, cy), (m + 8, cy)], "#000000", 0.6, "top")
            cv.line([(W - m, cy), (W - m - 8, cy)], "#000000", 0.6, "top")
        cv.text(m + 3, cy - rh / 2 - 2, "ABCD"[j], 7, "#000000", "middle", layer="top")
    # title block
    bw, bh = 330, 62
    x0, y0 = W - m - 8 - bw, m + 8
    cv.rect(x0, y0, x0 + bw, y0 + bh, "#ffffff", "#000000", 0.8, layer="top")
    cv.line([(x0, y0 + 40), (x0 + bw, y0 + 40)], "#000000", 0.5, "top")
    cv.line([(x0, y0 + 20), (x0 + bw, y0 + 20)], "#000000", 0.5, "top")
    cv.line([(x0 + 230, y0), (x0 + 230, y0 + 20)], "#000000", 0.5, "top")
    cv.text(x0 + 6, y0 + 46, "Hoja %s: %s" % (sheet.no, sheet.title), 11, "#000000", weight="bold", layer="top")
    cv.text(x0 + 6, y0 + 26, "DR-VF-01  Collar de cercado virtual", 8, "#000000", layer="top")
    cv.text(x0 + 6, y0 + 7, "Guia de dibujo (no es el archivo de Altium)", 7, "#555555", layer="top")
    cv.text(x0 + 236, y0 + 7, "Hoja %s de %d  %s" % (sheet.no, total, sheet.size), 8, "#000000", layer="top")
    cv.text(x0 + 236, y0 + 26, "Rev. 2026-11-08", 8, "#000000", layer="top")


def render_sheet(sheet, total):
    W, H = model.SIZES[sheet.size]
    cv = Canvas(W, H)
    frame(cv, sheet, total)
    for x, y, t in sheet.titles:
        cv.text(x, y, t, FS_TTL, "#333333", weight="bold", layer="top")
    for x, y, t, fs in sheet.texts:
        cv.text(x, y, t, fs, "#444444", layer="top")
    for p in sheet.parts:
        if len(p.sym.pins) > 2:
            draw_ic(cv, sheet, p)
        else:
            draw_loose(cv, sheet, p)
    for br in sheet.branches:
        draw_branch(cv, sheet, br)
    for tp in sheet.tps:
        draw_tp(cv, sheet, tp)
    body = "".join("".join(cv.layers[k]) for k in ("grid", "wire", "part", "text", "top"))
    return body


def wrap(body, W, H, region=None, scale=1.0):
    """Full sheet or a crop (x, y, w, h in sheet coordinates, y up)."""
    if region:
        x, y, w, h = region
        vb = "%g %g %g %g" % (x, H - (y + h), w, h)
        pw, ph = w * scale, h * scale
    else:
        vb, pw, ph = "0 0 %d %d" % (W, H), W * scale, H * scale
    return ('<svg xmlns="http://www.w3.org/2000/svg" viewBox="%s" width="%g" height="%g" '
            'font-family="Arial, Helvetica, sans-serif"><rect x="0" y="0" width="%d" height="%d" fill="%s"/>%s</svg>'
            % (vb, pw, ph, W, H, C_PAPER, body))


def draw_loose(cv, sheet, part):
    """Two-pin part placed with explicit pin dispositions (e.g. a connector)."""
    ox, oy = part.at
    pl = Placed(part, ox, oy, part.rot)
    draw_symbol(cv, pl)
    sym = part.sym
    x0, y0, x1, y1 = pl.extent()
    cv.text(x0, y1 + 8, part.ref, FS_REF, C_TXT, "start", weight="bold")
    if part.value:
        cv.text(x0 + tw(part.ref, FS_REF) * 1.25 + 8, y1 + 8, part.value, FS_REF, C_TXT, "start")
    seen = set()
    for p in sym.pins:
        disp = part.pins[p["des"]]
        h = pl.pt(p["hx"], p["hy"])
        key = (round(h[0]), round(h[1]), repr(vars(disp)) if hasattr(disp, "__dict__") else disp)
        if key in seen:
            continue
        seen.add(key)
        d = rotp(*DIRS[p["rot"]], pl.rot)
        if isinstance(disp, model.NC):
            cv.nc_cross(*h)
        elif isinstance(disp, model.Far):
            e = (h[0] + d[0] * disp.ext, h[1] + d[1] * disp.ext)
            cv.line([h, e], C_WIRE, 1.0)
            terminal(cv, sheet, disp.net, e, d)
        else:
            terminal(cv, sheet, disp, h, d)
