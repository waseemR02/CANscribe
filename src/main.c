/*
* Source file for CANscribe
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/crc.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>
#include <app_version.h>

#include <canscribe/lib/canscribe.h>

LOG_MODULE_REGISTER(can_read_test , CONFIG_LOG_DEFAULT_LEVEL);

/* msg size is in correspondence with cobs serialization */
#define UART_MSG_SIZE (sizeof(struct canscribe_msg) + 2)
#define UART_CAN_THREAD_STACK_SIZE 512
#define UART_CAN_THREAD_PRIORITY 2

/* queue to store up to 10 messages (aligned to 1-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, sizeof(struct canscribe_msg), 10, 1);
CAN_MSGQ_DEFINE(can_msgq, 10);

/* Define stack size for uart to can thread */
K_THREAD_STACK_DEFINE(uart_can_thread_stack, UART_CAN_THREAD_STACK_SIZE);

/* DT spec for uart */
static const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

/* DT spec for can module*/
const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

/* DT spec for led*/
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

/* receive buffer used in UART ISR callback */
static uint8_t rx_buf[100];
static int rx_buf_pos;
static uint8_t tx_buf[UART_MSG_SIZE];


struct can_frame uart_can_frame; /* uart --> can */
struct can_frame can_uart_frame; /* can --> uart */

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

		if (c == 0 && rx_buf_pos > 0) {
			// uart_fifo_read(uart_dev, &c, 1);
			/* terminate the message with 0x00 */
			rx_buf[rx_buf_pos] = 0;

			deserialize(rx_buf, (uint8_t *)&msg, sizeof(msg));

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
 * Validates against crc
 */ 
bool valid_crc(struct canscribe_msg *msg){
	uint32_t rcd_crc = msg->crc;
	uint32_t comp_crc = crc32_ieee((uint8_t *)&msg, sizeof(struct can_frame));

	if (rcd_crc != comp_crc) 
		return false;
	
	return true;
}


/*
 * This thread recieves uart messages and parses them to send to can bus
 */

struct k_thread uart_can_thread_data;

void uart_can_thread(void *unused1, void *unused2, void *unused3) {
	ARG_UNUSED(unused1);
	ARG_UNUSED(unused2);
	ARG_UNUSED(unused3);

	int err;
  ARG_UNUSED(err);
	struct canscribe_msg temp_msg;

	while(1) {
		if(k_msgq_get(&uart_msgq, &temp_msg, K_FOREVER)) {
			if(!valid_crc(&temp_msg)) // WARNING: Not sure if it will work
				continue;
			uart_can_frame = temp_msg.frame;
			can_send(can_dev, &uart_can_frame, K_FOREVER, NULL, NULL);
			LOG_INF("CAN frame sent: ID: %x", uart_can_frame.id);
			LOG_INF("CAN frame data: %d %d %d", uart_can_frame.data_32[0],
								uart_can_frame.data[4],
								uart_can_frame.data[5]);
			gpio_pin_toggle_dt(&led);
		}
	}
}

/*
 * Can filter to read all frames with extended IDs
 */ 
const struct can_filter can_uart_filter = {
	.flags = CAN_FILTER_IDE,
};

/*
 * The main function spawns the uart_to_can_thread and handles can_to_uart interface
 */
int main() {
	printk("\nCANscribe: v%s\n\n", APP_VERSION_STRING);

	int err;

	/* Device ready checks*/

	if (!device_is_ready(can_dev)) {
		LOG_ERR("CAN: Device %s not ready.", can_dev->name);
		return 0;
	}

	if (!device_is_ready(uart_dev)) {
		LOG_ERR("UART device not found!");
		return 0;
	}

	if (!gpio_is_ready_dt(&led))
	{
		LOG_ERR("Error: Led not ready.");
		return 0;
	}

	/* Configure devices */
	if (can_start(can_dev)) {
		LOG_ERR("Error starting CAN controller.");
		return 0;
	}

	int filter_id = can_add_rx_filter_msgq(can_dev, &can_msgq, &can_uart_filter);
	if (filter_id < 0)
	{
		LOG_ERR("Unable to add can msgq [%d]", filter_id);
		return 0;
	}

	err = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

	if (err < 0) {
		if (err == -ENOTSUP) {
			LOG_ERR("Interrupt-driven UART API support not enabled");
		} else if (err == -ENOSYS) {
			LOG_ERR("UART device does not support interrupt-driven API");
		} else {
			LOG_ERR("Error setting UART callback: %d", err);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev);

	if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0)
	{
		LOG_ERR("Error: Led not configured");
		return 0;
	}


	k_tid_t tx_tid = k_thread_create(&uart_can_thread_data, uart_can_thread_stack,
				 K_THREAD_STACK_SIZEOF(uart_can_thread_stack),
				 uart_can_thread, NULL, NULL, NULL,
				 UART_CAN_THREAD_PRIORITY, 0, K_NO_WAIT);

	if (!tx_tid) {
		LOG_ERR("ERROR spawning uart_to_can thread");
	}

	LOG_INF("Initialization completed successfully");

	/* Handles can to uart interface */
	struct canscribe_msg can_uart_msg;
	while(true) {
		/* Blocking Function */
		k_msgq_get(&can_msgq, &can_uart_frame, K_FOREVER);
		
		can_uart_msg.frame = can_uart_frame;
		can_uart_msg.crc = crc32_ieee((uint8_t *)&can_uart_frame, sizeof(struct can_frame));

		serialize(tx_buf, (uint8_t *)&can_uart_msg, sizeof(struct canscribe_msg));

		send_to_uart(tx_buf, UART_MSG_SIZE); // Look into how you are going to handle the size
	}
}
