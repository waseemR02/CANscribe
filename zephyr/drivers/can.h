#include <stdint.h>
#define CAN_MAX_DLEN    8U
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

struct can_frame {
	uint32_t id  : 29;
	uint8_t res0 : 3; /* reserved/padding. */
	uint8_t dlc;
	uint8_t flags;
	uint16_t res1;  /* reserved/padding. */
	union {
		uint8_t data[CAN_MAX_DLEN];
		uint32_t data_32[DIV_ROUND_UP(CAN_MAX_DLEN, sizeof(uint32_t))];
	};
};
