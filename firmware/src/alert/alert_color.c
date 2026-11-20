/*
 * alert_color.c - distance to fence boundary -> RGB alert color
 *
 * Pure C, no hardware dependency. See alert_color.h for the behavior.
 */

#include <math.h>

#include "alert_color.h"

static const alert_color_t BLINK_RED = { 255, 0, 0, true };

alert_color_t alert_color_from_distance(double distance_m, bool inside,
					double safe_m)
{
	alert_color_t color = { 0, 0, 0, false };
	double t;

	/* Fail safe: outside or any invalid input shows blinking red. */
	if (!inside || !isfinite(distance_m) || distance_m < 0.0 ||
	    !isfinite(safe_m) || safe_m <= 0.0)
		return BLINK_RED;

	t = distance_m / safe_m;
	if (t > 1.0)
		t = 1.0;

	/* Round to nearest; t is in [0, 1] so the casts cannot overflow. */
	color.r = (uint8_t)(255.0 * (1.0 - t) + 0.5);
	color.g = (uint8_t)(255.0 * t + 0.5);

	return color;
}
