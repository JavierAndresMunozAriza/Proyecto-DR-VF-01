/*
 * gnss_nrf91.c - GNSS backend for the nRF9151 modem (Nordic Thingy:91 X)
 *
 * The modem runs in GNSS-only mode (no LTE). The modem library calls the
 * event handler in interrupt context, so the handler only copies the data
 * and signals; everything else (logging included) happens in the caller's
 * thread.
 */

#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_at.h>
#include <nrf_modem_gnss.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "gnss.h"

LOG_MODULE_REGISTER(gnss, LOG_LEVEL_INF);

#define DEFAULT_FIX_INTERVAL_S 1 /* 1 = continuous fixes */
#define FIX_RETRY_S            0 /* 0 = keep searching until a fix is found */
#define AT_REPLY_LEN           64

/*
 * Data deleted to simulate a cold start. The TCXO offset is a property of the
 * device, not of the satellites, so it stays (as the modem library docs advise).
 */
#define COLD_START_DELETE_MASK (NRF_MODEM_GNSS_DELETE_EPHEMERIDES |            \
				NRF_MODEM_GNSS_DELETE_ALMANACS |                \
				NRF_MODEM_GNSS_DELETE_IONO_CORRECTION_DATA |    \
				NRF_MODEM_GNSS_DELETE_LAST_GOOD_FIX |           \
				NRF_MODEM_GNSS_DELETE_GPS_TOW |                 \
				NRF_MODEM_GNSS_DELETE_GPS_WEEK |                \
				NRF_MODEM_GNSS_DELETE_UTC_DATA |                \
				NRF_MODEM_GNSS_DELETE_GPS_TOW_PRECISION)

static K_SEM_DEFINE(fix_ready, 0, 1);
static struct k_spinlock fix_lock;
static gnss_fix_t latest_fix;
static gnss_satellites_t satellites;
static uint16_t fix_interval_s = DEFAULT_FIX_INTERVAL_S;
static bool opened;

/* Modem library calls return a negative errno or a positive modem error. */
static int to_errno(int err)
{
	return err < 0 ? err : -EIO;
}

/* The signal type tells the satellite system: ids overlap between GPS and Galileo. */
static gnss_satellites_t count_satellites(const struct nrf_modem_gnss_pvt_data_frame *pvt)
{
	gnss_satellites_t counts = { 0 };

	for (int i = 0; i < NRF_MODEM_GNSS_MAX_SATELLITES; i++) {
		if (pvt->sv[i].sv == 0)
			continue;

		counts.total++;
		switch (pvt->sv[i].signal) {
		case NRF_MODEM_GNSS_SIGNAL_GPS_L1_CA:
			counts.gps++;
			break;
		case NRF_MODEM_GNSS_SIGNAL_QZSS_L1_CA:
			counts.qzss++;
			break;
		case NRF_MODEM_GNSS_SIGNAL_GAL_E1_OS:
			counts.galileo++;
			break;
		default:
			break;
		}
	}

	return counts;
}

/* Runs in interrupt context: copy the data, signal, nothing else. */
static void gnss_event_handler(int event)
{
	struct nrf_modem_gnss_pvt_data_frame pvt;

	if (event != NRF_MODEM_GNSS_EVT_PVT)
		return;

	if (nrf_modem_gnss_read(&pvt, sizeof(pvt), NRF_MODEM_GNSS_DATA_PVT) != 0)
		return;

	const gnss_satellites_t counts = count_satellites(&pvt);

	K_SPINLOCK(&fix_lock) {
		satellites = counts;
	}

	if (!(pvt.flags & NRF_MODEM_GNSS_PVT_FLAG_FIX_VALID))
		return;

	K_SPINLOCK(&fix_lock) {
		latest_fix.latitude = pvt.latitude;
		latest_fix.longitude = pvt.longitude;
		latest_fix.accuracy_m = pvt.accuracy;
	}
	k_sem_give(&fix_ready);
}

