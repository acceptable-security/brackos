#include <arch/i386/idt.h>
#include <stdint.h>

idt_t idt;

void idt_empty_entry() {
    __asm__ volatile("iretl");
}

// Set a gate in the IDT
void idt_set_gate(unsigned int gate, uintptr_t address, uint16_t selector, uint8_t gate_type) {
    if ( gate >= IDT_GATE_COUNT ) {
        return;
    }

    // Figure out the storage segment
    uint8_t storage_segment = 1;

    if ( gate_type == IDT_GATE_32_INTERRUPT || gate_type == IDT_GATE_32_TRAP ) {
        storage_segment = 0;
    }

    // Store result
    idt.gates[gate] = (idt_gate_t) {
        .offset_low = gate & 0xFFFF,
        .selector = selector,
        .unused = 0,
        .gate_type = gate_type,
        .storage_segment = storage_segment,
        .descriptor_priv = 0,
        .present = 1,
        .offset_high = (gate >> 16) & 0xFFFF
    };
}

// Load the IDT
void idt_load() {
    __asm__ ( "lidt (%0)" : : "m"(idt) );
}

// Create an initial empty IDT
void idt_init() {
    for ( int i = 0; i < IDT_GATE_COUNT; i++ ) {
        idt_set_gate(i, (uintptr_t) idt_empty_entry, 0x08, IDT_GATE_32_INTERRUPT);
    }
}
