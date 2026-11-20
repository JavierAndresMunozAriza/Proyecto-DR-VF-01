/*
 * rgb_led.c - RGB LED adapter on top of the Zephyr PWM API
 */

#include <errno.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>

#include "rgb_led.h"

#define LED_COUNT 3

/* Order: red, green, blue. The overlay of each board defines the aliases. */
static const struct pwm_dt_spec leds[LED_COUNT] = {
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0)),
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led1)),
	PWM_DT_SPEC_GET(DT_ALIAS(pwm_led2)),
};

int rgb_led_init(void)
{
	for (int i = 0; i < LED_COUNT; i++) {
		if (!pwm_is_ready_dt(&leds[i]))
			return -ENODEV;
	}

	return 0;
}

int rgb_led_set(uint8_t r, uint8_t g, uint8_t b)
{
	const uint8_t level[LED_COUNT] = { r, g, b };

	for (int i = 0; i < LED_COUNT; i++) {
		uint32_t pulse = (uint32_t)(((uint64_t)leds[i].period * level[i]) / UINT8_MAX);
		int ret = pwm_set_pulse_dt(&leds[i], pulse);

		if (ret < 0)
			return ret;
	}

	return 0;
}
