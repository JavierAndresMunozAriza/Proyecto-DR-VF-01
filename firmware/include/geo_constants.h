/**
 * @file geo_constants.h
 * @brief Geographic constants shared by the pure-C modules
 */

#ifndef GEO_CONSTANTS_H
#define GEO_CONSTANTS_H

/** Meters in one degree of latitude (mean Earth radius of 6371 km) */
#define GEO_METERS_PER_DEG_LAT 111195.0

/** Pi; M_PI is not available with the strict -std=c99 the host tests use */
#define GEO_PI 3.14159265358979323846

/** Radians in one degree */
#define GEO_DEG_TO_RAD (GEO_PI / 180.0)

#endif /* GEO_CONSTANTS_H */
