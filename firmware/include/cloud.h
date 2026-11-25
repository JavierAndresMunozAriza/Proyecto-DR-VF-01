/**
 * @file cloud.h
 * @brief Cloud link port: fence download, fence report, telemetry upload
 *
 * Blocking, POSIX-style calls. They are made from one worker thread only
 * (cloud_sync), so an adapter may keep static buffers and is not reentrant.
 * Adapters: cloud_nrfcloud.c (nRF Cloud over CoAP, through the nRF9151
 * modem). The final board (ESP32-S3 and an external modem) gets its own
 * adapter with these same functions, so the application does not change.
 *
 * Fence exchange, as seen from the cloud: the user's portal writes the fence
 * blob (see fence_codec.h) as one base64 string, `desired.fence`, in the
 * device shadow. The device downloads only what differs from what it last
 * reported, and reports back the same string in `reported.fence`, plus
 * `reported.fence_version` and `reported.fence_status`. Reporting a fence the
 * device refused also echoes it, so a bad fence is not downloaded again.
 */

#ifndef CLOUD_H
#define CLOUD_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Longest operator code (MCC+MNC) in cloud_link_info_t, without the NUL */
#define CLOUD_OPERATOR_MAX 8

/** Radio access technology in use */
enum cloud_rat {
	CLOUD_RAT_UNKNOWN = 0,
	CLOUD_RAT_LTE_M,
	CLOUD_RAT_NB_IOT,
};

/** Radio information for the telemetry report and the field-test log */
typedef struct {
	enum cloud_rat rat;
	bool has_rsrp;
	int16_t rsrp_dbm;                           /**< LTE signal power */
	uint8_t band;                               /**< LTE band, 0 if unknown */
	char operator_id[CLOUD_OPERATOR_MAX + 1];   /**< MCC+MNC, "" if unknown */
} cloud_link_info_t;

/**
 * @brief Bring the link up and connect to the cloud
 *
 * Registers on the cellular network with PSM and eDRX requested, provisions
 * the device if it has no credentials yet and opens the cloud connection.
 * May take minutes on the first connection.
 *
 * @return 0 on success; a negative errno otherwise (-EACCES if the device is
 *         not claimed in the cloud account yet; the adapter logs the
 *         attestation token the user needs)
 */
int cloud_open(void);

/**
 * @brief Close the cloud connection; the cellular registration stays (PSM)
 */
void cloud_close(void);

/**
 * @brief Download the fence the user asked for, if there is a new one
 *
 * @param blob  Receives the fence blob, already decoded from base64
 * @param cap   Capacity of @p blob, FENCE_CODEC_MAX_SIZE is enough
 * @param len   Receives the blob length on success
 * @return 0 if a new fence was written to @p blob; -ENODATA if there is
 *         nothing new; -EBADMSG if the shadow holds a fence value that cannot
 *         be decoded (not valid base64, too large, not a string); the adapter
 *         reports it by itself when it can, so it is not downloaded again, and
 *         it is not a link error; another negative errno on a link error
 */
int cloud_fetch_fence(uint8_t *blob, size_t cap, size_t *len);

/**
 * @brief Report what the device did with a fence it downloaded
 *
 * @param blob     The exact bytes that cloud_fetch_fence() returned
 * @param len      Their length
 * @param version  The fence version, or 0 if the blob could not be decoded
 * @param status   0 if the fence was applied, otherwise the negative
 *                 fence_codec_status or fence_manager status that refused it
 * @return 0 on success; a negative errno otherwise
 */
int cloud_report_fence(const uint8_t *blob, size_t len, uint32_t version, int status);

/**
 * @brief Send one telemetry report
 *
 * @param json  A JSON object from telemetry_format_json(); the adapter wraps
 *              it in whatever envelope the cloud service needs
 * @return 0 on success; a negative errno otherwise
 */
int cloud_send_telemetry(const char *json);

/**
 * @brief True once after the adapter switched the whole radio off
 *
 * First-time provisioning powers the radio off, which also stops the GNSS
 * session. The application restarts the GNSS when this returns true. Safe to
 * call from any thread; it clears the flag.
 */
bool cloud_radio_was_reset(void);

/**
 * @brief Read the current radio information
 * @return 0 on success; a negative errno if it is not available
 */
int cloud_link_info(cloud_link_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* CLOUD_H */
