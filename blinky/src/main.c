/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
struct led {
	struct gpio_dt_spec spec;
	const char *gpio_pin_name;
};

static const struct led led_0 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(LED0_NODE, label, ""),
};

/* The devicetree node identifier for the "sw0" alias. */
#define BUTTON_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

struct sw {
	struct gpio_dt_spec spec;
	const char *gpio_pin_name;
};

static const struct sw sw_0 = {
	.spec = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(BUTTON_NODE, label, ""),
};

void main(void)
{	
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);

	if (!device_is_ready(led_0.spec.port)) {
		printk("Error: %s device is not ready\n", led_0.spec.port->name);
		return;
	}

	if (!device_is_ready(sw_0.spec.port)) {
		printk("Error: %s device is not ready\n", sw_0.spec.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&led_0.spec, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (LED '%s')\n",
			ret, led_0.spec.pin, led_0.gpio_pin_name);		
		return;
	}

	ret = gpio_pin_configure_dt(&sw_0.spec, GPIO_INPUT);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (SW '%s')\n",
			ret, sw_0.spec.pin, sw_0.gpio_pin_name);			
		return;
	}	

	while (1) {
		if( gpio_pin_get(sw_0.spec.port, sw_0.spec.pin) )
		{
			gpio_pin_toggle(led_0.spec.port, led_0.spec.pin);
		}
		k_msleep(SLEEP_TIME_MS);
	}
}
