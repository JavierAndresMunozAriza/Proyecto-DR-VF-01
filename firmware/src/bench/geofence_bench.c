/*
 * geofence_bench.c - on-target timing benchmark of geofence_check()
 *
 * The kernel cycle counter of the nRF9151 runs at 32768 Hz, so one reading has
 * a resolution of 30.5 us. A single call can be shorter than that, so the
 * benchmark times a whole batch of calls and reports the mean per call.
 */

#include <stdint.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "bench_polygon.h"
#include "demo_route.h"
#include "geofence.h"
#include "geofence_bench.h"
#include "timing.h"

LOG_MODULE_REGISTER(bench, LOG_LEVEL_INF);

#define BENCH_RADIUS_M     500.0
#define BENCH_POINTS       40 /* half of them inside the polygon, half outside */
#define BENCH_MAX_VERTICES 1024
#define TENTHS_PER_UNIT    10

static const unsigned int vertex_counts[] = { 4, 16, 64, 256, BENCH_MAX_VERTICES };

/* Time BENCH_POINTS checks on one polygon and log the mean per check. */
static void bench_polygon_size(unsigned int vertex_count, geopoint_t *vertices)
{
	const geopoint_t center = { DEMO_CENTER_LAT, DEMO_CENTER_LON };
	geopoint_t points[BENCH_POINTS];
	geofence_t fence = { 0 };
	geofence_result_t result;
	unsigned int wrong = 0;
	timing_stamp_t start;
	uint32_t total_us;
	uint32_t mean_tenths_us;

	if (bench_polygon_vertices(&center, BENCH_RADIUS_M, vertex_count, vertices) != 0 ||
	    geofence_create_polygon(&fence, vertices, (uint16_t)vertex_count) != GEOFENCE_SUCCESS) {
		LOG_ERR("bench: could not build a polygon of %u vertices", vertex_count);
		return;
	}

	for (unsigned int i = 0; i < BENCH_POINTS; i++) {
		if (bench_polygon_point(&center, BENCH_RADIUS_M, i, BENCH_POINTS, (i % 2) == 0,
					&points[i]) != 0) {
			LOG_ERR("bench: could not build a test point");
			geofence_destroy_polygon(&fence);
			return;
		}
	}

	start = timing_now();
	for (unsigned int i = 0; i < BENCH_POINTS; i++) {
		if (geofence_check(&points[i], &fence, &result) != GEOFENCE_SUCCESS ||
		    result.inside != ((i % 2) == 0))
			wrong++;
	}
	total_us = timing_elapsed_us(start);

	mean_tenths_us = total_us * TENTHS_PER_UNIT / BENCH_POINTS;
	LOG_INF("bench: %4u vertices, %u checks, total %u us, mean %u.%u us per check, %u wrong",
		vertex_count, BENCH_POINTS, total_us, mean_tenths_us / TENTHS_PER_UNIT,
		mean_tenths_us % TENTHS_PER_UNIT, wrong);

	geofence_destroy_polygon(&fence);
}

void geofence_bench_run(void)
{
	geopoint_t *vertices = malloc(BENCH_MAX_VERTICES * sizeof(*vertices));

	if (vertices == NULL) {
		LOG_ERR("bench: out of memory");
		return;
	}

	LOG_INF("bench: start, %d points per polygon, clock %u Hz", BENCH_POINTS,
		(unsigned int)CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC);

	for (size_t i = 0; i < ARRAY_SIZE(vertex_counts); i++)
		bench_polygon_size(vertex_counts[i], vertices);

	LOG_INF("bench: done");
	free(vertices);
}
