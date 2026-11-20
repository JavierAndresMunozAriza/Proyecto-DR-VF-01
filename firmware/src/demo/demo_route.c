/*
 * demo_route.c - simulated walk used by the demo mode
 *
 * Pure C, no hardware dependency. See demo_route.h.
 */

#include <stddef.h>

#include "demo_route.h"
#include "geo_constants.h"

#define STEP_M            2
#define TURN_STEP          (DEMO_ROUTE_STEPS / 2) /* farthest point of the walk */

int demo_route_point(const geopoint_t *center, unsigned int step, geopoint_t *out)
{
	unsigned int phase;
	unsigned int steps_north;

	if (center == NULL || out == NULL)
		return -1;

	/* Triangle wave: north for TURN_STEP steps, then back to the center. */
	phase = step % DEMO_ROUTE_STEPS;
	steps_north = phase <= TURN_STEP ? phase : DEMO_ROUTE_STEPS - phase;

	out->latitude = center->latitude + (steps_north * STEP_M) / GEO_METERS_PER_DEG_LAT;
	out->longitude = center->longitude;

	return 0;
}
