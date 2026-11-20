/**
 * @file fence_square.h
 * @brief Square geofence polygon centered on a point
 *
 * Pure C, no hardware dependency. Used by the proof of concept to fence the
 * place where the first GNSS fix was obtained.
 */

#ifndef FENCE_SQUARE_H
#define FENCE_SQUARE_H

#include "geofence.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FENCE_SQUARE_VERTICES 4

/**
 * @brief Compute the corners of a square centered on a point
 *
 * The square's sides run north-south and east-west, each @p half_side_m
 * meters from the center. Vertices are ordered southwest, southeast,
 * northeast, northwest, and the first vertex is not repeated.
 *
 * @param center       Center of the square
 * @param half_side_m  Distance from the center to each side, in meters (> 0)
 * @param out          Receives the four corners
 * @return 0 on success; -1 if an argument is NULL, the center is NaN or out
 *         of range, half_side_m is not a positive finite number, the center
 *         is beyond 85 degrees of latitude (where the geofence distance is
 *         no longer accurate) or the square would cross the antimeridian
 */
int fence_square_vertices(const geopoint_t *center, double half_side_m,
			  geopoint_t out[FENCE_SQUARE_VERTICES]);

#ifdef __cplusplus
}
#endif

#endif /* FENCE_SQUARE_H */
