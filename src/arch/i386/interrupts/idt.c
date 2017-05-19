#include <arch/i386/idt.h>
#include <stdint.h>
#include <kprint.h>

extern uintptr_t irq_common_stub;

idt_gate_t gates[IDT_GATE_COUNT];
idt_t idtp;

void idt_empty_entry() {
    __asm__ volatile("iretl");
}

// Set a gate in the IDT
void idt_set_gate(unsigned int gate, uintptr_t address, uint16_t selector, uint8_t flags) {
    if ( gate >= IDT_GATE_COUNT ) {
        return;
    }

    // Store result
    gates[gate] = (idt_gate_t) {
        .offset_low = address & 0xFFFF,
        .offset_high = (address >> 16) & 0xFFFF,

        .selector = selector,
        .flags = flags,
        .unused = 0
    };
}

// Load the IDT
void idt_load() {
    idtp.size = (sizeof(idt_gate_t) * IDT_GATE_COUNT) - 1;
    idtp.base = (uintptr_t) &gates;

    __asm__( "lidt %0" : : "g"(idtp) );
    kprintf("idt %p loaded\n", &idtp);
}

// Create an initial empty IDT
void idt_init() {
    for ( int i = 0; i < IDT_GATE_COUNT; i++ ) {
        // 0x8E == 32bit Interrupt Flag.
        // TODO - do actual flag ORs.
        idt_set_gate(i, (uintptr_t) idt_empty_entry, 0x08, 0x8E);
    }

    kprintf("idt setup\n");
}
