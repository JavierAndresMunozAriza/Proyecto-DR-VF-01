/*
 * test_geofence.c
 *
 * Host test suite for the geofence engine (single test binary).
 *
 * Every check asserts; the process exits non-zero if any check fails.
 * Host timing says nothing about the target MCU, so the only timing
 * output is a printed measurement and it is never asserted (RF-02, the
 * 50 ms budget, must be measured on the target hardware).
 */

#include "geofence.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Test harness */

static int g_checks;
static int g_failures;

#define CHECK(cond)                                                    \
	do {                                                           \
		g_checks++;                                            \
		if (!(cond)) {                                         \
			g_failures++;                                  \
			fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__,  \
				__LINE__, #cond);                      \
		}                                                      \
	} while (0)

#define CHECK_EQ(actual, expected)                                     \
	do {                                                           \
		long long a_ = (long long)(actual);                    \
		long long e_ = (long long)(expected);                  \
		g_checks++;                                            \
		if (a_ != e_) {                                        \
			g_failures++;                                  \
			fprintf(stderr,                                \
				"FAIL %s:%d: %s == %lld, expected %lld\n", \
				__FILE__, __LINE__, #actual, a_, e_);  \
		}                                                      \
	} while (0)

#define CHECK_NEAR(actual, expected, tol)                              \
	do {                                                           \
		double a_ = (actual);                                  \
		double e_ = (expected);                                \
		g_checks++;                                            \
		if (!(fabs(a_ - e_) <= (tol))) {                       \
			g_failures++;                                  \
			fprintf(stderr,                                \
				"FAIL %s:%d: %s == %f, expected %f\n", \
				__FILE__, __LINE__, #actual, a_, e_);  \
		}                                                      \
	} while (0)

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Fixtures */

/* Rectangular field: lat 40.7128..40.7188, lon -74.0060..-74.0000. */
static const geopoint_t test_field[] = {
	{ 40.7128, -74.0060 }, /* Southwest corner */
	{ 40.7128, -74.0000 }, /* Southeast corner */
	{ 40.7188, -74.0000 }, /* Northeast corner */
	{ 40.7188, -74.0060 }  /* Northwest corner */
};

/* Same rectangle, clockwise order. */
static const geopoint_t test_field_cw[] = {
	{ 40.7128, -74.0060 },
	{ 40.7188, -74.0060 },
	{ 40.7188, -74.0000 },
	{ 40.7128, -74.0000 }
};

/* Concave L shape: the notch is the square lat 1..2, lon 1..2. */
static const geopoint_t l_shape[] = {
	{ 0.0, 0.0 }, { 0.0, 2.0 }, { 1.0, 2.0 },
	{ 1.0, 1.0 }, { 2.0, 1.0 }, { 2.0, 0.0 }
};

#define TEST_FIELD_VERTICES ((uint16_t)ARRAY_SIZE(test_field))

typedef struct {
	geopoint_t point;
	bool expected_inside;
	const char *description;
} point_case_t;

static const point_case_t region_cases[] = {
	{ { 40.7158, -74.0030 }, true,  "inside, centre" },
	{ { 40.7100, -74.0030 }, false, "outside south" },
	{ { 40.7200, -74.0030 }, false, "outside north" },
	{ { 40.7158, -74.0100 }, false, "outside west" },
	{ { 40.7158, -73.9950 }, false, "outside east" },
	{ { 0.0, 0.0 },          false, "far away" }
};

/*
 * Boundary rule: the ray-casting rule is half-open. The south and
 * west edges and the south-west corner count as inside; the north and
 * east edges and the other three corners count as outside.
 */
static const point_case_t boundary_cases[] = {
	{ { 40.7128, -74.0030 }, true,  "south edge" },
	{ { 40.7158, -74.0060 }, true,  "west edge" },
	{ { 40.7188, -74.0030 }, false, "north edge" },
	{ { 40.7158, -74.0000 }, false, "east edge" },
	{ { 40.7128, -74.0060 }, true,  "south-west corner" },
	{ { 40.7128, -74.0000 }, false, "south-east corner" },
	{ { 40.7188, -74.0000 }, false, "north-east corner" },
	{ { 40.7188, -74.0060 }, false, "north-west corner" }
};

static const geofence_config_t valid_config = {
	.max_polygons = 10,
	.max_vertices_total = 1000,
	.tolerance_meters = 1.0
};

static void engine_up(void)
{
	geofence_deinit();
	CHECK_EQ(geofence_init(&valid_config), GEOFENCE_SUCCESS);
}

static void engine_down(void)
{
	geofence_deinit();
}

static bool same_point(const geopoint_t *a, const geopoint_t *b)
{
	return a->latitude == b->latitude && a->longitude == b->longitude;
}

/* geofence_init / geofence_deinit */

static void test_init_validation(void)
{
	geofence_config_t cfg;
	geofence_config_t zero;
	geofence_t fence = { 0 };

	geofence_deinit();

	CHECK_EQ(geofence_init(NULL), GEOFENCE_ERROR_INVALID_PARAM);

	memset(&zero, 0, sizeof(zero));
	CHECK_EQ(geofence_init(&zero), GEOFENCE_ERROR_INVALID_PARAM);

	cfg = valid_config;
	cfg.max_polygons = 0;
	CHECK_EQ(geofence_init(&cfg), GEOFENCE_ERROR_INVALID_PARAM);

	cfg = valid_config;
	cfg.max_vertices_total = 0;
	CHECK_EQ(geofence_init(&cfg), GEOFENCE_ERROR_INVALID_PARAM);

	/* None of the rejected calls may leave the engine initialized. */
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_ERROR_NOT_INITIALIZED);

	CHECK_EQ(geofence_init(&valid_config), GEOFENCE_SUCCESS);
	CHECK_EQ(geofence_init(&valid_config), GEOFENCE_ERROR_INVALID_PARAM);

	/* After deinit the engine can be initialized again. */
	geofence_deinit();
	CHECK_EQ(geofence_init(&valid_config), GEOFENCE_SUCCESS);

	geofence_destroy_polygon(&fence);
	engine_down();
}

/* geofence_create_polygon */

static void test_create_polygon(void)
{
	geofence_t fence = { 0 };
	uint16_t i;

	/* Not initialized. */
	geofence_deinit();
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_ERROR_NOT_INITIALIZED);

	engine_up();

	/* A NULL fence is an invalid parameter, not "not initialized". */
	CHECK_EQ(geofence_create_polygon(NULL, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_ERROR_INVALID_PARAM);
	CHECK_EQ(geofence_create_polygon(NULL, NULL, 0),
		 GEOFENCE_ERROR_INVALID_PARAM);

	/*
	 * NULL vertices with a positive count is an error and must
	 * not crash. The output structure is left empty and safe to
	 * destroy.
	 */
	fence.num_vertices = 7;
	fence.capacity = 9;
	CHECK_EQ(geofence_create_polygon(&fence, NULL, 3),
		 GEOFENCE_ERROR_INVALID_PARAM);
	CHECK(fence.vertices == NULL);
	CHECK_EQ(fence.num_vertices, 0);
	CHECK_EQ(fence.capacity, 0);
	CHECK_EQ(geofence_create_polygon(&fence, NULL, UINT16_MAX),
		 GEOFENCE_ERROR_INVALID_PARAM);
	geofence_destroy_polygon(&fence);

	/* Empty polygon: allowed, vertices can be NULL. */
	CHECK_EQ(geofence_create_polygon(&fence, NULL, 0), GEOFENCE_SUCCESS);
	CHECK(fence.vertices == NULL);
	CHECK_EQ(fence.num_vertices, 0);
	CHECK_EQ(fence.capacity, 0);
	geofence_destroy_polygon(&fence);

	/* Regular polygon: vertices are copied, capacity equals count. */
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	CHECK(fence.vertices != NULL);
	CHECK(fence.vertices != test_field);
	CHECK_EQ(fence.num_vertices, TEST_FIELD_VERTICES);
	CHECK_EQ(fence.capacity, TEST_FIELD_VERTICES);
	for (i = 0; i < TEST_FIELD_VERTICES; i++) {
		CHECK(same_point(&fence.vertices[i], &test_field[i]));
	}
	geofence_destroy_polygon(&fence);

	engine_down();
}

/* geofence_add_vertex */

static void test_add_vertex(void)
{
	geofence_t fence = { 0 };
	geopoint_t p;
	const geopoint_t inside = { 40.7158, -74.0030 };
	uint16_t i;
	uint32_t n;

	engine_up();

	/* A polygon created empty (NULL, 0) can grow. */
	CHECK_EQ(geofence_create_polygon(&fence, NULL, 0), GEOFENCE_SUCCESS);
	for (i = 0; i < TEST_FIELD_VERTICES; i++) {
		CHECK_EQ(geofence_add_vertex(&fence, &test_field[i]),
			 GEOFENCE_SUCCESS);
	}
	CHECK_EQ(fence.num_vertices, TEST_FIELD_VERTICES);
	CHECK(fence.capacity >= fence.num_vertices);
	CHECK(geofence_validate_polygon(&fence));
	CHECK(geofence_is_point_in_polygon_ray_casting(&inside, &fence));
	geofence_destroy_polygon(&fence);

	/* A polygon created with vertices can grow past its capacity. */
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	p.latitude = 40.7200;
	p.longitude = -74.0030;
	CHECK_EQ(geofence_add_vertex(&fence, &p), GEOFENCE_SUCCESS);
	CHECK_EQ(fence.num_vertices, TEST_FIELD_VERTICES + 1);
	CHECK(fence.capacity >= fence.num_vertices);
	for (i = 0; i < TEST_FIELD_VERTICES; i++) {
		CHECK(same_point(&fence.vertices[i], &test_field[i]));
	}
	CHECK(fence.num_vertices > TEST_FIELD_VERTICES &&
	      same_point(&fence.vertices[TEST_FIELD_VERTICES], &p));

	/* Several reallocations keep every stored vertex. */
	for (i = 0; i < 100; i++) {
		p.latitude = 0.001 * i;
		p.longitude = 0.002 * i;
		CHECK_EQ(geofence_add_vertex(&fence, &p), GEOFENCE_SUCCESS);
	}
	CHECK_EQ(fence.num_vertices, TEST_FIELD_VERTICES + 1 + 100);
	for (i = 0; i < 100; i++) {
		p.latitude = 0.001 * i;
		p.longitude = 0.002 * i;
		CHECK(fence.num_vertices > TEST_FIELD_VERTICES + 1 + i &&
		      same_point(&fence.vertices[TEST_FIELD_VERTICES + 1 + i],
				 &p));
	}
	geofence_destroy_polygon(&fence);

	/* Invalid coordinates are rejected and change nothing. */
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	p.latitude = 999.0;
	p.longitude = 999.0;
	CHECK_EQ(geofence_add_vertex(&fence, &p),
		 GEOFENCE_ERROR_INVALID_PARAM);
	p.latitude = 90.5;
	p.longitude = 0.0;
	CHECK_EQ(geofence_add_vertex(&fence, &p),
		 GEOFENCE_ERROR_INVALID_PARAM);
	p.latitude = 0.0;
	p.longitude = -180.5;
	CHECK_EQ(geofence_add_vertex(&fence, &p),
		 GEOFENCE_ERROR_INVALID_PARAM);
	p.latitude = NAN;
	p.longitude = 0.0;
	CHECK_EQ(geofence_add_vertex(&fence, &p),
		 GEOFENCE_ERROR_INVALID_PARAM);
	CHECK_EQ(fence.num_vertices, TEST_FIELD_VERTICES);
	CHECK_EQ(fence.capacity, TEST_FIELD_VERTICES);

	/* Limits of the valid range are accepted. */
	p.latitude = -90.0;
	p.longitude = 180.0;
	CHECK_EQ(geofence_add_vertex(&fence, &p), GEOFENCE_SUCCESS);

	/* NULL arguments. */
	CHECK_EQ(geofence_add_vertex(NULL, &p), GEOFENCE_ERROR_INVALID_PARAM);
	CHECK_EQ(geofence_add_vertex(&fence, NULL),
		 GEOFENCE_ERROR_INVALID_PARAM);
	geofence_destroy_polygon(&fence);

	/* An uninitialized engine refuses to add vertices. */
	CHECK_EQ(geofence_create_polygon(&fence, NULL, 0), GEOFENCE_SUCCESS);
	geofence_deinit();
	CHECK(geofence_add_vertex(&fence, &inside) < 0);
	CHECK_EQ(fence.num_vertices, 0);
	geofence_destroy_polygon(&fence);

	/* The vertex counter is 16 bits: the 65536th vertex is refused. */
	engine_up();
	CHECK_EQ(geofence_create_polygon(&fence, NULL, 0), GEOFENCE_SUCCESS);
	p.latitude = 10.0;
	p.longitude = 10.0;
	for (n = 0; n < UINT16_MAX; n++) {
		if (geofence_add_vertex(&fence, &p) != GEOFENCE_SUCCESS) {
			break;
		}
	}
	CHECK_EQ(n, UINT16_MAX);
	CHECK_EQ(fence.num_vertices, UINT16_MAX);
	CHECK_EQ(geofence_add_vertex(&fence, &p),
		 GEOFENCE_ERROR_TOO_MANY_VERTICES);
	CHECK_EQ(fence.num_vertices, UINT16_MAX);
	geofence_destroy_polygon(&fence);

	engine_down();
}

/* geofence_clear_polygon / geofence_destroy_polygon */

static void test_clear_and_destroy(void)
{
	geofence_t fence = { 0 };
	geopoint_t *storage;
	const geopoint_t inside = { 40.7158, -74.0030 };

	engine_up();
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	CHECK(geofence_is_point_in_polygon_ray_casting(&inside, &fence));
	storage = fence.vertices;

	/* Clear empties the polygon but keeps the storage. */
	geofence_clear_polygon(&fence);
	CHECK_EQ(fence.num_vertices, 0);
	CHECK_EQ(fence.capacity, TEST_FIELD_VERTICES);
	CHECK(fence.vertices == storage);
	CHECK(!geofence_validate_polygon(&fence));
	CHECK(!geofence_is_point_in_polygon_ray_casting(&inside, &fence));

	/* The kept storage is reused without reallocation. */
	for (uint16_t i = 0; i < TEST_FIELD_VERTICES; i++) {
		CHECK_EQ(geofence_add_vertex(&fence, &test_field[i]),
			 GEOFENCE_SUCCESS);
	}
	CHECK(fence.vertices == storage);
	CHECK(geofence_is_point_in_polygon_ray_casting(&inside, &fence));

	geofence_clear_polygon(NULL); /* must not crash */

	/* Destroy frees, zeroes, and is safe to repeat. */
	geofence_destroy_polygon(&fence);
	CHECK(fence.vertices == NULL);
	CHECK_EQ(fence.num_vertices, 0);
	CHECK_EQ(fence.capacity, 0);
	geofence_destroy_polygon(&fence);
	geofence_destroy_polygon(NULL); /* must not crash */

	engine_down();
}

/* geofence_validate_polygon */

static void test_validate_polygon(void)
{
	geofence_t fence = { 0 };
	geopoint_t bad[3] = {
		{ 0.0, 0.0 }, { 0.0, 1.0 }, { 1.0, 1.0 }
	};

	engine_up();

	CHECK(!geofence_validate_polygon(NULL));
	CHECK(!geofence_validate_polygon(&fence)); /* empty */

	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	CHECK(geofence_validate_polygon(&fence));
	geofence_destroy_polygon(&fence);

	CHECK_EQ(geofence_create_polygon(&fence, test_field, 2),
		 GEOFENCE_SUCCESS);
	CHECK(!geofence_validate_polygon(&fence)); /* fewer than 3 */
	geofence_destroy_polygon(&fence);

	CHECK_EQ(geofence_create_polygon(&fence, test_field, 3),
		 GEOFENCE_SUCCESS);
	CHECK(geofence_validate_polygon(&fence)); /* exactly 3 */
	geofence_destroy_polygon(&fence);

	bad[2].latitude = 91.0;
	CHECK_EQ(geofence_create_polygon(&fence, bad, 3), GEOFENCE_SUCCESS);
	CHECK(!geofence_validate_polygon(&fence)); /* bad latitude */
	geofence_destroy_polygon(&fence);

	bad[2].latitude = 1.0;
	bad[1].longitude = 181.0;
	CHECK_EQ(geofence_create_polygon(&fence, bad, 3), GEOFENCE_SUCCESS);
	CHECK(!geofence_validate_polygon(&fence)); /* bad longitude */
	geofence_destroy_polygon(&fence);

	bad[1].longitude = NAN;
	CHECK_EQ(geofence_create_polygon(&fence, bad, 3), GEOFENCE_SUCCESS);
	CHECK(!geofence_validate_polygon(&fence)); /* NaN */
	geofence_destroy_polygon(&fence);

	engine_down();
}

/* Ray casting: the cases that already passed before the audit */

static void test_ray_casting_regions(void)
{
	geofence_t fence = { 0 };
	geofence_t two = { 0 };
	const geopoint_t inside = { 40.7158, -74.0030 };
	geopoint_t bad;
	size_t i;

	engine_up();
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);

	for (i = 0; i < ARRAY_SIZE(region_cases); i++) {
		const point_case_t *tc = &region_cases[i];

		if (geofence_is_point_in_polygon_ray_casting(&tc->point,
							     &fence) !=
		    tc->expected_inside) {
			fprintf(stderr, "  case: %s\n", tc->description);
		}
		CHECK(geofence_is_point_in_polygon_ray_casting(&tc->point,
							       &fence) ==
		      tc->expected_inside);
	}

	/* Vertex order does not matter. */
	geofence_destroy_polygon(&fence);
	CHECK_EQ(geofence_create_polygon(&fence, test_field_cw,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	CHECK(geofence_is_point_in_polygon_ray_casting(&inside, &fence));

	/* Invalid input is reported as "outside" (false). */
	bad.latitude = 999.0;
	bad.longitude = 0.0;
	CHECK(!geofence_is_point_in_polygon_ray_casting(&bad, &fence));
	bad.latitude = NAN;
	bad.longitude = -74.0030;
	CHECK(!geofence_is_point_in_polygon_ray_casting(&bad, &fence));
	CHECK(!geofence_is_point_in_polygon_ray_casting(NULL, &fence));
	CHECK(!geofence_is_point_in_polygon_ray_casting(&inside, NULL));

	/* Fewer than three vertices is never "inside". */
	CHECK_EQ(geofence_create_polygon(&two, test_field, 2),
		 GEOFENCE_SUCCESS);
	CHECK(!geofence_is_point_in_polygon_ray_casting(&test_field[0],
							&two));
	geofence_destroy_polygon(&two);

	/* An uninitialized engine returns false, even for a point
	 * that is truly inside. */
	CHECK(geofence_is_point_in_polygon_ray_casting(&inside, &fence));
	geofence_deinit();
	CHECK(!geofence_is_point_in_polygon_ray_casting(&inside, &fence));

	geofence_destroy_polygon(&fence);
}

/* Boundary rule: every edge and every corner */

static void test_half_open_boundary(void)
{
	geofence_t fence = { 0 };
	size_t i;

	engine_up();
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);

	for (i = 0; i < ARRAY_SIZE(boundary_cases); i++) {
		const point_case_t *tc = &boundary_cases[i];
		bool ray = geofence_is_point_in_polygon_ray_casting(
			&tc->point, &fence);
		bool wind = geofence_is_point_in_polygon_winding_number(
			&tc->point, &fence);

		if (ray != tc->expected_inside ||
		    wind != tc->expected_inside) {
			fprintf(stderr, "  case: %s\n", tc->description);
		}
		CHECK(ray == tc->expected_inside);
		CHECK(wind == tc->expected_inside);
	}

	/* The rule does not depend on the vertex order. */
	geofence_destroy_polygon(&fence);
	CHECK_EQ(geofence_create_polygon(&fence, test_field_cw,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	for (i = 0; i < ARRAY_SIZE(boundary_cases); i++) {
		const point_case_t *tc = &boundary_cases[i];

		if (geofence_is_point_in_polygon_ray_casting(&tc->point,
							     &fence) !=
		    tc->expected_inside) {
			fprintf(stderr, "  case (cw): %s\n", tc->description);
		}
		CHECK(geofence_is_point_in_polygon_ray_casting(&tc->point,
							       &fence) ==
		      tc->expected_inside);
	}

	geofence_destroy_polygon(&fence);
	engine_down();
}

/* Winding number and optimized wrapper agree with ray casting */

static void check_algorithms_agree(const geofence_t *fence,
				   const geopoint_t *point)
{
	bool ray = geofence_is_point_in_polygon_ray_casting(point, fence);

	if (geofence_is_point_in_polygon_winding_number(point, fence) != ray ||
	    geofence_is_point_in_polygon_optimized(point, fence) != ray) {
		fprintf(stderr, "  disagree at (%f, %f)\n", point->latitude,
			point->longitude);
	}
	CHECK(geofence_is_point_in_polygon_winding_number(point, fence) ==
	      ray);
	CHECK(geofence_is_point_in_polygon_optimized(point, fence) == ray);
}

static void test_algorithms_agree(void)
{
	geofence_t rect = { 0 };
	geofence_t rect_cw = { 0 };
	geofence_t lshape = { 0 };
	geopoint_t p;
	size_t i;
	int row;
	int col;

	engine_up();
	CHECK_EQ(geofence_create_polygon(&rect, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	CHECK_EQ(geofence_create_polygon(&rect_cw, test_field_cw,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);
	CHECK_EQ(geofence_create_polygon(&lshape, l_shape,
					 (uint16_t)ARRAY_SIZE(l_shape)),
		 GEOFENCE_SUCCESS);

	for (i = 0; i < ARRAY_SIZE(region_cases); i++) {
		check_algorithms_agree(&rect, &region_cases[i].point);
		check_algorithms_agree(&rect_cw, &region_cases[i].point);
	}
	for (i = 0; i < ARRAY_SIZE(boundary_cases); i++) {
		check_algorithms_agree(&rect, &boundary_cases[i].point);
		check_algorithms_agree(&rect_cw, &boundary_cases[i].point);
	}

	/* Concave polygon on a grid that avoids the edges. */
	for (row = 0; row < 24; row++) {
		for (col = 0; col < 24; col++) {
			p.latitude = -0.25 + 0.1 * row + 0.013;
			p.longitude = -0.25 + 0.1 * col + 0.017;
			check_algorithms_agree(&lshape, &p);
		}
	}

	/* The notch is outside, both arms are inside. */
	p.latitude = 1.5;
	p.longitude = 1.5;
	CHECK(!geofence_is_point_in_polygon_winding_number(&p, &lshape));
	p.latitude = 1.5;
	p.longitude = 0.5;
	CHECK(geofence_is_point_in_polygon_winding_number(&p, &lshape));
	p.latitude = 0.5;
	p.longitude = 1.5;
	CHECK(geofence_is_point_in_polygon_winding_number(&p, &lshape));

	/* Same guards as ray casting. */
	p.latitude = 999.0;
	p.longitude = 0.0;
	CHECK(!geofence_is_point_in_polygon_winding_number(&p, &rect));
	CHECK(!geofence_is_point_in_polygon_winding_number(NULL, &rect));
	CHECK(!geofence_is_point_in_polygon_winding_number(&p, NULL));
	CHECK(!geofence_is_point_in_polygon_optimized(NULL, &rect));

	geofence_deinit();
	CHECK(!geofence_is_point_in_polygon_winding_number(
		&region_cases[0].point, &rect));
	CHECK(!geofence_is_point_in_polygon_optimized(
		&region_cases[0].point, &rect));

	geofence_destroy_polygon(&rect);
	geofence_destroy_polygon(&rect_cw);
	geofence_destroy_polygon(&lshape);
}

/* geofence_calculate_distance */

static void test_distance(void)
{
	const geopoint_t a = { 40.7158, -74.0030 };
	const geopoint_t b = { 40.7100, -74.0030 };
	const geopoint_t bad = { 999.0, 0.0 };
	const geopoint_t nan_point = { NAN, 0.0 };
	geopoint_t p;
	geopoint_t q;
	int lat;
	int lon;
	int nan_count = 0;
	int far_count = 0;

	/* 0.0058 degrees of latitude is about 644.93 m. */
	CHECK_NEAR(geofence_calculate_distance(&a, &b), 644.93, 0.01);
	CHECK_NEAR(geofence_calculate_distance(&b, &a), 644.93, 0.01);
	CHECK_NEAR(geofence_calculate_distance(&a, &a), 0.0, 1e-9);

	/* Invalid coordinates and NULL give -1.0. */
	CHECK_NEAR(geofence_calculate_distance(&a, &bad), -1.0, 0.0);
	CHECK_NEAR(geofence_calculate_distance(&bad, &a), -1.0, 0.0);
	CHECK_NEAR(geofence_calculate_distance(&a, &nan_point), -1.0, 0.0);
	CHECK_NEAR(geofence_calculate_distance(NULL, &a), -1.0, 0.0);
	CHECK_NEAR(geofence_calculate_distance(&a, NULL), -1.0, 0.0);

	/*
	 * Exact antipodes must give half the Earth circumference,
	 * never NaN. Scan an integer-degree grid of antipodal pairs.
	 */
	p.latitude = 12.0;
	p.longitude = 0.0;
	q.latitude = -12.0;
	q.longitude = -180.0;
	CHECK_NEAR(geofence_calculate_distance(&p, &q), 20015086.796, 1.0);

	for (lat = -90; lat <= 90; lat++) {
		for (lon = -180; lon <= 0; lon++) {
			double d;

			p.latitude = lat;
			p.longitude = lon;
			q.latitude = -lat;
			q.longitude = lon + 180;
			d = geofence_calculate_distance(&p, &q);
			if (isnan(d)) {
				nan_count++;
			} else if (fabs(d - 20015086.796) > 1.0) {
				far_count++;
			}
		}
	}
	CHECK_EQ(nan_count, 0);
	CHECK_EQ(far_count, 0);
}

/* geofence_get_closest_boundary_point (vertices only) */

static void test_closest_boundary_point(void)
{
	geofence_t fence = { 0 };
	geofence_t one = { 0 };
	geopoint_t closest = { 0.0, 0.0 };
	geopoint_t bad = { 999.0, 0.0 };
	const geopoint_t near_sw = { 40.7120, -74.0070 };
	const geopoint_t mid_south = { 40.7128, -74.0030 };
	double best;
	double dist;
	double d;
	size_t i;

	engine_up();
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);

	/* Closest vertex of a point just outside the south-west corner. */
	dist = geofence_get_closest_boundary_point(&near_sw, &fence, &closest);
	CHECK(same_point(&closest, &test_field[0]));
	CHECK_NEAR(dist, geofence_calculate_distance(&near_sw, &test_field[0]),
		   1e-9);
	CHECK(dist > 0.0);

	/*
	 * Contract: the result is the nearest vertex, not the nearest
	 * point of an edge. A point on the middle of the south edge
	 * still reports a vertex.
	 */
	dist = geofence_get_closest_boundary_point(&mid_south, &fence,
						   &closest);
	best = -1.0;
	for (i = 0; i < TEST_FIELD_VERTICES; i++) {
		d = geofence_calculate_distance(&mid_south, &test_field[i]);
		if (best < 0.0 || d < best) {
			best = d;
		}
	}
	CHECK_NEAR(dist, best, 1e-9);
	CHECK(same_point(&closest, &test_field[0]) ||
	      same_point(&closest, &test_field[1]));

	/* Error cases return -1.0 and leave the output untouched. */
	closest = near_sw;
	CHECK_NEAR(geofence_get_closest_boundary_point(NULL, &fence, &closest),
		   -1.0, 0.0);
	CHECK_NEAR(geofence_get_closest_boundary_point(&near_sw, NULL,
						       &closest), -1.0, 0.0);
	CHECK_NEAR(geofence_get_closest_boundary_point(&near_sw, &fence, NULL),
		   -1.0, 0.0);
	CHECK_NEAR(geofence_get_closest_boundary_point(&bad, &fence, &closest),
		   -1.0, 0.0);
	CHECK(same_point(&closest, &near_sw));

	CHECK_EQ(geofence_create_polygon(&one, test_field, 1),
		 GEOFENCE_SUCCESS);
	CHECK_NEAR(geofence_get_closest_boundary_point(&near_sw, &one,
						       &closest), -1.0, 0.0);
	geofence_destroy_polygon(&one);

	geofence_deinit();
	CHECK_NEAR(geofence_get_closest_boundary_point(&near_sw, &fence,
						       &closest), -1.0, 0.0);

	geofence_destroy_polygon(&fence);
}

/* Host measurement, printed only: it proves nothing about the target. */

#define MEASURE_ITERATIONS 10000

static void print_host_measurement(void)
{
	geofence_t fence = { 0 };
	const geopoint_t inside = { 40.7158, -74.0030 };
	volatile bool sink = false;
	clock_t start;
	clock_t end;
	double us_per_call;
	int i;

	engine_up();
	CHECK_EQ(geofence_create_polygon(&fence, test_field,
					 TEST_FIELD_VERTICES),
		 GEOFENCE_SUCCESS);

	start = clock();
	for (i = 0; i < MEASURE_ITERATIONS; i++) {
		sink = geofence_is_point_in_polygon_ray_casting(&inside,
								&fence);
	}
	end = clock();
	CHECK(sink);

	us_per_call = ((double)(end - start) * 1e6) /
		      ((double)CLOCKS_PER_SEC * MEASURE_ITERATIONS);
	printf("host measurement (not an RF-02 proof): %.3f us per ray-casting call\n",
	       us_per_call);

	geofence_destroy_polygon(&fence);
	engine_down();
}

/* Runner */

typedef struct {
	const char *name;
	void (*run)(void);
} test_entry_t;

static const test_entry_t tests[] = {
	{ "init_validation",         test_init_validation },
	{ "create_polygon",          test_create_polygon },
	{ "add_vertex",              test_add_vertex },
	{ "clear_and_destroy",       test_clear_and_destroy },
	{ "validate_polygon",        test_validate_polygon },
	{ "ray_casting_regions",     test_ray_casting_regions },
	{ "half_open_boundary",      test_half_open_boundary },
	{ "algorithms_agree",        test_algorithms_agree },
	{ "distance",                test_distance },
	{ "closest_boundary_point",  test_closest_boundary_point },
	{ "host_measurement",        print_host_measurement }
};

int main(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		int failures_before = g_failures;

		tests[i].run();
		printf("%s: %s\n", tests[i].name,
		       g_failures == failures_before ? "ok" : "FAILED");
		fflush(stdout);
	}

	printf("\n%zu tests, %d checks, %d failed\n", ARRAY_SIZE(tests),
	       g_checks, g_failures);
	return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
