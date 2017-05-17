// Device code for the 8259 PIC
#include <arch/i386/io.h>
#include <arch/i386/pic.h>
#include <kprint.h>
#include <stdint.h>

// Map the PICs to IDT offsets
void pic_enable(unsigned int master_offset, unsigned int slave_offset) {
    // Hold onto current masks
    uint8_t master_mask = inportb(PIC_DATA_1);
    uint8_t slave_mask = inportb(PIC_DATA_2);

    // Begin initialization
    outportb(PIC_CMD_1, PIC_INIT);
    io_wait();
    outportb(PIC_CMD_2, PIC_INIT);
    io_wait();


    // Load new offsets
    outportb(PIC_DATA_1, master_offset);
    io_wait();
    outportb(PIC_DATA_2, slave_offset);
    io_wait();

    // Set slave cascading up
    outportb(PIC_DATA_1, 4);
    io_wait();
    outportb(PIC_DATA_2, 2);
    io_wait();

    // Put both PICs in 8086 mode
    outportb(PIC_DATA_1, 0x01);
    io_wait();
    outportb(PIC_DATA_2, 0x01);
    io_wait();

    // Restore masks
    outportb(PIC_DATA_1, master_mask);
    io_wait();
    outportb(PIC_DATA_2, slave_mask);
    io_wait();

    kprintf("pic enabled\n");
}

// Disable the PIC
void pic_disable() {
    outportb(PIC_DATA_1, PIC_DISABLE);
    outportb(PIC_DATA_2, PIC_DISABLE);
}

// Send End-Of-Interrupt command
void pic_eoi(uint8_t irq) {
    if ( irq >= 8 ) {
        outportb(PIC_CMD_2, PIC_EOI);
    }

    outportb(PIC_CMD_1, PIC_EOI);
    io_wait();

    kprintf("pic eoi sent\n");
}

// Return the I
uint16_t pic_get_irr() {
    outportb(PIC_CMD_1, PIC_READ_IRR);
    outportb(PIC_CMD_2, PIC_READ_IRR);
    io_wait();
    return (inportb(PIC_CMD_2) << 8) | inportb(PIC_CMD_1);
}

uint16_t pic_get_isr() {
    outportb(PIC_CMD_1, PIC_READ_ISR);
    outportb(PIC_CMD_2, PIC_READ_ISR);
    io_wait();
    return (inportb(PIC_CMD_2) << 8) | inportb(PIC_CMD_1);
}

// Mask a certain interrupt on either master or slave
void pic_mask(uint8_t irq) {
    if ( irq >= 8 ) {
        irq -= 8;

        outportb(PIC_DATA_2, inportb(PIC_DATA_2) | (1 << irq));
    }
    else {
        outportb(PIC_DATA_1, inportb(PIC_DATA_1) | (1 << irq));
    }
}

// Unmask a certain interrupt on either master or slave
void pic_unmask(uint8_t irq) {
    if ( irq >= 8 ) {
        irq -= 8;

        outportb(PIC_DATA_2, inportb(PIC_DATA_2) & ~(1 << irq));
    }
    else {
        outportb(PIC_DATA_1, inportb(PIC_DATA_1) & ~(1 << irq));
    }
}
