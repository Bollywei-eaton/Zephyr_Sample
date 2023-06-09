/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/task_wdt/task_wdt.h>
#include "UserTask.h"

/*
 * To use this sample, either the devicetree's /aliases must have a
 * 'watchdog0' property, or one of the following watchdog compatibles
 * must have an enabled node.
 *
 * If the devicetree has a watchdog node, we get the watchdog device
 * from there. Otherwise, the task watchdog will be used without a
 * hardware watchdog fallback.
 */
#if DT_NODE_HAS_STATUS(DT_ALIAS(watchdog0), okay)
#define WDT_NODE DT_ALIAS(watchdog0)
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_window_watchdog)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32_window_watchdog)
#elif DT_HAS_COMPAT_STATUS_OKAY(st_stm32_watchdog)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32_watchdog)
#elif DT_HAS_COMPAT_STATUS_OKAY(nordic_nrf_wdt)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(nordic_nrf_wdt)
#elif DT_HAS_COMPAT_STATUS_OKAY(espressif_esp32_watchdog)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(espressif_esp32_watchdog)
#elif DT_HAS_COMPAT_STATUS_OKAY(silabs_gecko_wdog)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(silabs_gecko_wdog)
#elif DT_HAS_COMPAT_STATUS_OKAY(nxp_kinetis_wdog32)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(nxp_kinetis_wdog32)
#elif DT_HAS_COMPAT_STATUS_OKAY(microchip_xec_watchdog)
#define WDT_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(microchip_xec_watchdog)
#endif

/* 100 msec = 0.1 sec */
#define SLEEP_TIME_MS   500

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
static const struct led led_1 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED1_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(LED1_NODE, label, ""),
};
static const struct led led_2 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED2_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(LED2_NODE, label, ""),
};

/* The devicetree node identifier for the "sw0" alias. */
#define BUTTON_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

const struct sw sw_0 = {
	.spec = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(BUTTON_NODE, label, ""),
};
static struct gpio_callback button_cb_data;

extern struct k_sem key_sem;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	int ret;
	gpio_pin_toggle(led_2.spec.port, led_2.spec.pin);
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
	ret = gpio_pin_interrupt_configure_dt(&sw_0.spec,
					      GPIO_INT_DISABLE);						  
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, sw_0.spec.port->name, sw_0.spec.pin);
		return;
	}		
	k_sem_give(&key_sem);
}

void main(void)
{	
	int ret;

	printk("Hello World! %s\n", CONFIG_BOARD);
#ifdef WDT_NODE
	const struct device *hw_wdt_dev = DEVICE_DT_GET(WDT_NODE);
#else
	const struct device *hw_wdt_dev = NULL;
#endif

	if (!device_is_ready(hw_wdt_dev)) {
		printk("Hardware watchdog %s is not ready; ignoring it.\n",
		       hw_wdt_dev->name);
		hw_wdt_dev = NULL;
	}

	ret = task_wdt_init(hw_wdt_dev);
	if (ret != 0) {
		printk("task wdt init failure: %d\n", ret);
		return;
	}

	if (!device_is_ready(led_1.spec.port)) {
		printk("Error: %s device is not ready\n", led_1.spec.port->name);
		return;
	}	
	ret = gpio_pin_configure_dt(&led_1.spec, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (LED '%s')\n",
			ret, led_1.spec.pin, led_1.gpio_pin_name);		
		return;
	}
	if (!device_is_ready(led_2.spec.port)) {
		printk("Error: %s device is not ready\n", led_2.spec.port->name);
		return;
	}	
	ret = gpio_pin_configure_dt(&led_2.spec, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (LED '%s')\n",
			ret, led_2.spec.pin, led_2.gpio_pin_name);		
		return;
	}

	if (!device_is_ready(sw_0.spec.port)) {
		printk("Error: %s device is not ready\n", sw_0.spec.port->name);
		return;
	}
	ret = gpio_pin_configure_dt(&sw_0.spec, GPIO_INPUT);
	if (ret < 0) {
		printk("Error %d: failed to configure pin %d (SW '%s')\n",
			ret, sw_0.spec.pin, sw_0.gpio_pin_name);			
		return;
	}	
	ret = gpio_pin_interrupt_configure_dt(&sw_0.spec,
					      GPIO_INT_EDGE_TO_ACTIVE);						  
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, sw_0.spec.port->name, sw_0.spec.pin);
		return;
	}	
	gpio_init_callback(&button_cb_data, button_pressed, BIT(sw_0.spec.pin));
	gpio_add_callback(sw_0.spec.port, &button_cb_data);
	printk("Set up button at %s pin %d\n", sw_0.spec.port->name, sw_0.spec.pin);

	int task_wdt_id = task_wdt_add(1100U, task_wdt_callback, (void *)k_current_get() );

	while (1) {
		while( gpio_pin_get(sw_0.spec.port, sw_0.spec.pin) )
		{
			
		}
		task_wdt_feed(task_wdt_id);		
		k_msleep(SLEEP_TIME_MS);
	}
}

	