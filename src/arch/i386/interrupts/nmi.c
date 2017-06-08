#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <kprint.h>
#include <stdint.h>
#include <stdlib.h>

const char* nmi_strs[] = {
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

void nmi_handle(irq_regs_t* frame, int error) {
    kprintf("NMI: %s @ %p\n", nmi_strs[error], frame);

    if ( frame != NULL ) {
        irq_regs_print(frame);
    }

    __asm__ ("cli; hlt");
}

void nmi_init() {
    for ( int i = 0; i < 19; i++ ) {
        idt_set_gate(i, (uintptr_t) nmi_handle, 0x08, 0x8E);
    }

    kprintf("nmi setup\n");
}
