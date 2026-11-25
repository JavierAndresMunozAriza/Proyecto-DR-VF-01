#!/usr/bin/env python3
"""Local portal to draw a fence on a map and send it to a collar through nRF Cloud.

Run it on your own computer and open http://127.0.0.1:8765 in a browser:

    export NRFCLOUD_API_KEY=...      # nRF Cloud REST API key (never commit it)
    export NRFCLOUD_DEVICE_ID=...    # the Thingy:91 X device ID shown in nRF Cloud
    python3 server.py

Without those variables the portal still draws and validates fences, but it
refuses to send. The API key stays in this process; the browser never sees it.
The server listens on 127.0.0.1 only.
"""

import argparse
import base64
import http.server
import json
import os
import re
import time
import urllib.error
import urllib.parse
import urllib.request

import fence_codec

API_BASE = "https://api.nrfcloud.com/v1"
REQUEST_TIMEOUT_S = 15
MAX_BODY_BYTES = 64 * 1024
MAX_FENCE_ID = 0xFFFFFFFF
DEFAULT_FENCE_ID = 1
DEVICE_ID_PATTERN = re.compile(r"^[A-Za-z0-9_.:-]{1,128}$")
PORTAL_HEADER = "X-Portal"
WEB_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "web")


class RequestError(ValueError):
    """The browser sent something the portal refuses (HTTP 400)."""


def parse_fence_request(body):
    """Return (fence_id, vertices, dry_run) from the JSON body of POST /api/fence."""
    try:
        data = json.loads(body)
    except (UnicodeDecodeError, json.JSONDecodeError):
        raise RequestError("the body is not valid JSON") from None
    if not isinstance(data, dict) or not isinstance(data.get("vertices"), list):
        raise RequestError("expected an object with a vertices list")

    vertices = []
    for item in data["vertices"]:
        ok = (isinstance(item, list) and len(item) == 2 and
              all(isinstance(x, (int, float)) and not isinstance(x, bool) for x in item))
        if not ok:
            raise RequestError("each vertex must be [latitude, longitude]")
        try:
            vertex = (float(item[0]), float(item[1]))
        except OverflowError:
            raise RequestError("a coordinate is too large") from None
        vertices.append(vertex)

    fence_id = data.get("fence_id", DEFAULT_FENCE_ID)
    if (not isinstance(fence_id, int) or isinstance(fence_id, bool) or
            not 0 <= fence_id <= MAX_FENCE_ID):
        raise RequestError("fence_id must be an integer from 0 to 4294967295")
    return fence_id, vertices, bool(data.get("dry_run", False))


def build_shadow_patch(blob):
    """Body that asks the device to use this fence (see cloud.h for the exchange)."""
    return {"desired": {"fence": base64.b64encode(blob).decode("ascii")}}


class Cloud:
    """The small part of the nRF Cloud REST API the portal needs."""

    def __init__(self, api_key, device_id):
        self.api_key = api_key
        self.device_id = device_id

    @property
    def configured(self):
        return bool(self.api_key and self.device_id and DEVICE_ID_PATTERN.match(self.device_id))

    def _request(self, method, path, payload=None):
        url = f"{API_BASE}/devices/{urllib.parse.quote(self.device_id, safe='')}{path}"
        data = None if payload is None else json.dumps(payload).encode("utf-8")
        request = urllib.request.Request(url, data=data, method=method)
        request.add_header("Authorization", f"Bearer {self.api_key}")
        request.add_header("Content-Type", "application/json")
        try:
            with urllib.request.urlopen(request, timeout=REQUEST_TIMEOUT_S) as response:
                text = response.read().decode("utf-8")
                return json.loads(text) if text else {}
        except urllib.error.HTTPError as err:
            raise RuntimeError(f"nRF Cloud answered HTTP {err.code}") from None
        except (urllib.error.URLError, TimeoutError, json.JSONDecodeError):
            raise RuntimeError("nRF Cloud could not be reached") from None

    def send_fence(self, blob):
        self._request("PATCH", "/state", build_shadow_patch(blob))

    def reported_state(self):
        """What the device last reported. The response shape is not verified yet."""
        state = self._request("GET", "").get("state")
        reported = state.get("reported") if isinstance(state, dict) else None
        return reported if isinstance(reported, dict) else {}


class Handler(http.server.SimpleHTTPRequestHandler):
    cloud = Cloud(None, None)
    allowed_hosts = frozenset()

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def _send_json(self, status, obj):
        body = json.dumps(obj).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def _host_ok(self):
        # Refuses requests that reach this server through another name (DNS rebinding).
        return self.headers.get("Host", "") in self.allowed_hosts

    def do_GET(self):
        if not self._host_ok():
            self._send_json(403, {"error": "unexpected Host header"})
        elif self.path == "/api/status":
            self._status()
        else:
            super().do_GET()

    def do_POST(self):
        content_type = self.headers.get("Content-Type", "")
        if not self._host_ok():
            self._send_json(403, {"error": "unexpected Host header"})
        elif self.path != "/api/fence":
            self._send_json(404, {"error": "not found"})
        elif not content_type.startswith("application/json") or not self.headers.get(PORTAL_HEADER):
            # A page on another site cannot set these headers without a preflight.
            self._send_json(400, {"error": "missing JSON content type or portal header"})
        else:
            self._fence()

    def _status(self):
        if not self.cloud.configured:
            self._send_json(200, {"configured": False})
            return
        try:
            self._send_json(200, {"configured": True, "reported": self.cloud.reported_state()})
        except RuntimeError as err:
            self._send_json(502, {"configured": True, "error": str(err)})

    def _fence(self):
        try:
            length = int(self.headers.get("Content-Length", "0") or 0)
        except ValueError:
            length = 0
        if length <= 0 or length > MAX_BODY_BYTES:
            self._send_json(400, {"error": "the body is empty or too large"})
            return
        try:
            fence_id, vertices, dry_run = parse_fence_request(self.rfile.read(length))
            version = int(time.time())
            blob = fence_codec.encode(fence_id, version, vertices)
        except RequestError as err:
            self._send_json(400, {"error": str(err)})
            return
        except fence_codec.FenceError as err:
            self._send_json(422, {"ok": False, "code": err.code, "error": str(err)})
            return

        _, _, decoded = fence_codec.decode(blob)
        result = {"ok": True, "vertices": len(vertices), "bytes": len(blob),
                  "area_m2": round(fence_codec.area_m2(decoded)), "version": version,
                  "sent": False}
        if not dry_run:
            if not self.cloud.configured:
                self._send_json(503, {"ok": False, "error": "NRFCLOUD_API_KEY and "
                                      "NRFCLOUD_DEVICE_ID are not set"})
                return
            try:
                self.cloud.send_fence(blob)
            except RuntimeError as err:
                self._send_json(502, {"ok": False, "error": str(err)})
                return
            result["sent"] = True
        self._send_json(200, result)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--port", type=int, default=8765)
    args = parser.parse_args()

    Handler.cloud = Cloud(os.environ.get("NRFCLOUD_API_KEY"), os.environ.get("NRFCLOUD_DEVICE_ID"))
    Handler.allowed_hosts = frozenset({f"127.0.0.1:{args.port}", f"localhost:{args.port}"})
    server = http.server.ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"Portal on http://127.0.0.1:{args.port} "
          f"({'sending enabled' if Handler.cloud.configured else 'validation only: set '
           'NRFCLOUD_API_KEY and NRFCLOUD_DEVICE_ID to send'})")
    server.serve_forever()


if __name__ == "__main__":
    main()
