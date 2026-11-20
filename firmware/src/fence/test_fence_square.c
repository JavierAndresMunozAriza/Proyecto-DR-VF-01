/*
 * Host unit tests for fence_square_vertices(), checked end to end against
 * the real geofence engine. Exits non-zero if any check fails.
 */

#include <math.h>
#include <stdio.h>

#include "fence_square.h"
#include "geofence.h"

#define METERS_PER_DEG_LAT 111195.0
#define BOGOTA_LAT         4.6097
#define BOGOTA_LON         (-74.0817)
#define HALF_SIDE_M        30.0

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

static geopoint_t north_of(geopoint_t p, double meters)
{
	p.latitude += meters / METERS_PER_DEG_LAT;
	return p;
}

int main(void)
{
	const geopoint_t center = { BOGOTA_LAT, BOGOTA_LON };
	const geofence_config_t config = { 1, FENCE_SQUARE_VERTICES, 0.0 };
	geopoint_t corners[FENCE_SQUARE_VERTICES];
	geofence_t fence = { 0 };
	geofence_result_t result;
	geopoint_t bad = center;

	CHECK("valid square returns 0",
	      fence_square_vertices(&center, HALF_SIDE_M, corners) == 0);

	/* Build the engine polygon from the corners and ask the engine. */
	CHECK("engine init", geofence_init(&config) == GEOFENCE_SUCCESS);
	CHECK("engine accepts the corners",
	      geofence_create_polygon(&fence, corners, FENCE_SQUARE_VERTICES) ==
		      GEOFENCE_SUCCESS);

	CHECK("center is inside",
	      geofence_check(&center, &fence, &result) == GEOFENCE_SUCCESS &&
		      result.inside);
	CHECK("center is half_side from the edge",
	      fabs(result.distance_m - HALF_SIDE_M) < 0.3);

	const geopoint_t near_edge = north_of(center, 25.0);

	CHECK("25 m north is inside, 5 m from the edge",
	      geofence_check(&near_edge, &fence, &result) == GEOFENCE_SUCCESS &&
		      result.inside && fabs(result.distance_m - 5.0) < 0.3);

	const geopoint_t beyond = north_of(center, 40.0);

	CHECK("40 m north is outside, 10 m from the edge",
	      geofence_check(&beyond, &fence, &result) == GEOFENCE_SUCCESS &&
		      !result.inside && fabs(result.distance_m - 10.0) < 0.3);

	geofence_destroy_polygon(&fence);
	geofence_deinit();

	CHECK("NULL center rejected",
	      fence_square_vertices(NULL, HALF_SIDE_M, corners) == -1);
	CHECK("NULL output rejected",
	      fence_square_vertices(&center, HALF_SIDE_M, NULL) == -1);
	CHECK("zero half side rejected",
	      fence_square_vertices(&center, 0.0, corners) == -1);
	CHECK("negative half side rejected",
	      fence_square_vertices(&center, -5.0, corners) == -1);
	CHECK("NaN half side rejected",
	      fence_square_vertices(&center, NAN, corners) == -1);

	bad.latitude = NAN;
	CHECK("NaN latitude rejected",
	      fence_square_vertices(&bad, HALF_SIDE_M, corners) == -1);

	bad = center;
	bad.latitude = 86.0;
	CHECK("latitude beyond 85 degrees rejected",
	      fence_square_vertices(&bad, HALF_SIDE_M, corners) == -1);

	bad = center;
	bad.longitude = 179.9999;
	CHECK("square crossing the antimeridian rejected",
	      fence_square_vertices(&bad, HALF_SIDE_M, corners) == -1);

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
