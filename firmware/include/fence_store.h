/**
 * @file fence_store.h
 * @brief Fence storage port: keep one fence blob across reboots
 *
 * The fence manager calls these functions through a table, so it is tested on
 * the host with an in-memory store and runs on the board with the Zephyr
 * settings store (fence_store_settings.c). The blob is stored exactly as it
 * arrived (see fence_codec.h) and is validated again every time it is loaded.
 */

#ifndef FENCE_STORE_H
#define FENCE_STORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	/**
	 * Read the stored blob.
	 * @return 0 and the length in @p len; -ENOENT if nothing is stored;
	 *         another negative errno on a storage error
	 */
	int (*load)(void *ctx, uint8_t *blob, size_t cap, size_t *len);

	/**
	 * Replace the stored blob. The old blob must stay intact if this fails
	 * or power is lost halfway.
	 * @return 0 on success; a negative errno otherwise
	 */
	int (*save)(void *ctx, const uint8_t *blob, size_t len);

	void *ctx; /**< Passed back to every call */
} fence_store_ops_t;

/**
 * @brief Store backed by the Zephyr settings subsystem (Zephyr builds only)
 *
 * Initializes the settings subsystem on first use.
 *
 * @return The store table, or NULL if the settings subsystem failed to start
 */
const fence_store_ops_t *fence_store_settings(void);

#ifdef __cplusplus
}
#endif

#endif /* FENCE_STORE_H */
