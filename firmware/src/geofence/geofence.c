/*
 * geofence.c
 *
 * Point-in-Polygon algorithm implementation for cattle geofencing collar DR-VF-01
 */

#include "geofence.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* Constants */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EARTH_RADIUS_METERS 6371000.0
#define DEG_TO_RAD(deg) ((deg) * M_PI / 180.0)

/* Global state */

static bool g_initialized = false;

/* Internal helper functions */
static bool is_valid_coordinate(double lat, double lon) {
    return (lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0);
}

/**
 * @brief Initialize the geofence system
 */
int geofence_init(const geofence_config_t* config) {
    if (!config) {
        return GEOFENCE_ERROR_INVALID_PARAM;
    }
    
    g_initialized = true;
    return GEOFENCE_SUCCESS;
}

/**
 * @brief Deinitialize the geofence system
 */
void geofence_deinit(void) {
    g_initialized = false;
}

/**
 * @brief Create a new geofence polygon
 */
int geofence_create_polygon(geofence_t* fence, const geopoint_t* vertices, uint16_t num_vertices) {
    if (!g_initialized || !fence) {
        return GEOFENCE_ERROR_NOT_INITIALIZED;
    }
    
    memset(fence, 0, sizeof(geofence_t));
    
    if (num_vertices > 0) {
        fence->vertices = malloc(num_vertices * sizeof(geopoint_t));
        if (!fence->vertices) {
            return GEOFENCE_ERROR_OUT_OF_MEMORY;
        }
        
        memcpy(fence->vertices, vertices, num_vertices * sizeof(geopoint_t));
        fence->num_vertices = num_vertices;
        fence->capacity = num_vertices;
    }
    
    return GEOFENCE_SUCCESS;
}

/**
 * @brief Add a vertex to an existing polygon
 */
int geofence_add_vertex(geofence_t* fence, const geopoint_t* point) {
    if (!g_initialized || !fence || !point) {
        return GEOFENCE_ERROR_INVALID_PARAM;
    }
    
    if (fence->num_vertices >= fence->capacity) {
        return GEOFENCE_ERROR_TOO_MANY_VERTICES;
    }
    
    fence->vertices[fence->num_vertices] = *point;
    fence->num_vertices++;
    
    return GEOFENCE_SUCCESS;
}

/**
 * @brief Remove all vertices from a polygon
 */
void geofence_clear_polygon(geofence_t* fence) {
    if (!fence) {
        return;
    }
    
    fence->num_vertices = 0;
}

/**
 * @brief Destroy a geofence polygon and free its resources
 */
void geofence_destroy_polygon(geofence_t* fence) {
    if (!fence) {
        return;
    }
    
    if (fence->vertices) {
        free(fence->vertices);
        fence->vertices = NULL;
    }
    
    fence->num_vertices = 0;
    fence->capacity = 0;
}

/**
 * @brief Ray casting algorithm for point-in-polygon test
 */
bool geofence_is_point_in_polygon_ray_casting(const geopoint_t* point, const geofence_t* fence) {
    if (!g_initialized || !point || !fence) {
        return false;
    }
    
    if (fence->num_vertices < 3) {
        return false;
    }
    
    int crossings = 0;
    const uint16_t vertex_count = fence->num_vertices;
    
    for (uint16_t i = 0; i < vertex_count; i++) {
        const geopoint_t* v1 = &fence->vertices[i];
        const geopoint_t* v2 = &fence->vertices[(i + 1) % vertex_count];
        
        if (((v1->latitude > point->latitude) != (v2->latitude > point->latitude)) &&
            (point->longitude < (v2->longitude - v1->longitude) * 
             (point->latitude - v1->latitude) / (v2->latitude - v1->latitude) + v1->longitude)) {
            crossings++;
        }
    }
    
    return (crossings % 2) == 1;
}

/**
 * @brief Calculate distance between two geographic points
 */
double geofence_calculate_distance(const geopoint_t* point1, const geopoint_t* point2) {
    if (!point1 || !point2) {
        return -1.0;
    }
    
    double lat1_rad = DEG_TO_RAD(point1->latitude);
    double lat2_rad = DEG_TO_RAD(point2->latitude);
    double delta_lat_rad = DEG_TO_RAD(point2->latitude - point1->latitude);
    double delta_lon_rad = DEG_TO_RAD(point2->longitude - point1->longitude);
    
    double a = sin(delta_lat_rad/2) * sin(delta_lat_rad/2) +
               cos(lat1_rad) * cos(lat2_rad) * 
               sin(delta_lon_rad/2) * sin(delta_lon_rad/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return EARTH_RADIUS_METERS * c;
}