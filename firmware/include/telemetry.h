/**
 * @file telemetry.h
 * @brief Telemetry report: JSON payload and the policy that decides when to send
 *
 * Pure C, no hardware dependency. A cellular message costs energy and, on a
 * free cloud plan, counts against a monthly quota, so a report is sent when
 * something changed (state, inside or outside, fence version) and otherwise
 * only as a heartbeat. Changes are rate limited so a collar walking along the
 * fence line cannot flood the link.
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app_fsm.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Longest operator code kept in a report (MCC and MNC are 5 or 6 digits) */
#define TELEMETRY_OPERATOR_MAX 8

/** Largest report payload, in bytes, including the terminating NUL */
#define TELEMETRY_JSON_MAX 256

/** One report. A field with its has_ flag false is left out of the JSON. */
typedef struct {
	enum app_state state;
	uint32_t fence_id;
	uint32_t fence_version; /**< 0 means no fence from the cloud yet */
	bool has_inside;
	bool inside;
	bool has_distance;
	double distance_m;      /**< Distance to the nearest edge, >= 0 */
	bool has_position;
	double latitude;
	double longitude;
	uint32_t fix_age_s;     /**< Age of the position, used with has_position */
	bool has_rsrp;
	int16_t rsrp_dbm;       /**< LTE signal power */
	uint8_t band;           /**< LTE band, 0 if unknown */
	const char *operator_id; /**< MCC+MNC, letters and digits only, or NULL */
} telemetry_t;

/**
 * @brief Write a report as JSON
 *
 * An operator code with a character other than a letter or digit, or longer
 * than TELEMETRY_OPERATOR_MAX, is left out so the payload never needs
 * escaping.
 *
 * @return Length of the string, excluding the NUL; -1 if an argument is
 *         NULL, a number is NaN or infinite, or the result does not fit
 */
int telemetry_format_json(const telemetry_t *report, char *buf, size_t len);

/** Reporting policy settings, in ticks of whatever clock the caller uses */
typedef struct {
	uint32_t heartbeat_ticks;    /**< Send at least this often; 0 = only on change */
	uint32_t min_interval_ticks; /**< Never send more often than this */
} telemetry_config_t;

/** Policy memory: what the last sent report looked like */
typedef struct {
	bool sent_before;
	enum app_state state;
	bool has_inside;
	bool inside;
	uint32_t fence_version;
	uint32_t last_sent_tick;
} telemetry_policy_t;

/**
 * @brief Decide whether a report is due now
 *
 * The first call is always due. After that a report is due when the state,
 * the inside or outside result or the fence version differs from the last
 * sent one, or the heartbeat elapsed, but never before min_interval_ticks
 * since the last one. A change that arrives too early stays pending and
 * triggers when the interval has passed. When this returns true the policy
 * records the report as sent.
 *
 * @param policy   Policy memory, start it zeroed
 * @param config   Intervals
 * @param report   The report that would be sent
 * @param tick     Current time, wrapping around is handled
 * @return true if the caller should send the report now
 */
bool telemetry_due(telemetry_policy_t *policy, const telemetry_config_t *config,
		   const telemetry_t *report, uint32_t tick);

#ifdef __cplusplus
}
#endif

#endif /* TELEMETRY_H */
