/*
 * Host unit tests for the fence manager, against the real geofence engine and
 * the real fence codec, with an in-memory store. Exits non-zero if any check
 * fails. Run `make sanitize` too: it finds leaks when a fence is replaced.
 */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "fence_manager.h"
#include "geo_constants.h"

#define LAT0 4.6097
#define LON0 (-74.0817)
#define HALF_M 30.0
#define FAR_M 500.0

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

/* In-memory store that can be told to fail. */
struct fake_store {
	uint8_t blob[FENCE_CODEC_MAX_SIZE];
	size_t len;
	bool has;
	bool fail_save;
	bool fail_load;
	int saves;
};

static int fake_load(void *ctx, uint8_t *blob, size_t cap, size_t *len)
{
	struct fake_store *s = ctx;

	if (s->fail_load)
		return -EIO;
	if (!s->has)
		return -ENOENT;
	if (s->len > cap)
		return -ENOMEM;
	memcpy(blob, s->blob, s->len);
	*len = s->len;
	return 0;
}

static int fake_save(void *ctx, const uint8_t *blob, size_t len)
{
	struct fake_store *s = ctx;

	if (s->fail_save)
		return -EIO;
	memcpy(s->blob, blob, len);
	s->len = len;
	s->has = true;
	s->saves++;
	return 0;
}

static geopoint_t pt(double dlat_m, double dlon_m)
{
	geopoint_t p;

	p.latitude = LAT0 + dlat_m / GEO_METERS_PER_DEG_LAT;
	p.longitude = LON0 + dlon_m / (GEO_METERS_PER_DEG_LAT * cos(LAT0 * GEO_DEG_TO_RAD));
	return p;
}

/* Blob of a square around (dlat_m, 0) meters from the test point. */
static size_t make_blob(uint8_t *buf, uint32_t id, uint32_t version, double dlat_m)
{
	const geopoint_t square[4] = {
		pt(dlat_m - HALF_M, -HALF_M), pt(dlat_m - HALF_M, HALF_M),
		pt(dlat_m + HALF_M, HALF_M), pt(dlat_m + HALF_M, -HALF_M)
	};
	const fence_codec_header_t h = { id, version, 0 };
	size_t len = 0;

	if (fence_codec_encode(&h, square, 4, buf, FENCE_CODEC_MAX_SIZE, &len) != 0)
		return 0;
	return len;
}

static bool inside(const fence_manager_t *m, double dlat_m)
{
	const geopoint_t p = pt(dlat_m, 0);
	geofence_result_t res;

	if (fence_manager_fence(m) == NULL)
		return false;
	return geofence_check(&p, fence_manager_fence(m), &res) == 0 && res.inside;
}

static void test_apply(void)
{
	struct fake_store st = { 0 };
	const fence_store_ops_t ops = { fake_load, fake_save, &st };
	fence_manager_t m;
	uint8_t a[FENCE_CODEC_MAX_SIZE];
	uint8_t b[FENCE_CODEC_MAX_SIZE];
	uint8_t same[FENCE_CODEC_MAX_SIZE];
	uint8_t v0[FENCE_CODEC_MAX_SIZE];
	size_t la = make_blob(a, 1, 5, 0.0);
	size_t lb = make_blob(b, 1, 6, FAR_M);
	size_t ls = make_blob(same, 9, 5, FAR_M);
	size_t l0 = make_blob(v0, 1, 0, 0.0);

	fence_manager_init(&m, &ops);
	CHECK("a new manager has no fence", fence_manager_fence(&m) == NULL);

	CHECK("apply a valid fence", fence_manager_apply(&m, a, la) == FENCE_MANAGER_OK);
	CHECK("the fence is active", inside(&m, 0.0) && !inside(&m, FAR_M));
	CHECK("its version and id are kept", m.fence_version == 5 && m.fence_id == 1);
	CHECK("it was stored byte for byte",
	      st.has && st.len == la && memcmp(st.blob, a, la) == 0);

	CHECK("an equal version with another id is refused",
	      fence_manager_apply(&m, same, ls) == FENCE_MANAGER_ERR_NOT_NEWER);
	CHECK("the same fence again is already active, not an error",
	      fence_manager_apply(&m, a, la) == FENCE_MANAGER_ALREADY_ACTIVE);
	CHECK("a refused or repeated fence changes nothing",
	      inside(&m, 0.0) && m.fence_version == 5 && st.saves == 1 &&
	      memcmp(st.blob, a, la) == 0);

	CHECK("version 0 is reserved",
	      fence_manager_apply(&m, v0, l0) == FENCE_MANAGER_ERR_VERSION);

	CHECK("a newer fence replaces it", fence_manager_apply(&m, b, lb) == FENCE_MANAGER_OK);
	CHECK("the new fence is active, the old one is gone",
	      inside(&m, FAR_M) && !inside(&m, 0.0) && m.fence_version == 6);
	CHECK("the store holds the new one",
	      st.saves == 2 && st.len == lb && memcmp(st.blob, b, lb) == 0);

	fence_manager_release(&m);
	CHECK("release drops the fence but not the store",
	      fence_manager_fence(&m) == NULL && st.has);
	fence_manager_release(&m);
}

