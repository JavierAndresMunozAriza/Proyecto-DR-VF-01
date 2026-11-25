/*
 * app_demo.c - the button demo: a simulated walk across a fence
 *
 * Pressing the button plays a walk around a fixed point so the alert can be
 * shown without GNSS signal. Pressing it again leaves the demo, drops the
 * demo fence and brings back the stored fence, if there is one.
 */

#include <zephyr/logging/log.h>

#include "app_internal.h"
#include "demo_route.h"

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

static const geopoint_t demo_center = { DEMO_CENTER_LAT, DEMO_CENTER_LON };

/* Advance the simulated walk by one step and check it against the demo fence. */
void app_demo_step(struct app *app)
{
	geopoint_t point;

	if (demo_route_point(&demo_center, app->demo_step++, &point) == 0)
		app_evaluate_point(app, "demo", &point, 0.0f);
}

/* Forget the last result: it belongs to the fence that is about to change. */
static void clear_results(struct app *app)
{
	app->has_result = false;
	app->has_position = false;
}

/* Drop the demo fence, bring the stored one back and resume the normal states. */
static void leave_demo(struct app *app)
{
	fence_manager_release(&app->fences);
	clear_results(app);
	app->poor_fix_seen = false;

	if (fence_manager_load(&app->fences) == FENCE_MANAGER_OK)
		app_dispatch(app, APP_EVENT_FENCE_LOADED);
	if (!app->gnss_ok)
		app_dispatch(app, APP_EVENT_GNSS_FAILED);
}

/* The button toggles the demo: enter it with its own fence, or leave it. */
void app_demo_toggle(struct app *app)
{
	app_dispatch(app, APP_EVENT_DEMO_TOGGLE);

	if (app->state != APP_STATE_DEMO) {
		leave_demo(app);
		return;
	}

	app->demo_step = 0;
	clear_results(app);
	if (app_use_square_fence(app, &demo_center) == 0) {
		app_demo_step(app);
		return;
	}

	/* Could not build the demo fence: leave the demo again. */
	app_dispatch(app, APP_EVENT_DEMO_TOGGLE);
	leave_demo(app);
}
