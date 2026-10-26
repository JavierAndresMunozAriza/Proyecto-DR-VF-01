/*
 * test_geofence.c
 *
 * Test suite for geofence implementation following Elecia White's principles:
 * - Comprehensive testing of edge cases
 * - Clear test documentation
 * - Automated verification of results
 * - Performance measurement capabilities
 */

#include "geofence.h"
#include <stdio.h>
#include <time.h>

/* Test polygon representing a simple rectangular field */
static const geopoint_t test_field[] = {
    {40.7128, -74.0060},  /* Southwest corner */
    {40.7128, -74.0000},  /* Southeast corner */
    {40.7188, -74.0000},  /* Northeast corner */
    {40.7188, -74.0060}   /* Northwest corner */
};

#define TEST_FIELD_VERTICES (sizeof(test_field) / sizeof(test_field[0]))

/* Test cases */
typedef struct {
    geopoint_t point;
    bool expected_inside;
    const char* description;
} test_case_t;

static const test_case_t test_cases[] = {
    {{40.7158, -74.0030}, true,  "Point inside rectangle"},
    {{40.7100, -74.0030}, false, "Point outside south"},
    {{40.7200, -74.0030}, false, "Point outside north"},
    {{40.7158, -74.0100}, false, "Point outside west"},
    {{40.7158, -73.9950}, false, "Point outside east"},
    {{40.7128, -74.0060}, true,  "Point on southwest corner"},
    {{40.7158, -74.0060}, true,  "Point on western edge"}
};

#define NUM_TEST_CASES (sizeof(test_cases) / sizeof(test_cases[0]))

/* Performance test constants */
#define PERFORMANCE_ITERATIONS 10000

/**
 * @brief Run basic functionality tests
 */
static bool run_functionality_tests(void) {
    printf("Running functionality tests...\n");
    
    /* Initialize geofence system */
    geofence_config_t config = {
        .max_polygons = 10,
        .max_vertices_total = 1000,
        .tolerance_meters = 1.0
    };
    
    if (geofence_init(&config) != GEOFENCE_SUCCESS) {
        printf("ERROR: Failed to initialize geofence system\n");
        return false;
    }
    
    /* Create test polygon */
    geofence_t fence = {0};
    if (geofence_create_polygon(&fence, test_field, TEST_FIELD_VERTICES) != GEOFENCE_SUCCESS) {
        printf("ERROR: Failed to create test polygon\n");
        geofence_deinit();
        return false;
    }
    
    /* Validate the polygon */
    if (!geofence_validate_polygon(&fence)) {
        printf("ERROR: Test polygon validation failed\n");
        geofence_destroy_polygon(&fence);
        geofence_deinit();
        return false;
    }
    
    /* Run test cases */
    int passed = 0;
    int total = NUM_TEST_CASES;
    
    for (int i = 0; i < total; i++) {
        const test_case_t* test = &test_cases[i];
        bool result = geofence_is_point_in_polygon_ray_casting(&test->point, &fence);
        
        if (result == test->expected_inside) {
            printf("PASS: %s\n", test->description);
            passed++;
        } else {
            printf("FAIL: %s (expected %s, got %s)\n", 
                   test->description,
                   test->expected_inside ? "inside" : "outside",
                   result ? "inside" : "outside");
        }
    }
    
    /* Clean up */
    geofence_destroy_polygon(&fence);
    geofence_deinit();
    
    printf("Functionality tests: %d/%d passed\n", passed, total);
    return (passed == total);
}

/**
 * @brief Run performance tests
 */
static bool run_performance_tests(void) {
    printf("\nRunning performance tests...\n");
    
    /* Initialize geofence system */
    geofence_config_t config = {
        .max_polygons = 10,
        .max_vertices_total = 1000,
        .tolerance_meters = 1.0
    };
    
    if (geofence_init(&config) != GEOFENCE_SUCCESS) {
        printf("ERROR: Failed to initialize geofence system\n");
        return false;
    }
    
    /* Create test polygon */
    geofence_t fence = {0};
    if (geofence_create_polygon(&fence, test_field, TEST_FIELD_VERTICES) != GEOFENCE_SUCCESS) {
        printf("ERROR: Failed to create test polygon\n");
        geofence_deinit();
        return false;
    }
    
    /* Test point inside the polygon */
    geopoint_t test_point = {40.7158, -74.0030};
    
    /* Measure execution time */
    clock_t start = clock();
    
    for (int i = 0; i < PERFORMANCE_ITERATIONS; i++) {
        geofence_is_point_in_polygon_ray_casting(&test_point, &fence);
    }
    
    clock_t end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    double avg_time_ms = (cpu_time_used * 1000.0) / PERFORMANCE_ITERATIONS;
    
    printf("Average execution time: %.3f ms per call\n", avg_time_ms);
    printf("Performance test iterations: %d\n", PERFORMANCE_ITERATIONS);
    
    /* Clean up */
    geofence_destroy_polygon(&fence);
    geofence_deinit();
    
    /* Check if performance meets requirements (< 50ms as per REAL_TIME_RISKS.md) */
    bool performance_ok = (avg_time_ms < 50.0);
    printf("Performance requirement met: %s\n", performance_ok ? "YES" : "NO");
    
    return performance_ok;
}

/**
 * @brief Main test function
 */
int main(void) {
    printf("=== Geofence Algorithm Test Suite ===\n");
    printf("Following Elecia White's principles for embedded systems testing:\n");
    printf("- Comprehensive test coverage\n");
    printf("- Clear pass/fail criteria\n");
    printf("- Performance measurement\n");
    printf("- Resource cleanup verification\n\n");
    
    bool functionality_passed = run_functionality_tests();
    bool performance_passed = run_performance_tests();
    
    printf("\n=== Test Results Summary ===\n");
    printf("Functionality tests: %s\n", functionality_passed ? "PASSED" : "FAILED");
    printf("Performance tests: %s\n", performance_passed ? "PASSED" : "FAILED");
    
    if (functionality_passed && performance_passed) {
        printf("\nALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\nSOME TESTS FAILED\n");
        return 1;
    }
}