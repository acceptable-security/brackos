#include <arch/i386/apic.h>
#include <arch/i386/pic.h>
#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/io.h>

#include <kprint.h>
#include <stdbool.h>
#include <stdlib.h>

extern void irq_common_stub();

irq_regs_t* irq_current_regs;
bool irq_happening = false;

irq_handler_t* irq_handlers[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

void irq_send_eoi(uint16_t irq) {
    // Send appropriate EOI
    if ( apic_supported() && apic_is_enabled() ) {
        apic_eoi();
    }
    else {
        pic_eoi(irq);
    }
}

// Generic handler to pass control to the installed interrupt service routines.
void irq_general_handler(irq_regs_t* frame) {
    // Set the global variables for support functions
    irq_current_regs = frame;
    irq_happening = true;

    // Ignore the cascading bit
    uint16_t isr = pic_get_isr() & ~(1 << 2);
    int irq = __builtin_ctz(isr);

    irq_handler_t* handler = irq_handlers[irq];

    // Make sure it is actually installed.
    if ( handler ) {
        handler(frame);
    }

    // Send EOI
    irq_send_eoi(irq);

    // Remove the global variables for support functions
    irq_happening = false;
    irq_current_regs = NULL;
}

// Register an IRQ handler
void irq_register(uint8_t num, irq_handler_t* handler) {
    irq_handlers[num] = handler;
}

// Returns whether or not an IRQ is currently being serviced.
bool irq_is_happening() {
    return irq_happening;
}

// Returns the current stack from the IRQ if one is happening
irq_regs_t* irq_get_current_regs() {
    return irq_current_regs;
}

// Initializes 16 empty IRQs
void irq_init() {
    for ( int i = 0; i < 16; i++ ) {
        idt_set_gate(i + 0x20, (uintptr_t) irq_common_stub, 0x08, 0x8E);
    }

    kprintf("irqs setup\n");
}
