/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "blinkyleds.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

int main(void)
{
	bl_set_led_state("led_2", BL_CADENCE_BLINK, BL_DURATION_FOREVER);
	
	while (1) {
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
