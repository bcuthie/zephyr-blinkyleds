/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "blinkyleds.h"

int main(void)
{
	bl_set_led_state("led_2", BL_CADENCE_BLINK, BL_DURATION_FOREVER);
	
	return 0;
}
