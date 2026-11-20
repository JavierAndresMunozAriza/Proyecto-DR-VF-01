/*
 * button.c - user button adapter on top of the Zephyr GPIO API
 *
 * The interrupt only raises a flag; debouncing and everything else happens
 * in the caller's thread through button_pressed().
 */

#include <errno.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>

#include "button.h"

#define BUTTON_DEBOUNCE_MS 300

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);
static struct gpio_callback button_callback;
static atomic_t press_pending;
static int64_t last_accepted_ms = -BUTTON_DEBOUNCE_MS;

/* Runs in interrupt context: raise the flag, nothing else. */
static void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	atomic_set(&press_pending, 1);
}

int button_init(void)
{
	int err;

	if (!gpio_is_ready_dt(&button))
		return -ENODEV;

	err = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (err != 0)
		return err;

	gpio_init_callback(&button_callback, button_isr, BIT(button.pin));

	err = gpio_add_callback(button.port, &button_callback);
	if (err != 0)
		return err;

	return gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
}

bool button_pressed(void)
{
	int64_t now_ms;

	if (!atomic_cas(&press_pending, 1, 0))
		return false;

	now_ms = k_uptime_get();
	if (now_ms - last_accepted_ms < BUTTON_DEBOUNCE_MS)
		return false;

	last_accepted_ms = now_ms;

	return true;
}
