/**
 * @file test_geofence.c
 * @brief Test suite for geofence implementation
 *
 * This test suite follows the principles from "Making Embedded Systems" by Elecia White:
 * - Comprehensive testing of all functionality
 * - Clear pass/fail criteria
 * - Performance measurement for real-time constraints
 * - Edge case validation
 */

#include "geofence.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

/* Test polygon representing a simple rectangular field */
static const geopoint_t test_field[] = {
    {40.7128, -74.0060},  /* Southwest corner */
    {40.7128, -74.0000},  /* Southeast corner */
    {40.7188, -74.0000},  /* Northeast corner */
    {40.7188, -74.0060}   /* Northwest corner */
};

#define TEST_FIELD_VERTICES (sizeof(test_field) / sizeof(test_field[0]))

/* Test cases for point-in-polygon algorithm */
typedef struct {
    geopoint_t point;
    bool expected_result;
    const char* description;
} test_case_t;

static const test_case_t test_cases[] = {
    {{40.7158, -74.0030}, true,  "Point inside polygon"},
    {{40.7100, -74.0030}, false, "Point outside polygon"},
    {{40.7128, -74.0060}, true,  "Point on vertex"},
    {{40.7128, -74.0030}, true,  "Point on edge"},
    {{40.7158, -74.0060}, true,  "Point on vertical edge"}
};

#define NUM_TEST_CASES (sizeof(test_cases) / sizeof(test_cases[0]))

/**
 * @brief Run basic functionality tests
 */
static bool run_functionality_tests(void) {
    printf("Running functionality tests...\n");
    
    int passed = 0;
    int total = 0;
    
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
    
    /* Run test cases */
    for (size_t i = 0; i < NUM_TEST_CASES; i++) {
        const test_case_t* test = &test_cases[i];
        bool result = geofence_is_point_in_polygon_ray_casting(&test->point, &fence);
        
        total++;
        if (result == test->expected_result) {
            passed++;
            printf("PASS: %s\n", test->description);
        } else {
            printf("FAIL: %s (expected %s, got %s)\n", 
                   test->description,
                   test->expected_result ? "inside" : "outside",
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
    
    /* Test point for performance measurement */
    geopoint_t test_point = {40.7158, -74.0030};
    
    /* Measure execution time */
    clock_t start = clock();
    
    /* Run point-in-polygon test many times */
    const int iterations = 10000;
    for (int i = 0; i < iterations; i++) {
        geofence_is_point_in_polygon_ray_casting(&test_point, &fence);
    }
    
    clock_t end = clock();
    double elapsed_ms = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC;
    double avg_time_us = (elapsed_ms * 1000.0) / iterations;
    
    printf("Average execution time: %.2f microseconds per test\n", avg_time_us);
    
    /* Check if performance meets real-time requirements (< 50ms for all operations) */
    bool performance_ok = (avg_time_us < 50.0);
    printf("Performance requirement met: %s\n", performance_ok ? "YES" : "NO");
    
    /* Clean up */
    geofence_destroy_polygon(&fence);
    geofence_deinit();
    
    return performance_ok;
}

/**
 * @brief Main test function
 */
int main(void) {
    printf("=== Geofence Test Suite ===\n");
    printf("Following Elecia White's principle of comprehensive testing\n\n");
    
    bool functionality_passed = run_functionality_tests();
    bool performance_passed = run_performance_tests();
    
    printf("\n=== Test Results ===\n");
    printf("Functionality tests: %s\n", functionality_passed ? "PASSED" : "FAILED");
    printf("Performance tests: %s\n", performance_passed ? "PASSED" : "FAILED");
    
    if (functionality_passed && performance_passed) {
        printf("\nAll tests PASSED!\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}