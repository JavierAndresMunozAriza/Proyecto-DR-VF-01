#include "geofence.h"
#include <stdio.h>

int main(void) {
    printf("Simple geofence test\n");
    
    // Initialize geofence system
    geofence_config_t config = {
        .max_polygons = 10,
        .max_vertices_total = 1000,
        .tolerance_meters = 1.0
    };
    
    int result = geofence_init(&config);
    printf("geofence_init result: %d\n", result);
    
    if (result == GEOFENCE_SUCCESS) {
        printf("Geofence system initialized successfully\n");
        geofence_deinit();
    } else {
        printf("Failed to initialize geofence system\n");
        return 1;
    }
    
    return 0;
}