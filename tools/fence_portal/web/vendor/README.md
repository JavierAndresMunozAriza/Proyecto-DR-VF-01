# Vendored libraries

- `leaflet.js`, `leaflet.css`: Leaflet 1.9.4 (BSD-2-Clause, https://leafletjs.com),
  downloaded from `https://unpkg.com/leaflet@1.9.4/dist/`.
  - sha256 `leaflet.js`: `db49d009c841f5ca34a888c96511ae936fd9f5533e90d8b2c4d57596f4e5641a`
  - sha256 `leaflet.css`: `a7837102824184820dfa198d1ebcd109ff6d0ff9a2672a074b9a1b4d147d04c6`

They are served from this folder, not from a CDN, so a compromised CDN cannot
inject code into a page that can send fences to a collar.
