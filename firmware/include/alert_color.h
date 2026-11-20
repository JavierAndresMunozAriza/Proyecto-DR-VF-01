/**
 * @file alert_color.h
 * @brief Maps the distance to the fence boundary to an RGB alert color
 *
 * Pure C, no hardware dependency, so it is testable on the host. The LED
 * driver (rgb_led) consumes the result; this module never touches a pin.
 *
 * Behavior:
 *  - inside, distance >= safe_m: pure green
 *  - inside, 0 < distance < safe_m: linear mix from green toward red
 *  - inside, distance == 0: pure red, steady
 *  - outside the fence: red, blinking
 *  - invalid input (NaN/infinite/negative distance, safe_m <= 0): red,
 *    blinking (fail safe, same as outside)
 */

#ifndef ALERT_COLOR_H
#define ALERT_COLOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	bool blink;
} alert_color_t;

/**
 * @brief Compute the alert color for one geofence check
 *
 * @param distance_m Distance to the nearest edge in meters (>= 0)
 * @param inside     true if the point is inside the polygon
 * @param safe_m     Distance at or beyond which the color is fully green
 *                   (must be > 0)
 * @return The color to show; never fails, invalid input maps to blinking red
 */
alert_color_t alert_color_from_distance(double distance_m, bool inside,
					double safe_m);

#ifdef __cplusplus
}
#endif

#endif /* ALERT_COLOR_H */
