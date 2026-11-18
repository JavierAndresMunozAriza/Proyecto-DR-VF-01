/*
 * test_geofence_stress.c
 *
 * Deterministic randomized stress test for the geofence engine.
 *
 * A fixed seed (override: first argument) drives a xorshift generator, so
 * every failure is reproducible: the seed is printed with each failure.
 * Each check is counted as one case. The process exits non-zero if any
 * check fails.
 *
 * Sections:
 *   1. Random convex and star-shaped polygons: ray casting, winding number
 *      and the optimized wrapper agree; convex polygons match an
 *      independent half-plane reference; reversed and rotated vertex
 *      order give the same answers; distances are sane.
 *   2. Axis-aligned rectangles: the distance to the boundary matches an
 *      analytic reference and the haversine distance.
 *   3. Garbage input (NaN, infinities, out-of-range values, NULL, tiny
 *      polygons): no crash, and the documented error results.
 *   4. Vertex storage: growth, clear and destroy cycles.
 *   5. A very large polygon.
 */

#include "geofence.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_SEED 0x5EEDF00DCAFEBABEULL
#define MAX_REPORTED_FAILURES 20
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define METERS_PER_DEG_LAT (M_PI / 180.0 * 6371000.0)

static uint64_t g_seed;
static uint64_t g_state;
static unsigned long g_cases;
static unsigned long g_failures;

/*
 * Counts one case. The format string and its arguments describe the case
 * and are printed only on failure.
 */
