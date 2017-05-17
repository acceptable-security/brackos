#include <arch/i386/idt.h>
#include <kprint.h>
#include <stdint.h>

void nmi_handle(idt_reg_stack_t* frame, uint32_t error) {
    kprintf("ohno! %d\n", error);
    __asm__ ("cli; hlt");
}

void nmi_init() {
    for ( int i = 0; i < 19; i++ ) {
        idt_set_gate(i, (uintptr_t) nmi_handle, 0x08, 0x8E);
    }

    kprintf("nmi installed\n");
}
