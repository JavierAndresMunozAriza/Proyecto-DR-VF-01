/*
 * test_geofence_stress.c
 *
 * Deterministic randomized stress test for the geofence engine.
 *
 * A fixed seed (override: first argument) drives a xorshift generator, so
 * every failure is reproducible: the seed and the case context are printed
 * with each failure. Each check counts as one case. The process exits
 * non-zero if any check fails.
 *
 * Sections:
 *   1. Random convex and star-shaped polygons (some with a duplicated
 *      vertex): the three point-in-polygon algorithms agree, convex
 *      polygons match an independent half-plane reference, reversed and
 *      rotated vertex order give the same answers, distances are sane.
 *   2. Axis-aligned rectangles: the distance to the boundary matches an
 *      analytic reference and the haversine distance.
 *   3. Garbage input (NaN, infinities, exact and just-beyond limits, NULL,
 *      tiny polygons): no crash, and the documented error results.
 *   4. Vertex storage: growth, clear and destroy cycles.
 *   5. A polygon with the largest possible vertex count.
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

#define EARTH_RADIUS_METERS 6371000.0
#define METERS_PER_DEG_LAT (M_PI / 180.0 * EARTH_RADIUS_METERS)

/* Largest latitude of a generated center: the engine documents 85 degrees. */
#define MAX_TEST_LATITUDE 85.0
#define MAX_TEST_LONGITUDE 170.0

/* Tolerances */

/* Two distances that must be equal, in meters (rounding only). */
#define DISTANCE_TOLERANCE_M 1e-9
/* A point on an edge is at distance zero, up to the rounding of its generation. */
#define ON_EDGE_TOLERANCE_M 1e-6
/* Largest accepted gap between the flat distance and the haversine one. */
#define MAX_HAVERSINE_RELATIVE_ERROR 0.005
/* The edge distance never exceeds the vertex distance beyond this factor. */
#define VERTEX_DISTANCE_SLACK (1.0 + MAX_HAVERSINE_RELATIVE_ERROR)

static uint64_t g_seed;
static uint64_t g_state;
static unsigned long g_cases;
static unsigned long g_failures;

/* Where the current check is running, printed with every failure. */
static struct {
	const char *section;
	unsigned item;
	unsigned sub;
} g_context;

static void set_context(const char *section, unsigned item, unsigned sub)
{
	g_context.section = section;
	g_context.item = item;
	g_context.sub = sub;
}

static void report_failure(const char *file, int line, const char *what)
{
	g_failures++;
	if (g_failures <= MAX_REPORTED_FAILURES) {
		fprintf(stderr, "FAIL seed=0x%" PRIx64 " %s:%d: %s [%s %u/%u]\n",
			g_seed, file, line, what, g_context.section,
			g_context.item, g_context.sub);
	}
}

