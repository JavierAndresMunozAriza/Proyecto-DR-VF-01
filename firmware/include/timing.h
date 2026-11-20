/**
 * @file timing.h
 * @brief Sleep and elapsed-time helpers, isolating the RTOS timing API
 *
 * Application logic calls this module instead of the kernel directly, so the
 * logic does not depend on a specific RTOS or board.
 */

#ifndef TIMING_H
#define TIMING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque time stamp taken with timing_now() */
typedef uint32_t timing_stamp_t;

/** @brief Take a time stamp */
timing_stamp_t timing_now(void);

/** @brief Microseconds elapsed since @p start (valid for spans under ~1 h) */
uint32_t timing_elapsed_us(timing_stamp_t start);

/** @brief Sleep for @p ms milliseconds, yielding the CPU */
void timing_sleep_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* TIMING_H */
