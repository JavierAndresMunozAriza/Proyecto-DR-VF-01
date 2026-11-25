/**
 * @file fence_manager.h
 * @brief Owner of the active fence: load, replace, persist
 *
 * Pure C, no hardware dependency. The manager is the only writer of the
 * geofence engine's polygon (the engine has no lock, see geofence.h), so it
 * must be used from one thread, the one that also checks fixes against the
 * fence. A new fence goes through these steps, and any failure leaves the
 * current fence and the stored one untouched:
 *
 *   decode and validate (fence_codec) -> newer than the current one ->
 *   build the polygon -> save to the store -> replace the current polygon
 *
 * The sender's state wins: any valid fence replaces the current one, whatever
 * its version, so a wrong clock or a fence with a huge version cannot block
 * later updates. The id and version only identify a fence: the same id,
 * version and content again is a repeated download and changes nothing.
 * Version 0 is reserved for fences the device made itself (the square around
 * the first fix, the demo fence). A portal should use the time in seconds as
 * the version, so versions grow and are easy to read.
 */

#ifndef FENCE_MANAGER_H
#define FENCE_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "fence_codec.h"
#include "fence_store.h"
#include "geofence.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Manager results; fence_codec_status values (-1 to -10) pass through. */
enum fence_manager_status {
	FENCE_MANAGER_OK = 0,
	FENCE_MANAGER_NOTHING_STORED = 1,      /**< load: the store is empty */
	FENCE_MANAGER_ALREADY_ACTIVE = 2,      /**< apply: this exact fence is active */
	FENCE_MANAGER_ERR_VERSION = -21,       /**< version 0 is reserved */
	FENCE_MANAGER_ERR_STORE = -22,         /**< the store could not save or load */
	FENCE_MANAGER_ERR_ENGINE = -23,        /**< the geofence engine refused it */
	FENCE_MANAGER_ERR_ARG = -24            /**< NULL argument */
};

typedef struct {
	geofence_t fence;
	bool has_fence;
	uint32_t fence_id;
	uint32_t fence_version; /**< 0 for a fence made by the device */
	uint32_t blob_crc;      /**< CRC-32 of the active blob, to spot a repeated download */
	const fence_store_ops_t *store; /**< NULL: do not persist */

	/* Work buffers, kept here so the calls do not use the caller's stack.
	 * Private: do not touch.
	 */
	geopoint_t scratch_vertices[FENCE_CODEC_MAX_VERTICES];
	uint8_t scratch_blob[FENCE_CODEC_MAX_SIZE];
} fence_manager_t;

/**
 * @brief Start a manager with no fence
 * @param store  Where to persist fences, or NULL to keep them in memory only
 */
void fence_manager_init(fence_manager_t *manager, const fence_store_ops_t *store);

/**
 * @brief Load the stored fence, replacing the current one
 *
 * A stored fence that no longer validates is ignored and reported, never
 * used. The current fence is released only if a stored one is accepted.
 *
 * @return FENCE_MANAGER_OK if a stored fence is now active;
 *         FENCE_MANAGER_NOTHING_STORED if the store is empty (current fence
 *         unchanged); a negative status if the stored blob is invalid or the
 *         store failed (current fence unchanged)
 */
int fence_manager_load(fence_manager_t *manager);

/**
 * @brief Validate, persist and activate a fence blob received from outside
 *
 * @return FENCE_MANAGER_OK if the fence is now active and stored;
 *         FENCE_MANAGER_ALREADY_ACTIVE if this exact fence (same id, version
 *         and content) is already active (a repeated download; nothing
 *         changes and it counts as success); otherwise a negative fence_codec_status or
 *         fence_manager_status. On any error the current fence and the
 *         stored one are unchanged.
 */
int fence_manager_apply(fence_manager_t *manager, const uint8_t *blob, size_t len);

/**
 * @brief Use a fence made by the device, without storing it
 *
 * Its version is 0, so any fence from outside replaces it. The current fence
 * is released only if the new one is built.
 *
 * @return FENCE_MANAGER_OK, FENCE_MANAGER_ERR_ARG or FENCE_MANAGER_ERR_ENGINE
 */
int fence_manager_use_transient(fence_manager_t *manager, const geopoint_t *vertices,
				uint16_t n);

/**
 * @brief Drop the current fence; the stored one is kept
 */
void fence_manager_release(fence_manager_t *manager);

/**
 * @brief The active fence for geofence_check(), or NULL if there is none
 */
const geofence_t *fence_manager_fence(const fence_manager_t *manager);

#ifdef __cplusplus
}
#endif

#endif /* FENCE_MANAGER_H */
