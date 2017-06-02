#include <arch/i386/gdt.h>
#include <arch/i386/tss.h>
#include <stdlib.h>
#include <stdint.h>
#include <kprint.h>

extern gdt_desc_t our_gdt;

// Declared in arch/i386/gdt.asm
extern void (tss_set)(uint32_t tss);

// 4KiB alligned Task State Segment
tss_t tss __attribute__((aligned(4096)));

// Prints a list of the gdt entries.
// This was in here to debug the TSS. Left here in case it's needed later.
void gdt_print() {
    unsigned int entries = (our_gdt.size + 1) / sizeof(gdt_entry_t);

    gdt_entry_t* entries_start = our_gdt.entries;

    for ( int i = 0; i < entries; i++ ) {
        gdt_entry_t* entry = (entries_start + i);
        uint32_t address = (entry->base_high << 24) | (entry->base_middle << 16) | entry->base_low;

        kprintf("gdt[0x%x]: %p\n", i * sizeof(gdt_entry_t), address);
    }
}

// Set the stack pointer and stack segment in the TSS
void tss_update(uint32_t esp0, uint16_t ss0) {
    tss.esp0 = esp0;
    tss.ss0 = ss0;
}

// Initialize the TSS
void tss_init() {
    tss_update(0, 0);
    tss_set((uint32_t) &tss);

    kprintf("tss setup\n");
}
