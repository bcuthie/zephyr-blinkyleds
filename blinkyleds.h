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

#pragma once

typedef enum {
	BL_CADENCE_OFF		= 0,
	BL_CADENCE_ON,
	BL_CADENCE_WINK,
	BL_CADENCE_BLINK,
	BL_CADENCE_FLICKER,
	BL_CADENCE_FLASH,
	BL_CADENCE_DOUBLEFLASH,
	BL_CADENCE_LIGHTHOUSE,
	BL_CADENCE_FASTBLINK
} bl_cadence_id_t;

#define BL_DURATION_FOREVER		-1

#define BL_C_FLAG_ON 0x8000
#define BL_C_TIME_MASK(A) (A & 0x7FFF)
#define BL_C_END 0xFFFF
#define BL_C_ON(A) (A | BL_C_FLAG_ON)
#define BL_C_OFF(A) (A)

typedef struct cadence {
	struct cadence *next;
	const uint16_t *pattern;
	int timer;
	int idx;
	int state;
} bl_cadence_t;

#define DEFINE_CADENCE(A) { .pattern = A, .timer = 0, .idx = 0, .state = 0 }

void bl_register_cadence(bl_cadence_t *cadence);
int bl_set_led_state(const char * name, bl_cadence_id_t cadence_id, int duration_ms);
int bl_set_led_state_with_cadence(const char * name, bl_cadence_t *cadence, int duration_ms);
