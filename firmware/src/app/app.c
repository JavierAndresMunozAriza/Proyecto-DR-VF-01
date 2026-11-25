/*
 * app.c - application start-up, main loop and state machine dispatch
 *
 * Application states (see app_fsm.h):
 *  - NO_FENCE: no fence yet. The LED blinks blue. With CONFIG_DRVF_FENCE_AUTO_SQUARE
 *    the first accurate fix defines a square fence around it; otherwise the
 *    device waits for a fence from storage or the cloud. It blinks cyan while
 *    fixes arrive but are too inaccurate to use.
 *  - WAIT_FIX: a fence is ready, waiting for the first accurate fix.
 *  - MONITORING: every fix is checked against the fence and the distance to
 *    its boundary drives the LED (green far from the edge, toward red as it
 *    gets closer, blinking red outside).
 *  - FIX_LOST: fixes stopped arriving, the LED blinks magenta (not red, so
 *    it is not confused with "outside the fence") until one returns.
 *  - FAULT: the GNSS could not start, fail-safe blinking red.
 *  - DEMO: pressing the button plays a simulated walk across a fence around
 *    a fixed point, so the alert can be shown without GNSS signal (indoors).
 *    Pressing the button again leaves the demo.
 * The duration of each geofence check is logged (RF-02 data on real hardware).
 *
 * PoC exception to the deep-sleep rule: the loop wakes every APP_TICK_MS to
 * blink the LED, poll for fixes and read the button. The real application
 * will sleep and wake on the IMU interrupt.
 */

#include <stdbool.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "app.h"
#include "app_internal.h"
#include "button.h"
#include "cloud_sync.h"
#include "fence_codec.h"
#include "fence_store.h"
#include "geofence.h"
#include "geofence_bench.h"
#include "gnss.h"
#include "rgb_led.h"
#include "timing.h"

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define FENCE_MAX_POLYGONS 1
#define FENCE_NO_TOLERANCE 0.0

/* One instance, in static memory: it holds the fence manager's work buffers. */
static struct app app = { .state = APP_STATE_NO_FENCE };

/* Apply an event to the state machine and log the transition. */
void app_dispatch(struct app *a, enum app_event event)
{
	const enum app_state next = app_fsm_next(a->state, event);

	if (next != a->state)
		LOG_INF("State %s -> %s", app_fsm_state_name(a->state), app_fsm_state_name(next));

	a->state = next;
}

static const fence_store_ops_t *fence_store(void)
{
	return IS_ENABLED(CONFIG_DRVF_FENCE_STORAGE) ? fence_store_settings() : NULL;
}

static int init_modules(void)
{
	const geofence_config_t config = {
		.max_polygons = FENCE_MAX_POLYGONS,
		.max_vertices_total = FENCE_CODEC_MAX_VERTICES,
		.tolerance_meters = FENCE_NO_TOLERANCE,
	};
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

	/* Measurement builds only: runs before the GNSS and the button start. */
	if (IS_ENABLED(CONFIG_DRVF_GEOFENCE_BENCHMARK))
		geofence_bench_run();

	ret = button_init();
	if (ret < 0)
		LOG_WRN("Button not available, demo mode disabled: %d", ret);

	fence_manager_init(&app.fences, fence_store());
	if (IS_ENABLED(CONFIG_DRVF_FENCE_STORAGE) && app.fences.store == NULL)
		LOG_ERR("Fence storage not available, fences will not survive a reboot");

	/* The GNSS adapter initializes the modem library: the cloud comes after it. */
	app.gnss_ok = gnss_open() == 0;
	if (!app.gnss_ok) {
		LOG_ERR("GNSS not available");
		app_dispatch(&app, APP_EVENT_GNSS_FAILED);
	}

	app_sync_load_stored_fence(&app);

	ret = cloud_sync_start();
	if (ret < 0)
		LOG_ERR("Cloud sync not started: %d", ret);

	return 0;
}

int app_run(void)
{
	gnss_fix_t fix;
	int ret = init_modules();

	if (ret < 0)
		return ret;

	while (true) {
		if (button_pressed())
			app_demo_toggle(&app);

		if (app.state == APP_STATE_DEMO) {
			if (app.tick % APP_DEMO_STEP_TICKS == 0)
				app_demo_step(&app);
		} else if (app.state != APP_STATE_FAULT && gnss_read(&fix, 0) == 0) {
			app_process_fix(&app, &fix);
		}

		app_check_fix_timeout(&app);
		app_log_search_progress(&app);
		app_sync_poll(&app);
		app_led_update(&app);
		app.tick++;
		timing_sleep_ms(APP_TICK_MS);
	}

	return 0;
}
