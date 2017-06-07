#include <arch/i386/idt.h>
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
        kprintf("EAX: %X\n", frame->eax);
        kprintf("EBX: %X\n", frame->ebx);
        kprintf("ECX: %X\n", frame->ecx);
        kprintf("EDX: %X\n", frame->edx);
        kprintf("ESi: %X\n", frame->esi);
        kprintf("EDI: %X\n", frame->edi);
        kprintf("\nStack: %X:%X\n", frame->ebp, frame->esp);
        kprintf("EIP: %X\n", frame->eip);
    }

    __asm__ ("cli; hlt");
}

void nmi_init() {
    for ( int i = 0; i < 19; i++ ) {
        idt_set_gate(i, (uintptr_t) nmi_handle, 0x08, 0x8E);
    }

    kprintf("nmi setup\n");
}
