/*
 * fence_store_settings.c - fence storage on the Zephyr settings subsystem
 *
 * The blob is one settings entry, "drvf/fence". The settings backend (NVS)
 * writes a new entry before it drops the old one, so a power cut during a
 * save leaves the previous fence readable. See fence_store.h.
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>

#include "fence_store.h"

LOG_MODULE_REGISTER(fence_store, LOG_LEVEL_INF);

#define SUBTREE "drvf"
#define NAME    "fence"
#define KEY     SUBTREE "/" NAME

struct load_request {
	uint8_t *blob;
	size_t cap;
	size_t len;
	bool found;
	int error;
};

static int read_entry(const char *name, size_t len, settings_read_cb read_cb, void *cb_arg,
		      void *param)
{
	struct load_request *req = param;
	ssize_t n;

	if (strcmp(name, NAME) != 0)
		return 0;
	if (len > req->cap) {
		req->error = -ENOMEM;
		return 0;
	}

	n = read_cb(cb_arg, req->blob, len);
	if (n < 0) {
		req->error = (int)n;
		return 0;
	}
	req->len = (size_t)n;
	req->found = true;
	return 0;
}

static int store_load(void *ctx, uint8_t *blob, size_t cap, size_t *len)
{
	struct load_request req = { .blob = blob, .cap = cap };
	int err;

	ARG_UNUSED(ctx);

	err = settings_load_subtree_direct(SUBTREE, read_entry, &req);
	if (err != 0)
		return err;
	if (req.error != 0)
		return req.error;
	if (!req.found)
		return -ENOENT;

	*len = req.len;
	return 0;
}

static int store_save(void *ctx, const uint8_t *blob, size_t len)
{
	ARG_UNUSED(ctx);

	return settings_save_one(KEY, blob, len);
}

const fence_store_ops_t *fence_store_settings(void)
{
	static const fence_store_ops_t ops = {
		.load = store_load,
		.save = store_save,
		.ctx = NULL,
	};
	static bool ready;
	int err;

	if (!ready) {
		err = settings_subsys_init();
		if (err != 0) {
			LOG_ERR("settings_subsys_init failed: %d", err);
			return NULL;
		}
		ready = true;
	}
	return &ops;
}
