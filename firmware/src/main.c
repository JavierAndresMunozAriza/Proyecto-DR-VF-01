/*
 * main.c - Proof of concept: real GNSS + geofence + RGB alert on the Nordic
 * Thingy:91 X, with a button-activated demo mode.
 *
 * Application states (see app_fsm.h):
 *  - SEARCHING: the LED blinks blue until the first accurate fix, which
 *    defines the fence: a square centered on that position. It blinks cyan
 *    while fixes arrive but are too inaccurate to use.
 *  - MONITORING: every fix is checked against the fence and the distance to
 *    its boundary drives the LED (green far from the edge, toward red as it
 *    gets closer, blinking red outside).
 *  - FIX_LOST: fixes stopped arriving, the LED blinks magenta (not red, so
 *    it is not confused with "outside the fence") until one returns.
 *  - FAULT: the GNSS could not start, fail-safe blinking red.
 *  - DEMO: pressing the button plays a simulated walk across a fence around
 *    a fixed point, so the alert can be shown without GNSS signal (indoors).
 *    Pressing the button again returns to SEARCHING.
 * The duration of each geofence check is logged (RF-02 data on real hardware).
 *
 * PoC exception to the deep-sleep rule: the loop wakes every TICK_MS to
 * blink the LED, poll for fixes and read the button. The real application
 * will sleep and wake on the IMU interrupt.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "alert_color.h"
#include "app_fsm.h"
#include "button.h"
#include "demo_route.h"
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
#define POOR_FIX_HOLD_TICKS  (3 * TICKS_PER_SECOND) /* how long the cyan hint stays */
#define DEMO_STEP_TICKS      TICKS_PER_SECOND       /* one demo step per second */

#define FENCE_MAX_POLYGONS   1
#define FENCE_NO_TOLERANCE   0.0

/* Fictitious center of the demo fence; the demo never uses the real position. */
#define DEMO_CENTER_LAT      4.6097
#define DEMO_CENTER_LON      (-74.0817)

struct app {
	geofence_t fence;
	bool has_fence;
	bool gnss_ok;
	enum app_state state;
	alert_color_t color; /* alert color while MONITORING or in the DEMO */
	bool blink_on;
	uint32_t tick;
	uint32_t last_fix_tick;
	bool poor_fix_seen;
	uint32_t poor_fix_tick;
	uint32_t demo_step;
};

static const geopoint_t demo_center = { DEMO_CENTER_LAT, DEMO_CENTER_LON };

