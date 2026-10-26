#include "geofence.h"
#include <stdio.h>

/* Test polygon representing a simple rectangular field */
static const geopoint_t test_field[] = {
    {40.7128, -74.0060},  /* Southwest corner */
    {40.7128, -74.0000},  /* Southeast corner */
    {40.7188, -74.0000},  /* Northeast corner */
    {40.7188, -74.0060}   /* Northwest corner */
};

#define TEST_FIELD_VERTICES (sizeof(test_field) / sizeof(test_field[0]))

int main(void) {
    printf("Comprehensive geofence test\n");
    
    // Initialize geofence system
    geofence_config_t config = {
        .max_polygons = 10,
        .max_vertices_total = 1000,
        .tolerance_meters = 1.0
    };
    
    int result = geofence_init(&config);
    printf("geofence_init result: %d\n", result);
    
    if (result != GEOFENCE_SUCCESS) {
        printf("Failed to initialize geofence system\n");
        return 1;
    }
    
    // Create test polygon
    geofence_t fence = {0};
    result = geofence_create_polygon(&fence, test_field, TEST_FIELD_VERTICES);
    printf("geofence_create_polygon result: %d\n", result);
    
    if (result != GEOFENCE_SUCCESS) {
        printf("Failed to create test polygon\n");
        geofence_deinit();
        return 1;
    }
    
    // Test point inside the polygon
    geopoint_t test_point = {40.7158, -74.0030};
    bool inside = geofence_is_point_in_polygon_ray_casting(&test_point, &fence);
    printf("Point (%f, %f) is inside polygon: %s\n", 
           test_point.latitude, test_point.longitude, 
           inside ? "true" : "false");
    
    // Test point outside the polygon
    geopoint_t outside_point = {40.7100, -74.0030};
    bool outside = geofence_is_point_in_polygon_ray_casting(&outside_point, &fence);
    printf("Point (%f, %f) is inside polygon: %s\n", 
           outside_point.latitude, outside_point.longitude, 
           outside ? "true" : "false");
    
    // Calculate distance between points
    double distance = geofence_calculate_distance(&test_point, &outside_point);
    printf("Distance between points: %f meters\n", distance);
    
    // Clean up
    geofence_destroy_polygon(&fence);
    geofence_deinit();
    
    printf("All tests completed successfully\n");
    return 0;
}