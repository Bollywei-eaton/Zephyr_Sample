/**
 *****************************************************************************************
 * @file
 * @details See header file for module overview.
 * @copyright 2022 Eaton Corporation. All Rights Reserved.
 *
 *****************************************************************************************
 */

#include "UserTask.h"
#include <zephyr/drivers/watchdog.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/task_wdt/task_wdt.h>

/* size of stack area used by each thread */
#define BLINK_STACKSIZE 512

/* scheduling priority used by each thread */
#define BLINK_PRIORITY 7
#define BLINK_TIME_MS   500


#define SOFT_TIMER_STACKSIZE 512
#define SOFT_TIMER_PRIORITY 7
static struct k_thread soft_timer_thread;

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
static const struct led led_0 = {
	.spec = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0}),
	.gpio_pin_name = DT_PROP_OR(LED0_NODE, label, ""),
};

struct k_sem key_sem;
extern const struct sw sw_0;

void soft_timer(void)
{
	int ret;

	printk("soft_timer task is running\n");
	k_sem_init(&key_sem, 0, 1);

	while (1) {
		if (k_sem_take(&key_sem, K_FOREVER) == 0) {
			printk("Get key_sem\n");
		}		
		k_msleep(250);
		ret = gpio_pin_interrupt_configure_dt(&sw_0.spec,
							GPIO_INT_EDGE_TO_ACTIVE);						  
		if (ret != 0) {
			printk("Error %d: failed to configure interrupt on %s pin %d\n",
				ret, sw_0.spec.port->name, sw_0.spec.pin);
			return;
		}			
	}
}


void task_wdt_callback(int channel_id, void *user_data)
{
	printk("Task watchdog channel %d callback, thread: %s\n",
		channel_id, k_thread_name_get((k_tid_t)user_data));

	/*
	 * If the issue could be resolved, call task_wdt_feed(channel_id) here
	 * to continue operation.
	 *
	 * Otherwise we can perform some cleanup and reset the device.
	 */

	printk("Resetting device...\n");

	sys_reboot(SYS_REBOOT_COLD);
}

K_THREAD_STACK_DEFINE(soft_timer_event_stack, SOFT_TIMER_STACKSIZE);
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
	k_tid_t tid = k_thread_create(&soft_timer_thread, soft_timer_event_stack,
			SOFT_TIMER_STACKSIZE,
			(k_thread_entry_t)soft_timer, NULL, NULL, NULL,
			K_PRIO_COOP(SOFT_TIMER_PRIORITY),
			0, K_NO_WAIT);	
	k_thread_name_set(tid, "soft_timer");
	k_thread_start(&soft_timer_thread);
	/*
	 * Add a new task watchdog channel with custom callback function and
	 * the current thread ID as user data.
	 */
	int task_wdt_id = task_wdt_add(1100U, task_wdt_callback,
		(void *)k_current_get());			
	while (1) {
		gpio_pin_toggle(led_0.spec.port, led_0.spec.pin);
		task_wdt_feed(task_wdt_id);
		k_msleep(BLINK_TIME_MS);
	}
}

// K_THREAD_DEFINE(blink_id, BLINK_STACKSIZE, blink, NULL, NULL, NULL,
// 		BLINK_PRIORITY, 0, 2000);  //Start running task after 2000ms
K_THREAD_DEFINE(blink_id, BLINK_STACKSIZE, blink, NULL, NULL, NULL,
		BLINK_PRIORITY, 0, 0);			