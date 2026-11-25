/*
 * Host unit tests for the fence codec. Exits non-zero if any check fails.
 *
 * The shared vectors in test_vectors.txt come from gen_vectors.py, an
 * independent Python implementation of the format, so passing them proves the
 * C codec and the Python (and browser) codecs agree byte for byte.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fence_codec.h"
#include "geo_constants.h"
#include "geofence.h"

#define LAT0 4.6097
#define LON0 (-74.0817)

static int failures;

#define CHECK(name, cond)                                  \
	do {                                               \
		if (cond) {                                \
			printf("ok   %s\n", name);         \
		} else {                                   \
			printf("FAIL %s\n", name);         \
			failures++;                        \
		}                                          \
	} while (0)

static geopoint_t pt(double dlat_m, double dlon_m)
{
	geopoint_t p;

	p.latitude = LAT0 + dlat_m / GEO_METERS_PER_DEG_LAT;
	p.longitude = LON0 + dlon_m / (GEO_METERS_PER_DEG_LAT *
				       cos(LAT0 * GEO_DEG_TO_RAD));
	return p;
}

static int status_from_name(const char *name)
{
	static const struct {
		const char *name;
		int status;
	} map[] = {
		{ "OK", FENCE_CODEC_OK },
		{ "SIZE", FENCE_CODEC_ERR_SIZE },
		{ "CRC", FENCE_CODEC_ERR_CRC },
		{ "FORMAT", FENCE_CODEC_ERR_FORMAT },
		{ "VERTEX_COUNT", FENCE_CODEC_ERR_VERTEX_COUNT },
		{ "RANGE", FENCE_CODEC_ERR_RANGE },
		{ "NOT_SIMPLE", FENCE_CODEC_ERR_NOT_SIMPLE },
		{ "AREA", FENCE_CODEC_ERR_AREA },
	};

	for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
		if (strcmp(map[i].name, name) == 0)
			return map[i].status;
	}
	return 1; /* not a status: the vector file is wrong */
}

static size_t unhex(const char *hex, uint8_t *out, size_t cap)
{
	size_t n = strlen(hex) / 2;

	if (n > cap)
		return 0;
	for (size_t i = 0; i < n; i++) {
		unsigned int byte;

		if (sscanf(hex + 2 * i, "%2x", &byte) != 1)
			return 0;
		out[i] = (uint8_t)byte;
	}
	return n;
}

static void test_crc_and_size(void)
{
	const uint8_t check[] = "123456789";

	CHECK("crc32 of 123456789 is 0xCBF43926",
	      fence_codec_crc32(check, 9) == 0xCBF43926u);
	CHECK("crc32 of nothing is 0", fence_codec_crc32(check, 0) == 0u);
	CHECK("size with 3 vertices is the minimum",
	      fence_codec_encoded_size(3) == FENCE_CODEC_MIN_SIZE);
	CHECK("size with 128 vertices is 532",
	      fence_codec_encoded_size(128) == 532 &&
	      FENCE_CODEC_MAX_SIZE == 532);
}

static void test_round_trip_and_engine(void)
{
	const geopoint_t square[4] = {
		pt(-30, -30), pt(-30, 30), pt(30, 30), pt(30, -30)
	};
	const fence_codec_header_t in = { 42, 9, 0 };
	fence_codec_header_t out_header;
	geopoint_t decoded[FENCE_CODEC_MAX_VERTICES];
	uint8_t blob[FENCE_CODEC_MAX_SIZE];
	size_t len = 0;
	geofence_t fence = { 0 };
	geofence_result_t res;
	geofence_config_t config = { 1, FENCE_CODEC_MAX_VERTICES, 0.0 };
	geopoint_t center;
	geopoint_t far_away;
	int rc;
	int close_enough = 1;

	rc = fence_codec_encode(&in, square, 4, blob, sizeof(blob), &len);
	CHECK("encode a 60 m square", rc == FENCE_CODEC_OK && len == 36);
	rc = fence_codec_decode(blob, len, decoded, FENCE_CODEC_MAX_VERTICES,
				&out_header);
	CHECK("decode what was encoded", rc == FENCE_CODEC_OK);
	CHECK("header fields survive",
	      out_header.fence_id == 42 && out_header.fence_version == 9 &&
	      out_header.num_vertices == 4);
	for (int i = 0; i < 4; i++) {
		/* 5.5 cm in degrees of latitude, plus a little margin */
		if (fabs(decoded[i].latitude - square[i].latitude) > 6e-7 ||
		    fabs(decoded[i].longitude - square[i].longitude) > 6e-7)
			close_enough = 0;
	}
	CHECK("each vertex is within 6 cm of the requested one", close_enough);

	/* End to end: a decoded fence works in the real engine. */
	CHECK("engine initializes", geofence_init(&config) == 0);
	CHECK("engine accepts the decoded polygon",
	      geofence_create_polygon(&fence, decoded, 4) == 0 &&
	      geofence_validate_polygon(&fence));
	center = pt(0, 0);
	far_away = pt(100, 0);
	CHECK("the center is inside",
	      geofence_check(&center, &fence, &res) == 0 && res.inside);
	CHECK("a point 100 m away is outside",
	      geofence_check(&far_away, &fence, &res) == 0 && !res.inside);
	geofence_destroy_polygon(&fence);
	geofence_deinit();
}

