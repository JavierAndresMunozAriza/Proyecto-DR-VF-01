/**
 * @file button.h
 * @brief User button adapter
 *
 * Board-independent interface. Each board provides the devicetree alias
 * sw0 in its overlay.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configure the button and its interrupt
 * @return 0 on success, -ENODEV if the GPIO device is not ready, or a
 *         negative errno from the GPIO driver
 */
int button_init(void);

/**
 * @brief Tell whether the button was pressed since the last call
 *
 * Consumes the press: it returns true once per press. Presses closer than
 * the debounce time to the previous accepted press are ignored.
 */
bool button_pressed(void);

#ifdef __cplusplus
}
#endif

#endif /* BUTTON_H */
