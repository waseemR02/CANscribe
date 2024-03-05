/*
* Source file for CANscribe
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <app_version.h>

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

struct canscribe_msg {
	struct can_frame frame;
	uint32_t crc;
};

struct can_frame tx_frame;

// #define CRC_CHECKED_SIZE sizeof(frame)

// crc32_ieee(frame, 122);


/*
 * Deserialized with cobs
 * Returns 0 on succes else -1
 */
void deserialize(uint8_t *message, struct canscribe_msg *msg) {

}
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
		struct canscribe_msg msg;

		if (c == 0x00 && rx_buf_pos > 0) {
			// uart_fifo_read(uart_dev, &c, 1);
			/* terminate the message with 0x00 */
			rx_buf[rx_buf_pos] = 0x00;

			deserialize(rx_buf, &msg);

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &msg, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Sends raw bytes to uart_dev
 */
void send_to_uart(uint8_t *buf, uint8_t len) {
	for (int i = 0; i < len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}


/*
 * Checks if the message is valid
 * 
 */
bool valid_crc(struct canscribe_msg *msg) {
	return true;
}


/*
 * This thread recieves uart messages and parses them to send to can bus
 */
void rx_thread(void *unused1, void *unused2, void *unused3) {
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	int err;
	struct canscribe_msg temp_msg;

	while(1) {
		if(k_msgq_get(&uart_msgq, &temp_msg, K_FOREVER)) {
			if(!valid_crc(&temp_msg)) // WARNING: Not sure if it will work
				continue;
			tx_frame = temp_msg.frame;
		}
	}
}


int main() {
	printk("\nCANscribe: v%s\n\n", APP_VERSION_STRING);

}
