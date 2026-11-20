/*
 * Host unit tests for demo_route_point(), checked against the real geofence
 * engine with the same 60 m square fence the application uses.
 * Exits non-zero if any check fails.
 */

#include <math.h>
#include <stdio.h>

#include "demo_route.h"
#include "fence_square.h"
#include "geofence.h"

#define HALF_SIDE_M 30.0
#define TOLERANCE_M 0.3

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
static geofence_t fence;

/* Check the walker at a step: inside or outside, and the distance to the edge. */
static int at_step(unsigned int step, bool want_inside, double want_distance_m)
{
	geopoint_t point;
	geofence_result_t result;

	if (demo_route_point(&center, step, &point) != 0)
		return 0;
	if (geofence_check(&point, &fence, &result) != GEOFENCE_SUCCESS)
		return 0;

	return result.inside == want_inside &&
	       fabs(result.distance_m - want_distance_m) < TOLERANCE_M;
}

int main(void)
{
	const geofence_config_t config = { 1, FENCE_SQUARE_VERTICES, 0.0 };
	geopoint_t corners[FENCE_SQUARE_VERTICES];
	geopoint_t a;
	geopoint_t b;

	CHECK("square fence", fence_square_vertices(&center, HALF_SIDE_M, corners) == 0);
	CHECK("engine init", geofence_init(&config) == GEOFENCE_SUCCESS);
	CHECK("engine accepts the corners",
	      geofence_create_polygon(&fence, corners, FENCE_SQUARE_VERTICES) ==
		      GEOFENCE_SUCCESS);

	CHECK("step 0 starts at the center, 30 m from the edge", at_step(0, true, 30.0));
	CHECK("step 10 is 20 m north, 10 m from the edge", at_step(10, true, 10.0));
	CHECK("step 14 is 28 m north, 2 m from the edge", at_step(14, true, 2.0));
	CHECK("step 16 is 32 m north, outside by 2 m", at_step(16, false, 2.0));
	CHECK("step 20 is 40 m north, outside by 10 m", at_step(20, false, 10.0));
	CHECK("step 30 walks back, 20 m north, 10 m inside", at_step(30, true, 10.0));
	CHECK("step 40 is back at the center", at_step(40, true, 30.0));
	CHECK("the route repeats: step 60 equals step 20", at_step(60, false, 10.0));

	CHECK("round trip length", DEMO_ROUTE_STEPS == 40);
	CHECK("NULL center rejected", demo_route_point(NULL, 0, &a) == -1);
	CHECK("NULL output rejected", demo_route_point(&center, 0, NULL) == -1);

	CHECK("steps 0 and 40 give the same point",
	      demo_route_point(&center, 0, &a) == 0 &&
		      demo_route_point(&center, DEMO_ROUTE_STEPS, &b) == 0 &&
		      a.latitude == b.latitude && a.longitude == b.longitude);

	geofence_destroy_polygon(&fence);
	geofence_deinit();

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
