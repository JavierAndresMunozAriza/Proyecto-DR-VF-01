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
 */
typedef struct {
    geopoint_t* vertices;    /**< Array of polygon vertices */
    uint16_t num_vertices;   /**< Number of vertices in polygon */
    uint16_t capacity;        /**< Maximum capacity of vertices array */
} geofence_t;

/**
 * @brief Geofence system configuration
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
 * @return 0 on success, negative error code on failure
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
 * @param vertices Array of initial vertices (can be NULL)
 * @param num_vertices Number of initial vertices
 * @return 0 on success, negative error code on failure
 * 
 * Following Elecia White's principle of "Make functions do one thing well"
 */
int geofence_create_polygon(geofence_t* fence, const geopoint_t* vertices, uint16_t num_vertices);

/**
 * @brief Add a vertex to an existing polygon
 * @param fence Target geofence polygon
 * @param point Vertex to add
 * @return 0 on success, negative error code on failure
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
 * @return true if point is inside polygon, false otherwise
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
 * @return true if point is inside polygon, false otherwise
 * 
 * More mathematically robust than ray casting for complex polygons
 * Time complexity: O(n) where n is number of vertices
 * Space complexity: O(1)
 */
bool geofence_is_point_in_polygon_winding_number(const geopoint_t* point, const geofence_t* fence);

/**
 * @brief Optimized point-in-polygon test with caching
 * @param point Test point coordinates
 * @param fence Target geofence polygon
 * @return true if point is inside polygon, false otherwise
 * 
 * Uses spatial indexing for improved performance on large polygons
 * Following Elecia White's principle of "Optimize based on measured data"
 */
bool geofence_is_point_in_polygon_optimized(const geopoint_t* point, const geofence_t* fence);

/* Utility functions */

/**
 * @brief Calculate distance between two geographic points
 * @param point1 First point
 * @param point2 Second point
 * @return Distance in meters using haversine formula
 */
double geofence_calculate_distance(const geopoint_t* point1, const geopoint_t* point2);

/**
 * @brief Get the closest point on a polygon boundary to a test point
 * @param point Test point
 * @param fence Target polygon
 * @param closest_point Output parameter for closest boundary point
 * @return Distance to closest point in meters
 */
double geofence_get_closest_boundary_point(const geopoint_t* point, const geofence_t* fence, geopoint_t* closest_point);

/**
 * @brief Validate geofence polygon for correctness
 * @param fence Polygon to validate
 * @return true if polygon is valid, false otherwise
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