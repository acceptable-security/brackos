#include <arch/i386/idt.h>
#include <arch/i386/apic.h>
#include <arch/i386/irq.h>
#include <kprint.h>
#include <stdint.h>
#include <stdlib.h>

const char* exception_strs[] = {
    "Dividing by Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Uknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check"
};

void exception_handle(irq_regs_t* frame, int error) {
    if ( error > 19 ) {
        kprintf("exceptions: invalid (%d) @ %p\n", error, frame);
    }
    else {
        kprintf("exceptions: %s @ %p\n", exception_strs[error], frame);
    }

    kprintf("cpu %d\n", lapic_get_id());

    if ( frame != NULL ) {
        irq_regs_print(frame);
    }

    __asm__ ("cli; hlt");
}

void exception_init() {
    for ( int i = 0; i < 19; i++ ) {
        idt_set_gate(i, (uintptr_t) exception_handle, 0x08, 0x8E);
    }

    kprintf("exceptions: setup complete\n");
}
