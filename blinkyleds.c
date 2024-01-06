
/* 	
	MIT License

	Copyright 2024 Systemix Software, Inc.

	Author: Brian Cuthie

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led.h>
#include <zephyr/logging/log.h>
#include "blinkyleds.h"

#define TIME_QUANTUM_MS 20

static K_MUTEX_DEFINE(led_mtx);

#define LEDS_NODE DT_CHOSEN(zephyr_blinkyleds)
static const struct device *const led_dev = DEVICE_DT_GET(LEDS_NODE);

static const uint16_t LEDPAT_off[] = {BL_C_OFF(100), BL_C_END};
static const uint16_t LEDPAT_on[] = {BL_C_ON(100), BL_C_END};
static const uint16_t LEDPAT_wink[] = {BL_C_ON(980), BL_C_OFF(20), BL_C_END};
static const uint16_t LEDPAT_blink[] = {BL_C_ON(500), BL_C_OFF(500), BL_C_END};
static const uint16_t LEDPAT_flicker[] = {BL_C_ON(20), BL_C_OFF(20), BL_C_END};
static const uint16_t LEDPAT_flash[] = {BL_C_ON(20), BL_C_OFF(980), BL_C_END};
static const uint16_t LEDPAT_doubleflash[] = {BL_C_ON(20), BL_C_OFF(20), BL_C_ON(20), BL_C_OFF(940), BL_C_END};
static const uint16_t LEDPAT_lighthouse[] = {BL_C_ON(250), BL_C_OFF(5000), BL_C_END};
static const uint16_t LEDPAT_fastblink[] = {BL_C_ON(250), BL_C_OFF(250), BL_C_END};

static bl_cadence_t *cadence_list = NULL;

#define CADENCE_COUNT 9

static bl_cadence_t LED_cadences[] = {
        DEFINE_CADENCE(LEDPAT_off),
        DEFINE_CADENCE(LEDPAT_on),
        DEFINE_CADENCE(LEDPAT_wink),
        DEFINE_CADENCE(LEDPAT_blink),
        DEFINE_CADENCE(LEDPAT_flicker),
        DEFINE_CADENCE(LEDPAT_flash),
        DEFINE_CADENCE(LEDPAT_doubleflash),
        DEFINE_CADENCE(LEDPAT_lighthouse),
        DEFINE_CADENCE(LEDPAT_fastblink),
};

#define BL_LED_STATE_OFF 0
#define BL_LED_STATE_ON 1

static typedef struct {
	bl_cadence_t *cadence;
	int duration_ms;
	const char * name;
	int state;
} led_inst_t;

#define DEFINE_LED(A) \
	{ \
		.cadence = NULL, \
		.duration_ms = 0, \
		.name = DT_NODE_FULL_NAME(A), \
		.state = BL_LED_STATE_OFF \
	}

static volatile led_inst_t led[] = { 
	DT_FOREACH_CHILD_SEP(LEDS_NODE, DEFINE_LED, (,)),
	{ .name = NULL }
};

static volatile led_inst_t * led_by_name(const char * name)
{
	volatile led_inst_t *p = led;

	while (p->name != NULL) {
		if (strcmp(p->name, name) == 0) {
			return p;
		}
		p++;
	}

	return NULL;
}

void bl_register_cadence(bl_cadence_t *cadence)
{
	k_mutex_lock(&led_mtx, K_FOREVER);
	cadence->next = cadence_list;
	cadence_list = cadence;
	k_mutex_unlock(&led_mtx);
}

int bl_set_led_state_with_cadence(const char * name, bl_cadence_t *cadence, int duration_ms)
{
	int ret = 0;

	k_mutex_lock(&led_mtx, K_FOREVER);

	volatile led_inst_t *p = led_by_name(name);

	if (p == NULL) {
		ret = -ENOENT;
		goto exit;
	}
	
	p->cadence = cadence;
	p->duration_ms = duration_ms;

exit:
	k_mutex_unlock(&led_mtx);

	return ret;
}

int bl_set_led_state(const char * name, bl_cadence_id_t cadence_id, int duration_ms)
{
	if ((cadence_id < 0) || (cadence_id > CADENCE_COUNT - 1)) {
		return -ENOTSUP;
	}

	return bl_set_led_state_with_cadence(name, &LED_cadences[cadence_id], duration_ms);
}


static void leds_main(void *, void *, void *)
{
	k_mutex_lock(&led_mtx, K_FOREVER);

	for (int i = 0; i < CADENCE_COUNT; i++) {
		bl_register_cadence(&LED_cadences[i]);
	}

	k_mutex_unlock(&led_mtx);

	for (;;) {

		k_mutex_lock(&led_mtx, K_FOREVER);

		/* manage the LEDs */

		for (int i = 0; led[i].name != NULL; i++) {
			volatile led_inst_t *p = &led[i];

			int new_state;

			if (p->duration_ms == 0) {
				new_state = BL_LED_STATE_OFF;
			}
			else {	
				new_state = p->cadence->state;
			}

			if (new_state != p->state) {
				if (new_state == BL_LED_STATE_ON) {
					led_on(led_dev, i);
				}
				else {
					led_off(led_dev, i);
				}
				p->state = new_state;
			}

			if ((p->duration_ms != BL_DURATION_FOREVER) && (p->duration_ms > 0)) {
				p->duration_ms -= MIN(p->duration_ms, TIME_QUANTUM_MS);
			}
		}

		/* manage the cadences */

		bl_cadence_t *c = cadence_list;

		while (c != NULL) {

			if (c->timer == 0) {

				c->idx++;
				if (c->pattern[c->idx] == BL_C_END) {
					c->idx = 0;
				}

				c->timer = BL_C_TIME_MASK(c->pattern[c->idx]);
				c->state = c->pattern[c->idx] & BL_C_FLAG_ON ? 1 : 0;
			}
			else {
				c->timer = c->timer - MIN(c->timer, TIME_QUANTUM_MS);
			}

			c = c->next;
		}

		k_mutex_unlock(&led_mtx);

		k_sleep(K_MSEC(TIME_QUANTUM_MS));
	}
}

K_THREAD_DEFINE(blinkyleds, 512, leds_main, NULL, NULL, NULL, 3, 0, 200);