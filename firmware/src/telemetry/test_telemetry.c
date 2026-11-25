/*
 * Host unit tests for the telemetry report. Exits non-zero if any check fails.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "telemetry.h"

static int failures;

#define CHECK(name, cond)                                  \
	do {                                               \
		if (cond) {                                \
			printf("ok   %s\n", name);         \
		} else {                                   \
			printf("FAIL %s\n", name);         \
			failures++;                        \
		}                                          \
	} while (0)

static telemetry_t full_report(void)
{
	telemetry_t t = {
		.state = APP_STATE_MONITORING,
		.fence_id = 7,
		.fence_version = 3,
		.has_inside = true,
		.inside = true,
		.has_distance = true,
		.distance_m = 12.345,
		.has_position = true,
		.latitude = 4.6097,
		.longitude = -74.0817,
		.fix_age_s = 4,
		.has_rsrp = true,
		.rsrp_dbm = -98,
		.band = 28,
		.operator_id = "732123",
	};

	return t;
}

static void test_json(void)
{
	char buf[TELEMETRY_JSON_MAX];
	telemetry_t t = full_report();
	int n;

	n = telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("a full report formats",
	      n > 0 && (size_t)n == strlen(buf));
	CHECK("a full report is exactly the documented JSON",
	      strcmp(buf, "{\"state\":\"MONITORING\",\"fence_id\":7,"
			  "\"fence_version\":3,\"inside\":true,\"dist_m\":12.35,"
			  "\"lat\":4.6097000,\"lon\":-74.0817000,\"fix_age_s\":4,"
			  "\"rsrp_dbm\":-98,\"band\":28,\"operator\":\"732123\"}") == 0);

	t = (telemetry_t){ .state = APP_STATE_NO_FENCE };
	n = telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("an empty report keeps only state and fence",
	      n > 0 && strcmp(buf, "{\"state\":\"NO_FENCE\",\"fence_id\":0,"
				   "\"fence_version\":0}") == 0);

	t = full_report();
	t.inside = false;
	telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("outside is reported as false", strstr(buf, "\"inside\":false") != NULL);

	t = full_report();
	t.has_inside = false;
	t.has_rsrp = false;
	t.has_position = false;
	t.has_distance = false;
	t.band = 0;
	t.operator_id = NULL;
	telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("absent fields are left out",
	      strstr(buf, "inside") == NULL && strstr(buf, "rsrp") == NULL &&
	      strstr(buf, "lat") == NULL && strstr(buf, "dist_m") == NULL &&
	      strstr(buf, "band") == NULL && strstr(buf, "operator") == NULL);

	t = full_report();
	t.operator_id = "73\"2,1";
	telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("an operator code that needs escaping is dropped",
	      strstr(buf, "operator") == NULL && buf[strlen(buf) - 1] == '}');

	t = full_report();
	t.operator_id = "123456789";
	telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("an operator code that is too long is dropped",
	      strstr(buf, "operator") == NULL);

	t = full_report();
	t.state = (enum app_state)99;
	telemetry_format_json(&t, buf, sizeof(buf));
	CHECK("an invalid state is reported as UNKNOWN",
	      strstr(buf, "\"state\":\"UNKNOWN\"") != NULL);
}

static void test_json_refusals(void)
{
	char buf[TELEMETRY_JSON_MAX];
	char small[20];
	telemetry_t t = full_report();

	CHECK("NULL report is refused", telemetry_format_json(NULL, buf, sizeof(buf)) == -1);
	CHECK("NULL buffer is refused", telemetry_format_json(&t, NULL, 100) == -1);
	CHECK("zero length is refused", telemetry_format_json(&t, buf, 0) == -1);
	CHECK("a buffer that is too small is refused",
	      telemetry_format_json(&t, small, sizeof(small)) == -1);
	t.distance_m = NAN;
	CHECK("a NaN distance is refused",
	      telemetry_format_json(&t, buf, sizeof(buf)) == -1);
	t = full_report();
	t.latitude = INFINITY;
	CHECK("an infinite latitude is refused",
	      telemetry_format_json(&t, buf, sizeof(buf)) == -1);
	t = full_report();
	CHECK("the largest report fits TELEMETRY_JSON_MAX",
	      telemetry_format_json(&t, buf, TELEMETRY_JSON_MAX) > 0);
}

static void test_policy(void)
{
	const telemetry_config_t cfg = { .heartbeat_ticks = 3600, .min_interval_ticks = 30 };
	telemetry_policy_t p = { 0 };
	telemetry_t t = full_report();

	CHECK("the first report is always due", telemetry_due(&p, &cfg, &t, 0));
	CHECK("an identical report right after is not due",
	      !telemetry_due(&p, &cfg, &t, 31));
	CHECK("an identical report just before the heartbeat is not due",
	      !telemetry_due(&p, &cfg, &t, 3599));
	CHECK("the heartbeat makes it due", telemetry_due(&p, &cfg, &t, 3600));

	t.inside = false;
	CHECK("going outside after the interval is due",
	      telemetry_due(&p, &cfg, &t, 3700));
	CHECK("sending again right away is not due",
	      !telemetry_due(&p, &cfg, &t, 3701));

	t.inside = true;
	CHECK("coming back inside too early waits for the interval",
	      !telemetry_due(&p, &cfg, &t, 3710));
	CHECK("the pending change fires once the interval has passed",
	      telemetry_due(&p, &cfg, &t, 3730));

	t.state = APP_STATE_FIX_LOST;
	CHECK("a state change is due", telemetry_due(&p, &cfg, &t, 4000));
	t.fence_version = 4;
	CHECK("a new fence version is due", telemetry_due(&p, &cfg, &t, 4100));
	t.has_inside = false;
	CHECK("losing the inside result is due", telemetry_due(&p, &cfg, &t, 4200));

	/* Changes in distance, position or signal alone are not reportable events. */
	t.distance_m += 50.0;
	t.latitude += 0.001;
	t.rsrp_dbm = -120;
	CHECK("a moving collar that stays on the same side is not due",
	      !telemetry_due(&p, &cfg, &t, 4300));
}

static void test_policy_edges(void)
{
	telemetry_config_t cfg = { .heartbeat_ticks = 0, .min_interval_ticks = 0 };
	telemetry_policy_t p = { 0 };
	telemetry_t t = full_report();

	CHECK("first report", telemetry_due(&p, &cfg, &t, 100));
	CHECK("heartbeat 0 never repeats an unchanged report",
	      !telemetry_due(&p, &cfg, &t, 100000));
	t.inside = false;
	CHECK("with no minimum a change is due at once",
	      telemetry_due(&p, &cfg, &t, 100001));

	/* The tick counter wraps around: time keeps moving forward. */
	cfg.heartbeat_ticks = 100;
	cfg.min_interval_ticks = 10;
	p = (telemetry_policy_t){ 0 };
	CHECK("first report near the wrap", telemetry_due(&p, &cfg, &t, 0xFFFFFFF0u));
	CHECK("not due just before the heartbeat across the wrap",
	      !telemetry_due(&p, &cfg, &t, 0x00000030u));
	CHECK("due once the heartbeat passed across the wrap",
	      telemetry_due(&p, &cfg, &t, 0x00000054u));
}

int main(void)
{
	test_json();
	test_json_refusals();
	test_policy();
	test_policy_edges();

	if (failures != 0) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
