/*
 * geofence.c
 *
 * Point-in-polygon engine for the cattle geofencing collar DR-VF-01.
 *
 * Pure C: no ESP-IDF, Zephyr or FreeRTOS dependency, so the engine builds
 * and is tested on the host. See geofence.h for the contract and the known
 * limitations.
 */

#include "geofence.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Constants */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EARTH_RADIUS_METERS 6371000.0
#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)

#define MAX_LATITUDE_DEG 90.0
#define MAX_LONGITUDE_DEG 180.0

/* Value returned by the distance functions when the input is invalid. */
#define DISTANCE_ERROR (-1.0)

/* Fewest vertices that enclose an area, and that form a boundary. */
#define MIN_POLYGON_VERTICES 3
#define MIN_BOUNDARY_VERTICES 2

/* Vertex storage growth used by geofence_add_vertex(). */
#define INITIAL_CAPACITY 8
#define CAPACITY_GROWTH_FACTOR 2

/* Global state */

static bool g_initialized;
/* Validated and stored by geofence_init(); the limits are not enforced yet. */
static geofence_config_t g_config;

/* Internal helper functions */

static bool is_valid_coordinate(double lat, double lon)
{
	return lat >= -MAX_LATITUDE_DEG && lat <= MAX_LATITUDE_DEG &&
	       lon >= -MAX_LONGITUDE_DEG && lon <= MAX_LONGITUDE_DEG;
}

/*
 * Common precondition of the point-in-polygon queries. False means "not
 * inside" for every caller (the API cannot report errors, see "Known
 * limitations" in the header).
 */
static bool can_query_point(const geopoint_t *point, const geofence_t *fence)
{
	return g_initialized && point && fence && fence->vertices &&
	       fence->num_vertices >= MIN_POLYGON_VERTICES &&
	       is_valid_coordinate(point->latitude, point->longitude);
}

/*
 * Cross product of (A - O) and (B - O), with longitude as x and latitude
 * as y. Positive when B is to the left of the directed line O -> A.
 */
static double cross_product(const geopoint_t *o, const geopoint_t *a,
			    const geopoint_t *b)
{
	return (a->longitude - o->longitude) * (b->latitude - o->latitude) -
	       (a->latitude - o->latitude) * (b->longitude - o->longitude);
}

/*
 * Make room for one more vertex. Doubles the capacity (first allocation:
 * INITIAL_CAPACITY) and clamps it to the 16-bit vertex counter.
 */
static int grow_capacity(geofence_t *fence)
{
	uint32_t new_capacity;
	geopoint_t *grown;

	if (fence->capacity >= UINT16_MAX) {
		return GEOFENCE_ERROR_TOO_MANY_VERTICES;
	}

	new_capacity = fence->capacity == 0 ?
		       INITIAL_CAPACITY :
		       (uint32_t)fence->capacity * CAPACITY_GROWTH_FACTOR;
	if (new_capacity > UINT16_MAX) {
		new_capacity = UINT16_MAX;
	}

	grown = realloc(fence->vertices, new_capacity * sizeof(*grown));
	if (!grown) {
		return GEOFENCE_ERROR_OUT_OF_MEMORY;
	}

	fence->vertices = grown;
	fence->capacity = (uint16_t)new_capacity;
	return GEOFENCE_SUCCESS;
}

/* Public API */

int geofence_init(const geofence_config_t *config)
{
	if (!config || g_initialized) {
		return GEOFENCE_ERROR_INVALID_PARAM;
	}

	if (config->max_polygons == 0 || config->max_vertices_total == 0) {
		return GEOFENCE_ERROR_INVALID_PARAM;
	}

	g_config = *config;
	g_initialized = true;

	return GEOFENCE_SUCCESS;
}

void geofence_deinit(void)
{
	memset(&g_config, 0, sizeof(g_config));
	g_initialized = false;
}

int geofence_create_polygon(geofence_t *fence, const geopoint_t *vertices,
			    uint16_t num_vertices)
{
	if (!g_initialized) {
		return GEOFENCE_ERROR_NOT_INITIALIZED;
	}

	if (!fence) {
		return GEOFENCE_ERROR_INVALID_PARAM;
	}

	/* Always leave an empty, destroyable structure, even on error. */
	memset(fence, 0, sizeof(*fence));

	if (num_vertices == 0) {
		return GEOFENCE_SUCCESS;
	}

	if (!vertices) {
		return GEOFENCE_ERROR_INVALID_PARAM;
	}

	fence->vertices = malloc(num_vertices * sizeof(*fence->vertices));
	if (!fence->vertices) {
		return GEOFENCE_ERROR_OUT_OF_MEMORY;
	}

	memcpy(fence->vertices, vertices,
	       num_vertices * sizeof(*fence->vertices));
	fence->num_vertices = num_vertices;
	fence->capacity = num_vertices;

	return GEOFENCE_SUCCESS;
}

int geofence_add_vertex(geofence_t *fence, const geopoint_t *point)
{
	int ret;

	if (!g_initialized || !fence || !point) {
		return GEOFENCE_ERROR_INVALID_PARAM;
	}

	if (!is_valid_coordinate(point->latitude, point->longitude)) {
		return GEOFENCE_ERROR_INVALID_PARAM;
	}

	if (fence->num_vertices >= fence->capacity) {
		ret = grow_capacity(fence);
		if (ret != GEOFENCE_SUCCESS) {
			return ret;
		}
	}

	fence->vertices[fence->num_vertices] = *point;
	fence->num_vertices++;

	return GEOFENCE_SUCCESS;
}

