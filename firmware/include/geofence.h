/*
 * geofence.h
 *
 * Point-in-Polygon algorithm implementation for cattle geofencing collar DR-VF-01
 *
 * This implementation follows the principles from "Making Embedded Systems" by Elecia White:
 * - Clear, maintainable code with explicit functionality
 * - Robust error handling and boundary checking
 * - Efficient resource usage for embedded applications
 * - Portable implementation for multiple platforms
 *
 * Boundary rule (half-open)
 *
 * A point that lies exactly on the polygon boundary is classified by a
 * half-open rule, the same for ray casting, winding number and the
 * optimized wrapper. For an axis-aligned rectangle (latitude grows to
 * the north, longitude to the east):
 *   - inside:  the south edge, the west edge, and the south-west corner;
 *   - outside: the north edge, the east edge, and the south-east,
 *              north-east and north-west corners.
 * Other polygons follow the same rule edge by edge. Do not rely on the
 * result for a point that is exactly on the boundary.
 *
 * Known limitations
 *
 * - Invalid input: the point-in-polygon functions return bool, so
 *   "outside" cannot be told apart from "invalid input". An invalid
 *   point (NULL, NaN or out-of-range coordinates), an invalid or too
 *   small polygon, and an uninitialized engine all return false. The
 *   caller must validate the GNSS fix and the engine state before it
 *   treats false as "outside".
 * - Antimeridian: polygons that cross longitude +/-180 are not
 *   supported, because longitudes are compared as plain numbers.
 * - Re-creation: geofence_create_polygon() does not free the storage of
 *   the structure it receives. Calling it twice on the same structure
 *   leaks the first array; call geofence_destroy_polygon() first.
 */

#ifndef GEOFENCE_H
#define GEOFENCE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Geofence data structures */

/**
 * @brief Geographic coordinate point
 */
typedef struct {
    double latitude;   /**< Latitude in decimal degrees (-90 to 90) */
    double longitude;  /**< Longitude in decimal degrees (-180 to 180) */
} geopoint_t;

/**
 * @brief Geofence polygon definition
 *
 * Owns its vertex storage: allocate it with geofence_create_polygon() or
 * geofence_add_vertex() and release it with geofence_destroy_polygon().
 * The edges join consecutive vertices and the last vertex joins the
 * first one, so the first vertex must not be repeated at the end.
 */
typedef struct {
    geopoint_t* vertices;    /**< Array of polygon vertices */
    uint16_t num_vertices;   /**< Number of vertices in polygon */
    uint16_t capacity;        /**< Maximum capacity of vertices array */
} geofence_t;

/**
 * @brief Geofence system configuration
 *
 * geofence_init() requires non-zero max_polygons and max_vertices_total
 * and stores the configuration. The limits and the tolerance are not
 * enforced by the engine yet.
 */
typedef struct {
    uint32_t max_polygons;     /**< Maximum number of concurrent polygons */
    uint32_t max_vertices_total; /**< Total maximum vertices across all polygons */
    double tolerance_meters;   /**< Distance tolerance for boundary checking */
} geofence_config_t;

/* Geofence system initialization */

/**
 * @brief Initialize the geofence system
 * @param config Configuration parameters for geofence system
 * @return GEOFENCE_SUCCESS on success, GEOFENCE_ERROR_INVALID_PARAM if
 *         config is NULL, a limit is zero, or the system is already
 *         initialized (call geofence_deinit() first)
 *
 * Following Elecia White's principle of "Initialize everything explicitly"
 */
int geofence_init(const geofence_config_t* config);

/**
 * @brief Deinitialize the geofence system and free resources
 */
void geofence_deinit(void);

/* Polygon management */

/**
 * @brief Create a new geofence polygon
 * @param fence Pointer to geofence structure to initialize
 * @param vertices Array of initial vertices, copied into the polygon.
 *        May be NULL only when num_vertices is 0.
 * @param num_vertices Number of initial vertices
 * @return GEOFENCE_SUCCESS on success;
 *         GEOFENCE_ERROR_NOT_INITIALIZED if geofence_init() was not
 *         called; GEOFENCE_ERROR_INVALID_PARAM if fence is NULL, or
 *         vertices is NULL with num_vertices > 0;
 *         GEOFENCE_ERROR_OUT_OF_MEMORY if the copy cannot be allocated.
 *
 * Once the engine is initialized and fence is not NULL, fence is always
 * left empty (and safe to destroy) when an error is returned. With
 * num_vertices equal to 0 the polygon is empty and vertices are added
 * later with geofence_add_vertex(). The previous contents of fence are
 * not freed (see "Known limitations" above).
 *
 * Following Elecia White's principle of "Make functions do one thing well"
 */
int geofence_create_polygon(geofence_t* fence, const geopoint_t* vertices, uint16_t num_vertices);

