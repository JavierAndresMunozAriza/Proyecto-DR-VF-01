/*
 * app_internal.h - state shared by the files that make up the application
 *
 * Private to src/app/. Each file owns one responsibility and receives the
 * application state as an argument; there is no global state outside app.c.
 */

#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>

#include "alert_color.h"
#include "app_fsm.h"
#include "fence_manager.h"
#include "gnss.h"
#include "telemetry.h"

#define APP_TICK_MS            250
#define APP_TICKS_PER_SECOND   (MSEC_PER_SEC / APP_TICK_MS)
#define APP_SAFE_DISTANCE_M    20.0
#define APP_FENCE_HALF_SIDE_M  30.0
#define APP_MAX_FIX_ACCURACY_M 50.0f
#define APP_FIX_TIMEOUT_TICKS  (10 * APP_TICKS_PER_SECOND)
#define APP_POOR_FIX_HOLD_TICKS (3 * APP_TICKS_PER_SECOND) /* how long the cyan hint stays */
#define APP_DEMO_STEP_TICKS    APP_TICKS_PER_SECOND        /* one demo step per second */

struct app {
	fence_manager_t fences;
	enum app_state state;
	bool gnss_ok;

	/* LED */
	alert_color_t color; /* alert color while MONITORING or in the DEMO */
	bool blink_on;

	/* Time, in ticks of APP_TICK_MS */
	uint32_t tick;
	uint32_t last_fix_tick;
	bool poor_fix_seen;
	uint32_t poor_fix_tick;

	uint32_t demo_step;

	/* Last result, for the telemetry report */
	bool has_result;
	bool inside;
	double distance_m;
	bool has_position;
	geopoint_t position;
	uint32_t position_tick;

	telemetry_policy_t telemetry_policy;
};

/* app.c */
void app_dispatch(struct app *app, enum app_event event);

/* app_fix.c */
int app_use_square_fence(struct app *app, const geopoint_t *center);
void app_evaluate_point(struct app *app, const char *source, const geopoint_t *point,
			float accuracy_m);
void app_process_fix(struct app *app, const gnss_fix_t *fix);
void app_check_fix_timeout(struct app *app);
void app_log_search_progress(const struct app *app);

/* app_demo.c */
void app_demo_toggle(struct app *app);
void app_demo_step(struct app *app);

/* app_led.c */
alert_color_t app_failsafe_color(void);
void app_led_update(struct app *app);

/* app_sync.c */
void app_sync_load_stored_fence(struct app *app);
void app_sync_poll(struct app *app);

#endif /* APP_INTERNAL_H */
