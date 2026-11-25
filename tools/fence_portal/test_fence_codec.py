"""Check the Python codec against the vectors shared with the C codec."""

import pathlib
import unittest

import fence_codec

VECTORS = (pathlib.Path(__file__).resolve().parents[2] / "firmware" / "src" /
           "fence_codec" / "test_vectors.txt")

BOGOTA = (4.6097, -74.0817)


def square(half_m=30.0, dlat_m=0.0):
    lat0, lon0 = BOGOTA
    dlat = half_m / fence_codec.METERS_PER_DEG_LAT
    dlon = dlat
    c = lat0 + dlat_m / fence_codec.METERS_PER_DEG_LAT
    return [(c - dlat, lon0 - dlon), (c - dlat, lon0 + dlon),
            (c + dlat, lon0 + dlon), (c + dlat, lon0 - dlon)]


def load_vectors():
    out = []
    for line in VECTORS.read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        name, expect, hexdata = line.split()
        out.append((name, expect, bytes.fromhex(hexdata)))
    return out


class SharedVectors(unittest.TestCase):
    def test_every_vector_gives_the_expected_result(self):
        vectors = load_vectors()
        self.assertGreaterEqual(len(vectors), 15)
        for name, expect, blob in vectors:
            with self.subTest(name=name):
                if expect == "OK":
                    fence_id, version, vertices = fence_codec.decode(blob)
                    degrees = [(lat / 1e7, lon / 1e7) for lat, lon in vertices]
                    # Re-encoding what was decoded reproduces the same bytes.
                    self.assertEqual(fence_codec.encode(fence_id, version, degrees), blob)
                else:
                    with self.assertRaises(fence_codec.FenceError) as ctx:
                        fence_codec.decode(blob)
                    self.assertEqual(ctx.exception.code, expect)


class Encoder(unittest.TestCase):
    def test_round_trip_keeps_each_vertex_within_6_cm(self):
        blob = fence_codec.encode(42, 9, square())
        fence_id, version, vertices = fence_codec.decode(blob)
        self.assertEqual((fence_id, version, len(vertices)), (42, 9, 4))
        for (lat, lon), (want_lat, want_lon) in zip(vertices, square()):
            self.assertLess(abs(lat / 1e7 - want_lat), 6e-7)
            self.assertLess(abs(lon / 1e7 - want_lon), 6e-7)

    def test_size_for_128_vertices_is_532(self):
        self.assertEqual(fence_codec.encoded_size(128), 532)

    def test_refusals(self):
        lat0, lon0 = BOGOTA
        bowtie = [(lat0, lon0), (lat0, lon0 + 0.0006), (lat0 + 0.0006, lon0),
                  (lat0 + 0.0006, lon0 + 0.0006)]
        too_long = [(lat0, lon0), (lat0, lon0 + 0.04), (lat0 + 0.0005, lon0 + 0.04),
                    (lat0 + 0.0005, lon0)]
        cases = [
            ("bowtie", bowtie, "NOT_SIMPLE"),
            ("two vertices", square()[:2], "VERTEX_COUNT"),
            ("edge over 3.6 km", too_long, "EDGE_TOO_LONG"),
            ("latitude 86", [(86.0, 0.0), (86.0, 0.001), (86.001, 0.001)], "RANGE"),
            ("tiny", [(lat0, lon0), (lat0, lon0 + 0.00002), (lat0 + 0.00002, lon0)], "AREA"),
            ("nan", [(float("nan"), 0.0)] * 3, "RANGE"),
        ]
        for name, vertices, code in cases:
            with self.subTest(name=name):
                with self.assertRaises(fence_codec.FenceError) as ctx:
                    fence_codec.encode(1, 1, vertices)
                self.assertEqual(ctx.exception.code, code)


if __name__ == "__main__":
    unittest.main()
