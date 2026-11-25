/*
 * app_fix.c - GNSS fixes: accuracy gate, fence checks, timeout, search progress
 */

#include <zephyr/logging/log.h>

#include "alert_color.h"
#include "app_internal.h"
#include "fence_square.h"
#include "geofence.h"
#include "gnss.h"
#include "timing.h"

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

/* Use a square fence centered on a point, without storing it. Returns 0 on success. */
int app_use_square_fence(struct app *app, const geopoint_t *center)
{
	geopoint_t corners[FENCE_SQUARE_VERTICES];

	if (fence_square_vertices(center, APP_FENCE_HALF_SIDE_M, corners) != 0) {
		LOG_WRN("Point not usable as a fence center");
		return -1;
	}

	if (fence_manager_use_transient(&app->fences, corners, FENCE_SQUARE_VERTICES) !=
	    FENCE_MANAGER_OK) {
		LOG_ERR("The fence could not be built");
		return -1;
	}

	LOG_INF("Fence created: %.0f m square around %.6f, %.6f", 2.0 * APP_FENCE_HALF_SIDE_M,
		center->latitude, center->longitude);
	return 0;
}

/* Check a point against the fence and update the alert color. */
void app_evaluate_point(struct app *app, const char *source, const geopoint_t *point,
			float accuracy_m)
{
	const geofence_t *fence = fence_manager_fence(&app->fences);
	geofence_result_t result;
	timing_stamp_t start;
	uint32_t check_us;
	int ret;

	if (fence == NULL)
		return;

	start = timing_now();
	ret = geofence_check(point, fence, &result);
	check_us = timing_elapsed_us(start);

	if (ret != GEOFENCE_SUCCESS) {
		/* Bad point: never confuse it with "outside", show the fail-safe color. */
		app->color = app_failsafe_color();
		app->has_result = false;
		LOG_WRN("geofence_check error %d", ret);
		return;
	}

	app->color = alert_color_from_distance(result.distance_m, result.inside,
					       APP_SAFE_DISTANCE_M);
	app->has_result = true;
	app->inside = result.inside;
	app->distance_m = result.distance_m;
	LOG_INF("%s %.6f, %.6f acc=%.1f m inside=%d dist=%.2f m rgb=%u,%u,%u blink=%d check=%u us",
		source, point->latitude, point->longitude, (double)accuracy_m, result.inside,
		result.distance_m, app->color.r, app->color.g, app->color.b, app->color.blink,
		check_us);
}

/* Handle one GNSS fix: gate its accuracy, make the fence if needed, check it. */
void app_process_fix(struct app *app, const gnss_fix_t *fix)
{
	const geopoint_t point = { fix->latitude, fix->longitude };

	if (fix->accuracy_m > APP_MAX_FIX_ACCURACY_M) {
		LOG_INF("Fix ignored, accuracy %.1f m is worse than %.0f m",
			(double)fix->accuracy_m, (double)APP_MAX_FIX_ACCURACY_M);
		app->poor_fix_seen = true;
		app->poor_fix_tick = app->tick;
		return;
	}

	if (app->state == APP_STATE_NO_FENCE) {
		/* Without a stored or downloaded fence, the first fix defines one. */
		if (!IS_ENABLED(CONFIG_DRVF_FENCE_AUTO_SQUARE))
			return;
		if (app_use_square_fence(app, &point) != 0)
			return;
		app_dispatch(app, APP_EVENT_FENCE_CREATED);
	}

	app_evaluate_point(app, "fix", &point, fix->accuracy_m);
	app->position = point;
	app->has_position = true;
	app->position_tick = app->tick;
	app->last_fix_tick = app->tick;
	app_dispatch(app, APP_EVENT_FIX);
}

void app_check_fix_timeout(struct app *app)
{
	if (app->state == APP_STATE_MONITORING &&
	    app->tick - app->last_fix_tick > APP_FIX_TIMEOUT_TICKS)
		app_dispatch(app, APP_EVENT_FIX_TIMEOUT);
}

/* While no fix is available, tell once per second how many satellites are tracked. */
void app_log_search_progress(const struct app *app)
{
	gnss_satellites_t satellites;
	const bool searching = app->state == APP_STATE_NO_FENCE ||
			       app->state == APP_STATE_WAIT_FIX ||
			       app->state == APP_STATE_FIX_LOST;

	if (!searching || app->tick % APP_TICKS_PER_SECOND != 0)
		return;

	if (gnss_ioctl(GNSS_IOCTL_GET_SATELLITES, &satellites) == 0)
		LOG_INF("%s: %u satellites tracked (GPS %u, QZSS %u, Galileo %u)",
			app_fsm_state_name(app->state), satellites.total, satellites.gps,
			satellites.qzss, satellites.galileo);
}