int gnss_test(void)
{
	char reply[AT_REPLY_LEN];

	if (nrf_modem_at_cmd(reply, sizeof(reply), "AT+CGMR") != 0) {
		LOG_WRN("The modem did not answer the firmware version request");
		return -EIO;
	}

	reply[strcspn(reply, "\r\n")] = '\0';
	LOG_INF("Modem firmware: %s", reply);

	return 0;
}

int gnss_open(void)
{
	int err;

	err = nrf_modem_lib_init();
	if (err != 0) {
		LOG_ERR("Modem library init failed: %d", err);
		return to_errno(err);
	}

	/* Informative: a failed self-test does not stop the receiver. */
	(void)gnss_test();

	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_GNSS);
	if (err != 0) {
		LOG_ERR("Could not activate GNSS mode: %d", err);
		return to_errno(err);
	}

	if (IS_ENABLED(CONFIG_DRVF_GNSS_COLD_START)) {
		/* Debug only: the GNSS must not be running yet, which is the case here. */
		if (nrf_modem_gnss_nv_data_delete(COLD_START_DELETE_MASK) != 0)
			LOG_WRN("Could not delete the stored GNSS data");
		else
			LOG_INF("Cold start: stored GNSS data deleted");
	}

	if (nrf_modem_gnss_event_handler_set(gnss_event_handler) != 0 ||
	    nrf_modem_gnss_fix_retry_set(FIX_RETRY_S) != 0 ||
	    nrf_modem_gnss_fix_interval_set(fix_interval_s) != 0) {
		LOG_ERR("Could not configure GNSS");
		return -EIO;
	}

	if (nrf_modem_gnss_start() != 0) {
		LOG_ERR("Could not start GNSS");
		return -EIO;
	}

	opened = true;

	return 0;
}

int gnss_restart(void)
{
	int err;

	if (!opened)
		return -ENODEV;

	/* The session may already be gone: a failed stop is not an error here. */
	(void)nrf_modem_gnss_stop();

	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_ACTIVATE_GNSS);
	if (err != 0) {
		LOG_ERR("Could not activate GNSS mode again: %d", err);
		return to_errno(err);
	}

	if (nrf_modem_gnss_start() != 0) {
		LOG_ERR("Could not start GNSS again");
		return -EIO;
	}

	LOG_INF("GNSS started again");
	return 0;
}

int gnss_read(gnss_fix_t *fix, uint32_t timeout_ms)
{
	if (fix == NULL)
		return -EINVAL;

	if (k_sem_take(&fix_ready, K_MSEC(timeout_ms)) != 0)
		return -EAGAIN;

	K_SPINLOCK(&fix_lock) {
		*fix = latest_fix;
	}

	return 0;
}

int gnss_write(const void *data, size_t len)
{
	ARG_UNUSED(data);
	ARG_UNUSED(len);

	return -ENOTSUP;
}

int gnss_ioctl(enum gnss_ioctl_cmd cmd, void *arg)
{
	if (arg == NULL)
		return -EINVAL;

	switch (cmd) {
	case GNSS_IOCTL_GET_SATELLITES:
		K_SPINLOCK(&fix_lock) {
			*(gnss_satellites_t *)arg = satellites;
		}
		return 0;

	case GNSS_IOCTL_SET_FIX_INTERVAL:
		if (opened)
			return -EBUSY;
		fix_interval_s = *(const uint16_t *)arg;
		return 0;

	default:
		return -EINVAL;
	}
}

int gnss_close(void)
{
	int err;

	if (nrf_modem_gnss_stop() != 0) {
		LOG_ERR("Could not stop GNSS");
		return -EIO;
	}

	err = lte_lc_func_mode_set(LTE_LC_FUNC_MODE_DEACTIVATE_GNSS);
	if (err != 0) {
		LOG_ERR("Could not deactivate GNSS mode: %d", err);
		return to_errno(err);
	}

	opened = false;

	return 0;
}
