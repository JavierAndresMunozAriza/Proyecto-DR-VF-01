"""Fence blob codec: the Python twin of firmware/src/fence_codec/.

Same wire format and the same acceptance rules as the C codec (see
firmware/include/fence_codec.h), so the portal refuses a fence before it is
sent instead of the collar refusing it after. The C and Python codecs are
checked against the same vectors (firmware/src/fence_codec/test_vectors.txt).
"""

import math
import struct
import zlib

VERSION = 1
MAX_VERTICES = 128
MIN_VERTICES = 3
DELTA_UNIT = 10                  # 1e-7 degrees per delta step
MIN_AREA_M2 = 100.0
METERS_PER_DEG_LAT = 111195.0
UNITS_PER_DEG = 1e7
MAX_LAT_DEG = 85.0
MAX_LON_DEG = 180.0
MAX_LAT_UNITS = int(MAX_LAT_DEG * UNITS_PER_DEG)
MAX_LON_UNITS = int(MAX_LON_DEG * UNITS_PER_DEG)
INT16_MIN, INT16_MAX = -32768, 32767

HEADER_SIZE = 12
FIRST_VERTEX_SIZE = 8
CRC_SIZE = 4
DELTA_SIZE = 4
MIN_LEN = HEADER_SIZE + FIRST_VERTEX_SIZE + CRC_SIZE


class FenceError(ValueError):
    """A fence the codec refuses. `code` matches the C status names."""

    def __init__(self, code, message=""):
        super().__init__(f"{code}: {message}" if message else code)
        self.code = code


def encoded_size(n):
    return HEADER_SIZE + FIRST_VERTEX_SIZE + DELTA_SIZE * (n - 1) + CRC_SIZE


def _rdiv_unit(a):
    """Divide by DELTA_UNIT rounding half away from zero (as the C code)."""
    half = DELTA_UNIT // 2
    if a >= 0:
        return (a + half) // DELTA_UNIT
    return -((-a + half) // DELTA_UNIT)


def _llround(x):
    return int(math.copysign(math.floor(abs(x) + 0.5), x))


def _in_range(lat, lon):
    return abs(lat) <= MAX_LAT_UNITS and abs(lon) <= MAX_LON_UNITS


def _cross(a, b, c):
    return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])


def _sign(v):
    return (v > 0) - (v < 0)


def _in_box(a, b, p):
    return (min(a[0], b[0]) <= p[0] <= max(a[0], b[0]) and
            min(a[1], b[1]) <= p[1] <= max(a[1], b[1]))


def _segments_touch(p1, p2, p3, p4):
    d1 = _sign(_cross(p3, p4, p1))
    d2 = _sign(_cross(p3, p4, p2))
    d3 = _sign(_cross(p1, p2, p3))
    d4 = _sign(_cross(p1, p2, p4))
    if d1 * d2 < 0 and d3 * d4 < 0:
        return True
    return ((d1 == 0 and _in_box(p3, p4, p1)) or
            (d2 == 0 and _in_box(p3, p4, p2)) or
            (d3 == 0 and _in_box(p1, p2, p3)) or
            (d4 == 0 and _in_box(p1, p2, p4)))


def _is_simple(v):
    n = len(v)
    for i in range(n):
        q, a, b = v[i], v[(i - 1) % n], v[(i + 1) % n]
        if q == b:
            return False
        u = (a[0] - q[0], a[1] - q[1])
        w = (b[0] - q[0], b[1] - q[1])
        if _cross((0, 0), u, w) == 0 and u[0] * w[0] + u[1] * w[1] > 0:
            return False
    for i in range(n):
        for j in range(i + 2, n):
            if i == 0 and j == n - 1:
                continue
            if _segments_touch(v[i], v[i + 1], v[j], v[(j + 1) % n]):
                return False
    return True


def area_m2(v):
    """Area of the polygon in square meters (flat approximation)."""
    twice = sum(_cross(v[0], v[i], v[i + 1]) for i in range(1, len(v) - 1))
    m_per_unit = METERS_PER_DEG_LAT / UNITS_PER_DEG
    lat0_rad = math.radians(v[0][0] / UNITS_PER_DEG)
    return abs(twice) / 2.0 * m_per_unit * m_per_unit * math.cos(lat0_rad)


