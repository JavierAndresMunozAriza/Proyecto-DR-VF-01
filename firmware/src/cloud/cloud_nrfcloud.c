/*
 * cloud_nrfcloud.c - cloud port adapter: nRF Cloud over CoAP (Thingy:91 X)
 *
 * The nRF9151 modem carries the LTE-M/NB-IoT link; PSM and eDRX are requested
 * from Kconfig when the modem library starts (see overlay-cloud.conf). Every
 * call comes from one worker thread (see cloud.h), so the buffers are static
 * and nothing here is reentrant.
 *
 * GNSS and LTE share the modem: this adapter only activates LTE (functional
 * mode ACTIVATE_LTE, "without changing GNSS"), it never touches the GNSS
 * session, except while nRF Provisioning writes credentials, which needs the
 * radio fully off (see provisioning_event()).
 */

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include <date_time.h>
#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <modem/nrf_modem_lib.h>
#include <net/nrf_cloud.h>
#include <net/nrf_cloud_coap.h>
#include <net/nrf_cloud_defs.h>
#include <net/nrf_provisioning.h>
#include <nrf_modem.h>
#include <nrf_modem_at.h>
#include <zephyr/data/json.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/coap.h>
#include <zephyr/sys/base64.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include "cloud.h"
#include "fence_codec.h"

LOG_MODULE_REGISTER(cloud, LOG_LEVEL_INF);

/* Custom nRF Cloud application id of the telemetry messages */
#define TELEMETRY_APP_ID "DRVF"

/* Confirmable CoAP messages: the call returns only after the cloud answered */
#define CONFIRMABLE true

/* Version string reported to the shadow on connect; NULL reports none */
#define APP_VERSION NULL

/* Pause after provisioning before the first connect (library workaround) */
#define AFTER_PROVISION_DELAY_S 10

/* Poll period while waiting for the date and time */
#define DATE_TIME_POLL_S 1

/* base64 text of the largest fence blob, without the NUL */
#define FENCE_B64_MAX (4 * ((FENCE_CODEC_MAX_SIZE + 2) / 3))

/* Fixed text of the shadow report around the fence string, with margin for
 * the version (10 digits) and the status (11 characters)
 */
#define REPORT_OVERHEAD 96

/* "fence" key of the shadow delta */
#define FENCE_KEY "fence"

/* MCC+MNC is the fourth field of the modem monitor report */
#define MONITOR_CMD     "AT%XMONITOR"
#define MONITOR_PLMN_FMT                                                      \
	"%%XMONITOR: %*u,%*[^,],%*[^,],\"%" STRINGIFY(CLOUD_OPERATOR_MAX) "[^\"]\""

/* Modem library, LTE state */
static K_SEM_DEFINE(registered_sem, 0, 1);
static bool handlers_registered;

/* Cloud state */
static bool stack_ready;
static bool connected;

/* Provisioning state */
static K_SEM_DEFINE(provisioning_sem, 0, 1);
static int provisioning_result;
static bool gnss_was_active;
static bool refused_provision_tried;
static atomic_t radio_reset_pending;

/* Shadow exchange buffers */
static char shadow_buf[CONFIG_DRVF_CLOUD_SHADOW_BUF_SIZE];
static char fence_b64[FENCE_B64_MAX + 1];
static char report_buf[FENCE_B64_MAX + REPORT_OVERHEAD];
static char message_buf[CONFIG_DRVF_CLOUD_MSG_BUF_SIZE];

struct fence_delta {
	char fence[FENCE_B64_MAX + 1];
};

static const struct json_obj_descr fence_delta_descr[] = {
	JSON_OBJ_DESCR_PRIM(struct fence_delta, fence, JSON_TOK_STRING_BUF),
};

static struct fence_delta delta;

/* The cloud libraries return negative errno (device) or positive CoAP codes. */
static int to_errno(int err)
{
	return err > 0 ? -EIO : err;
}

static bool is_registered(enum lte_lc_nw_reg_status status)
{
	return status == LTE_LC_NW_REG_REGISTERED_HOME ||
	       status == LTE_LC_NW_REG_REGISTERED_ROAMING;
}

static void lte_event_handler(const struct lte_lc_evt *const evt)
{
	if (evt->type != LTE_LC_EVT_NW_REG_STATUS)
		return;

	if (is_registered(evt->nw_reg_status))
		k_sem_give(&registered_sem);
}

/* The GNSS adapter normally started the modem library; do not repeat it. */
static int modem_ensure_init(void)
{
	int err;

	if (nrf_modem_is_initialized())
		return 0;

	err = nrf_modem_lib_init();
	if (err != 0) {
		LOG_ERR("Modem library init failed: %d", err);
		return err < 0 ? err : -EIO;
	}

	return 0;
}

