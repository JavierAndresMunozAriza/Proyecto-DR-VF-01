/*
 * main.c - Step 2 proof of concept: real GNSS + geofence + RGB alert on the
 * Nordic Thingy:91 X.
 *
 * Application states (see app_fsm.h):
 *  - SEARCHING: the LED blinks blue until the first accurate fix, which
 *    defines the fence: a square centered on that position.
 *  - MONITORING: every fix is checked against the fence and the distance to
 *    its boundary drives the LED (green far from the edge, toward red as it
 *    gets closer, blinking red outside).
 *  - FIX_LOST: fixes stopped arriving, the LED shows the fail-safe blinking
 *    red until one returns.
 *  - FAULT: the GNSS could not start, fail-safe blinking red.
 * The duration of each geofence check is logged (RF-02 data on real hardware).
 *
 * PoC exception to the deep-sleep rule: the loop wakes every TICK_MS to
 * blink the LED and poll for fixes. The real application will sleep and wake
 * on the IMU interrupt.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "alert_color.h"
#include "app_fsm.h"
#include "fence_square.h"
#include "geofence.h"
#include "gnss.h"
#include "rgb_led.h"
#include "timing.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define TICK_MS              250
#define TICKS_PER_SECOND     (MSEC_PER_SEC / TICK_MS)
#define SAFE_DISTANCE_M      20.0
#define FENCE_HALF_SIDE_M    30.0
#define MAX_FIX_ACCURACY_M   50.0f
#define FIX_TIMEOUT_TICKS    (10 * TICKS_PER_SECOND)
#define SEARCH_BLUE_LEVEL    255

#define FENCE_MAX_POLYGONS   1
#define FENCE_NO_TOLERANCE   0.0

struct app {
	geofence_t fence;
	enum app_state state;
	alert_color_t color; /* alert color while MONITORING */
	bool blink_on;
	uint32_t tick;
	uint32_t last_fix_tick;
};

static const char *const state_name[] = {
	[APP_STATE_SEARCHING] = "SEARCHING",
	[APP_STATE_MONITORING] = "MONITORING",
	[APP_STATE_FIX_LOST] = "FIX_LOST",
	[APP_STATE_FAULT] = "FAULT",
};

/* Fail safe: same color as outside the fence or an invalid reading. */
static alert_color_t failsafe_color(void)
{
	return alert_color_from_distance(NAN, false, SAFE_DISTANCE_M);
}

/* Apply an event to the state machine and log the transition. */
static void dispatch(struct app *app, enum app_event event)
{
	const enum app_state next = app_fsm_next(app->state, event);

	if (next != app->state)
		LOG_INF("State %s -> %s", state_name[app->state], state_name[next]);

	app->state = next;
}

/* Create the fence centered on the first accurate fix. Returns 0 on success. */
static int create_fence(struct app *app, const geopoint_t *center)
{
	geopoint_t corners[FENCE_SQUARE_VERTICES];
	int ret;

	if (fence_square_vertices(center, FENCE_HALF_SIDE_M, corners) != 0) {
		LOG_WRN("Fix not usable as a fence center");
		return -1;
	}

	ret = geofence_create_polygon(&app->fence, corners, FENCE_SQUARE_VERTICES);
	if (ret != GEOFENCE_SUCCESS) {
		LOG_ERR("geofence_create_polygon failed: %d", ret);
		return -1;
	}

	LOG_INF("Fence created: %.0f m square around %.6f, %.6f",
		2.0 * FENCE_HALF_SIDE_M, center->latitude, center->longitude);

	return 0;
}

