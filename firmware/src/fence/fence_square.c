/*
 * fence_square.c - square geofence polygon centered on a point
 *
 * Pure C, no hardware dependency. See fence_square.h.
 */

#include <math.h>
#include <stddef.h>

#include "fence_square.h"

#define METERS_PER_DEG_LAT 111195.0
#define MAX_LATITUDE_DEG   85.0
#define MAX_LONGITUDE_DEG  180.0
#define PI                 3.14159265358979323846
#define DEG_TO_RAD         (PI / 180.0)

int fence_square_vertices(const geopoint_t *center, double half_side_m,
			  geopoint_t out[FENCE_SQUARE_VERTICES])
{
	double dlat;
	double dlon;
	double south;
	double north;
	double west;
	double east;

	if (center == NULL || out == NULL)
		return -1;
	if (!isfinite(half_side_m) || half_side_m <= 0.0)
		return -1;
	if (!isfinite(center->latitude) || !isfinite(center->longitude))
		return -1;
	if (fabs(center->latitude) > MAX_LATITUDE_DEG ||
	    fabs(center->longitude) > MAX_LONGITUDE_DEG)
		return -1;

	/* A degree of longitude shrinks with the cosine of the latitude. */
	dlat = half_side_m / METERS_PER_DEG_LAT;
	dlon = dlat / cos(center->latitude * DEG_TO_RAD);

	south = center->latitude - dlat;
	north = center->latitude + dlat;
	west = center->longitude - dlon;
	east = center->longitude + dlon;

	/* The engine does not support the antimeridian. */
	if (west < -MAX_LONGITUDE_DEG || east > MAX_LONGITUDE_DEG)
		return -1;

	out[0] = (geopoint_t){ south, west };
	out[1] = (geopoint_t){ south, east };
	out[2] = (geopoint_t){ north, east };
	out[3] = (geopoint_t){ north, west };

	return 0;
}
