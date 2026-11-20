/*
 * Host unit tests for alert_color_from_distance().
 * Exits non-zero if any check fails.
 */

#include <math.h>
#include <stdio.h>

#include "alert_color.h"

static int failures;

#define CHECK_COLOR(name, c, er, eg, eb, eblink)                              \
	do {                                                                  \
		if ((c).r != (er) || (c).g != (eg) || (c).b != (eb) ||        \
		    (c).blink != (eblink)) {                                  \
			printf("FAIL %s: got (%u,%u,%u,%d) want (%d,%d,%d,%d)\n", \
			       name, (c).r, (c).g, (c).b, (c).blink, er, eg,  \
			       eb, eblink);                                   \
			failures++;                                           \
		} else {                                                      \
			printf("ok   %s\n", name);                            \
		}                                                             \
	} while (0)

int main(void)
{
	alert_color_t c;

	c = alert_color_from_distance(20.0, true, 20.0);
	CHECK_COLOR("green at the safe distance", c, 0, 255, 0, false);

	c = alert_color_from_distance(500.0, true, 20.0);
	CHECK_COLOR("green far beyond the safe distance", c, 0, 255, 0, false);

	c = alert_color_from_distance(0.0, true, 20.0);
	CHECK_COLOR("steady red on the boundary", c, 255, 0, 0, false);

	c = alert_color_from_distance(10.0, true, 20.0);
	CHECK_COLOR("half way is an even mix", c, 128, 128, 0, false);

	c = alert_color_from_distance(5.0, true, 20.0);
	CHECK_COLOR("close to the edge is mostly red", c, 191, 64, 0, false);

	c = alert_color_from_distance(5.0, false, 20.0);
	CHECK_COLOR("outside blinks red", c, 255, 0, 0, true);

	c = alert_color_from_distance(NAN, true, 20.0);
	CHECK_COLOR("NaN distance blinks red", c, 255, 0, 0, true);

	c = alert_color_from_distance(INFINITY, true, 20.0);
	CHECK_COLOR("infinite distance blinks red", c, 255, 0, 0, true);

	c = alert_color_from_distance(-1.0, true, 20.0);
	CHECK_COLOR("negative distance blinks red", c, 255, 0, 0, true);

	c = alert_color_from_distance(10.0, true, 0.0);
	CHECK_COLOR("zero safe distance blinks red", c, 255, 0, 0, true);

	c = alert_color_from_distance(10.0, true, NAN);
	CHECK_COLOR("NaN safe distance blinks red", c, 255, 0, 0, true);

	c = alert_color_searching(false);
	CHECK_COLOR("searching without any fix blinks blue", c, 0, 0, 255, true);

	c = alert_color_searching(true);
	CHECK_COLOR("searching with a poor-accuracy fix blinks cyan", c, 0, 255, 255, true);

	if (failures) {
		printf("%d check(s) failed\n", failures);
		return 1;
	}
	printf("all checks passed\n");
	return 0;
}
