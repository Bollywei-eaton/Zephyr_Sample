/**
 *****************************************************************************************
 * @file
 * @details See header file for module overview.
 * @copyright 2022 Eaton Corporation. All Rights Reserved.
 *
 *****************************************************************************************
 */
#include <zephyr/zephyr.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include "Comm.h"


/* size of stack area used by each thread */
#define COMM_STACKSIZE 1024

/* scheduling priority used by each thread */
#define COMM_PRIORITY 6

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_usr_uart)

#define MSG_SIZE 32

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

extern struct k_thread _k_thread_obj_blink_id;
/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	while (uart_irq_rx_ready(uart_dev)) {

		uart_fifo_read(uart_dev, &c, 1);

		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

void comm(void)
{
	char tx_buf[MSG_SIZE];

	printk("comm task is running\n");

	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return;
	}

	/* configure interrupt and callback to receive data */
	uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	uart_irq_rx_enable(uart_dev);

	print_uart("Hello! I'm your echo bot.\r\n");
	print_uart("Tell me something and press enter:\r\n");

	/* indefinitely wait for input from the user */
	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
		if( memcmp(tx_buf, "stop", 4) == 0 )
		{
			k_thread_suspend( &_k_thread_obj_blink_id );
			printk("Abort blink task\n");
		}
		else if( memcmp(tx_buf, "start", 5) == 0 )
		{
			k_thread_resume( &_k_thread_obj_blink_id );
			printk("Resume blink task\n");
		}		
		else
		{
			print_uart("Echo: ");
			print_uart(tx_buf);
			print_uart("\r\n");
		}
	}
}

K_THREAD_DEFINE(comm_id, COMM_STACKSIZE, comm, NULL, NULL, NULL,
		COMM_PRIORITY, 0, 0);	