static void test_encode_refusals(void)
{
	const geopoint_t square[4] = {
		pt(-30, -30), pt(-30, 30), pt(30, 30), pt(30, -30)
	};
	const geopoint_t bowtie[4] = {
		pt(-30, -30), pt(-30, 30), pt(30, -30), pt(30, 30)
	};
	const geopoint_t too_long[4] = {
		pt(-30, -30), pt(-30, 4000), pt(30, 4000), pt(30, -30)
	};
	const fence_codec_header_t h = { 1, 1, 0 };
	uint8_t blob[FENCE_CODEC_MAX_SIZE];
	size_t len = 123;

	CHECK("encode refuses NULL header",
	      fence_codec_encode(NULL, square, 4, blob, sizeof(blob), &len) ==
	      FENCE_CODEC_ERR_ARG);
	CHECK("encode refuses NULL vertices",
	      fence_codec_encode(&h, NULL, 4, blob, sizeof(blob), &len) ==
	      FENCE_CODEC_ERR_ARG);
	CHECK("encode refuses a buffer that is too small",
	      fence_codec_encode(&h, square, 4, blob, 35, &len) ==
	      FENCE_CODEC_ERR_BUFFER);
	CHECK("encode refuses 2 vertices",
	      fence_codec_encode(&h, square, 2, blob, sizeof(blob), &len) ==
	      FENCE_CODEC_ERR_VERTEX_COUNT);
	CHECK("encode refuses a self-intersecting polygon",
	      fence_codec_encode(&h, bowtie, 4, blob, sizeof(blob), &len) ==
	      FENCE_CODEC_ERR_NOT_SIMPLE);
	CHECK("encode refuses an edge over 3.6 km",
	      fence_codec_encode(&h, too_long, 4, blob, sizeof(blob), &len) ==
	      FENCE_CODEC_ERR_EDGE_TOO_LONG);
	CHECK("a refused encode leaves out_len untouched", len == 123);
}

static void test_decode_arguments(void)
{
	uint8_t blob[FENCE_CODEC_MAX_SIZE];
	geopoint_t out[FENCE_CODEC_MAX_VERTICES];
	fence_codec_header_t h;
	char line[2 * FENCE_CODEC_MAX_SIZE + 128];
	size_t len = 0;
	FILE *f = fopen("test_vectors.txt", "r");

	CHECK("vector file opens (run make from this folder)", f != NULL);
	if (f == NULL)
		return;
	while (fgets(line, sizeof(line), f) != NULL) {
		char name[64];
		char expect[32];
		char hex[2 * FENCE_CODEC_MAX_SIZE + 64];

		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf(line, "%63s %31s %s", name, expect, hex) == 3 &&
		    strcmp(name, "square_ccw") == 0) {
			len = unhex(hex, blob, sizeof(blob));
			break;
		}
	}
	fclose(f);
	CHECK("the square vector was read", len == 36);

	CHECK("decode refuses NULL buffer",
	      fence_codec_decode(NULL, len, out, 4, &h) == FENCE_CODEC_ERR_ARG);
	CHECK("decode refuses NULL output",
	      fence_codec_decode(blob, len, NULL, 4, &h) == FENCE_CODEC_ERR_ARG);
	CHECK("decode refuses NULL header",
	      fence_codec_decode(blob, len, out, 4, NULL) == FENCE_CODEC_ERR_ARG);
	CHECK("decode refuses an output that is too small",
	      fence_codec_decode(blob, len, out, 3, &h) == FENCE_CODEC_ERR_BUFFER);
}

