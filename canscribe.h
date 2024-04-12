#include <zephyr/drivers/can.h>

struct canscribe_msg {
	struct can_frame frame;
	uint32_t crc;
};

void serialize(uint8_t *buf, uint8_t *msg, int len);
void deserialize(uint8_t *buf, uint8_t *msg, int len);