static const char *const state_name[] = {
	[APP_STATE_SEARCHING] = "SEARCHING",
	[APP_STATE_MONITORING] = "MONITORING",
	[APP_STATE_FIX_LOST] = "FIX_LOST",
	[APP_STATE_FAULT] = "FAULT",
	[APP_STATE_DEMO] = "DEMO",
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

/* Create the fence centered on a point. Returns 0 on success. */
static int create_fence(struct app *app, const geopoint_t *center)
{
	geopoint_t corners[FENCE_SQUARE_VERTICES];
	int ret;

	if (fence_square_vertices(center, FENCE_HALF_SIDE_M, corners) != 0) {
		LOG_WRN("Point not usable as a fence center");
		return -1;
	}

	ret = geofence_create_polygon(&app->fence, corners, FENCE_SQUARE_VERTICES);
	if (ret != GEOFENCE_SUCCESS) {
		LOG_ERR("geofence_create_polygon failed: %d", ret);
		return -1;
	}

	app->has_fence = true;
	LOG_INF("Fence created: %.0f m square around %.6f, %.6f",
		2.0 * FENCE_HALF_SIDE_M, center->latitude, center->longitude);

	return 0;
}

/* The engine requires destroying a polygon before creating another one. */
static void release_fence(struct app *app)
{
	if (!app->has_fence)
		return;

	geofence_destroy_polygon(&app->fence);
	app->has_fence = false;
}

/* Check a point against the fence and update the alert color. */
static void evaluate_point(struct app *app, const char *source, const geopoint_t *point,
			   float accuracy_m)
{
	geofence_result_t result;
	timing_stamp_t start = timing_now();
	const int ret = geofence_check(point, &app->fence, &result);
	const uint32_t check_us = timing_elapsed_us(start);

	if (ret != GEOFENCE_SUCCESS) {
		/* Bad point: never confuse it with "outside", show the fail-safe color. */
		app->color = failsafe_color();
		LOG_WRN("geofence_check error %d", ret);
		return;
	}

	app->color = alert_color_from_distance(result.distance_m, result.inside,
					       SAFE_DISTANCE_M);
	LOG_INF("%s %.6f, %.6f acc=%.1f m inside=%d dist=%.2f m rgb=%u,%u,%u blink=%d check=%u us",
		source, point->latitude, point->longitude, (double)accuracy_m, result.inside,
		result.distance_m, app->color.r, app->color.g, app->color.b,
		app->color.blink, check_us);
}

/* Handle one GNSS fix: gate its accuracy, create the fence if needed, check it. */
static void process_fix(struct app *app, const gnss_fix_t *fix)
{
	const geopoint_t point = { fix->latitude, fix->longitude };

	if (fix->accuracy_m > MAX_FIX_ACCURACY_M) {
		LOG_INF("Fix ignored, accuracy %.1f m is worse than %.0f m",
			(double)fix->accuracy_m, (double)MAX_FIX_ACCURACY_M);
		app->poor_fix_seen = true;
		app->poor_fix_tick = app->tick;
		return;
	}

	if (app->state == APP_STATE_SEARCHING) {
		if (create_fence(app, &point) != 0)
			return;
		dispatch(app, APP_EVENT_FENCE_CREATED);
	}

	evaluate_point(app, "fix", &point, fix->accuracy_m);
	app->last_fix_tick = app->tick;
	dispatch(app, APP_EVENT_FIX);
}

/* Advance the simulated walk by one step and check it against the demo fence. */
static void demo_step(struct app *app)
{
	geopoint_t point;

	if (demo_route_point(&demo_center, app->demo_step++, &point) == 0)
		evaluate_point(app, "demo", &point, 0.0f);
}

/* The button toggles the demo: enter it with its own fence, or leave it for SEARCHING. */
static void toggle_demo(struct app *app)
{
	release_fence(app);
	dispatch(app, APP_EVENT_DEMO_TOGGLE);

	if (app->state == APP_STATE_DEMO) {
		app->demo_step = 0;
		if (create_fence(app, &demo_center) == 0) {
			demo_step(app);
			return;
		}
		/* Could not build the demo fence: leave the demo again. */
		dispatch(app, APP_EVENT_DEMO_TOGGLE);
	}

	/* Back to searching: the next accurate fix creates a new fence. */
	app->poor_fix_seen = false;
	if (!app->gnss_ok)
		dispatch(app, APP_EVENT_GNSS_FAILED);
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
		out = alert_color_searching(app->poor_fix_seen &&
					    app->tick - app->poor_fix_tick <= POOR_FIX_HOLD_TICKS);
		break;
	case APP_STATE_MONITORING:
	case APP_STATE_DEMO:
		out = app->color;
		break;
	case APP_STATE_FIX_LOST:
		out = alert_color_fix_lost();
		break;
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

	ret = button_init();
	if (ret < 0)
		LOG_WRN("Button not available, demo mode disabled: %d", ret);

	app.gnss_ok = gnss_open() == 0;
	if (!app.gnss_ok) {
		LOG_ERR("GNSS not available");
		dispatch(&app, APP_EVENT_GNSS_FAILED);
	}

	while (true) {
		if (button_pressed())
			toggle_demo(&app);

		if (app.state == APP_STATE_DEMO) {
			if (app.tick % DEMO_STEP_TICKS == 0)
				demo_step(&app);
		} else if (app.state != APP_STATE_FAULT && gnss_read(&fix, 0) == 0) {
			process_fix(&app, &fix);
		}

		check_fix_timeout(&app);
		log_search_progress(&app);
		led_update(&app);
		app.tick++;
		timing_sleep_ms(TICK_MS);
	}

	return 0;
}
