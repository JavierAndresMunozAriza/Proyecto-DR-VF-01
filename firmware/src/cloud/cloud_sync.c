/*
 * cloud_sync.c - cloud worker thread and its queues to the application
 *
 * One thread keeps the cloud link up, polls for a new fence every
 * CONFIG_DRVF_CLOUD_POLL_INTERVAL_S seconds and sends the reports the
 * application posts. It talks to the network only through cloud.h, so the
 * same file serves every board. See cloud_sync.h.
 */

#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "cloud.h"
#include "cloud_sync.h"
#include "fence_codec.h"

LOG_MODULE_REGISTER(cloud_sync, LOG_LEVEL_INF);

#define WORKER_PRIORITY        5   /* preemptible, below the application thread */
#define OPEN_RETRY_MIN_S       30
#define OPEN_RETRY_MAX_S       900
#define MAX_CONSECUTIVE_ERRORS 3   /* then close and reconnect */
#define INBOUND_DEPTH          1
#define OUTBOUND_DEPTH         3
#define MSGQ_ALIGN             4
#define TELEMETRY_MAX_AGE_MS ((int64_t)CONFIG_DRVF_TELEMETRY_HEARTBEAT_S * MSEC_PER_SEC)
#define POLL_INTERVAL_MS       ((int64_t)CONFIG_DRVF_CLOUD_POLL_INTERVAL_S * MSEC_PER_SEC)

struct fence_msg {
	size_t len;
	uint8_t blob[FENCE_CODEC_MAX_SIZE];
};

enum job_type {
	JOB_REPORT,
	JOB_TELEMETRY,
};

struct job {
	enum job_type type;
	int64_t queued_ms; /* uptime when the application posted it */
	union {
		telemetry_t telemetry;
		struct {
			size_t len;
			uint32_t version;
			int status;
			uint8_t blob[FENCE_CODEC_MAX_SIZE];
		} report;
	};
};

/* Queues are set up in cloud_sync_start(): K_MSGQ_DEFINE would make them global. */
static char inbound_storage[INBOUND_DEPTH * sizeof(struct fence_msg)] __aligned(MSGQ_ALIGN);
static char outbound_storage[OUTBOUND_DEPTH * sizeof(struct job)] __aligned(MSGQ_ALIGN);
static struct k_msgq inbound_fences;
static struct k_msgq outbound_jobs;
static K_THREAD_STACK_DEFINE(worker_stack, CONFIG_DRVF_CLOUD_THREAD_STACK_SIZE);

static struct k_thread worker_thread;

/* Add the radio information, format the report and send it. */
static int send_telemetry(const telemetry_t *queued)
{
	telemetry_t report = *queued;
	cloud_link_info_t info;
	char json[TELEMETRY_JSON_MAX];

	if (cloud_link_info(&info) == 0) {
		report.has_rsrp = info.has_rsrp;
		report.rsrp_dbm = info.rsrp_dbm;
		report.band = info.band;
		report.operator_id = info.operator_id[0] != '\0' ? info.operator_id : NULL;
	}

	if (telemetry_format_json(&report, json, sizeof(json)) < 0) {
		LOG_ERR("Telemetry report could not be formatted");
		return -EINVAL;
	}
	return cloud_send_telemetry(json);
}

static int run_job(const struct job *job)
{
	if (job->type == JOB_TELEMETRY) {
		/* After a long outage the queue holds old reports: the cloud would show them as new. */
		if (k_uptime_get() - job->queued_ms > TELEMETRY_MAX_AGE_MS) {
			LOG_INF("Telemetry report dropped, it is too old");
			return 0;
		}
		return send_telemetry(&job->telemetry);
	}

	return cloud_report_fence(job->report.blob, job->report.len, job->report.version,
				  job->report.status);
}