/* Turn LTE on (GNSS is left as it is) and wait until the network accepts us. */
static int lte_attach(void)
{
	enum lte_lc_nw_reg_status status;
	int err;

	if (!handlers_registered) {
		lte_lc_register_handler(lte_event_handler);
		handlers_registered = true;
	}

	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_LTE);
	if (err != 0) {
		LOG_ERR("Could not activate LTE: %d", err);
		return err < 0 ? err : -EIO;
	}

	k_sem_reset(&registered_sem);
	if (lte_lc_nw_reg_status_get(&status) == 0 && is_registered(status))
		return 0;

	LOG_INF("Waiting for the network (up to %d s)", CONFIG_DRVF_CLOUD_REGISTER_TIMEOUT_S);
	if (k_sem_take(&registered_sem, K_SECONDS(CONFIG_DRVF_CLOUD_REGISTER_TIMEOUT_S)) != 0) {
		LOG_ERR("Network registration timed out");
		return -ETIMEDOUT;
	}

	return 0;
}

/* The nRF Cloud JWT is signed with the current time, so it must be valid. */
static int wait_date_time(void)
{
	if (date_time_is_valid())
		return 0;

	(void)date_time_update_async(NULL);
	for (int s = 0; s < CONFIG_DRVF_CLOUD_DATE_TIME_TIMEOUT_S; s += DATE_TIME_POLL_S) {
		if (date_time_is_valid())
			return 0;
		k_sleep(K_SECONDS(DATE_TIME_POLL_S));
	}

	LOG_ERR("No valid date and time from the network");
	return -ETIMEDOUT;
}

static void log_attestation_token(const struct nrf_provisioning_callback_data *event)
{
	LOG_WRN("Device not claimed. Claim it on nrfcloud.com with this attestation token:");
	if (event->token == NULL) {
		LOG_WRN("The attestation token is not available");
		return;
	}

	LOG_WRN("\n----- BEGIN ATTESTATION TOKEN -----\n%.*s.%.*s\n----- END ATTESTATION TOKEN -----",
		event->token->attest_sz, event->token->attest,
		event->token->cose_sz, event->token->cose);
}

static void provisioning_finish(int result)
{
	provisioning_result = result;
	k_sem_give(&provisioning_sem);
}

/*
 * Credentials can only be written with the radio off, and "off" includes GNSS:
 * a running GNSS session is lost. Remember it and switch it on again.
 */
static void provisioning_radio_off(void)
{
	enum lte_lc_func_mode mode;

	gnss_was_active = lte_lc_func_mode_get(&mode) == 0 &&
			  (mode == LTE_LC_FUNC_MODE_ACTIVATE_GNSS ||
			   mode == LTE_LC_FUNC_MODE_NORMAL);
	if (gnss_was_active) {
		LOG_WRN("Provisioning stops the GNSS; it must be started again afterwards");
		atomic_set(&radio_reset_pending, 1);
	}

	if (lte_lc_func_mode_set(LTE_LC_FUNC_MODE_OFFLINE) != 0)
		LOG_ERR("Could not switch the radio off for provisioning");
}

static void provisioning_radio_on(void)
{
	if (lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_LTE) != 0)
		LOG_ERR("Could not switch LTE on after provisioning");

	if (gnss_was_active &&
	    lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_GNSS) != 0)
		LOG_ERR("Could not switch the GNSS on after provisioning");
}

/* Runs in the provisioning work queue: no long blocking calls. */
static void provisioning_event(const struct nrf_provisioning_callback_data *event)
{
	switch (event->type) {
	case NRF_PROVISIONING_EVENT_NEED_LTE_DEACTIVATED:
		provisioning_radio_off();
		break;
	case NRF_PROVISIONING_EVENT_NEED_LTE_ACTIVATED:
		provisioning_radio_on();
		break;
	case NRF_PROVISIONING_EVENT_DONE:
	case NRF_PROVISIONING_EVENT_NO_COMMANDS:
		provisioning_finish(0);
		break;
	case NRF_PROVISIONING_EVENT_FAILED_TOO_MANY_COMMANDS:
		LOG_ERR("Provisioning: too many commands for the device");
		provisioning_finish(0);
		break;
	case NRF_PROVISIONING_EVENT_FAILED:
		LOG_ERR("Provisioning failed");
		provisioning_finish(-EIO);
		break;
	case NRF_PROVISIONING_EVENT_FAILED_NO_VALID_DATETIME:
		LOG_ERR("Provisioning failed: no valid date and time");
		provisioning_finish(-ETIMEDOUT);
		break;
	case NRF_PROVISIONING_EVENT_FAILED_DEVICE_NOT_CLAIMED:
		log_attestation_token(event);
		provisioning_finish(-EACCES);
		break;
	case NRF_PROVISIONING_EVENT_FAILED_WRONG_ROOT_CA:
		LOG_ERR("Provisioning failed: wrong root CA certificate");
		provisioning_finish(-EPROTO);
		break;
	case NRF_PROVISIONING_EVENT_FATAL_ERROR:
		LOG_ERR("Provisioning: fatal error");
		provisioning_finish(-EPROTO);
		break;
	default:
		break;
	}
}

