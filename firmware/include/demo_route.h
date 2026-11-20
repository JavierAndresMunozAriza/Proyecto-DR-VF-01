/**
 * @file demo_route.h
 * @brief Simulated walk used by the demo mode
 *
 * Pure C, no hardware dependency. The walk starts at the fence center, goes
 * north in 2 m steps until it is 40 m from the center (10 m beyond the edge
 * of a 60 m square fence), and comes back. It repeats forever, so the LED
 * shows the full sequence: green, toward red, blinking red outside, and back.
 */

#ifndef DEMO_ROUTE_H
#define DEMO_ROUTE_H

#include "geofence.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fictitious point used by the demo and the benchmark, so neither depends on
 * the real position of the board (a plaza in Bogota)
 */
#define DEMO_CENTER_LAT 4.6097
#define DEMO_CENTER_LON (-74.0817)

/** Steps in one full round trip; the route repeats after this many steps */
#define DEMO_ROUTE_STEPS 40

/**
 * @brief Position of the simulated walker at a given step
 *
 * @param center  Point where the walk starts and ends (the fence center)
 * @param step    Step number; any value, the route repeats every
 *                DEMO_ROUTE_STEPS steps
 * @param out     Receives the position
 * @return 0 on success, -1 if @p center or @p out is NULL
 */
int demo_route_point(const geopoint_t *center, unsigned int step, geopoint_t *out);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_ROUTE_H */
