/*
* Source file for CANscribe
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <string.h>

LOG_MODULE_REGISTER(can_read_test , CONFIG_LOG_DEFAULT_LEVEL);

#define MSG_SIZE 100

/* queue to store up to 10 messages (aligned to 1-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 1);
CAN_MSGQ_DEFINE(rx_msgq, 10);

/* DT spec for uart */
static const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

/* DT spec for can module*/
const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

/* DT spec for led*/
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* receive buffer used in UART ISR callback */
static uint8_t rx_buf[MSG_SIZE];
static int rx_buf_pos;


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

	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if (c == 0x00 && rx_buf_pos > 0) {
			uart_fifo_read(uart_dev, &c, 1);
			/* terminate the message with 0x00 */
			rx_buf[rx_buf_pos] = 0x00;

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


