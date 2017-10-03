#include <arch/i386/apic.h>
#include <arch/i386/pic.h>
#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/io.h>

#include <kprint.h>
#include <stdbool.h>
#include <stdlib.h>

extern void irq_common_stub();
extern void irq_empty_stub();

irq_regs_t* irq_current_regs;
bool irq_happening = false;

irq_handler_t* irq_handlers[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

void irq_send_eoi(uint16_t irq) {
    // Send appropriate EOI
    if ( lapic_is_enabled() ) {
        lapic_eoi();
    }
    else {
        pic_eoi(irq);
    }
}

int irq_get_current() {
    if ( lapic_is_enabled() ) {
        uint16_t isr = pic_get_isr() & ~(1 << 2);
        return __builtin_ctz(isr);
    }
    else {
        return lapic_inservice_routine();
    }
}

// Generic handler to pass control to the installed interrupt service routines.
void irq_general_handler(irq_regs_t* frame) {
    // Set the global variables for support functions
    irq_current_regs = frame;
    irq_happening = true;

    int irq = irq_get_current();

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

// Print the registers
void irq_regs_print(irq_regs_t* regs) {
    kprintf("EAX: %X\n", regs->eax);
    kprintf("EBX: %X\n", regs->ebx);
    kprintf("ECX: %X\n", regs->ecx);
    kprintf("EDX: %X\n", regs->edx);
    kprintf("ESi: %X\n", regs->esi);
    kprintf("EDI: %X\n", regs->edi);
    kprintf("\nStack: %X:%X\n", regs->ebp, regs->esp);
    kprintf("EIP: %X\n", regs->eip);
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
    // Setup the 16 common IRQs
    for ( int i = 0; i < 16; i++ ) {
        idt_set_gate(i + 0x20, (uintptr_t) irq_common_stub, 0x08, 0x8E);
    }

    kprintf("irqs setup\n");
}
