"""Tests of the portal server: request parsing and the HTTP API with a fake cloud."""

import base64
import http.server
import json
import threading
import unittest
import urllib.error
import urllib.request

import fence_codec
import server

LAT, LON, D = 4.6097, -74.0817, 0.00027
SQUARE = [[LAT - D, LON - D], [LAT - D, LON + D], [LAT + D, LON + D], [LAT + D, LON - D]]
BOWTIE = [[LAT, LON], [LAT, LON + 0.0006], [LAT + 0.0006, LON], [LAT + 0.0006, LON + 0.0006]]


class ParseRequest(unittest.TestCase):
    def parse(self, obj):
        return server.parse_fence_request(json.dumps(obj).encode())

    def test_valid_request(self):
        fence_id, vertices, dry_run = self.parse({"vertices": SQUARE})
        self.assertEqual((fence_id, len(vertices), dry_run), (server.DEFAULT_FENCE_ID, 4, False))

    def test_refusals(self):
        bad = [b"not json", b"[]", b'{"vertices": 3}', b'{"vertices": [[1]]}',
               b'{"vertices": [["a", "b"]]}', b'{"vertices": [[true, 1]]}',
               json.dumps({"vertices": SQUARE, "fence_id": -1}).encode(),
               json.dumps({"vertices": SQUARE, "fence_id": 2 ** 32}).encode(),
               json.dumps({"vertices": SQUARE, "fence_id": True}).encode()]
        for body in bad:
            with self.subTest(body=body):
                with self.assertRaises(server.RequestError):
                    server.parse_fence_request(body)

    def test_a_huge_integer_coordinate_is_a_request_error(self):
        body = b'{"vertices": [[1, 1], [1, 1], [' + b"9" * 400 + b", 1]]}"
        with self.assertRaises(server.RequestError):
            server.parse_fence_request(body)

    def test_shadow_patch_carries_the_blob_as_base64(self):
        blob = fence_codec.encode(1, 5, [tuple(v) for v in SQUARE])
        patch = server.build_shadow_patch(blob)
        self.assertEqual(list(patch), ["desired"])
        self.assertEqual(base64.b64decode(patch["desired"]["fence"]), blob)


class FakeCloud:
    configured = True

    def __init__(self):
        self.sent = []

    def send_fence(self, blob):
        self.sent.append(blob)

    def reported_state(self):
        return {"fence_version": 7}


class Api(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        server.Handler.cloud = FakeCloud()
        cls.httpd = http.server.ThreadingHTTPServer(("127.0.0.1", 0), server.Handler)
        cls.port = cls.httpd.server_address[1]
        server.Handler.allowed_hosts = frozenset({f"127.0.0.1:{cls.port}"})
        threading.Thread(target=cls.httpd.serve_forever, daemon=True).start()

    @classmethod
    def tearDownClass(cls):
        cls.httpd.shutdown()
        cls.httpd.server_close()

    def call(self, path, body=None, headers=None):
        url = f"http://127.0.0.1:{self.port}{path}"
        data = None if body is None else json.dumps(body).encode()
        request = urllib.request.Request(url, data=data)
        if body is not None:
            request.add_header("Content-Type", "application/json")
            request.add_header(server.PORTAL_HEADER, "1")
        for key, value in (headers or {}).items():
            request.add_header(key, value)
        try:
            with urllib.request.urlopen(request, timeout=5) as response:
                return response.status, json.loads(response.read())
        except urllib.error.HTTPError as err:
            return err.code, json.loads(err.read())

    def setUp(self):
        server.Handler.cloud = FakeCloud()

    def test_dry_run_validates_and_does_not_send(self):
        status, body = self.call("/api/fence", {"vertices": SQUARE, "dry_run": True})
        self.assertEqual(status, 200)
        self.assertTrue(body["ok"])
        self.assertEqual((body["vertices"], body["sent"]), (4, False))
        self.assertGreater(body["area_m2"], 100)
        self.assertEqual(server.Handler.cloud.sent, [])

    def test_send_puts_a_valid_blob_in_the_shadow(self):
        status, body = self.call("/api/fence", {"vertices": SQUARE, "fence_id": 3})
        self.assertEqual((status, body["sent"]), (200, True))
        (blob,) = server.Handler.cloud.sent
        fence_id, version, vertices = fence_codec.decode(blob)
        self.assertEqual((fence_id, version, len(vertices)), (3, body["version"], 4))

    def test_a_fence_the_device_would_refuse_is_not_sent(self):
        status, body = self.call("/api/fence", {"vertices": BOWTIE})
        self.assertEqual((status, body["code"]), (422, "NOT_SIMPLE"))
        self.assertEqual(server.Handler.cloud.sent, [])

    def test_a_malformed_request_is_refused(self):
        status, _ = self.call("/api/fence", {"vertices": "x"})
        self.assertEqual(status, 400)

    def test_sending_without_credentials_is_refused(self):
        class Unconfigured(FakeCloud):
            configured = False

        server.Handler.cloud = Unconfigured()
        status, body = self.call("/api/fence", {"vertices": SQUARE})
        self.assertEqual(status, 503)
        self.assertFalse(body["ok"])

    def test_cloud_failure_is_a_502_without_secrets(self):
        class Failing(FakeCloud):
            def send_fence(self, blob):
                raise RuntimeError("nRF Cloud could not be reached")

        server.Handler.cloud = Failing()
        status, body = self.call("/api/fence", {"vertices": SQUARE})
        self.assertEqual((status, body["error"]), (502, "nRF Cloud could not be reached"))

    def test_another_host_header_is_refused(self):
        status, _ = self.call("/api/status", headers={"Host": "evil.example"})
        self.assertEqual(status, 403)

    def test_a_post_without_the_portal_header_is_refused(self):
        url = f"http://127.0.0.1:{self.port}/api/fence"
        request = urllib.request.Request(url, data=b'{"vertices": []}',
                                         headers={"Content-Type": "text/plain"})
        with self.assertRaises(urllib.error.HTTPError) as ctx:
            urllib.request.urlopen(request, timeout=5)
        self.assertEqual(ctx.exception.code, 400)

    def test_status_returns_what_the_device_reported(self):
        status, body = self.call("/api/status")
        self.assertEqual((status, body["reported"]), (200, {"fence_version": 7}))

    def test_a_non_numeric_content_length_is_refused(self):
        url = f"http://127.0.0.1:{self.port}/api/fence"
        request = urllib.request.Request(url, data=b"{}", headers={
            "Content-Type": "application/json", server.PORTAL_HEADER: "1"})
        request.add_unredirected_header("Content-Length", "abc")
        with self.assertRaises((urllib.error.HTTPError, urllib.error.URLError, OSError)):
            urllib.request.urlopen(request, timeout=5)

    def test_unknown_post_path(self):
        status, _ = self.call("/api/other", {"vertices": SQUARE})
        self.assertEqual(status, 404)


if __name__ == "__main__":
    unittest.main()