/* Ask the provisioning service for credentials; blocks until it finishes. */
static int provision_device(void)
{
	int err;

	k_sem_reset(&provisioning_sem);
	err = nrf_provisioning_trigger_manually();
	if (err != 0) {
		LOG_ERR("Could not start provisioning: %d", err);
		return to_errno(err);
	}

	LOG_INF("Provisioning (up to %d s)", CONFIG_DRVF_CLOUD_PROVISION_TIMEOUT_S);
	if (k_sem_take(&provisioning_sem, K_SECONDS(CONFIG_DRVF_CLOUD_PROVISION_TIMEOUT_S)) != 0) {
		LOG_ERR("Provisioning timed out");
		return -ETIMEDOUT;
	}

	if (provisioning_result == 0)
		k_sleep(K_SECONDS(AFTER_PROVISION_DELAY_S));

	return provisioning_result;
}

/* One-time initialization of the cloud libraries. */
static int stack_init(void)
{
	int err;

	if (stack_ready)
		return 0;

	err = nrf_cloud_coap_init();
	if (err != 0) {
		LOG_ERR("nrf_cloud_coap_init failed: %d", err);
		return to_errno(err);
	}

	err = nrf_provisioning_init(provisioning_event);
	if (err != 0 && err != -EALREADY) {
		LOG_ERR("nrf_provisioning_init failed: %d", err);
		return to_errno(err);
	}

	stack_ready = true;

	return 0;
}

static bool is_auth_error(int err)
{
	return err == -EACCES || err == -ENOEXEC || err == -ECONNREFUSED;
}

int cloud_open(void)
{
	int err;

	if (connected)
		return 0;

	err = modem_ensure_init();
	if (err == 0)
		err = lte_attach();
	if (err == 0)
		err = wait_date_time();
	if (err == 0)
		err = stack_init();
	if (err != 0)
		return err;

	err = nrf_cloud_coap_connect(APP_VERSION);
	/*
	 * -ECONNREFUSED also means any socket or handshake failure. Provisioning
	 * switches the radio off, so it is tried once per boot for that error.
	 */
	if (is_auth_error(err) && !(err == -ECONNREFUSED && refused_provision_tried)) {
		refused_provision_tried = refused_provision_tried || err == -ECONNREFUSED;
		LOG_WRN("No valid credentials (%d), provisioning the device", err);
		err = provision_device();
		if (err != 0)
			return err;

		err = nrf_cloud_coap_connect(APP_VERSION);
	}

	if (err != 0) {
		LOG_ERR("Could not connect to nRF Cloud: %d", err);
		return to_errno(err);
	}

	connected = true;
	LOG_INF("Connected to nRF Cloud");

	return 0;
}

void cloud_close(void)
{
	int err = nrf_cloud_coap_disconnect();

	if (err != 0)
		LOG_WRN("nRF Cloud disconnect failed: %d", err);

	connected = false;
}

/* Shadow report with the fence text, its version and what the device did with it. */
static int format_fence_report(const char *fence_text, uint32_t version, int status)
{
	const int n = snprintk(report_buf, sizeof(report_buf),
			       "{\"" FENCE_KEY "\":\"%s\",\"fence_version\":%" PRIu32
			       ",\"fence_status\":%d}",
			       fence_text, version, status);

	return (n < 0 || (size_t)n >= sizeof(report_buf)) ? -EMSGSIZE : 0;
}

/* Only the characters of a base64 text: it can go in a JSON string unescaped. */
static bool is_base64_text(const char *text)
{
	for (; *text != '\0'; text++) {
		const char c = *text;
		const bool plain = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
				   (c >= 'a' && c <= 'z') || c == '+' || c == '/' || c == '=';

		if (!plain)
			return false;
	}
	return true;
}

/*
 * Echo a fence text that cannot be decoded, with the format error as status, so
 * it is not downloaded again at every poll. Best effort: a text with other
 * characters would need escaping and is left alone.
 */
static void report_malformed_fence(void)
{
	if (!is_base64_text(delta.fence) ||
	    format_fence_report(delta.fence, 0, FENCE_CODEC_ERR_FORMAT) != 0)
		return;

	(void)nrf_cloud_coap_shadow_state_update(report_buf);
}

/*
 * The delta holds only what differs from the reported state. JSON text that
 * does not decode includes a "fence" string longer than any valid fence.
 */
