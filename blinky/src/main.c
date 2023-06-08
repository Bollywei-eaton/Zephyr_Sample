/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/drivers/gpio.h>

/* size of stack area used by each thread */
#define BLINK_STACKSIZE 1024

/* scheduling priority used by each thread */
#define BLINK_PRIORITY 7

/* 100 msec = 0.1 sec */
#define SLEEP_TIME_MS   100

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
/* The devicetree node identifier for the "led1" alias. */
#define LED1_NODE DT_ALIAS(led1)
#if !DT_NODE_HAS_STATUS(LED1_NODE, okay)
#error "Unsupported board: led1 devicetree alias is not defined"
#endif
/* The devicetree node identifier for the "led2" alias. */
#define LED2_NODE DT_ALIAS(led2)
#if !DT_NODE_HAS_STATUS(LED2_NODE, okay)
#error "Unsupported board: led2 devicetree alias is not defined"
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
static const struct led led_1 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED1_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(LED1_NODE, label, ""),
};
static const struct led led_2 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED1_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(LED1_NODE, label, ""),
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

	if (!device_is_ready(led_1.spec.port)) {
		printk("Error: %s device is not ready\n", led_1.spec.port->name);
		return;
	}	

	if (!device_is_ready(sw_0.spec.port)) {
		printk("Error: %s device is not ready\n", sw_0.spec.port->name);
		return;
	}

	ret = gpio_pin_configure_dt(&led_1.spec, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (LED '%s')\n",
			ret, led_1.spec.pin, led_1.gpio_pin_name);		
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
			gpio_pin_toggle(led_1.spec.port, led_1.spec.pin);
		}
		k_msleep(SLEEP_TIME_MS);
	}
}

void blink(void)
{
	int ret;

	printk("blink task is running\n");

	if (!device_is_ready(led_0.spec.port)) {
		printk("Error: %s device is not ready\n", led_0.spec.port->name);
		return;
	}	
	ret = gpio_pin_configure_dt(&led_0.spec, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (LED '%s')\n",
			ret, led_0.spec.pin, led_0.gpio_pin_name);		
		return;
	}	
	while (1) {
		gpio_pin_toggle(led_0.spec.port, led_0.spec.pin);
		k_msleep(SLEEP_TIME_MS);
	}
}

K_THREAD_DEFINE(blink_id, BLINK_STACKSIZE, blink, NULL, NULL, NULL,
		BLINK_PRIORITY, 0, 2000);  //Start running task after 2000ms
// K_THREAD_DEFINE(blink_id, BLINK_STACKSIZE, blink, NULL, NULL, NULL,
// 		BLINK_PRIORITY, 0, 0);		