void geofence_clear_polygon(geofence_t *fence)
{
	if (!fence) {
		return;
	}

	fence->num_vertices = 0;
}

void geofence_destroy_polygon(geofence_t *fence)
{
	if (!fence) {
		return;
	}

	free(fence->vertices);
	fence->vertices = NULL;
	fence->num_vertices = 0;
	fence->capacity = 0;
}

bool geofence_validate_polygon(const geofence_t *fence)
{
	uint16_t i;

	if (!fence || !fence->vertices ||
	    fence->num_vertices < MIN_POLYGON_VERTICES) {
		return false;
	}

	for (i = 0; i < fence->num_vertices; i++) {
		if (!is_valid_coordinate(fence->vertices[i].latitude,
					 fence->vertices[i].longitude)) {
			return false;
		}
	}

	return true;
}

/*
 * Ray casting: the ray runs toward increasing longitude and an edge counts
 * when the point's latitude is in the half-open span [lower, upper) of the
 * edge. That half-open span is what makes the south and west boundary
 * inside and the north and east boundary outside (see geofence.h).
 */
bool geofence_is_point_in_polygon_ray_casting(const geopoint_t *point,
					      const geofence_t *fence)
{
	int crossings = 0;
	uint16_t vertex_count;
	uint16_t i;

	if (!can_query_point(point, fence)) {
		return false;
	}

	vertex_count = fence->num_vertices;

	for (i = 0; i < vertex_count; i++) {
		const geopoint_t *v1 = &fence->vertices[i];
		const geopoint_t *v2 = &fence->vertices[(i + 1) % vertex_count];

		if (((v1->latitude > point->latitude) !=
		     (v2->latitude > point->latitude)) &&
		    (point->longitude <
		     (v2->longitude - v1->longitude) *
				     (point->latitude - v1->latitude) /
				     (v2->latitude - v1->latitude) +
			     v1->longitude)) {
			crossings++;
		}
	}

	return (crossings % 2) == 1;
}

/*
 * Winding number: signed edge crossings of the same ray, with the same
 * half-open latitude span as ray casting. Non-zero means inside.
 */
bool geofence_is_point_in_polygon_winding_number(const geopoint_t *point,
						 const geofence_t *fence)
{
	int winding_number = 0;
	uint16_t vertex_count;
	uint16_t i;

	if (!can_query_point(point, fence)) {
		return false;
	}

	vertex_count = fence->num_vertices;

	for (i = 0; i < vertex_count; i++) {
		const geopoint_t *v1 = &fence->vertices[i];
		const geopoint_t *v2 = &fence->vertices[(i + 1) % vertex_count];

		if (v1->latitude <= point->latitude) {
			/* Upward crossing, point to the left of the edge. */
			if (v2->latitude > point->latitude &&
			    cross_product(v1, v2, point) > 0) {
				winding_number++;
			}
		} else {
			/* Downward crossing, point to the right of the edge. */
			if (v2->latitude <= point->latitude &&
			    cross_product(v1, v2, point) < 0) {
				winding_number--;
			}
		}
	}

	return winding_number != 0;
}

bool geofence_is_point_in_polygon_optimized(const geopoint_t *point,
					    const geofence_t *fence)
{
	return geofence_is_point_in_polygon_ray_casting(point, fence);
}

double geofence_calculate_distance(const geopoint_t *point1,
				   const geopoint_t *point2)
{
	double lat1_rad;
	double lat2_rad;
	double delta_lat_rad;
	double delta_lon_rad;
	double a;
	double c;

	if (!point1 || !point2) {
		return DISTANCE_ERROR;
	}

	if (!is_valid_coordinate(point1->latitude, point1->longitude) ||
	    !is_valid_coordinate(point2->latitude, point2->longitude)) {
		return DISTANCE_ERROR;
	}

	/* Haversine formula for great-circle distance. */
	lat1_rad = DEG_TO_RAD(point1->latitude);
	lat2_rad = DEG_TO_RAD(point2->latitude);
	delta_lat_rad = DEG_TO_RAD(point2->latitude - point1->latitude);
	delta_lon_rad = DEG_TO_RAD(point2->longitude - point1->longitude);

	a = sin(delta_lat_rad / 2) * sin(delta_lat_rad / 2) +
	    cos(lat1_rad) * cos(lat2_rad) * sin(delta_lon_rad / 2) *
		    sin(delta_lon_rad / 2);

	/* Rounding can push a just outside [0, 1] near the antipode, which would give NaN. */
	if (a < 0.0) {
		a = 0.0;
	} else if (a > 1.0) {
		a = 1.0;
	}

	c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return EARTH_RADIUS_METERS * c;
}

double geofence_get_closest_boundary_point(const geopoint_t *point,
					   const geofence_t *fence,
					   geopoint_t *closest_point)
{
	double min_distance = DISTANCE_ERROR;
	geopoint_t best_point = { 0.0, 0.0 };
	uint16_t i;

	if (!g_initialized || !point || !fence || !closest_point ||
	    !fence->vertices || fence->num_vertices < MIN_BOUNDARY_VERTICES) {
		return DISTANCE_ERROR;
	}

	/* Vertices only: the distance to the edges is not computed. */
	for (i = 0; i < fence->num_vertices; i++) {
		double distance =
			geofence_calculate_distance(point, &fence->vertices[i]);

		if (distance < 0.0) {
			return DISTANCE_ERROR; /* invalid point or vertex */
		}

		if (min_distance < 0.0 || distance < min_distance) {
			min_distance = distance;
			best_point = fence->vertices[i];
		}
	}

	*closest_point = best_point;
	return min_distance;
}
