/*
 * app_led.c - drive the RGB LED from the application state
 */

#include <math.h>

#include <zephyr/logging/log.h>

#include "alert_color.h"
#include "app_internal.h"
#include "rgb_led.h"

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

/* Fail safe: same color as outside the fence or an invalid reading. */
alert_color_t app_failsafe_color(void)
{
	return alert_color_from_distance(NAN, false, APP_SAFE_DISTANCE_M);
}

/* Color for the current state, before blinking is applied. */
static alert_color_t state_color(const struct app *app)
{
	switch (app->state) {
	case APP_STATE_NO_FENCE:
	case APP_STATE_WAIT_FIX:
		return alert_color_searching(app->poor_fix_seen &&
					     app->tick - app->poor_fix_tick <=
						     APP_POOR_FIX_HOLD_TICKS);
	case APP_STATE_MONITORING:
	case APP_STATE_DEMO:
		return app->color;
	case APP_STATE_FIX_LOST:
		return alert_color_fix_lost();
	case APP_STATE_FAULT:
	default:
		return app_failsafe_color();
	}
}

/* Drive the LED from the current state; called every tick. */
void app_led_update(struct app *app)
{
	alert_color_t out = state_color(app);
	int ret;

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