/* Ask the cloud for a new fence and hand it to the application. */
static int poll_fence(void)
{
	struct fence_msg msg;
	int err;

	err = cloud_fetch_fence(msg.blob, sizeof(msg.blob), &msg.len);
	if (err == -ENODATA)
		return 0;
	if (err == -EBADMSG) {
		/* The cloud holds a fence value nobody can decode: not a link error. */
		LOG_WRN("The cloud holds a fence that cannot be decoded");
		return 0;
	}
	if (err != 0) {
		LOG_WRN("Fence poll failed: %d", err);
		return err;
	}

	LOG_INF("Downloaded a fence of %u bytes", (unsigned int)msg.len);
	if (k_msgq_put(&inbound_fences, &msg, K_NO_WAIT) != 0)
		LOG_WRN("The application is still busy, the fence comes again at the next poll");
	return 0;
}

static void open_with_backoff(void)
{
	int delay_s = OPEN_RETRY_MIN_S;

	while (cloud_open() != 0) {
		LOG_WRN("Cloud connection failed, retrying in %d s", delay_s);
		k_sleep(K_SECONDS(delay_s));
		delay_s = MIN(delay_s * 2, OPEN_RETRY_MAX_S);
	}
	LOG_INF("Cloud connected");
}

static void worker_main(void *p1, void *p2, void *p3)
{
	int64_t next_poll_ms = k_uptime_get();

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	while (true) {
		unsigned int errors = 0;

		open_with_backoff();

		while (errors < MAX_CONSECUTIVE_ERRORS) {
			const int64_t wait_ms = MAX(next_poll_ms - k_uptime_get(), 0);
			struct job job;
			int err;

			if (k_msgq_get(&outbound_jobs, &job, K_MSEC(wait_ms)) == 0) {
				err = run_job(&job);
			} else {
				err = poll_fence();
				next_poll_ms = k_uptime_get() + POLL_INTERVAL_MS;
			}
			errors = err != 0 ? errors + 1 : 0;
		}

		LOG_WRN("Too many errors, reconnecting");
		cloud_close();
	}
}

int cloud_sync_start(void)
{
	k_tid_t tid;

	k_msgq_init(&inbound_fences, inbound_storage, sizeof(struct fence_msg), INBOUND_DEPTH);
	k_msgq_init(&outbound_jobs, outbound_storage, sizeof(struct job), OUTBOUND_DEPTH);

	tid = k_thread_create(&worker_thread, worker_stack,
				      K_THREAD_STACK_SIZEOF(worker_stack), worker_main, NULL,
				      NULL, NULL, K_PRIO_PREEMPT(WORKER_PRIORITY), 0, K_NO_WAIT);

	if (tid == NULL)
		return -ENOMEM;

	/* The name is a debug aid and is not available without CONFIG_THREAD_NAME. */
	(void)k_thread_name_set(tid, "cloud_sync");
	return 0;
}

bool cloud_sync_radio_was_reset(void)
{
	return cloud_radio_was_reset();
}

int cloud_sync_get_fence(uint8_t *blob, size_t cap, size_t *len)
{
	struct fence_msg msg;

	if (k_msgq_get(&inbound_fences, &msg, K_NO_WAIT) != 0)
		return -EAGAIN;
	if (msg.len > cap)
		return -EMSGSIZE;

	memcpy(blob, msg.blob, msg.len);
	*len = msg.len;
	return 0;
}

int cloud_sync_post_report(const uint8_t *blob, size_t len, uint32_t version, int status)
{
	struct job job = { .type = JOB_REPORT, .queued_ms = k_uptime_get() };

	if (len > sizeof(job.report.blob))
		return -EMSGSIZE;

	job.report.len = len;
	job.report.version = version;
	job.report.status = status;
	memcpy(job.report.blob, blob, len);

	return k_msgq_put(&outbound_jobs, &job, K_NO_WAIT) == 0 ? 0 : -ENOSPC;
}

int cloud_sync_post_telemetry(const telemetry_t *report)
{
	struct job job = { .type = JOB_TELEMETRY, .queued_ms = k_uptime_get(),
			   .telemetry = *report };

	/* The pointer would dangle in the other thread: the worker fills the field. */
	job.telemetry.operator_id = NULL;
	return k_msgq_put(&outbound_jobs, &job, K_NO_WAIT) == 0 ? 0 : -ENOSPC;
}
