/*
 * fence_codec.c - wire format and validation of a geofence polygon
 *
 * Pure C, no hardware dependency. See fence_codec.h for the blob layout.
 *
 * All geometry checks use exact integer arithmetic on the coordinates in
 * 1e-7 degrees. Whether two segments cross does not change when each axis is
 * scaled, so the checks need no projection to meters. Differences between
 * two vertices stay below 4.3e7 units (127 edges of at most 327680 units), so
 * every product fits in an int64_t.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "fence_codec.h"
#include "geo_constants.h"

/* Blob field offsets, see the layout in fence_codec.h */
#define OFF_VERSION        0
#define OFF_FLAGS          1
#define OFF_COUNT          2
#define OFF_FENCE_ID       4
#define OFF_FENCE_VERSION  8
#define OFF_LAT0           12
#define OFF_LON0           16
#define OFF_DELTAS         20

#define HEADER_SIZE        OFF_LAT0
#define FIRST_VERTEX_SIZE  (OFF_DELTAS - OFF_LAT0)
#define CRC_SIZE           4
#define DELTA_SIZE         4
#define MIN_LEN            (OFF_DELTAS + CRC_SIZE)

#define UNITS_PER_DEG      1e7
#define MAX_LAT_DEG        85.0    /* the engine is accurate up to here */
#define MAX_LON_DEG        180.0   /* no antimeridian support */
#define MAX_LAT_UNITS      ((int64_t)(MAX_LAT_DEG * UNITS_PER_DEG))
#define MAX_LON_UNITS      ((int64_t)(MAX_LON_DEG * UNITS_PER_DEG))

struct ipt {
	int32_t lat;
	int32_t lon;
};

static uint16_t rd16(const uint8_t *p)
{
	return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t rd32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
	       ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void wr16(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t)(v & 0xFF);
	p[1] = (uint8_t)(v >> 8);
}

static void wr32(uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)(v & 0xFF);
	p[1] = (uint8_t)((v >> 8) & 0xFF);
	p[2] = (uint8_t)((v >> 16) & 0xFF);
	p[3] = (uint8_t)(v >> 24);
}