#define EXPECT(cond)                                                    \
	do {                                                            \
		g_cases++;                                              \
		if (!(cond)) {                                          \
			report_failure(__FILE__, __LINE__, #cond);      \
		}                                                       \
	} while (0)

/* Like EXPECT for a distance: the failure message carries both values. */
#define EXPECT_NEAR(actual, expected, tol)                                 \
	do {                                                               \
		const double a_ = (actual);                                \
		const double e_ = (expected);                              \
		g_cases++;                                                 \
		if (!(fabs(a_ - e_) <= (tol))) {                           \
			char what_[96];                                    \
			snprintf(what_, sizeof(what_), "%s = %g, expected %g", \
				 #actual, a_, e_);                         \
			report_failure(__FILE__, __LINE__, what_);         \
		}                                                          \
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

/* Helpers */

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

static bool coordinate_ok(const geopoint_t *p)
{
	return p->latitude >= -90.0 && p->latitude <= 90.0 &&
	       p->longitude >= -180.0 && p->longitude <= 180.0;
}

static double clamp(double v, double lo, double hi)
{
	return fmin(fmax(v, lo), hi);
}

/* Section 1: random polygons */

#define MAX_VERTS 24
#define POINTS_PER_POLYGON 40
#define POLYGON_COUNT 6000
/* Test points fall up to this many polygon sizes from the centroid. */
#define POINT_SPREAD 1.5
/* One polygon in DUPLICATE_ONE_IN has a vertex repeated (a zero-length edge). */
#define DUPLICATE_ONE_IN 10
/* One test point in ON_EDGE_ONE_IN is generated on an edge. */
#define ON_EDGE_ONE_IN 8

typedef struct {
	geopoint_t v[MAX_VERTS];
	uint16_t n;
	geopoint_t centroid;
	double size; /* largest distance from the centroid, per axis, in degrees */
} poly_t;

typedef struct {
	poly_t poly;
	bool convex;
	geofence_t plain;
	geofence_t reversed;
	geofence_t rotated;
} poly_case_t;

/*
 * Vertices on an ellipse around a random center, in counter-clockwise
 * order. A convex polygon keeps the full radius; a star-shaped one scales
 * each radius, which makes it concave. Every angular gap stays under 180
 * degrees, so the star-shaped polygon is simple.
 */
static void make_polygon(poly_t *p, bool star)
{
	const double center_lat = rng_range(-MAX_TEST_LATITUDE, MAX_TEST_LATITUDE);
	const double center_lon = rng_range(-MAX_TEST_LONGITUDE, MAX_TEST_LONGITUDE);
	const double radius = rng_range(0.0005, 0.05);
	const double scale_lat = rng_range(0.5, 1.5);
	const double scale_lon = rng_range(0.5, 1.5);
	const unsigned min_n = star ? 4 : 3;
	/* One slot is kept free for a possible duplicated vertex. */
	const unsigned ellipse_n = min_n + rng_below(MAX_VERTS - 1 - min_n + 1);
	unsigned i;

	p->n = (uint16_t)ellipse_n;
	for (i = 0; i < ellipse_n; i++) {
		const double angle =
			2.0 * M_PI * ((double)i + rng_range(0.2, 0.8)) / ellipse_n;
		const double r = star ? radius * rng_range(0.3, 1.0) : radius;

		p->v[i].latitude = center_lat + r * scale_lat * sin(angle);
		p->v[i].longitude = center_lon + r * scale_lon * cos(angle);
	}

	/* Repeat one vertex right after itself: a zero-length edge. */
	if (rng_below(DUPLICATE_ONE_IN) == 0) {
		const unsigned k = rng_below(p->n);

		memmove(&p->v[k + 1], &p->v[k], (p->n - k) * sizeof(p->v[0]));
		p->n++;
	}

	p->centroid.latitude = 0.0;
	p->centroid.longitude = 0.0;
	for (i = 0; i < p->n; i++) {
		p->centroid.latitude += p->v[i].latitude / p->n;
		p->centroid.longitude += p->v[i].longitude / p->n;
	}
	p->size = 0.0;
	for (i = 0; i < p->n; i++) {
		p->size = fmax(p->size, fabs(p->v[i].latitude - p->centroid.latitude));
		p->size = fmax(p->size, fabs(p->v[i].longitude - p->centroid.longitude));
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
 * every counter-clockwise edge (zero-length edges are skipped). Returns
 * false and leaves *inside unset when the point is too close to an edge
 * for the reference to be trusted.
 */
static bool convex_reference(const poly_t *p, const geopoint_t *pt, bool *inside)
{
	double min_signed = INFINITY;
	unsigned i;

	for (i = 0; i < p->n; i++) {
		const geopoint_t *a = &p->v[i];
		const geopoint_t *b = &p->v[(i + 1) % p->n];

		if (a->latitude == b->latitude && a->longitude == b->longitude) {
			continue;
		}
		min_signed = fmin(min_signed, signed_distance(a, b, pt));
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

	*on_edge = rng_below(ON_EDGE_ONE_IN) == 0;
	if (*on_edge) {
		const unsigned i = rng_below(p->n);
		const geopoint_t *a = &p->v[i];
		const geopoint_t *b = &p->v[(i + 1) % p->n];
		const double t = rng_unit();

		pt.latitude = a->latitude + t * (b->latitude - a->latitude);
		pt.longitude = a->longitude + t * (b->longitude - a->longitude);
	} else {
		pt.latitude = p->centroid.latitude +
			      rng_range(-POINT_SPREAD, POINT_SPREAD) * p->size;
		pt.longitude = p->centroid.longitude +
			       rng_range(-POINT_SPREAD, POINT_SPREAD) * p->size;
	}

	return pt;
}

static void make_case(poly_case_t *c, bool convex)
{
	geopoint_t tmp[MAX_VERTS];
	unsigned shift;
	unsigned i;

	make_polygon(&c->poly, !convex);
	c->convex = convex;
	c->plain = (geofence_t){ 0 };
	c->reversed = (geofence_t){ 0 };
	c->rotated = (geofence_t){ 0 };

	EXPECT(geofence_create_polygon(&c->plain, c->poly.v, c->poly.n) ==
	       GEOFENCE_SUCCESS);

	for (i = 0; i < c->poly.n; i++) {
		tmp[i] = c->poly.v[c->poly.n - 1 - i];
	}
	EXPECT(geofence_create_polygon(&c->reversed, tmp, c->poly.n) ==
	       GEOFENCE_SUCCESS);

	shift = 1 + rng_below(c->poly.n - 1);
	for (i = 0; i < c->poly.n; i++) {
		tmp[i] = c->poly.v[(i + shift) % c->poly.n];
	}
	EXPECT(geofence_create_polygon(&c->rotated, tmp, c->poly.n) ==
	       GEOFENCE_SUCCESS);
}

static void destroy_case(poly_case_t *c)
{
	geofence_destroy_polygon(&c->plain);
	geofence_destroy_polygon(&c->reversed);
	geofence_destroy_polygon(&c->rotated);
}

static void check_point(const poly_case_t *c)
{
	bool on_edge;
	const geopoint_t pt = random_point(&c->poly, &on_edge);
	geofence_result_t result = { false, -1.0 };
	geopoint_t closest;
	bool reference_inside = false;
	bool ray;
	double distance;
	double vertex_distance;

	if (!coordinate_ok(&pt)) {
		return;
	}

	ray = geofence_is_point_in_polygon_ray_casting(&pt, &c->plain);
	distance = geofence_distance_to_boundary(&pt, &c->plain);

	/*
	 * Away from the boundary the algorithms agree and the vertex order
	 * does not matter. A point generated "on an edge" is only within
	 * rounding of it, and each algorithm rounds differently, so it is
	 * not compared (the header tells callers not to rely on the
	 * boundary).
	 */
	if (!on_edge) {
		EXPECT(ray == geofence_is_point_in_polygon_winding_number(&pt, &c->plain));
		EXPECT(ray == geofence_is_point_in_polygon_optimized(&pt, &c->plain));
		EXPECT(ray == geofence_is_point_in_polygon_ray_casting(&pt, &c->reversed));
		EXPECT(ray == geofence_is_point_in_polygon_ray_casting(&pt, &c->rotated));
	}

	/* The distance does not depend on the vertex order either. */
	EXPECT_NEAR(geofence_distance_to_boundary(&pt, &c->reversed), distance,
		    ON_EDGE_TOLERANCE_M);
	EXPECT_NEAR(geofence_distance_to_boundary(&pt, &c->rotated), distance,
		    ON_EDGE_TOLERANCE_M);

	/* The distance is finite, never negative and never beyond the nearest vertex. */
	EXPECT(isfinite(distance) && distance >= 0.0);
	vertex_distance = geofence_get_closest_boundary_point(&pt, &c->plain, &closest);
	EXPECT(distance <= vertex_distance * VERTEX_DISTANCE_SLACK + ON_EDGE_TOLERANCE_M);

	/* A point generated on an edge is at distance zero. */
	if (on_edge) {
		EXPECT_NEAR(distance, 0.0, ON_EDGE_TOLERANCE_M);
	}

	/* geofence_check matches the standalone functions. */
	EXPECT(geofence_check(&pt, &c->plain, &result) == GEOFENCE_SUCCESS);
	EXPECT(result.inside == ray);
	EXPECT_NEAR(result.distance_m, distance, DISTANCE_TOLERANCE_M);

	/* Convex polygons: independent half-plane reference. */
	if (c->convex && convex_reference(&c->poly, &pt, &reference_inside)) {
		EXPECT(ray == reference_inside);
		EXPECT(distance > 0.0);
	}
}

static void stress_polygons(void)
{
	unsigned i;
	unsigned j;

	for (i = 0; i < POLYGON_COUNT; i++) {
		poly_case_t c;

		set_context("polygon", i, 0);
		make_case(&c, (i % 2) == 0);
		EXPECT(geofence_validate_polygon(&c.plain));

		for (j = 0; j < POINTS_PER_POLYGON; j++) {
			set_context("polygon", i, j);
			check_point(&c);
		}

		destroy_case(&c);
	}
}

/* Section 2: rectangles with an analytic distance */

#define RECTANGLE_COUNT 20000
#define POINTS_PER_RECTANGLE 10

static void stress_rectangles(double *worst_relative_error)
{
	unsigned i;
	unsigned j;

	for (i = 0; i < RECTANGLE_COUNT; i++) {
		const double south = rng_range(-MAX_TEST_LATITUDE, MAX_TEST_LATITUDE);
		const double west = rng_range(-MAX_TEST_LONGITUDE, MAX_TEST_LONGITUDE);
		const double height = rng_range(0.0005, 0.05);
		const double width = rng_range(0.0005, 0.05);
		const double north = south + height;
		const double east = west + width;
		const geopoint_t corners[4] = {
			{ south, west }, { south, east }, { north, east }, { north, west }
		};
		geofence_t fence = { 0 };

		set_context("rectangle", i, 0);
		EXPECT(geofence_create_polygon(&fence, corners, 4) == GEOFENCE_SUCCESS);

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

			set_context("rectangle", i, j);

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
			EXPECT_NEAR(distance, reference, ON_EDGE_TOLERANCE_M);

			/* Outside: also compare with the haversine distance to the nearest point. */
			if (outside && reference > 1.0) {
				const geopoint_t nearest = {
					clamp(pt.latitude, south, north),
					clamp(pt.longitude, west, east)
				};
				const double haversine =
					geofence_calculate_distance(&pt, &nearest);
				const double error = fabs(distance - haversine) / haversine;

				EXPECT(error < MAX_HAVERSINE_RELATIVE_ERROR);
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
/* Weight of ordinary valid values against the special ones below. */
#define GARBAGE_VALID_WEIGHT 12

/*
 * A coordinate that is often invalid or on a limit: special floating-point
 * values, the exact limit, just beyond it, or an ordinary valid value.
 */
static double garbage_value(double limit)
{
	static const double special[] = {
		NAN, INFINITY, -INFINITY, 1e308, -1e308, 0.0, -0.0, 1e-320, 1e-308
	};
	enum { EXACT, EXACT_NEGATIVE, BEYOND, BEYOND_NEGATIVE, LIMIT_KINDS };
	const unsigned pick =
		rng_below(ARRAY_SIZE(special) + LIMIT_KINDS + GARBAGE_VALID_WEIGHT);

	if (pick < ARRAY_SIZE(special)) {
		return special[pick];
	}

	switch (pick - ARRAY_SIZE(special)) {
	case EXACT:
		return limit;
	case EXACT_NEGATIVE:
		return -limit;
	case BEYOND:
		return limit * (1.0 + 1e-9);
	case BEYOND_NEGATIVE:
		return -limit * (1.0 + 1e-9);
	default:
		return rng_range(-limit, limit);
	}
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
		const geopoint_t *pp = &pt;
		bool point_ok;
		bool polygon_ok = count >= 3;
		bool ray;
		bool winding;
		unsigned v;
		int status;
		double distance;

		set_context("garbage", i, 0);

		for (v = 0; v < count; v++) {
			vertices[v].latitude = garbage_value(90.0);
			vertices[v].longitude = garbage_value(180.0);
			polygon_ok = polygon_ok && coordinate_ok(&vertices[v]);
		}
		pt.latitude = garbage_value(90.0);
		pt.longitude = garbage_value(180.0);
		if (null_point) {
			pp = NULL;
		}
		point_ok = pp && coordinate_ok(pp);

		EXPECT(geofence_create_polygon(&fence, vertices, (uint16_t)count) ==
		       GEOFENCE_SUCCESS);

		/* Every call must survive; the results below are the documented ones. */
		ray = geofence_is_point_in_polygon_ray_casting(pp, &fence);
		winding = geofence_is_point_in_polygon_winding_number(pp, &fence);
		distance = geofence_distance_to_boundary(pp, &fence);
		(void)geofence_get_closest_boundary_point(pp, &fence, &closest);
		(void)geofence_validate_polygon(&fence);
		status = geofence_check(pp, &fence, &result);

		if (!point_ok) {
			EXPECT(!ray && !winding);
			EXPECT(distance < 0.0);
			EXPECT(status == GEOFENCE_ERROR_INVALID_PARAM);
		} else if (count < 3) {
			EXPECT(!ray);
			EXPECT(distance < 0.0);
			EXPECT(status == GEOFENCE_ERROR_EMPTY_POLYGON);
		} else if (polygon_ok) {
			EXPECT(status == GEOFENCE_SUCCESS);
			EXPECT(isfinite(distance) && distance >= 0.0);
		} else {
			EXPECT(status == GEOFENCE_ERROR_INVALID_PARAM);
			EXPECT(distance < 0.0);
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

		set_context("storage", i, 0);

		for (v = 0; v < total; v++) {
			expected[v].latitude = rng_range(-90.0, 90.0);
			expected[v].longitude = rng_range(-180.0, 180.0);
		}

		EXPECT(geofence_create_polygon(&fence, initial ? expected : NULL,
					       (uint16_t)initial) == GEOFENCE_SUCCESS);
		for (v = initial; v < total; v++) {
			EXPECT(geofence_add_vertex(&fence, &expected[v]) ==
			       GEOFENCE_SUCCESS);
		}

		EXPECT(fence.num_vertices == total && fence.capacity >= total);
		EXPECT(total == 0 ||
		       memcmp(fence.vertices, expected, total * sizeof(expected[0])) == 0);

		/* Clearing keeps the storage and allows adding again. */
		geofence_clear_polygon(&fence);
		EXPECT(fence.num_vertices == 0);
		if (total > 0) {
			EXPECT(geofence_add_vertex(&fence, &expected[0]) ==
			       GEOFENCE_SUCCESS);
			EXPECT(fence.num_vertices == 1);
		}

		geofence_destroy_polygon(&fence);
		EXPECT(fence.vertices == NULL && fence.num_vertices == 0 &&
		       fence.capacity == 0);
		/* A second destroy is harmless. */
		geofence_destroy_polygon(&fence);
	}
}

/* Section 5: the largest possible polygon */

#define LARGE_VERTICES UINT16_MAX
#define LARGE_RADIUS_DEG 0.05
#define LARGE_CENTER_LAT 45.0

static void stress_large_polygon(void)
{
	static geopoint_t ring[LARGE_VERTICES];
	const geopoint_t center = { LARGE_CENTER_LAT, 10.0 };
	/*
	 * On the north-south axis, a fifth of the radius below the northern
	 * edge. The ring is narrower east-west than north-south in meters at
	 * 45 degrees, but this point is closer to the top than the radius of
	 * curvature there, so the nearest boundary point is the top itself.
	 */
	const geopoint_t inner = { LARGE_CENTER_LAT + 0.8 * LARGE_RADIUS_DEG, 10.0 };
	const geopoint_t outer = { LARGE_CENTER_LAT + 2.0 * LARGE_RADIUS_DEG, 10.0 };
	geofence_t fence = { 0 };
	geofence_result_t result;
	unsigned i;

	set_context("large polygon", 0, 0);

	for (i = 0; i < LARGE_VERTICES; i++) {
		const double angle = 2.0 * M_PI * i / LARGE_VERTICES;

		ring[i].latitude = center.latitude + LARGE_RADIUS_DEG * sin(angle);
		ring[i].longitude = center.longitude + LARGE_RADIUS_DEG * cos(angle);
	}

	EXPECT(geofence_create_polygon(&fence, ring, LARGE_VERTICES) == GEOFENCE_SUCCESS);
	EXPECT(geofence_validate_polygon(&fence));

	EXPECT(geofence_check(&center, &fence, &result) == GEOFENCE_SUCCESS &&
	       result.inside);
	EXPECT(geofence_check(&inner, &fence, &result) == GEOFENCE_SUCCESS &&
	       result.inside);
	EXPECT(geofence_check(&outer, &fence, &result) == GEOFENCE_SUCCESS &&
	       !result.inside);
	EXPECT_NEAR(geofence_distance_to_boundary(&inner, &fence),
		    0.2 * LARGE_RADIUS_DEG * METERS_PER_DEG_LAT, 1.0);
	EXPECT(geofence_is_point_in_polygon_ray_casting(&inner, &fence) ==
	       geofence_is_point_in_polygon_winding_number(&inner, &fence));

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
