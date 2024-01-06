# zephyr-blinkyleds

## LED cadence manager for Zephyr RTOS

Blinkyleds manages the cadence of LEDs in zephyr so you don't have to. Application code
can set LED state, including cadence and duration, then move on to other tasks without
needing to worry about updating LEDs. This allows you to decouple LED control from the
rest of your application, making code cleaner and easier to maintain.


Blinkyleds offers the following features:

- Compact: only a single source file (plus header)
- Simple "set it, and forget it" LED control
- Provides 9 standard LED cadences
- Requires almost no application confiugration when using standard cadences
- completely static memory usage
- supports user defined cadences
- LED cadences are synchronized
- blinkyleds is thread safe

## Basic Usage

To use blinkyleds add the `blinkyleds.c` and `blinkyleds.h` files to your
zephyr application. You'll also need to enable the Zephyr LED driver
because blinkyleds uses that to control the LEDs. Include

`CONFIG_LED=y`

in your application's prj.conf file.

LEDs are referenced by their device tree node name. So, for example, if your device tree
has a `leds` section like this:

```

chosen {
    zephyr,blinkyleds = &blinkyleds;
};

blinkyleds: leds {
    compatible = "gpio-leds";
	status = "okay";

    led_1 {
        gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
    };

    led_2 {
        gpios = <&gpioa 6 GPIO_ACTIVE_HIGH>;
    };

};

otherleds {
    compatible = "gpio-leds";
	status = "okay";

    led_other {
        gpios = <&gpioa 7 GPIO_ACTIVE_HIGH>;
    };
};


```

you would refer to LEDs as "led_1" and "led_2" when using the blinkyleds API. The
`chosen` dts entry allows you to separate LEDs into those that you want blinkyleds
to manage, and others that you may control directly with your application.

For instance,

`bl_set_led_state("led_1", BL_CADENCE_BLINK, BL_DURATION_FOREVER);` 

will cause the LED described by device tree node "led_1" to blink at a rate of one
cycle per second, indefinitely.

When it's desireable to have a LED state persist for only a limited amount of time,
blinkyleds lets you specify a duration. An activity indicator 
might call

`bl_set_led_state("led_1", BL_CADENCE_ON, 75);`

to cause led_1 to illuminate for 75ms, then automatically turn off. Repeated calls supercede previous ones, so
in this example the more activity the more the LED will illuminate. Called frequently enough, the LED would illuminate
continually.

It's also useful, sometimes, to specify longer durations. Say you want to indicate some
error condition for a few seconds. With blinkyleds

`bl_set_led_state("led_1", BL_CADENCE_FLICKER, 5000);`

will cause led_1 to flicker for 5 seconds, then automatically turn off.

Because blinkyleds synchronizes cadences, you can call the API as often as needed without 
generating noticeable effects on the LEDs themselves. For instance, 

`bl_set_led_state("led_1", BL_CADENCE_BLINK, BL_DURATION_FOREVER);` 

results in the same behavior, no matter how many times, or how often, it is 
invoked. This means your application can set LED state when necessary and convenient, 
without regard to cadence timing.

## Custom Cadences

While blinkyleds provides a number of useful cadences, it's also possible for an
application to define custom cadences.

```
static const uint16_t my_special_pattern[] = {
    BL_C_ON(980), 
    BL_C_OFF(20), 
    BL_C_ON(980), 
    BL_C_OFF(20), 
    BL_C_END
};

static cadence_t my_cadence = DEFINE_CADENCE(my_special_pattern);
```

When using custom cadences it's necessary to register them with blinkylights 
before use. This is usually best done at application startup with something like

`bl_register_cadence(&my_cadence);`

You use custom cadences by calling a slightly different API

`bl_set_led_state_with_cadence("led_1", &my_cadence, BL_DURATION_FOREVER);`

but otherwise they operate in the same manner as previously described.

Note that blinkyleds APIs cannot be called from interrupt context.

