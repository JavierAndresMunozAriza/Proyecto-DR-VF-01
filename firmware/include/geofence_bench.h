/**
 * @file geofence_bench.h
 * @brief On-target timing benchmark of geofence_check()
 *
 * Enabled with CONFIG_DRVF_GEOFENCE_BENCHMARK. It times batches of
 * geofence_check() calls on regular polygons with a growing number of
 * vertices and logs the mean time per call.
 */

#ifndef GEOFENCE_BENCH_H
#define GEOFENCE_BENCH_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Run the benchmark once and log the results
 *
 * The geofence engine must be initialized. Takes a few seconds, so call it
 * before the application starts its normal work.
 */
void geofence_bench_run(void);

#ifdef __cplusplus
}
#endif

#endif /* GEOFENCE_BENCH_H */