uint32_t fence_codec_crc32(const uint8_t *data, size_t len)
{
	uint32_t crc = 0xFFFFFFFFu;

	for (size_t i = 0; i < len; i++) {
		crc ^= data[i];
		for (int bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
	}
	return ~crc;
}

size_t fence_codec_encoded_size(uint16_t n)
{
	if (n == 0)
		return 0;
	return HEADER_SIZE + FIRST_VERTEX_SIZE + DELTA_SIZE * (size_t)(n - 1) +
	       CRC_SIZE;
}

/* Integer division by 10 rounding half away from zero. */
static int64_t rdiv_unit(int64_t a)
{
	const int64_t half = FENCE_CODEC_DELTA_UNIT / 2;

	if (a >= 0)
		return (a + half) / FENCE_CODEC_DELTA_UNIT;
	return -((-a + half) / FENCE_CODEC_DELTA_UNIT);
}

static bool in_range(int64_t lat, int64_t lon)
{
	return lat >= -MAX_LAT_UNITS && lat <= MAX_LAT_UNITS &&
	       lon >= -MAX_LON_UNITS && lon <= MAX_LON_UNITS;
}

static int sign64(int64_t v)
{
	return (v > 0) - (v < 0);
}

/* Twice the signed area of the triangle a, b, c (positive: counterclockwise) */
static int64_t cross(const struct ipt *a, const struct ipt *b,
		     const struct ipt *c)
{
	return ((int64_t)b->lat - a->lat) * ((int64_t)c->lon - a->lon) -
	       ((int64_t)b->lon - a->lon) * ((int64_t)c->lat - a->lat);
}

static bool same_point(const struct ipt *a, const struct ipt *b)
{
	return a->lat == b->lat && a->lon == b->lon;
}

/* p is on the line through a and b; is it inside the segment's bounding box? */
static bool in_box(const struct ipt *a, const struct ipt *b,
		   const struct ipt *p)
{
	return p->lat >= (a->lat < b->lat ? a->lat : b->lat) &&
	       p->lat <= (a->lat > b->lat ? a->lat : b->lat) &&
	       p->lon >= (a->lon < b->lon ? a->lon : b->lon) &&
	       p->lon <= (a->lon > b->lon ? a->lon : b->lon);
}

/* True if the closed segments p1-p2 and p3-p4 share at least one point. */
static bool segments_touch(const struct ipt *p1, const struct ipt *p2,
			   const struct ipt *p3, const struct ipt *p4)
{
	const int d1 = sign64(cross(p3, p4, p1));
	const int d2 = sign64(cross(p3, p4, p2));
	const int d3 = sign64(cross(p1, p2, p3));
	const int d4 = sign64(cross(p1, p2, p4));

	if (d1 * d2 < 0 && d3 * d4 < 0)
		return true;
	if (d1 == 0 && in_box(p3, p4, p1))
		return true;
	if (d2 == 0 && in_box(p3, p4, p2))
		return true;
	if (d3 == 0 && in_box(p1, p2, p3))
		return true;
	if (d4 == 0 && in_box(p1, p2, p4))
		return true;
	return false;
}

static bool is_simple(const struct ipt *v, size_t n)
{
	/* No zero-length edge, and no spike where two edges overlap. */
	for (size_t i = 0; i < n; i++) {
		const struct ipt *q = &v[i];
		const struct ipt *a = &v[(i + n - 1) % n];
		const struct ipt *b = &v[(i + 1) % n];
		const struct ipt u = { a->lat - q->lat, a->lon - q->lon };
		const struct ipt w = { b->lat - q->lat, b->lon - q->lon };
		const struct ipt zero = { 0, 0 };

		if (same_point(q, b))
			return false;
		if (cross(&zero, &u, &w) == 0 &&
		    (int64_t)u.lat * w.lat + (int64_t)u.lon * w.lon > 0)
			return false;
	}

	/* No two edges that are not neighbors may touch. */
	for (size_t i = 0; i < n; i++) {
		for (size_t j = i + 2; j < n; j++) {
			if (i == 0 && j == n - 1)
				continue; /* neighbors through the closing edge */
			if (segments_touch(&v[i], &v[i + 1], &v[j],
					   &v[(j + 1) % n]))
				return false;
		}
	}
	return true;
}

static double area_m2(const struct ipt *v, size_t n)
{
	int64_t twice = 0;
	double lat0_rad;
	double m_per_unit;

	for (size_t i = 1; i + 1 < n; i++)
		twice += cross(&v[0], &v[i], &v[i + 1]);
	if (twice < 0)
		twice = -twice;

	lat0_rad = ((double)v[0].lat / UNITS_PER_DEG) * GEO_DEG_TO_RAD;
	m_per_unit = GEO_METERS_PER_DEG_LAT / UNITS_PER_DEG;
	return ((double)twice / 2.0) * m_per_unit * m_per_unit * cos(lat0_rad);
}

/* Shape checks shared by encode and decode; v is already in range. */
static int check_polygon(const struct ipt *v, size_t n)
{
	if (!is_simple(v, n))
		return FENCE_CODEC_ERR_NOT_SIMPLE;
	if (area_m2(v, n) < FENCE_CODEC_MIN_AREA_M2)
		return FENCE_CODEC_ERR_AREA;
	return FENCE_CODEC_OK;
}

int fence_codec_encode(const fence_codec_header_t *header,
		       const geopoint_t *vertices, uint16_t n,
		       uint8_t *buf, size_t buf_len, size_t *out_len)
{
	struct ipt dec[FENCE_CODEC_MAX_VERTICES];
	int16_t dlat[FENCE_CODEC_MAX_VERTICES];
	int16_t dlon[FENCE_CODEC_MAX_VERTICES];
	size_t total;
	size_t pos;
	int rc;

	if (header == NULL || vertices == NULL || buf == NULL ||
	    out_len == NULL)
		return FENCE_CODEC_ERR_ARG;
	if (n < FENCE_CODEC_MIN_VERTICES || n > FENCE_CODEC_MAX_VERTICES)
		return FENCE_CODEC_ERR_VERTEX_COUNT;
	total = fence_codec_encoded_size(n);
	if (buf_len < total)
		return FENCE_CODEC_ERR_BUFFER;

	for (size_t i = 0; i < n; i++) {
		int64_t lat;
		int64_t lon;

		if (!isfinite(vertices[i].latitude) ||
		    !isfinite(vertices[i].longitude) ||
		    fabs(vertices[i].latitude) > MAX_LAT_DEG ||
		    fabs(vertices[i].longitude) > MAX_LON_DEG)
			return FENCE_CODEC_ERR_RANGE;
		lat = llround(vertices[i].latitude * UNITS_PER_DEG);
		lon = llround(vertices[i].longitude * UNITS_PER_DEG);

		if (i > 0) {
			/* Deltas are taken from the decoded neighbor. */
			const int64_t a = rdiv_unit(lat - dec[i - 1].lat);
			const int64_t b = rdiv_unit(lon - dec[i - 1].lon);

			if (a < INT16_MIN || a > INT16_MAX ||
			    b < INT16_MIN || b > INT16_MAX)
				return FENCE_CODEC_ERR_EDGE_TOO_LONG;
			dlat[i] = (int16_t)a;
			dlon[i] = (int16_t)b;
			lat = dec[i - 1].lat + a * FENCE_CODEC_DELTA_UNIT;
			lon = dec[i - 1].lon + b * FENCE_CODEC_DELTA_UNIT;
		}
		if (!in_range(lat, lon))
			return FENCE_CODEC_ERR_RANGE;
		dec[i].lat = (int32_t)lat;
		dec[i].lon = (int32_t)lon;
	}

	rc = check_polygon(dec, n);
	if (rc != FENCE_CODEC_OK)
		return rc;

	buf[OFF_VERSION] = FENCE_CODEC_VERSION;
	buf[OFF_FLAGS] = 0;
	wr16(buf + OFF_COUNT, n);
	wr32(buf + OFF_FENCE_ID, header->fence_id);
	wr32(buf + OFF_FENCE_VERSION, header->fence_version);
	wr32(buf + OFF_LAT0, (uint32_t)dec[0].lat);
	wr32(buf + OFF_LON0, (uint32_t)dec[0].lon);
	pos = OFF_DELTAS;
	for (size_t i = 1; i < n; i++) {
		wr16(buf + pos, (uint16_t)dlat[i]);
		wr16(buf + pos + 2, (uint16_t)dlon[i]);
		pos += DELTA_SIZE;
	}
	wr32(buf + pos, fence_codec_crc32(buf, pos));
	*out_len = total;
	return FENCE_CODEC_OK;
}

int fence_codec_decode(const uint8_t *buf, size_t len,
		       geopoint_t *out, size_t out_cap,
		       fence_codec_header_t *header)
{
	struct ipt v[FENCE_CODEC_MAX_VERTICES];
	size_t pos;
	uint16_t n;
	int rc;

	if (buf == NULL || out == NULL || header == NULL)
		return FENCE_CODEC_ERR_ARG;
	if (len < MIN_LEN)
		return FENCE_CODEC_ERR_SIZE;
	if (buf[OFF_VERSION] != FENCE_CODEC_VERSION || buf[OFF_FLAGS] != 0)
		return FENCE_CODEC_ERR_FORMAT;
	n = rd16(buf + OFF_COUNT);
	if (n < FENCE_CODEC_MIN_VERTICES || n > FENCE_CODEC_MAX_VERTICES)
		return FENCE_CODEC_ERR_VERTEX_COUNT;
	if (len != fence_codec_encoded_size(n))
		return FENCE_CODEC_ERR_SIZE;
	if (rd32(buf + len - CRC_SIZE) !=
	    fence_codec_crc32(buf, len - CRC_SIZE))
		return FENCE_CODEC_ERR_CRC;
	if (out_cap < n)
		return FENCE_CODEC_ERR_BUFFER;

	v[0].lat = (int32_t)rd32(buf + OFF_LAT0);
	v[0].lon = (int32_t)rd32(buf + OFF_LON0);
	if (!in_range(v[0].lat, v[0].lon))
		return FENCE_CODEC_ERR_RANGE;
	pos = OFF_DELTAS;
	for (size_t i = 1; i < n; i++) {
		const int64_t lat = (int64_t)v[i - 1].lat +
			(int64_t)(int16_t)rd16(buf + pos) *
			FENCE_CODEC_DELTA_UNIT;
		const int64_t lon = (int64_t)v[i - 1].lon +
			(int64_t)(int16_t)rd16(buf + pos + 2) *
			FENCE_CODEC_DELTA_UNIT;

		if (!in_range(lat, lon))
			return FENCE_CODEC_ERR_RANGE;
		v[i].lat = (int32_t)lat;
		v[i].lon = (int32_t)lon;
		pos += DELTA_SIZE;
	}

	rc = check_polygon(v, n);
	if (rc != FENCE_CODEC_OK)
		return rc;

	for (size_t i = 0; i < n; i++) {
		out[i].latitude = (double)v[i].lat / UNITS_PER_DEG;
		out[i].longitude = (double)v[i].lon / UNITS_PER_DEG;
	}
	header->fence_id = rd32(buf + OFF_FENCE_ID);
	header->fence_version = rd32(buf + OFF_FENCE_VERSION);
	header->num_vertices = n;
	return FENCE_CODEC_OK;
}
