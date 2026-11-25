/*
 * fence_manager.c - owner of the active fence: load, replace, persist
 *
 * Pure C, no hardware dependency. See fence_manager.h for the rules.
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "fence_manager.h"

/* Build a polygon in the engine from validated vertices. */
static int build_polygon(geofence_t *out, const geopoint_t *vertices, uint16_t n)
{
	*out = (geofence_t){ 0 };
	if (geofence_create_polygon(out, vertices, n) != GEOFENCE_SUCCESS)
		return FENCE_MANAGER_ERR_ENGINE;
	if (!geofence_validate_polygon(out)) {
		geofence_destroy_polygon(out);
		return FENCE_MANAGER_ERR_ENGINE;
	}
	return FENCE_MANAGER_OK;
}

/* Replace the current polygon with a built one. Cannot fail. */
static void adopt(fence_manager_t *manager, geofence_t *fresh, uint32_t id, uint32_t version)
{
	fence_manager_release(manager);
	manager->fence = *fresh;
	manager->has_fence = true;
	manager->fence_id = id;
	manager->fence_version = version;
}

/*
 * Decode a blob and make it the active fence. When @p persist is true it must
 * also be newer than the current fence and is saved before it replaces it.
 */
static int accept_blob(fence_manager_t *manager, const uint8_t *blob, size_t len, bool persist)
{
	fence_codec_header_t header;
	geofence_t fresh;
	int rc;

	rc = fence_codec_decode(blob, len, manager->scratch_vertices,
				FENCE_CODEC_MAX_VERTICES, &header);
	if (rc != FENCE_CODEC_OK)
		return rc;
	if (header.fence_version == 0)
		return FENCE_MANAGER_ERR_VERSION;
	if (persist && manager->has_fence) {
		if (header.fence_version == manager->fence_version &&
		    header.fence_id == manager->fence_id)
			return FENCE_MANAGER_ALREADY_ACTIVE;
		if (header.fence_version <= manager->fence_version)
			return FENCE_MANAGER_ERR_NOT_NEWER;
	}

	rc = build_polygon(&fresh, manager->scratch_vertices, header.num_vertices);
	if (rc != FENCE_MANAGER_OK)
		return rc;

	if (persist && manager->store != NULL &&
	    manager->store->save(manager->store->ctx, blob, len) != 0) {
		geofence_destroy_polygon(&fresh);
		return FENCE_MANAGER_ERR_STORE;
	}

	adopt(manager, &fresh, header.fence_id, header.fence_version);
	return FENCE_MANAGER_OK;
}

void fence_manager_init(fence_manager_t *manager, const fence_store_ops_t *store)
{
	if (manager == NULL)
		return;
	*manager = (fence_manager_t){ .store = store };
}

int fence_manager_load(fence_manager_t *manager)
{
	size_t len = 0;
	int rc;

	if (manager == NULL)
		return FENCE_MANAGER_ERR_ARG;
	if (manager->store == NULL)
		return FENCE_MANAGER_NOTHING_STORED;

	rc = manager->store->load(manager->store->ctx, manager->scratch_blob,
				  sizeof(manager->scratch_blob), &len);
	if (rc == -ENOENT)
		return FENCE_MANAGER_NOTHING_STORED;
	if (rc != 0)
		return FENCE_MANAGER_ERR_STORE;

	return accept_blob(manager, manager->scratch_blob, len, false);
}

int fence_manager_apply(fence_manager_t *manager, const uint8_t *blob, size_t len)
{
	if (manager == NULL || blob == NULL)
		return FENCE_MANAGER_ERR_ARG;
	return accept_blob(manager, blob, len, true);
}

int fence_manager_use_transient(fence_manager_t *manager, const geopoint_t *vertices, uint16_t n)
{
	geofence_t fresh;
	int rc;

	if (manager == NULL || vertices == NULL)
		return FENCE_MANAGER_ERR_ARG;

	rc = build_polygon(&fresh, vertices, n);
	if (rc != FENCE_MANAGER_OK)
		return rc;

	adopt(manager, &fresh, 0, 0);
	return FENCE_MANAGER_OK;
}

void fence_manager_release(fence_manager_t *manager)
{
	if (manager == NULL || !manager->has_fence)
		return;

	geofence_destroy_polygon(&manager->fence);
	manager->has_fence = false;
	manager->fence_id = 0;
	manager->fence_version = 0;
}

const geofence_t *fence_manager_fence(const fence_manager_t *manager)
{
	if (manager == NULL || !manager->has_fence)
		return NULL;
	return &manager->fence;
}