#define EXPECT(cond, ...)                                                     \
	do {                                                                  \
		g_cases++;                                                    \
		if (!(cond)) {                                                \
			g_failures++;                                         \
			if (g_failures <= MAX_REPORTED_FAILURES) {            \
				fprintf(stderr,                               \
					"FAIL seed=0x%" PRIx64 " %s:%d: %s -- ", \
					g_seed, __FILE__, __LINE__, #cond);   \
				fprintf(stderr, __VA_ARGS__);                 \
				fputc('\n', stderr);                          \
			}                                                     \
		}                                                             \
	} while (0)

/* Random numbers (xorshift64*) */

static uint64_t rng_next(void)
{
	g_state ^= g_state >> 12;
	g_state ^= g_state << 25;
	g_state ^= g_state >> 27;
	return g_state * 0x2545F4914F6CDD1DULL;
}

static double rng_unit(void)
{
	return (double)(rng_next() >> 11) * (1.0 / 9007199254740992.0);
}

static double rng_range(double lo, double hi)
{
	return lo + (hi - lo) * rng_unit();
}

static unsigned rng_below(unsigned n)
{
	return (unsigned)(rng_next() % n);
}

/* Engine setup */

static void engine_up(void)
{
	const geofence_config_t config = { 4, 1000000, 1.0 };

	geofence_deinit();
	if (geofence_init(&config) != GEOFENCE_SUCCESS) {
		fprintf(stderr, "geofence_init failed\n");
		exit(EXIT_FAILURE);
	}
}

static double meters_per_deg_lon(double latitude)
{
	return METERS_PER_DEG_LAT * cos(latitude * M_PI / 180.0);
}

/* Section 1: random polygons */

#define MAX_VERTS 24
#define POINTS_PER_POLYGON 40
#define POLYGON_COUNT 6000
/* Test points fall up to this many polygon sizes from the centroid. */
#define POINT_SPREAD 1.5

typedef struct {
	geopoint_t v[MAX_VERTS];
	uint16_t n;
} poly_t;

/*
 * Vertices on an ellipse around a random center, in counter-clockwise
 * order. A convex polygon keeps the full radius; a star-shaped one scales
 * each radius, which makes it concave. Every angular gap stays under 180
 * degrees, so the star-shaped polygon is simple.
 */
static void make_polygon(poly_t *p, bool star)
{
	const double center_lat = rng_range(-60.0, 60.0);
	const double center_lon = rng_range(-170.0, 170.0);
	const double radius = rng_range(0.0005, 0.05);
	const double scale_lat = rng_range(0.5, 1.5);
	const double scale_lon = rng_range(0.5, 1.5);
	const unsigned min_n = star ? 4 : 3;
	unsigned i;

	p->n = (uint16_t)(min_n + rng_below(MAX_VERTS - min_n + 1));
	for (i = 0; i < p->n; i++) {
		const double angle =
			2.0 * M_PI * ((double)i + rng_range(0.2, 0.8)) / p->n;
		const double r = star ? radius * rng_range(0.3, 1.0) : radius;

		p->v[i].latitude = center_lat + r * scale_lat * sin(angle);
		p->v[i].longitude = center_lon + r * scale_lon * cos(angle);
	}
}

/* Signed distance, in degrees, from p to the line through a and b: > 0 on the left. */
static double signed_distance(const geopoint_t *a, const geopoint_t *b,
			      const geopoint_t *p)
{
	const double ex = b->longitude - a->longitude;
	const double ey = b->latitude - a->latitude;

	return (ex * (p->latitude - a->latitude) -
		ey * (p->longitude - a->longitude)) / hypot(ex, ey);
}

/*
 * Independent convex reference: inside when the point is on the left of
 * every counter-clockwise edge. Returns false and leaves *inside unset when
 * the point is too close to an edge for the reference to be trusted.
 */
static bool convex_reference(const poly_t *p, const geopoint_t *pt, bool *inside)
{
	double min_signed = INFINITY;
	unsigned i;

	for (i = 0; i < p->n; i++) {
		const double s = signed_distance(&p->v[i], &p->v[(i + 1) % p->n], pt);

		if (s < min_signed) {
			min_signed = s;
		}
	}

	if (fabs(min_signed) < 1e-9) {
		return false;
	}

	*inside = min_signed > 0.0;
	return true;
}

/* A random test point: around the polygon, or exactly on one of its edges. */
static geopoint_t random_point(const poly_t *p, bool *on_edge)
{
	geopoint_t pt;
	unsigned i;

	*on_edge = rng_below(8) == 0;
	if (*on_edge) {
		const geopoint_t *a;
		const geopoint_t *b;
		double t = rng_unit();

		i = rng_below(p->n);
		a = &p->v[i];
		b = &p->v[(i + 1) % p->n];
		pt.latitude = a->latitude + t * (b->latitude - a->latitude);
		pt.longitude = a->longitude + t * (b->longitude - a->longitude);
		return pt;
	}

	/* Around the vertex centroid, up to 1.5 polygon sizes away. */
	{
		double lat = 0.0;
		double lon = 0.0;
		double size = 0.0;

		for (i = 0; i < p->n; i++) {
			lat += p->v[i].latitude;
			lon += p->v[i].longitude;
		}
		lat /= p->n;
		lon /= p->n;
		for (i = 0; i < p->n; i++) {
			size = fmax(size, fabs(p->v[i].latitude - lat));
			size = fmax(size, fabs(p->v[i].longitude - lon));
		}

		pt.latitude = lat + rng_range(-POINT_SPREAD, POINT_SPREAD) * size;
		pt.longitude = lon + rng_range(-POINT_SPREAD, POINT_SPREAD) * size;
	}
	return pt;
}

static void make_variants(const poly_t *p, geofence_t *plain,
			  geofence_t *reversed, geofence_t *rotated)
{
	geopoint_t tmp[MAX_VERTS];
	unsigned shift = 1 + rng_below(p->n - 1);
	unsigned i;

	EXPECT(geofence_create_polygon(plain, p->v, p->n) == GEOFENCE_SUCCESS,
	       "create plain");

	for (i = 0; i < p->n; i++) {
		tmp[i] = p->v[p->n - 1 - i];
	}
	EXPECT(geofence_create_polygon(reversed, tmp, p->n) == GEOFENCE_SUCCESS,
	       "create reversed");

	for (i = 0; i < p->n; i++) {
		tmp[i] = p->v[(i + shift) % p->n];
	}
	EXPECT(geofence_create_polygon(rotated, tmp, p->n) == GEOFENCE_SUCCESS,
	       "create rotated");
}

static void check_point(const poly_t *p, bool convex, const geofence_t *plain,
			const geofence_t *reversed, const geofence_t *rotated,
			unsigned poly_index, unsigned point_index)
{
	bool on_edge;
	const geopoint_t pt = random_point(p, &on_edge);
	geofence_result_t result = { false, -1.0 };
	geopoint_t closest;
	bool reference_inside = false;
	bool ray;
	double distance;
	double vertex_distance;

	if (pt.latitude < -90.0 || pt.latitude > 90.0 ||
	    pt.longitude < -180.0 || pt.longitude > 180.0) {
		return;
	}

	ray = geofence_is_point_in_polygon_ray_casting(&pt, plain);
	distance = geofence_distance_to_boundary(&pt, plain);

	/*
	 * Away from the boundary the algorithms agree and the vertex order does
	 * not matter. A point generated "on an edge" is only within rounding of
	 * it, and each algorithm rounds differently, so it is not compared (the
	 * header tells callers not to rely on the boundary).
	 */
	if (!on_edge) {
		EXPECT(ray == geofence_is_point_in_polygon_winding_number(&pt, plain),
		       "ray vs winding, polygon %u point %u", poly_index, point_index);
		EXPECT(ray == geofence_is_point_in_polygon_optimized(&pt, plain),
		       "ray vs optimized, polygon %u point %u", poly_index, point_index);
		EXPECT(ray == geofence_is_point_in_polygon_ray_casting(&pt, reversed),
		       "reversed order, polygon %u point %u", poly_index, point_index);
		EXPECT(ray == geofence_is_point_in_polygon_ray_casting(&pt, rotated),
		       "rotated start, polygon %u point %u", poly_index, point_index);
	}
	/* The distance does not depend on the vertex order either. */
	EXPECT(fabs(distance - geofence_distance_to_boundary(&pt, reversed)) < 1e-6,
	       "distance reversed, polygon %u point %u", poly_index, point_index);
	EXPECT(fabs(distance - geofence_distance_to_boundary(&pt, rotated)) < 1e-6,
	       "distance rotated, polygon %u point %u", poly_index, point_index);

	/* The distance is finite, never negative and never beyond the nearest vertex. */
	EXPECT(isfinite(distance) && distance >= 0.0,
	       "distance %f, polygon %u point %u", distance, poly_index, point_index);
	vertex_distance = geofence_get_closest_boundary_point(&pt, plain, &closest);
	EXPECT(distance <= vertex_distance * 1.01 + 1e-6,
	       "edge %f > vertex %f, polygon %u point %u", distance,
	       vertex_distance, poly_index, point_index);

	/* A point generated on an edge is at distance zero. */
	if (on_edge) {
		EXPECT(distance < 1e-6, "on-edge distance %g, polygon %u point %u",
		       distance, poly_index, point_index);
	}

	/* geofence_check matches the standalone functions. */
	EXPECT(geofence_check(&pt, plain, &result) == GEOFENCE_SUCCESS,
	       "check, polygon %u point %u", poly_index, point_index);
	EXPECT(result.inside == ray && result.distance_m == distance,
	       "check result, polygon %u point %u", poly_index, point_index);

	/* Convex polygons: independent half-plane reference. */
	if (convex && convex_reference(p, &pt, &reference_inside)) {
		EXPECT(ray == reference_inside,
		       "convex reference, polygon %u point %u", poly_index,
		       point_index);
		EXPECT(distance > 0.0, "inside/outside with zero distance, polygon %u point %u",
		       poly_index, point_index);
	}
}

static void stress_polygons(void)
{
	unsigned i;
	unsigned j;

	for (i = 0; i < POLYGON_COUNT; i++) {
		const bool convex = (i % 2) == 0;
		poly_t poly;
		geofence_t plain = { 0 };
		geofence_t reversed = { 0 };
		geofence_t rotated = { 0 };

		make_polygon(&poly, !convex);
		make_variants(&poly, &plain, &reversed, &rotated);
		EXPECT(geofence_validate_polygon(&plain), "validate, polygon %u", i);

		for (j = 0; j < POINTS_PER_POLYGON; j++) {
			check_point(&poly, convex, &plain, &reversed, &rotated, i, j);
		}

		geofence_destroy_polygon(&plain);
		geofence_destroy_polygon(&reversed);
		geofence_destroy_polygon(&rotated);
	}
}

/* Section 2: rectangles with an analytic distance */

#define RECTANGLE_COUNT 20000
#define POINTS_PER_RECTANGLE 10
/* Largest accepted gap between the flat distance and the haversine one. */
#define MAX_HAVERSINE_RELATIVE_ERROR 0.005

static double clamp(double v, double lo, double hi)
{
	return fmin(fmax(v, lo), hi);
}

static void stress_rectangles(double *worst_relative_error)
{
	unsigned i;
	unsigned j;

	for (i = 0; i < RECTANGLE_COUNT; i++) {
		const double south = rng_range(-60.0, 60.0);
		const double west = rng_range(-170.0, 170.0);
		const double height = rng_range(0.0005, 0.05);
		const double width = rng_range(0.0005, 0.05);
		const double north = south + height;
		const double east = west + width;
		const geopoint_t corners[4] = {
			{ south, west }, { south, east }, { north, east }, { north, west }
		};
		geofence_t fence = { 0 };

		EXPECT(geofence_create_polygon(&fence, corners, 4) == GEOFENCE_SUCCESS,
		       "create rectangle %u", i);

		for (j = 0; j < POINTS_PER_RECTANGLE; j++) {
			const geopoint_t pt = {
				south + rng_range(-1.0, 2.0) * height,
				west + rng_range(-1.0, 2.0) * width
			};
			const double mlon = meters_per_deg_lon(pt.latitude);
			const double dx = fmax(fmax(west - pt.longitude, 0.0),
					       pt.longitude - east);
			const double dy = fmax(fmax(south - pt.latitude, 0.0),
					       pt.latitude - north);
			const bool outside = dx > 0.0 || dy > 0.0;
			double reference;
			double distance;

			if (outside) {
				reference = hypot(dx * mlon, dy * METERS_PER_DEG_LAT);
			} else {
				reference = fmin(fmin(pt.longitude - west,
						      east - pt.longitude) * mlon,
						 fmin(pt.latitude - south,
						      north - pt.latitude) *
							 METERS_PER_DEG_LAT);
			}

			distance = geofence_distance_to_boundary(&pt, &fence);
			EXPECT(fabs(distance - reference) < 1e-6,
			       "rectangle %u point %u: %f vs %f", i, j, distance,
			       reference);

			/* Outside: also compare with the haversine distance to the nearest point. */
			if (outside && reference > 1.0) {
				const geopoint_t nearest = {
					clamp(pt.latitude, south, north),
					clamp(pt.longitude, west, east)
				};
				const double haversine =
					geofence_calculate_distance(&pt, &nearest);
				const double error = fabs(distance - haversine) / haversine;

				EXPECT(error < MAX_HAVERSINE_RELATIVE_ERROR,
				       "haversine gap %g, rectangle %u point %u", error, i, j);
				if (error > *worst_relative_error) {
					*worst_relative_error = error;
				}
			}
		}

		geofence_destroy_polygon(&fence);
	}
}

/* Section 3: garbage input */

#define GARBAGE_COUNT 100000
#define GARBAGE_VERTICES 6

static double garbage_value(double valid_limit)
{
	static const double special[] = {
		NAN, INFINITY, -INFINITY, 1e308, -1e308, 0.0, -0.0,
		1e-320, 1e-308
	};
	const unsigned pick = rng_below(24);

	if (pick < ARRAY_SIZE(special)) {
		return special[pick];
	}
	if (pick == 9) {
		return valid_limit * (1.0 + 1e-9);
	}
	if (pick == 10) {
		return -valid_limit * (1.0 + 1e-9);
	}
	return rng_range(-valid_limit, valid_limit);
}

static bool coordinate_ok(const geopoint_t *p)
{
	return p->latitude >= -90.0 && p->latitude <= 90.0 &&
	       p->longitude >= -180.0 && p->longitude <= 180.0;
}

static void stress_garbage(void)
{
	unsigned i;

	for (i = 0; i < GARBAGE_COUNT; i++) {
		geopoint_t vertices[GARBAGE_VERTICES];
		geopoint_t pt;
		geopoint_t closest;
		geofence_t fence = { 0 };
		geofence_result_t result = { false, -1.0 };
		const unsigned count = rng_below(GARBAGE_VERTICES + 1);
		const bool null_point = rng_below(20) == 0;
		bool point_ok;
		bool polygon_ok = count >= 3;
		unsigned v;
		int status;
		double distance;

		for (v = 0; v < count; v++) {
			vertices[v].latitude = garbage_value(90.0);
			vertices[v].longitude = garbage_value(180.0);
			polygon_ok = polygon_ok && coordinate_ok(&vertices[v]);
		}
		pt.latitude = garbage_value(90.0);
		pt.longitude = garbage_value(180.0);
		point_ok = !null_point && coordinate_ok(&pt);

		EXPECT(geofence_create_polygon(&fence, vertices, (uint16_t)count) ==
			       GEOFENCE_SUCCESS,
		       "create garbage polygon %u", i);

		/* Every call must survive; the results below are the documented ones. */
		(void)geofence_is_point_in_polygon_ray_casting(null_point ? NULL : &pt, &fence);
		(void)geofence_is_point_in_polygon_winding_number(null_point ? NULL : &pt, &fence);
		distance = geofence_distance_to_boundary(null_point ? NULL : &pt, &fence);
		(void)geofence_get_closest_boundary_point(null_point ? NULL : &pt, &fence, &closest);
		(void)geofence_validate_polygon(&fence);
		status = geofence_check(null_point ? NULL : &pt, &fence, &result);

		if (!point_ok) {
			EXPECT(!geofence_is_point_in_polygon_ray_casting(null_point ? NULL : &pt, &fence),
			       "invalid point inside, case %u", i);
			EXPECT(!geofence_is_point_in_polygon_winding_number(null_point ? NULL : &pt, &fence),
			       "invalid point inside (winding), case %u", i);
			EXPECT(distance < 0.0, "invalid point distance, case %u", i);
			EXPECT(status == GEOFENCE_ERROR_INVALID_PARAM,
			       "invalid point status %d, case %u", status, i);
		} else if (count < 3) {
			EXPECT(!geofence_is_point_in_polygon_ray_casting(&pt, &fence),
			       "tiny polygon inside, case %u", i);
			EXPECT(distance < 0.0, "tiny polygon distance, case %u", i);
			EXPECT(status == GEOFENCE_ERROR_EMPTY_POLYGON,
			       "tiny polygon status %d, case %u", status, i);
		} else if (polygon_ok) {
			EXPECT(status == GEOFENCE_SUCCESS,
			       "valid input status %d, case %u", status, i);
			EXPECT(isfinite(distance) && distance >= 0.0,
			       "valid input distance %g, case %u", distance, i);
		} else {
			EXPECT(status == GEOFENCE_ERROR_INVALID_PARAM,
			       "invalid vertex status %d, case %u", status, i);
			EXPECT(distance < 0.0, "invalid vertex distance, case %u", i);
		}

		geofence_destroy_polygon(&fence);
	}
}

/* Section 4: vertex storage */

#define STORAGE_CYCLES 3000
#define STORAGE_MAX_VERTICES 300

static void stress_storage(void)
{
	static geopoint_t expected[STORAGE_MAX_VERTICES];
	unsigned i;

	for (i = 0; i < STORAGE_CYCLES; i++) {
		geofence_t fence = { 0 };
		const unsigned total = rng_below(STORAGE_MAX_VERTICES + 1);
		const unsigned initial = total == 0 ? 0 : rng_below(total + 1);
		unsigned v;

		for (v = 0; v < total; v++) {
			expected[v].latitude = rng_range(-90.0, 90.0);
			expected[v].longitude = rng_range(-180.0, 180.0);
		}

		EXPECT(geofence_create_polygon(&fence, initial ? expected : NULL,
					       (uint16_t)initial) == GEOFENCE_SUCCESS,
		       "create cycle %u", i);
		for (v = initial; v < total; v++) {
			EXPECT(geofence_add_vertex(&fence, &expected[v]) ==
				       GEOFENCE_SUCCESS,
			       "add vertex %u cycle %u", v, i);
		}

		EXPECT(fence.num_vertices == total && fence.capacity >= total,
		       "count %u capacity %u, cycle %u", (unsigned)fence.num_vertices,
		       (unsigned)fence.capacity, i);
		EXPECT(total == 0 ||
			       memcmp(fence.vertices, expected,
				      total * sizeof(expected[0])) == 0,
		       "vertex contents, cycle %u", i);

		/* Clearing keeps the storage and allows adding again. */
		geofence_clear_polygon(&fence);
		EXPECT(fence.num_vertices == 0, "clear, cycle %u", i);
		if (total > 0) {
			EXPECT(geofence_add_vertex(&fence, &expected[0]) ==
				       GEOFENCE_SUCCESS,
			       "add after clear, cycle %u", i);
			EXPECT(fence.num_vertices == 1, "count after clear, cycle %u", i);
		}

		geofence_destroy_polygon(&fence);
		EXPECT(fence.vertices == NULL && fence.num_vertices == 0 &&
			       fence.capacity == 0,
		       "destroy, cycle %u", i);
		/* A second destroy is harmless. */
		geofence_destroy_polygon(&fence);
	}
}

/* Section 5: a very large polygon */

#define LARGE_VERTICES 60000
#define LARGE_RADIUS_DEG 0.05

static void stress_large_polygon(void)
{
	static geopoint_t ring[LARGE_VERTICES];
	const geopoint_t center = { 45.0, 10.0 };
	const geopoint_t inner = { 45.0 + 0.5 * LARGE_RADIUS_DEG, 10.0 };
	const geopoint_t outer = { 45.0 + 2.0 * LARGE_RADIUS_DEG, 10.0 };
	geofence_t fence = { 0 };
	geofence_result_t result;
	unsigned i;

	for (i = 0; i < LARGE_VERTICES; i++) {
		const double angle = 2.0 * M_PI * i / LARGE_VERTICES;

		ring[i].latitude = center.latitude + LARGE_RADIUS_DEG * sin(angle);
		ring[i].longitude = center.longitude + LARGE_RADIUS_DEG * cos(angle);
	}

	EXPECT(geofence_create_polygon(&fence, ring, LARGE_VERTICES) == GEOFENCE_SUCCESS,
	       "create large polygon");
	EXPECT(geofence_validate_polygon(&fence), "validate large polygon");

	EXPECT(geofence_check(&center, &fence, &result) == GEOFENCE_SUCCESS &&
		       result.inside,
	       "large polygon center");
	EXPECT(geofence_check(&inner, &fence, &result) == GEOFENCE_SUCCESS &&
		       result.inside,
	       "large polygon inner point");
	EXPECT(geofence_check(&outer, &fence, &result) == GEOFENCE_SUCCESS &&
		       !result.inside,
	       "large polygon outer point");
	/* The ring is wider east-west than north-south in meters at 45 degrees. */
	EXPECT(fabs(geofence_distance_to_boundary(&inner, &fence) -
		    0.5 * LARGE_RADIUS_DEG * METERS_PER_DEG_LAT) < 1.0,
	       "large polygon inner distance");
	EXPECT(geofence_is_point_in_polygon_ray_casting(&inner, &fence) ==
		       geofence_is_point_in_polygon_winding_number(&inner, &fence),
	       "large polygon algorithms");

	geofence_destroy_polygon(&fence);
}

int main(int argc, char **argv)
{
	double worst_relative_error = 0.0;
	clock_t start;

	g_seed = argc > 1 ? strtoull(argv[1], NULL, 0) : DEFAULT_SEED;
	if (g_seed == 0) {
		g_seed = DEFAULT_SEED;
	}
	g_state = g_seed;

	engine_up();
	start = clock();

	stress_polygons();
	stress_rectangles(&worst_relative_error);
	stress_garbage();
	stress_storage();
	stress_large_polygon();

	geofence_deinit();

	printf("seed 0x%" PRIx64 ": %lu cases, %lu failed, worst flat-vs-haversine error %.4f%%, %.2f s\n",
	       g_seed, g_cases, g_failures, worst_relative_error * 100.0,
	       (double)(clock() - start) / CLOCKS_PER_SEC);
	return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
