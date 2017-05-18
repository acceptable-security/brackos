#ifndef _PIT_H
#define _PIT_H

#include <stdint.h>

// Ports
#define PIT_DATA_CHAN_0 0x40
#define PIT_DATA_CHAN_1 0x41
#define PIT_DATA_CHAN_2 0x42
#define PIT_CMD_CHAN    0x43

// Binary mode options
#define PIT_MODE_BINARY 0 // 16 bit binary mode
#define PIT_MODE_BCD    1 // 4 digit BCD mode

// Operating mode options
#define PIT_OP_MODE_INT_ON_TERM 0 // Interrupt on terminal count
#define PIT_OP_MODE_ONE_SHOT    1 // Hardware re-triggerable one shot
#define PIT_OP_MODE_RATE_GEN    2 // Rate generator
#define PIT_OP_MODE_SQUARE_GEN  3 // Square wave generator
#define PIT_OP_MODE_SOFT_STROBE 4 // Software triggered strobe
#define PIT_OP_MODE_HARD_STROBE 5 // Hardware triggered strobe

// Access mode options
#define PIT_ACC_MODE_LATCH_COUNT 0 // Latch count value command
#define PIT_ACC_MODE_LOBYTE      1 // Read only the low byte
#define PIT_ACC_MODE_HIBYTE      2 // Read only the high byte
#define PIT_ACC_MODE_LOHIBYTE    3 // Read the low byte then the high byte.

// Channel selection and read-back options
#define PIT_SEL_CHAN_0  0 // Channel 0
#define PIT_SEL_CHAN_1  1 // Channel 1
#define PIT_SEL_CHAN_2  2 // Channel 2
#define PIT_SEL_READ    3 // Read back the command

#define PIT_BASE_FREQ 1193181

// Command packet for the PIT_CMD_CHAN.
typedef struct {
    uint8_t bin_mode : 1;
    uint8_t operating_mode : 3;
    uint8_t access_mode : 2;
    uint8_t select_channel : 2;
} __attribute((packed)) pit_cmd_t;

void pit_sleep(uint32_t ms);
void pit_init();

#endif
