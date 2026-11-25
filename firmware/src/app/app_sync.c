/*
 * app_sync.c - fences from storage and from the cloud, and the telemetry
 *
 * The application thread is the only writer of the fence (fence_manager.h).
 * A fence downloaded by the cloud worker is applied here, and the result is
 * posted back so the cloud learns whether the device accepted it.
 */

#include <errno.h>

#include <zephyr/logging/log.h>

#include "app_internal.h"
#include "cloud_sync.h"
#include "fence_codec.h"
#include "gnss.h"

LOG_MODULE_DECLARE(app, LOG_LEVEL_INF);

#define HEARTBEAT_TICKS (CONFIG_DRVF_TELEMETRY_HEARTBEAT_S * APP_TICKS_PER_SECOND)
#define MIN_INTERVAL_TICKS (CONFIG_DRVF_TELEMETRY_MIN_INTERVAL_S * APP_TICKS_PER_SECOND)

/* At boot: use the stored fence, if any. */
void app_sync_load_stored_fence(struct app *app)
{
	const int rc = fence_manager_load(&app->fences);
	fence_codec_header_t info;

	if (rc == FENCE_MANAGER_OK && fence_manager_info(&app->fences, &info)) {
		LOG_INF("Stored fence loaded: id %u version %u", (unsigned int)info.fence_id,
			(unsigned int)info.fence_version);
		app_dispatch(app, APP_EVENT_FENCE_LOADED);
	} else if (rc == FENCE_MANAGER_NOTHING_STORED) {
		LOG_INF("No stored fence");
	} else {
		LOG_WRN("The stored fence is not usable (%d), ignoring it", rc);
	}
}

/* Apply a fence the cloud worker downloaded and report the outcome. */
static void apply_downloaded_fence(struct app *app)
{
	uint8_t blob[FENCE_CODEC_MAX_SIZE];
	size_t len = 0;
	fence_codec_header_t info = { 0, 0, 0 };
	int rc;
	int status;

	/* The demo fence is in use: the downloaded one comes again after the demo. */
	if (app->state == APP_STATE_DEMO)
		return;
	if (cloud_sync_get_fence(blob, sizeof(blob), &len) != 0)
		return;

	rc = fence_manager_apply(&app->fences, blob, len);

	/* A flash failure may be temporary: stay silent so the fence is downloaded again. */
	if (rc == FENCE_MANAGER_ERR_STORE) {
		LOG_WRN("The fence could not be stored, it will be tried again at the next poll");
		return;
	}
	/* A repeated download of the active fence counts as applied. */
	status = rc == FENCE_MANAGER_ALREADY_ACTIVE ? FENCE_MANAGER_OK : rc;
	if (status == FENCE_MANAGER_OK)
		(void)fence_manager_info(&app->fences, &info);

	if (rc == FENCE_MANAGER_OK) {
		LOG_INF("New fence applied: id %u version %u, %u vertices",
			(unsigned int)info.fence_id, (unsigned int)info.fence_version,
			(unsigned int)info.num_vertices);
		/* The old fence's last result does not describe the new one. */
		app->has_result = false;
		app_dispatch(app, APP_EVENT_FENCE_UPDATED);
	} else if (status != FENCE_MANAGER_OK) {
		LOG_WRN("Downloaded fence refused: %d", rc);
	}

	if (cloud_sync_post_report(blob, len, info.fence_version, status) != 0)
		LOG_WRN("The report of the fence could not be queued");
}

static telemetry_t build_report(const struct app *app)
{
	const bool monitoring = app->state == APP_STATE_MONITORING;
	fence_codec_header_t info = { 0, 0, 0 };
	const bool has_fence = fence_manager_info(&app->fences, &info);
	const telemetry_t report = {
		.state = app->state,
		.fence_id = has_fence ? info.fence_id : 0,
		.fence_version = has_fence ? info.fence_version : 0,
		.has_inside = monitoring && app->has_result,
		.inside = app->inside,
		.has_distance = monitoring && app->has_result,
		.distance_m = app->distance_m,
		.has_position = app->has_position,
		.latitude = app->position.latitude,
		.longitude = app->position.longitude,
		.fix_age_s = (app->tick - app->position_tick) / APP_TICKS_PER_SECOND,
	};

	return report;
}

/* Queue a report when something changed, or as a heartbeat. */
static void report_telemetry(struct app *app)
{
	const telemetry_config_t config = {
		.heartbeat_ticks = HEARTBEAT_TICKS,
		.min_interval_ticks = MIN_INTERVAL_TICKS,
	};
	telemetry_t report;

	/* The demo positions are simulated: never report them. */
	if (!IS_ENABLED(CONFIG_DRVF_CLOUD) || app->state == APP_STATE_DEMO)
		return;

	report = build_report(app);

	if (telemetry_due(&app->telemetry_policy, &config, &report, app->tick) &&
	    cloud_sync_post_telemetry(&report) != 0)
		LOG_WRN("Telemetry queue full, report dropped");
}

/* First-time provisioning stops the GNSS session: start it again. */
static void restart_gnss_if_needed(struct app *app)
{
	if (!cloud_sync_radio_was_reset() || !app->gnss_ok)
		return;

	if (gnss_restart() != 0)
		LOG_ERR("The GNSS could not be restarted after provisioning");
}

/* Called every tick. */
void app_sync_poll(struct app *app)
{
	restart_gnss_if_needed(app);
	apply_downloaded_fence(app);
	report_telemetry(app);
}
