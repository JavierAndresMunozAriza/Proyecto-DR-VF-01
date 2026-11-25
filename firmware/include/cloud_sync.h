/**
 * @file cloud_sync.h
 * @brief Cloud worker thread and its queues to the application
 *
 * The application thread never blocks on the network. This module owns one
 * thread that keeps the cloud link up, polls for a new fence, and sends
 * what the application posts. All the exchange goes through message queues:
 *
 *   cloud -> app: a downloaded fence blob  (cloud_sync_get_fence)
 *   app -> cloud: the result of applying it (cloud_sync_post_report)
 *                 a telemetry report         (cloud_sync_post_telemetry)
 *
 * Without CONFIG_DRVF_CLOUD every function is an inline stub, so the
 * application code has no #ifdef and the offline proof of concept builds as
 * before.
 */

#ifndef CLOUD_SYNC_H
#define CLOUD_SYNC_H

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "telemetry.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_DRVF_CLOUD

/**
 * @brief Start the cloud worker thread
 * @return 0 on success; a negative errno if the thread could not start
 */
int cloud_sync_start(void);

/**
 * @brief Take a downloaded fence blob, without waiting
 * @return 0 and the blob in @p blob (length in @p len); -EAGAIN if none
 *         arrived; -EMSGSIZE if @p cap is too small
 */
int cloud_sync_get_fence(uint8_t *blob, size_t cap, size_t *len);

/**
 * @brief Tell the cloud what happened to a downloaded fence
 *
 * Queued, never blocks. @p blob is copied.
 *
 * @return 0 if queued; -ENOSPC if the queue is full (the fence is downloaded
 *         again at the next poll); -EMSGSIZE if @p len is too large
 */
int cloud_sync_post_report(const uint8_t *blob, size_t len, uint32_t version, int status);

/**
 * @brief True once after the radio was switched off and the GNSS must restart
 *
 * First-time provisioning stops the GNSS session; see cloud_radio_was_reset().
 */
bool cloud_sync_radio_was_reset(void);

/**
 * @brief Queue a telemetry report; the worker adds the radio information
 * @return 0 if queued; -ENOSPC if the queue is full (the report is dropped)
 */
int cloud_sync_post_telemetry(const telemetry_t *report);

#else /* !CONFIG_DRVF_CLOUD */

static inline int cloud_sync_start(void)
{
	return 0;
}

static inline int cloud_sync_get_fence(uint8_t *blob, size_t cap, size_t *len)
{
	(void)blob;
	(void)cap;
	(void)len;
	return -EAGAIN;
}

static inline int cloud_sync_post_report(const uint8_t *blob, size_t len, uint32_t version,
					 int status)
{
	(void)blob;
	(void)len;
	(void)version;
	(void)status;
	return 0;
}

static inline bool cloud_sync_radio_was_reset(void)
{
	return false;
}

static inline int cloud_sync_post_telemetry(const telemetry_t *report)
{
	(void)report;
	return 0;
}

#endif /* CONFIG_DRVF_CLOUD */

#ifdef __cplusplus
}
#endif

#endif /* CLOUD_SYNC_H */
