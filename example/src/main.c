#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include "blinkyleds.h"

#define DELAY	10000

bl_cadence_id_t cadence[] = {
	BL_CADENCE_OFF,
	BL_CADENCE_ON,
	BL_CADENCE_WINK,
	BL_CADENCE_BLINK,
	BL_CADENCE_FLICKER,
	BL_CADENCE_FLASH,
	BL_CADENCE_DOUBLEFLASH,
	BL_CADENCE_LIGHTHOUSE,
	BL_CADENCE_FASTBLINK
};

char * cadence_str[] = {
	"BL_CADENCE_OFF",
	"BL_CADENCE_ON",
	"BL_CADENCE_WINK",
	"BL_CADENCE_BLINK",
	"BL_CADENCE_FLICKER",
	"BL_CADENCE_FLASH",
	"BL_CADENCE_DOUBLEFLASH",
	"BL_CADENCE_LIGHTHOUSE",
	"BL_CADENCE_FASTBLINK"
};

int main(void)
{
	unsigned int index = 0;

	/* demo each of the cadences */
	while (1) {
		printf("using cadence (%s)\n", cadence_str[index]);
		bl_set_led_state("led_1", cadence[index], BL_DURATION_FOREVER);
		index = ((index + 1) % 9);

		k_sleep(K_MSEC(DELAY));
	}
	
	return 0;
}
