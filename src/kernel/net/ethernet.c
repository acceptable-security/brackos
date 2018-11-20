#include <stdint.h>

typedef struct {
	uint8_t preamble[7];
	uint8_t start_frame;
	uint8_t dest[6];
	uint8_t src[6];
	uint16_t type_len;
} __attribute__((packed)) eth_frame_t;	