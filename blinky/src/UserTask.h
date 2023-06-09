/**
 *****************************************************************************************
 *	@file BLR_IOT_Manager.h
 *
 *	@brief This file contains declarations of BLR IOT manager
 *
 *	@version C++ Style Guide Version 1.0
 *
 *	@copyright 2022 Eaton Corporation. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef __USER_TASK_H__
#define __USER_TASK_H__
#include <zephyr/zephyr.h>
#include <zephyr/drivers/gpio.h>

struct led {
	struct gpio_dt_spec spec;
	const char *gpio_pin_name;
};

struct sw {
	struct gpio_dt_spec spec;
	const char *gpio_pin_name;
};

void task_wdt_callback(int channel_id, void *user_data);

#endif  // __USER_TASK_H__
