/**
 * @file gnss.h
 * @brief GNSS adapter: board-independent access to position fixes
 *
 * POSIX-style interface (open, read, write, ioctl, test, close). Each board
 * provides one backend (nRF9151 modem GNSS on the Thingy:91 X, the external
 * u-blox MAX-M10S on the final PCB). Application logic only sees valid fixes
 * through this interface.
 */

#ifndef GNSS_H
#define GNSS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** A valid position fix */
typedef struct {
	double latitude;    /**< Decimal degrees */
	double longitude;   /**< Decimal degrees */
	float accuracy_m;   /**< Horizontal accuracy (1-sigma), meters */
} gnss_fix_t;

/** Commands for gnss_ioctl() */
enum gnss_ioctl_cmd {
	/** Get the satellites currently tracked; arg is an `unsigned int *` */
	GNSS_IOCTL_GET_SATELLITES,
	/**
	 * Set the seconds between fixes (1 = continuous, the default); arg is a
	 * `const uint16_t *`. Only accepted before gnss_open(), otherwise -EBUSY.
	 */
	GNSS_IOCTL_SET_FIX_INTERVAL,
};

/**
 * @brief Power up the receiver and start fixes
 * @return 0 on success, a negative errno otherwise
 */
int gnss_open(void);

/**
 * @brief Read the latest valid fix not read yet
 *
 * Only valid fixes are returned; a fix the receiver reports as invalid is
 * never delivered, so a lost signal shows up as a timeout.
 *
 * @param fix         Receives the fix
 * @param timeout_ms  How long to wait for a new fix; 0 does not wait
 * @return 0 on success, -EAGAIN if no new valid fix arrived in time, -EINVAL
 *         if @p fix is NULL
 */
int gnss_read(gnss_fix_t *fix, uint32_t timeout_ms);

/**
 * @brief Send data to the receiver
 *
 * No backend accepts data yet (assistance data injection is not
 * implemented), so this always fails.
 *
 * @return -ENOTSUP
 */
int gnss_write(const void *data, size_t len);

/**
 * @brief Query or configure the receiver, see enum gnss_ioctl_cmd
 * @return 0 on success, -EINVAL for a NULL arg or an unknown command, -EBUSY
 *         if the command is not allowed now, or another negative errno
 */
int gnss_ioctl(enum gnss_ioctl_cmd cmd, void *arg);

/**
 * @brief Self-test: check that the receiver answers
 * @return 0 if it does, a negative errno otherwise
 */
int gnss_test(void);

/**
 * @brief Stop the receiver and power it down
 * @return 0 on success, a negative errno otherwise
 */
int gnss_close(void);

#ifdef __cplusplus
}
#endif

#endif /* GNSS_H */
