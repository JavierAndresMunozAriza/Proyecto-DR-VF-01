/**
 * @file rgb_led.h
 * @brief RGB LED adapter (PWM-driven)
 *
 * Board-independent interface. Each board provides the devicetree aliases
 * pwm-led0 (red), pwm-led1 (green) and pwm-led2 (blue) in its overlay.
 */

#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check that the three PWM channels are ready
 * @return 0 on success, -ENODEV if a PWM device is not ready
 */
int rgb_led_init(void);

/**
 * @brief Set the LED color; 0 is off, 255 is full brightness per channel
 * @return 0 on success, a negative errno from the PWM driver otherwise
 */
int rgb_led_set(uint8_t r, uint8_t g, uint8_t b);

#ifdef __cplusplus
}
#endif

#endif /* RGB_LED_H */