static void test_apply_errors(void)
{
	struct fake_store st = { 0 };
	const fence_store_ops_t ops = { fake_load, fake_save, &st };
	fence_manager_t m;
	uint8_t a[FENCE_CODEC_MAX_SIZE];
	uint8_t b[FENCE_CODEC_MAX_SIZE];
	uint8_t bad[FENCE_CODEC_MAX_SIZE];
	size_t la = make_blob(a, 1, 5, 0.0);
	size_t lb = make_blob(b, 1, 6, FAR_M);

	fence_manager_init(&m, &ops);
	fence_manager_apply(&m, a, la);

	memcpy(bad, b, lb);
	bad[20] ^= 0xFF;
	CHECK("a corrupt blob keeps the codec status",
	      fence_manager_apply(&m, bad, lb) == FENCE_CODEC_ERR_CRC);
	CHECK("a corrupt blob changes nothing",
	      inside(&m, 0.0) && st.saves == 1 && memcmp(st.blob, a, la) == 0);

	st.fail_save = true;
	CHECK("a failed save is reported",
	      fence_manager_apply(&m, b, lb) == FENCE_MANAGER_ERR_STORE);
	CHECK("a failed save keeps the current fence and the stored one",
	      inside(&m, 0.0) && m.fence_version == 5 && memcmp(st.blob, a, la) == 0);
	st.fail_save = false;

	CHECK("NULL manager", fence_manager_apply(NULL, b, lb) == FENCE_MANAGER_ERR_ARG);
	CHECK("NULL blob", fence_manager_apply(&m, NULL, lb) == FENCE_MANAGER_ERR_ARG);
	fence_manager_release(&m);
}

static void test_load(void)
{
	struct fake_store st = { 0 };
	const fence_store_ops_t ops = { fake_load, fake_save, &st };
	fence_manager_t m;
	uint8_t a[FENCE_CODEC_MAX_SIZE];
	size_t la = make_blob(a, 3, 8, 0.0);

	fence_manager_init(&m, &ops);
	CHECK("an empty store reports nothing stored",
	      fence_manager_load(&m) == FENCE_MANAGER_NOTHING_STORED &&
	      fence_manager_fence(&m) == NULL);

	st.fail_load = true;
	CHECK("a store failure is reported",
	      fence_manager_load(&m) == FENCE_MANAGER_ERR_STORE);
	st.fail_load = false;

	memcpy(st.blob, a, la);
	st.len = la;
	st.has = true;
	CHECK("a stored fence is loaded", fence_manager_load(&m) == FENCE_MANAGER_OK);
	CHECK("the loaded fence is active with its version",
	      inside(&m, 0.0) && m.fence_version == 8 && m.fence_id == 3);

	st.blob[20] ^= 0xFF;
	CHECK("a corrupt stored fence is refused",
	      fence_manager_load(&m) == FENCE_CODEC_ERR_CRC);
	CHECK("a refused load keeps the current fence", inside(&m, 0.0));
	fence_manager_release(&m);
}

static void test_transient_and_restore(void)
{
	struct fake_store st = { 0 };
	const fence_store_ops_t ops = { fake_load, fake_save, &st };
	fence_manager_t m;
	uint8_t a[FENCE_CODEC_MAX_SIZE];
	uint8_t b[FENCE_CODEC_MAX_SIZE];
	const geopoint_t demo[4] = {
		pt(FAR_M - HALF_M, -HALF_M), pt(FAR_M - HALF_M, HALF_M),
		pt(FAR_M + HALF_M, HALF_M), pt(FAR_M + HALF_M, -HALF_M)
	};
	const geopoint_t line[2] = { pt(0, 0), pt(1, 1) };
	size_t la = make_blob(a, 1, 5, 0.0);
	size_t lb = make_blob(b, 1, 6, 0.0);

	fence_manager_init(&m, &ops);
	fence_manager_apply(&m, a, la);

	CHECK("a transient fence replaces the stored one in use",
	      fence_manager_use_transient(&m, demo, 4) == FENCE_MANAGER_OK &&
	      inside(&m, FAR_M) && !inside(&m, 0.0) && m.fence_version == 0);
	CHECK("a transient fence is not stored", st.saves == 1 && memcmp(st.blob, a, la) == 0);

	fence_manager_release(&m);
	CHECK("after the demo the stored fence comes back",
	      fence_manager_load(&m) == FENCE_MANAGER_OK && inside(&m, 0.0) &&
	      m.fence_version == 5);

	fence_manager_use_transient(&m, demo, 4);
	CHECK("any fence from outside replaces a transient one",
	      fence_manager_apply(&m, b, lb) == FENCE_MANAGER_OK && inside(&m, 0.0));

	CHECK("a transient fence that is not a polygon is refused",
	      fence_manager_use_transient(&m, line, 2) == FENCE_MANAGER_ERR_ENGINE);
	CHECK("a refused transient keeps the current fence", inside(&m, 0.0));
	CHECK("transient NULL arguments",
	      fence_manager_use_transient(&m, NULL, 4) == FENCE_MANAGER_ERR_ARG &&
	      fence_manager_use_transient(NULL, demo, 4) == FENCE_MANAGER_ERR_ARG);
	fence_manager_release(&m);
}

static void test_without_store(void)
{
	fence_manager_t m;
	uint8_t a[FENCE_CODEC_MAX_SIZE];
	size_t la = make_blob(a, 1, 5, 0.0);

	fence_manager_init(&m, NULL);
	CHECK("without a store a fence is still applied",
	      fence_manager_apply(&m, a, la) == FENCE_MANAGER_OK && inside(&m, 0.0));
	CHECK("without a store there is nothing to load",
	      fence_manager_load(&m) == FENCE_MANAGER_NOTHING_STORED && inside(&m, 0.0));
	fence_manager_release(&m);
}

int main(void)
{
	const geofence_config_t config = { 1, FENCE_CODEC_MAX_VERTICES, 0.0 };

	if (geofence_init(&config) != 0) {
		printf("FAIL geofence_init\n");
		return 1;
	}
	test_apply();
	test_apply_errors();
	test_load();
	test_transient_and_restore();
	test_without_store();
	geofence_deinit();

	if (failures != 0) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
