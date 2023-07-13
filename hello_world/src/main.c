/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

/* size of stack area used by each thread */
#define TASK2_STACKSIZE 1024

/* scheduling priority used by each thread */
#define TASK2_PRIORITY 7

#define HELLO_WORLD_STACKSIZE 1024
static struct k_thread hello_world_thread;
#define SOFT_TIMER_PRIORITY 7

void hello_world(void)
{
	int num = 0;

	printk("hello_world task is running\n");

	while (1) {
		printk("hello world! %d\n", num++);
		k_msleep(1000);			
	}
}

K_THREAD_STACK_DEFINE(hello_world_event_stack, HELLO_WORLD_STACKSIZE);

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
	k_tid_t tid = k_thread_create(&hello_world_thread, hello_world_event_stack,
			HELLO_WORLD_STACKSIZE,
			(k_thread_entry_t)hello_world, NULL, NULL, NULL,
			K_PRIO_COOP(SOFT_TIMER_PRIORITY),
			0, K_NO_WAIT);	
	k_thread_name_set(tid, "soft_timer");		
	return 0;
}


void task2(void)
{
	printk("task2 is running\n");
	
	while (1) {
		printk("Heart beat!\n");
		k_msleep( 5000 );
	}
}

K_THREAD_DEFINE(task2_id, TASK2_STACKSIZE, task2, NULL, NULL, NULL,
		TASK2_PRIORITY, 0, 0);