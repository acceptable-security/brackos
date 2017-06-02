#include <arch/i386/irq.h>

#include <drivers/ps2.h>
#include <drivers/scancodes.h>

#include <kprint.h>

#include <stdint.h>
#include <stdbool.h>

// TODO - make this work

// Called whenever PS/2 keyboards throw an interrupt
void ps2_kb_interrupt(irq_regs_t* frame) {
    uint8_t scancode = ps2_read_data();
    bool down = !(scancode & 0x80);
    scancode = scancode & ~0x80;

    uint8_t ascii = kb_us[scancode];

    if ( ascii > 0x7F ) {
        // TODO
    }
    else {
        // TODO
        if ( down ) kprintf("%c", ascii);
    }

    ps2_flush_buffer();
}

bool ps2_kb_setup(uint8_t port) {
    irq_register(1, ps2_kb_interrupt);
    return true;
}
