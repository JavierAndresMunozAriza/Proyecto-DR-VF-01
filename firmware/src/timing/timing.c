/*
 * timing.c - timing helpers on top of the Zephyr kernel
 */

#include <zephyr/kernel.h>

#include "timing.h"

timing_stamp_t timing_now(void)
{
	return k_cycle_get_32();
}

uint32_t timing_elapsed_us(timing_stamp_t start)
{
	return k_cyc_to_us_near32(k_cycle_get_32() - start);
}

void timing_sleep_ms(uint32_t ms)
{
	k_msleep(ms);
}
