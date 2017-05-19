#include <arch/i386/apic.h>
#include <arch/i386/pic.h>
#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/io.h>

#include <kprint.h>
#include <stdlib.h>

extern void irq_common_stub();

irq_handler_t* irq_handlers[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

void irq_send_eoi(uint16_t irq) {
    if ( apic_supported() && apic_is_enabled() ) {
        apic_eoi();
    }
    else {
        pic_eoi(irq);
    }
}

// Generic handler to pass control to the installed interrupt service routines.
void irq_general_handler(idt_reg_stack_t* frame) {
    // Ignore the cascading bit
    uint16_t isr = pic_get_isr() & ~(1 << 2);
    int irq = __builtin_ctz(isr);

    irq_handler_t* handler = irq_handlers[irq];

    // Make sure it is actually installed.
    if ( handler ) {
        handler(frame);
    }

    irq_send_eoi(irq);
}

void irq_register(uint8_t num, irq_handler_t* handler) {
    irq_handlers[num] = handler;
}

void irq_init() {
    for ( int i = 0; i < 16; i++ ) {
        idt_set_gate(i + 0x20, (uintptr_t) irq_common_stub, 0x08, 0x8E);
    }

    kprintf("irqs setup\n");
}