int cloud_fetch_fence(uint8_t *blob, size_t cap, size_t *len)
{
	size_t resp_len = sizeof(shadow_buf);
	size_t out_len;
	int64_t fields;
	int err;

	if (blob == NULL || len == NULL)
		return -EINVAL;

	/* A call that answers with no data must not leave the last delta to be parsed again. */
	shadow_buf[0] = '\0';
	err = nrf_cloud_coap_shadow_get(shadow_buf, &resp_len, true, COAP_CONTENT_FORMAT_APP_JSON);
	if (err == -E2BIG) {
		LOG_WRN("The shadow delta does not fit the buffer");
		return -EBADMSG;
	}
	if (err != 0)
		return to_errno(err);

	if (resp_len == 0 || shadow_buf[0] == '\0')
		return -ENODATA;

	memset(&delta, 0, sizeof(delta));
	fields = json_obj_parse(shadow_buf, resp_len, fence_delta_descr,
				ARRAY_SIZE(fence_delta_descr), &delta);
	if (fields < 0) {
		LOG_WRN("The shadow delta is not usable JSON (%d)", (int)fields);
		return -EBADMSG;
	}

	if (!(fields & BIT(0)))
		return -ENODATA;

	err = base64_decode(blob, cap, &out_len, (const uint8_t *)delta.fence,
			    strlen(delta.fence));
	if (err != 0 || out_len == 0) {
		LOG_WRN("The fence is not valid base64 or does not fit (%d)", err);
		report_malformed_fence();
		return -EBADMSG;
	}

	*len = out_len;

	return 0;
}

int cloud_report_fence(const uint8_t *blob, size_t len, uint32_t version, int status)
{
	size_t b64_len = 0;
	int err;

	if (blob == NULL && len != 0)
		return -EINVAL;

	if (len > FENCE_CODEC_MAX_SIZE)
		return -EMSGSIZE;

	if (base64_encode((uint8_t *)fence_b64, sizeof(fence_b64), &b64_len, blob, len) != 0)
		return -EMSGSIZE;
	fence_b64[b64_len] = '\0';

	err = format_fence_report(fence_b64, version, status);
	if (err != 0)
		return err;

	return to_errno(nrf_cloud_coap_shadow_state_update(report_buf));
}

int cloud_send_telemetry(const char *json)
{
	int64_t ts_ms;
	int n;

	if (json == NULL)
		return -EINVAL;

	n = snprintk(message_buf, sizeof(message_buf),
		     "{\"" NRF_CLOUD_JSON_APPID_KEY "\":\"" TELEMETRY_APP_ID "\","
		     "\"" NRF_CLOUD_JSON_MSG_TYPE_KEY "\":\"" NRF_CLOUD_JSON_MSG_TYPE_VAL_DATA "\","
		     "\"" NRF_CLOUD_JSON_DATA_KEY "\":%s",
		     json);
	if (n < 0 || (size_t)n >= sizeof(message_buf))
		return -EMSGSIZE;

	/* The timestamp is left out while the date is not valid. */
	if (date_time_now(&ts_ms) == 0) {
		const int m = snprintk(message_buf + n, sizeof(message_buf) - (size_t)n,
				       ",\"" NRF_CLOUD_MSG_TIMESTAMP_KEY "\":%" PRId64, ts_ms);

		if (m < 0 || (size_t)m >= sizeof(message_buf) - (size_t)n)
			return -EMSGSIZE;
		n += m;
	}
	if ((size_t)n + 1 >= sizeof(message_buf))
		return -EMSGSIZE;
	message_buf[n] = '}';
	message_buf[n + 1] = '\0';

	return to_errno(nrf_cloud_coap_json_message_send(message_buf, false, CONFIRMABLE));
}

/* MCC+MNC of the serving network; "" if the modem does not report one. */
static void read_operator(char *out, size_t size)
{
	char plmn[CLOUD_OPERATOR_MAX + 1] = "";

	if (nrf_modem_at_scanf(MONITOR_CMD, MONITOR_PLMN_FMT, plmn) == 1)
		strncpy(out, plmn, size - 1);
}

bool cloud_radio_was_reset(void)
{
	return atomic_set(&radio_reset_pending, 0) != 0;
}

int cloud_link_info(cloud_link_info_t *info)
{
	int rsrp;
	uint8_t band;

	if (info == NULL)
		return -EINVAL;

	memset(info, 0, sizeof(*info));

	if (modem_info_get_rsrp(&rsrp) == 0) {
		info->has_rsrp = true;
		info->rsrp_dbm = (int16_t)rsrp;
	}

	if (modem_info_get_current_band(&band) == 0)
		info->band = band;

	read_operator(info->operator_id, sizeof(info->operator_id));

	if (!info->has_rsrp && info->band == 0 && info->operator_id[0] == '\0')
		return -ENODATA;

	return 0;
}
