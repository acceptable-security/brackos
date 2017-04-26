#include <stdint.h>

// Ports for internal use
#define PIC_CMD_1  0x20
#define PIC_CMD_2  0xA0
#define PIC_DATA_1 0x21
#define PIC_DATA_2 0xA1

// Commands
#define PIC_EOI     0x20 // End of interrupt
#define PIC_INIT    0x11 // Enable in cascading mode
#define PIC_DISABLE 0xFF // Disable PIC


void pic_enable(unsigned int master_offset, unsigned int slave_offset);
void pic_disable();
void pic_eoi(uint8_t irq);
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);
