/*
 * bench_polygon.c - regular polygons and test points for the geofence benchmark
 *
 * Pure C, no hardware dependency. See bench_polygon.h.
 */

#include <math.h>
#include <stddef.h>

#include "bench_polygon.h"
#include "geo_constants.h"

#define FULL_TURN          (2.0 * GEO_PI)
#define INSIDE_RADIUS_FRACTION  0.4 /* of the circumradius: inside even for 4 vertices */
#define OUTSIDE_RADIUS_FRACTION 1.5 /* beyond every vertex */

/* The point at a bearing (radians from north) and distance (meters) from the center. */
static geopoint_t offset_point(const geopoint_t *center, double bearing_rad, double distance_m)
{
	const double north_m = distance_m * cos(bearing_rad);
	const double east_m = distance_m * sin(bearing_rad);
	geopoint_t point;

	point.latitude = center->latitude + north_m / GEO_METERS_PER_DEG_LAT;
	point.longitude = center->longitude +
			  east_m / (GEO_METERS_PER_DEG_LAT * cos(center->latitude * GEO_DEG_TO_RAD));

	return point;
}

int bench_polygon_vertices(const geopoint_t *center, double radius_m, unsigned int n,
			   geopoint_t *out)
{
	if (center == NULL || out == NULL || !(radius_m > 0.0) ||
	    n < BENCH_POLYGON_MIN_VERTICES)
		return -1;

	for (unsigned int i = 0; i < n; i++)
		out[i] = offset_point(center, FULL_TURN * i / n, radius_m);

	return 0;
}

int bench_polygon_point(const geopoint_t *center, double radius_m, unsigned int index,
			unsigned int count, bool inside, geopoint_t *out)
{
	const double fraction = inside ? INSIDE_RADIUS_FRACTION : OUTSIDE_RADIUS_FRACTION;

	if (center == NULL || out == NULL || !(radius_m > 0.0) || count == 0)
		return -1;

	*out = offset_point(center, FULL_TURN * index / count, radius_m * fraction);

	return 0;
}
