/*
 * Host unit tests for bench_polygon_*(), checked against the real geofence
 * engine for the vertex counts the benchmark uses. Exits non-zero if any
 * check fails.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "bench_polygon.h"
#include "geofence.h"

#define RADIUS_M   500.0
#define MAX_N      1024
#define TEST_COUNT 40

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

static const geopoint_t center = { 4.6097, -74.0817 };
static geopoint_t vertices[MAX_N];

/* Build an n-gon, then check every test point is on the side it claims. */
static int polygon_classifies_points(unsigned int n)
{
	const geofence_config_t config = { 1, MAX_N, 0.0 };
	geofence_t fence = { 0 };
	int ok = 1;

	if (bench_polygon_vertices(&center, RADIUS_M, n, vertices) != 0 ||
	    geofence_init(&config) != GEOFENCE_SUCCESS ||
	    geofence_create_polygon(&fence, vertices, (uint16_t)n) != GEOFENCE_SUCCESS)
		return 0;

	for (unsigned int i = 0; i < TEST_COUNT && ok; i++) {
		const bool inside = (i % 2) == 0;
		geofence_result_t result;
		geopoint_t point;

		ok = bench_polygon_point(&center, RADIUS_M, i, TEST_COUNT, inside, &point) == 0 &&
		     geofence_check(&point, &fence, &result) == GEOFENCE_SUCCESS &&
		     result.inside == inside && result.distance_m > 1.0;
	}

	geofence_destroy_polygon(&fence);
	geofence_deinit();

	return ok;
}

int main(void)
{
	geopoint_t p;
	char name[64];
	const unsigned int counts[] = { 4, 16, 64, 256, 1024 };

	for (size_t i = 0; i < sizeof(counts) / sizeof(counts[0]); i++) {
		snprintf(name, sizeof(name), "%u vertices: inside and outside points classified",
			 counts[i]);
		CHECK(name, polygon_classifies_points(counts[i]));
	}

	CHECK("vertex 0 is due north of the center",
	      bench_polygon_vertices(&center, RADIUS_M, 8, vertices) == 0 &&
		      fabs(vertices[0].longitude - center.longitude) < 1e-9 &&
		      vertices[0].latitude > center.latitude);
	CHECK("vertex 2 of 8 is due east of the center",
	      fabs(vertices[2].latitude - center.latitude) < 1e-7 &&
		      vertices[2].longitude > center.longitude);

	CHECK("NULL center rejected for the vertices",
	      bench_polygon_vertices(NULL, RADIUS_M, 8, vertices) == -1);
	CHECK("NULL output rejected for the vertices",
	      bench_polygon_vertices(&center, RADIUS_M, 8, NULL) == -1);
	CHECK("zero radius rejected for the vertices",
	      bench_polygon_vertices(&center, 0.0, 8, vertices) == -1);
	CHECK("fewer than 4 vertices rejected",
	      bench_polygon_vertices(&center, RADIUS_M, 3, vertices) == -1);

	CHECK("NULL center rejected for a point",
	      bench_polygon_point(NULL, RADIUS_M, 0, 10, true, &p) == -1);
	CHECK("NULL output rejected for a point",
	      bench_polygon_point(&center, RADIUS_M, 0, 10, true, NULL) == -1);
	CHECK("zero radius rejected for a point",
	      bench_polygon_point(&center, 0.0, 0, 10, true, &p) == -1);
	CHECK("zero count rejected for a point",
	      bench_polygon_point(&center, RADIUS_M, 0, 0, true, &p) == -1);

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
