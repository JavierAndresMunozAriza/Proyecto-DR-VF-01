/*
 * telemetry.c - telemetry report: JSON payload and reporting policy
 *
 * Pure C, no hardware dependency. See telemetry.h.
 */

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "telemetry.h"

#define LAT_LON_DECIMALS 7 /* about 1 cm */
#define DISTANCE_DECIMALS 2

struct writer {
	char *buf;
	size_t len;
	size_t pos;
	bool failed;
};

static void put(struct writer *w, const char *fmt, ...)
{
	va_list args;
	int n;

	if (w->failed)
		return;
	va_start(args, fmt);
	n = vsnprintf(w->buf + w->pos, w->len - w->pos, fmt, args);
	va_end(args);
	if (n < 0 || (size_t)n >= w->len - w->pos) {
		w->failed = true;
		return;
	}
	w->pos += (size_t)n;
}

/* Letters and digits only, so the code can go in a JSON string unescaped. */
static bool operator_is_plain(const char *code)
{
	size_t n = 0;

	for (; code[n] != '\0'; n++) {
		const char c = code[n];
		const bool plain = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
				   (c >= 'a' && c <= 'z');

		if (!plain || n >= TELEMETRY_OPERATOR_MAX)
			return false;
	}
	return n > 0;
}

int telemetry_format_json(const telemetry_t *report, char *buf, size_t len)
{
	struct writer w = { buf, len, 0, false };

	if (report == NULL || buf == NULL || len == 0)
		return -1;
	if (report->has_distance && !isfinite(report->distance_m))
		return -1;
	if (report->has_position &&
	    (!isfinite(report->latitude) || !isfinite(report->longitude)))
		return -1;

	put(&w, "{\"state\":\"%s\",\"fence_id\":%lu,\"fence_version\":%lu",
	    app_fsm_state_name(report->state), (unsigned long)report->fence_id,
	    (unsigned long)report->fence_version);
	if (report->has_inside)
		put(&w, ",\"inside\":%s", report->inside ? "true" : "false");
	if (report->has_distance)
		put(&w, ",\"dist_m\":%.*f", DISTANCE_DECIMALS, report->distance_m);
	if (report->has_position)
		put(&w, ",\"lat\":%.*f,\"lon\":%.*f,\"fix_age_s\":%lu", LAT_LON_DECIMALS,
		    report->latitude, LAT_LON_DECIMALS, report->longitude,
		    (unsigned long)report->fix_age_s);
	if (report->has_rsrp)
		put(&w, ",\"rsrp_dbm\":%d", (int)report->rsrp_dbm);
	if (report->band != 0)
		put(&w, ",\"band\":%u", (unsigned int)report->band);
	if (report->operator_id != NULL && operator_is_plain(report->operator_id))
		put(&w, ",\"operator\":\"%s\"", report->operator_id);
	put(&w, "}");

	return w.failed ? -1 : (int)w.pos;
}

static bool changed_since_last(const telemetry_policy_t *policy, const telemetry_t *report)
{
	if (report->state != policy->state ||
	    report->fence_version != policy->fence_version ||
	    report->has_inside != policy->has_inside)
		return true;
	return report->has_inside && report->inside != policy->inside;
}

bool telemetry_due(telemetry_policy_t *policy, const telemetry_config_t *config,
		   const telemetry_t *report, uint32_t tick)
{
	/* Unsigned subtraction keeps working when the tick counter wraps. */
	const uint32_t elapsed = tick - policy->last_sent_tick;
	bool due;

	if (policy->sent_before) {
		if (elapsed < config->min_interval_ticks)
			return false;
		due = changed_since_last(policy, report) ||
		      (config->heartbeat_ticks != 0 && elapsed >= config->heartbeat_ticks);
		if (!due)
			return false;
	}

	policy->sent_before = true;
	policy->state = report->state;
	policy->has_inside = report->has_inside;
	policy->inside = report->inside;
	policy->fence_version = report->fence_version;
	policy->last_sent_tick = tick;
	return true;
}
