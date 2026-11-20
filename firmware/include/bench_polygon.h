/**
 * @file bench_polygon.h
 * @brief Regular polygons and test points for the geofence timing benchmark
 *
 * Pure C, no hardware dependency. The benchmark times geofence_check() on
 * regular polygons with a growing number of vertices; this module builds
 * them and the points to check, so their geometry can be tested on the host.
 */

#ifndef BENCH_POLYGON_H
#define BENCH_POLYGON_H

#include <stdbool.h>

#include "geofence.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Fewest vertices accepted: with 3 the inner test radius would be an edge midpoint */
#define BENCH_POLYGON_MIN_VERTICES 4

/**
 * @brief Corners of a regular polygon inscribed in a circle
 *
 * Vertex i lies at the bearing 360 * i / n degrees (0 = north) on a circle
 * of radius @p radius_m around @p center.
 *
 * @param center    Center of the circle
 * @param radius_m  Circumradius in meters (> 0)
 * @param n         Number of vertices (>= BENCH_POLYGON_MIN_VERTICES)
 * @param out       Receives @p n vertices
 * @return 0 on success, -1 on a NULL pointer, a non-positive radius or too
 *         few vertices
 */
int bench_polygon_vertices(const geopoint_t *center, double radius_m, unsigned int n,
			   geopoint_t *out);

/**
 * @brief A test point that is surely inside or surely outside the polygon
 *
 * Point @p index of @p count lies at the bearing 360 * index / count degrees,
 * at 0.4 times the radius (inside) or 1.5 times the radius (outside). Both
 * are away from every edge, so the result does not depend on rounding.
 *
 * @return 0 on success, -1 on a NULL pointer, a non-positive radius or
 *         @p count == 0
 */
int bench_polygon_point(const geopoint_t *center, double radius_m, unsigned int index,
			unsigned int count, bool inside, geopoint_t *out);

#ifdef __cplusplus
}
#endif

#endif /* BENCH_POLYGON_H */