def _check_polygon(v):
    if not _is_simple(v):
        raise FenceError("NOT_SIMPLE", "edges cross, touch or overlap")
    if area_m2(v) < MIN_AREA_M2:
        raise FenceError("AREA", f"smaller than {MIN_AREA_M2:g} m2")


def encode(fence_id, fence_version, vertices):
    """Encode [(lat, lon), ...] in degrees into a blob. Raises FenceError."""
    n = len(vertices)
    if n < MIN_VERTICES or n > MAX_VERTICES:
        raise FenceError("VERTEX_COUNT", f"{n} vertices, need {MIN_VERTICES} to {MAX_VERTICES}")

    decoded = []
    deltas = []
    for i, (lat_deg, lon_deg) in enumerate(vertices):
        if (not math.isfinite(lat_deg) or not math.isfinite(lon_deg) or
                abs(lat_deg) > MAX_LAT_DEG or abs(lon_deg) > MAX_LON_DEG):
            raise FenceError("RANGE", f"vertex {i} is out of range")
        lat = _llround(lat_deg * UNITS_PER_DEG)
        lon = _llround(lon_deg * UNITS_PER_DEG)
        if i > 0:
            dlat = _rdiv_unit(lat - decoded[-1][0])
            dlon = _rdiv_unit(lon - decoded[-1][1])
            if not (INT16_MIN <= dlat <= INT16_MAX and INT16_MIN <= dlon <= INT16_MAX):
                raise FenceError("EDGE_TOO_LONG", f"edge {i - 1}-{i} is over 3.6 km")
            deltas.append((dlat, dlon))
            lat = decoded[-1][0] + dlat * DELTA_UNIT
            lon = decoded[-1][1] + dlon * DELTA_UNIT
        if not _in_range(lat, lon):
            raise FenceError("RANGE", f"vertex {i} is out of range")
        decoded.append((lat, lon))

    _check_polygon(decoded)

    body = struct.pack("<BBHII", VERSION, 0, n, fence_id, fence_version)
    body += struct.pack("<ii", *decoded[0])
    for dlat, dlon in deltas:
        body += struct.pack("<hh", dlat, dlon)
    return body + struct.pack("<I", zlib.crc32(body) & 0xFFFFFFFF)


def decode(blob):
    """Decode a blob. Returns (fence_id, version, [(lat, lon) in 1e-7 deg])."""
    if len(blob) < MIN_LEN:
        raise FenceError("SIZE", "too short")
    version, flags, n, fence_id, fence_version = struct.unpack_from("<BBHII", blob, 0)
    if version != VERSION or flags != 0:
        raise FenceError("FORMAT", f"version {version}, flags {flags}")
    if n < MIN_VERTICES or n > MAX_VERTICES:
        raise FenceError("VERTEX_COUNT", f"{n} vertices")
    if len(blob) != encoded_size(n):
        raise FenceError("SIZE", "length does not match the header")
    (crc,) = struct.unpack_from("<I", blob, len(blob) - CRC_SIZE)
    if crc != zlib.crc32(blob[:-CRC_SIZE]) & 0xFFFFFFFF:
        raise FenceError("CRC")

    lat, lon = struct.unpack_from("<ii", blob, HEADER_SIZE)
    if not _in_range(lat, lon):
        raise FenceError("RANGE", "vertex 0 is out of range")
    vertices = [(lat, lon)]
    pos = HEADER_SIZE + FIRST_VERTEX_SIZE
    for i in range(1, n):
        dlat, dlon = struct.unpack_from("<hh", blob, pos)
        lat += dlat * DELTA_UNIT
        lon += dlon * DELTA_UNIT
        if not _in_range(lat, lon):
            raise FenceError("RANGE", f"vertex {i} is out of range")
        vertices.append((lat, lon))
        pos += DELTA_SIZE

    _check_polygon(vertices)
    return fence_id, fence_version, vertices