/* Check one fix against the fence and update the alert color. */
static void process_fix(struct app *app, const gnss_fix_t *fix)
{
	const geopoint_t point = { fix->latitude, fix->longitude };
	geofence_result_t result;
	timing_stamp_t start;
	uint32_t check_us;
	int ret;

	if (fix->accuracy_m > MAX_FIX_ACCURACY_M) {
		LOG_INF("Fix ignored, accuracy %.1f m is worse than %.0f m",
			(double)fix->accuracy_m, (double)MAX_FIX_ACCURACY_M);
		return;
	}

	if (app->state == APP_STATE_SEARCHING) {
		if (create_fence(app, &point) != 0)
			return;
		dispatch(app, APP_EVENT_FENCE_CREATED);
	}

	start = timing_now();
	ret = geofence_check(&point, &app->fence, &result);
	check_us = timing_elapsed_us(start);

	app->last_fix_tick = app->tick;
	dispatch(app, APP_EVENT_FIX);

	if (ret != GEOFENCE_SUCCESS) {
		/* Bad fix: never confuse it with "outside", show the fail-safe color. */
		app->color = failsafe_color();
		LOG_WRN("geofence_check error %d", ret);
		return;
	}

	app->color = alert_color_from_distance(result.distance_m, result.inside,
					       SAFE_DISTANCE_M);
	LOG_INF("fix %.6f, %.6f acc=%.1f m inside=%d dist=%.2f m rgb=%u,%u,%u blink=%d check=%u us",
		fix->latitude, fix->longitude, (double)fix->accuracy_m, result.inside,
		result.distance_m, app->color.r, app->color.g, app->color.b,
		app->color.blink, check_us);
}

static void check_fix_timeout(struct app *app)
{
	if (app->state == APP_STATE_MONITORING &&
	    app->tick - app->last_fix_tick > FIX_TIMEOUT_TICKS)
		dispatch(app, APP_EVENT_FIX_TIMEOUT);
}

/* While no fix is available, tell once per second how many satellites are tracked. */
static void log_search_progress(const struct app *app)
{
	unsigned int satellites = 0;

	if ((app->state != APP_STATE_SEARCHING && app->state != APP_STATE_FIX_LOST) ||
	    app->tick % TICKS_PER_SECOND != 0)
		return;

	if (gnss_ioctl(GNSS_IOCTL_GET_SATELLITES, &satellites) == 0)
		LOG_INF("Searching: %u satellites tracked", satellites);
}

/* Drive the LED from the current state; called every tick. */
static void led_update(struct app *app)
{
	alert_color_t out;
	int ret;

	switch (app->state) {
	case APP_STATE_SEARCHING:
		out = (alert_color_t){ 0, 0, SEARCH_BLUE_LEVEL, true };
		break;
	case APP_STATE_MONITORING:
		out = app->color;
		break;
	case APP_STATE_FIX_LOST:
	case APP_STATE_FAULT:
	default:
		out = failsafe_color();
		break;
	}

	if (out.blink) {
		app->blink_on = !app->blink_on;
		if (!app->blink_on) {
			out.r = 0;
			out.g = 0;
			out.b = 0;
		}
	} else {
		app->blink_on = true;
	}

	ret = rgb_led_set(out.r, out.g, out.b);
	if (ret < 0)
		LOG_WRN("rgb_led_set failed: %d", ret);
}

int main(void)
{
	const geofence_config_t config = {
		.max_polygons = FENCE_MAX_POLYGONS,
		.max_vertices_total = FENCE_SQUARE_VERTICES,
		.tolerance_meters = FENCE_NO_TOLERANCE,
	};
	struct app app = { .state = APP_STATE_SEARCHING };
	gnss_fix_t fix;
	int ret;

	ret = rgb_led_init();
	if (ret < 0) {
		LOG_ERR("rgb_led_init failed: %d", ret);
		return ret;
	}

	ret = geofence_init(&config);
	if (ret != GEOFENCE_SUCCESS) {
		LOG_ERR("geofence_init failed: %d", ret);
		return ret;
	}

	if (gnss_open() != 0) {
		LOG_ERR("GNSS not available");
		dispatch(&app, APP_EVENT_GNSS_FAILED);
	}

	while (true) {
		if (app.state != APP_STATE_FAULT && gnss_read(&fix, 0) == 0)
			process_fix(&app, &fix);

		check_fix_timeout(&app);
		log_search_progress(&app);
		led_update(&app);
		app.tick++;
		timing_sleep_ms(TICK_MS);
	}

	return 0;
}
