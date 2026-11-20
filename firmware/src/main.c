/*
 * main.c - Step 1 proof of concept: geofence engine + RGB alert on the
 * Nordic Thingy:91 X, driven by a SIMULATED route (no GNSS yet).
 *
 * The route walks from 40 m inside the south edge of the test field to
 * 10 m outside and back, in 2 m steps. Per step it runs geofence_check(),
 * maps the distance to a color and logs the check duration (first RF-02
 * data on real hardware).
 *
 * PoC exception to the deep-sleep rule: the loop wakes every TICK_MS to
 * blink the LED and advance the simulated route. The real application will
 * sleep and wake on the IMU interrupt once GNSS and the FSM are added.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "alert_color.h"
#include "geofence.h"
#include "rgb_led.h"
#include "timing.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define TICK_MS            250
#define TICKS_PER_STEP     4
#define SAFE_DISTANCE_M    20.0
#define STEP_M             2
#define START_OFFSET_M     40    /* meters inside the south edge */
#define END_OFFSET_M       (-10) /* meters outside the south edge */
#define METERS_PER_DEG_LAT 111195.0

#define FENCE_MAX_POLYGONS 1
#define FENCE_NO_TOLERANCE 0.0

/* Same rectangular field as the host tests. Vertices 0 and 1 form the south edge. */
static const geopoint_t field[] = {
	{ 40.7128, -74.0060 }, /* Southwest */
	{ 40.7128, -74.0000 }, /* Southeast */
	{ 40.7188, -74.0000 }, /* Northeast */
	{ 40.7188, -74.0060 }, /* Northwest */
};

/* The simulated route moves along the south-north axis, offset from the south edge. */
enum route_direction {
	ROUTE_TOWARD_END = -1,   /* offset decreasing: from inside toward outside */
	ROUTE_TOWARD_START = 1,  /* offset increasing: back to the start */
};

struct route {
	int offset_m;
	enum route_direction direction;
};

/* LED output state machine: a steady color, or a color blinking on and off. */
enum led_state {
	LED_STATE_SOLID,
	LED_STATE_BLINK,
};

struct app {
	geofence_t fence;
	struct route route;
	alert_color_t color;
	enum led_state led;
	bool blink_on;
	uint32_t tick;
};

/* Fail safe: same color as outside the fence or an invalid reading. */
static alert_color_t failsafe_color(void)
{
	return alert_color_from_distance(NAN, false, SAFE_DISTANCE_M);
}

static geopoint_t route_point(const struct route *route)
{
	geopoint_t point = {
		.latitude = field[0].latitude + route->offset_m / METERS_PER_DEG_LAT,
		.longitude = (field[0].longitude + field[1].longitude) / 2.0,
	};

	return point;
}

static void route_advance(struct route *route)
{
	route->offset_m += route->direction * STEP_M;
	if (route->offset_m <= END_OFFSET_M)
		route->direction = ROUTE_TOWARD_START;
	else if (route->offset_m >= START_OFFSET_M)
		route->direction = ROUTE_TOWARD_END;
}

static void set_color(struct app *app, alert_color_t color)
{
	app->color = color;
	app->led = color.blink ? LED_STATE_BLINK : LED_STATE_SOLID;
}

/* Run one geofence check on the current route point and update the color. */
static void check_step(struct app *app)
{
	const geopoint_t point = route_point(&app->route);
	geofence_result_t result;
	const timing_stamp_t start = timing_now();
	const int ret = geofence_check(&point, &app->fence, &result);
	const uint32_t check_us = timing_elapsed_us(start);

	if (ret == GEOFENCE_SUCCESS) {
		set_color(app, alert_color_from_distance(result.distance_m, result.inside,
							 SAFE_DISTANCE_M));
		LOG_INF("offset %d m inside=%d dist=%.2f m rgb=%u,%u,%u blink=%d check=%u us",
			app->route.offset_m, result.inside, result.distance_m,
			app->color.r, app->color.g, app->color.b, app->color.blink, check_us);
	} else {
		/* Bad fix: never confuse it with "outside", show the fail-safe color. */
		set_color(app, failsafe_color());
		LOG_WRN("geofence_check error %d", ret);
	}

	route_advance(&app->route);
}

/* Drive the LED according to the current state; called every tick. */
static void led_update(struct app *app)
{
	alert_color_t out = app->color;
	int ret;

	switch (app->led) {
	case LED_STATE_BLINK:
		app->blink_on = !app->blink_on;
		if (!app->blink_on) {
			out.r = 0;
			out.g = 0;
			out.b = 0;
		}
		break;
	case LED_STATE_SOLID:
	default:
		app->blink_on = true;
		break;
	}

	ret = rgb_led_set(out.r, out.g, out.b);
	if (ret < 0)
		LOG_WRN("rgb_led_set failed: %d", ret);
}

int main(void)
{
	const geofence_config_t config = {
		.max_polygons = FENCE_MAX_POLYGONS,
		.max_vertices_total = ARRAY_SIZE(field),
		.tolerance_meters = FENCE_NO_TOLERANCE,
	};
	struct app app = {
		.route = { .offset_m = START_OFFSET_M, .direction = ROUTE_TOWARD_END },
	};
	int ret;

	set_color(&app, failsafe_color());

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

	ret = geofence_create_polygon(&app.fence, field, ARRAY_SIZE(field));
	if (ret != GEOFENCE_SUCCESS) {
		LOG_ERR("geofence_create_polygon failed: %d", ret);
		return ret;
	}

	LOG_INF("Geofence ready, %u vertices, safe distance %d m",
		app.fence.num_vertices, (int)SAFE_DISTANCE_M);

	while (true) {
		if (app.tick % TICKS_PER_STEP == 0)
			check_step(&app);

		led_update(&app);
		app.tick++;
		timing_sleep_ms(TICK_MS);
	}

	return 0;
}
