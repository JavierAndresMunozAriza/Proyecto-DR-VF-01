/**
 * @file app.h
 * @brief Application entry point: initializes every module and runs the loop
 *
 * The application is split by responsibility under src/app/: app.c (start-up,
 * loop and state machine dispatch), app_fix.c (GNSS fixes and fence checks),
 * app_demo.c (the button demo), app_led.c (alert LED) and app_sync.c (fences
 * from storage and cloud, telemetry). main.c only calls app_run().
 */

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the application and run its loop
 * @return A negative errno if a mandatory module could not start; it does not
 *         return otherwise
 */
int app_run(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_H */