/**
 * @brief Add a vertex to an existing polygon
 * @param fence Target geofence polygon
 * @param point Vertex to add; latitude in [-90, 90], longitude in
 *        [-180, 180]
 * @return GEOFENCE_SUCCESS on success; GEOFENCE_ERROR_INVALID_PARAM if
 *         the engine is not initialized, fence or point is NULL, or the
 *         coordinates are out of range or NaN;
 *         GEOFENCE_ERROR_OUT_OF_MEMORY if the storage cannot grow;
 *         GEOFENCE_ERROR_TOO_MANY_VERTICES if the polygon already holds
 *         65535 vertices.
 *
 * The storage grows with realloc when it is full: the capacity doubles
 * (8 for a polygon created empty), so the vertices pointer can change.
 */
int geofence_add_vertex(geofence_t* fence, const geopoint_t* point);

/**
 * @brief Remove all vertices from a polygon
 * @param fence Target geofence polygon
 */
void geofence_clear_polygon(geofence_t* fence);

/**
 * @brief Destroy a geofence polygon and free its resources
 * @param fence Target geofence polygon
 */
void geofence_destroy_polygon(geofence_t* fence);

/* Point-in-Polygon algorithms */

/**
 * @brief Ray casting algorithm for point-in-polygon test
 * @param point Test point coordinates
 * @param fence Target geofence polygon
 * @return true if point is inside polygon, false otherwise (including
 *         invalid input, see "Known limitations" above)
 *
 * Points on the boundary follow the half-open rule described at the top
 * of this file.
 *
 * This implementation follows Elecia White's principle of
 * "Choose efficient algorithms appropriate for the target system"
 * Time complexity: O(n) where n is number of vertices
 * Space complexity: O(1)
 */
bool geofence_is_point_in_polygon_ray_casting(const geopoint_t* point, const geofence_t* fence);

/**
 * @brief Winding number algorithm for point-in-polygon test
 * @param point Test point coordinates
 * @param fence Target geofence polygon
 * @return true if point is inside polygon, false otherwise (including
 *         invalid input, see "Known limitations" above)
 *
 * Non-zero winding rule with the same half-open boundary rule as ray
 * casting. For a simple (non-self-intersecting) polygon it gives the
 * same answer as ray casting; the two differ only for self-intersecting
 * polygons, where ray casting uses the even-odd rule.
 * Time complexity: O(n) where n is number of vertices
 * Space complexity: O(1)
 */
bool geofence_is_point_in_polygon_winding_number(const geopoint_t* point, const geofence_t* fence);

/**
 * @brief Point-in-polygon test, currently a thin wrapper over ray casting
 * @param point Test point coordinates
 * @param fence Target geofence polygon
 * @return Same result as geofence_is_point_in_polygon_ray_casting()
 *
 * No caching and no spatial indexing: the name is reserved for a future
 * optimization, and today it only forwards to ray casting.
 */
bool geofence_is_point_in_polygon_optimized(const geopoint_t* point, const geofence_t* fence);

/* Utility functions */

/**
 * @brief Calculate distance between two geographic points
 * @param point1 First point
 * @param point2 Second point
 * @return Distance in meters using haversine formula, or -1.0 if a
 *         point is NULL or has coordinates out of range or NaN
 */
double geofence_calculate_distance(const geopoint_t* point1, const geopoint_t* point2);

/**
 * @brief Get the polygon vertex closest to a test point
 * @param point Test point
 * @param fence Target polygon (at least 2 vertices)
 * @param closest_point Output parameter for the closest vertex
 * @return Distance to that vertex in meters, or -1.0 on invalid input
 *         (engine not initialized, NULL argument, fewer than 2 vertices,
 *         or invalid coordinates). closest_point is written only on
 *         success.
 *
 * Despite the name, this measures to the vertices only, not to the
 * edges: for a point next to the middle of a long edge the result is the
 * distance to a corner, not the distance to the border. It is not a
 * distance to the boundary and must not be used as one.
 */
double geofence_get_closest_boundary_point(const geopoint_t* point, const geofence_t* fence, geopoint_t* closest_point);

/**
 * @brief Validate geofence polygon for correctness
 * @param fence Polygon to validate
 * @return true if the polygon has at least 3 vertices and every
 *         coordinate is in range, false otherwise (including NULL). It
 *         does not check for self-intersection and does not need
 *         geofence_init().
 *
 * Following Elecia White's principle of "Validate inputs rigorously"
 */
bool geofence_validate_polygon(const geofence_t* fence);

/* Error codes */

#define GEOFENCE_SUCCESS              0
#define GEOFENCE_ERROR_INVALID_PARAM  -1
#define GEOFENCE_ERROR_OUT_OF_MEMORY   -2
#define GEOFENCE_ERROR_TOO_MANY_VERTICES -3
#define GEOFENCE_ERROR_EMPTY_POLYGON   -4
#define GEOFENCE_ERROR_NOT_INITIALIZED -5

#ifdef __cplusplus
}
#endif

#endif /* GEOFENCE_H */