static void test_vectors(void)
{
	FILE *f = fopen("test_vectors.txt", "r");
	char line[2 * FENCE_CODEC_MAX_SIZE + 256];
	int count = 0;

	CHECK("vector file opens", f != NULL);
	if (f == NULL)
		return;
	while (fgets(line, sizeof(line), f) != NULL) {
		char name[64];
		char expect[32];
		char hex[2 * FENCE_CODEC_MAX_SIZE + 128];
		uint8_t blob[FENCE_CODEC_MAX_SIZE + 8];
		geopoint_t out[FENCE_CODEC_MAX_VERTICES];
		uint8_t again[FENCE_CODEC_MAX_SIZE];
		fence_codec_header_t h = { 0, 0, 0 };
		size_t len;
		size_t again_len = 0;
		int want;
		int got;
		char label[160];

		if (line[0] == '#' || line[0] == '\n')
			continue;
		if (sscanf(line, "%63s %31s %s", name, expect, hex) != 3) {
			CHECK("vector line parses", 0);
			continue;
		}
		count++;
		want = status_from_name(expect);
		len = unhex(hex, blob, sizeof(blob));
		got = fence_codec_decode(blob, len, out,
					 FENCE_CODEC_MAX_VERTICES, &h);
		snprintf(label, sizeof(label), "vector %s -> %s", name, expect);
		CHECK(label, got == want);
		if (want != FENCE_CODEC_OK)
			continue;

		/* The C encoder must reproduce the Python bytes exactly. */
		snprintf(label, sizeof(label), "vector %s re-encodes byte for byte",
			 name);
		CHECK(label,
		      fence_codec_encode(&h, out, h.num_vertices, again,
					 sizeof(again), &again_len) ==
		      FENCE_CODEC_OK &&
		      again_len == len && memcmp(again, blob, len) == 0);
	}
	fclose(f);
	CHECK("at least 15 vectors were run", count >= 15);
}

static void test_failed_decode_writes_nothing(void)
{
	/* "bowtie" fails late, after the vertices are known. */
	FILE *f = fopen("test_vectors.txt", "r");
	char line[2 * FENCE_CODEC_MAX_SIZE + 256];
	geopoint_t out[FENCE_CODEC_MAX_VERTICES];
	fence_codec_header_t h = { 11, 22, 33 };
	int checked = 0;

	if (f == NULL)
		return;
	memset(out, 0x5A, sizeof(out));
	while (fgets(line, sizeof(line), f) != NULL) {
		char name[64];
		char expect[32];
		char hex[2 * FENCE_CODEC_MAX_SIZE + 128];
		uint8_t blob[FENCE_CODEC_MAX_SIZE + 8];
		uint8_t pattern[sizeof(out)];
		size_t len;

		if (sscanf(line, "%63s %31s %s", name, expect, hex) != 3 ||
		    strcmp(name, "bowtie") != 0)
			continue;
		len = unhex(hex, blob, sizeof(blob));
		memset(pattern, 0x5A, sizeof(pattern));
		CHECK("bowtie is refused",
		      fence_codec_decode(blob, len, out,
					 FENCE_CODEC_MAX_VERTICES, &h) ==
		      FENCE_CODEC_ERR_NOT_SIMPLE);
		CHECK("a refused decode leaves the output untouched",
		      memcmp(out, pattern, sizeof(out)) == 0);
		CHECK("a refused decode leaves the header untouched",
		      h.fence_id == 11 && h.fence_version == 22 &&
		      h.num_vertices == 33);
		checked = 1;
	}
	fclose(f);
	CHECK("the bowtie vector was found", checked);
}

int main(void)
{
	test_crc_and_size();
	test_round_trip_and_engine();
	test_encode_refusals();
	test_decode_arguments();
	test_vectors();
	test_failed_decode_writes_nothing();

	if (failures != 0) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
