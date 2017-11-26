#include <arch/i386/apic.h>
#include <arch/i386/paging.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mem/vasa.h>
#include <kprint.h>

extern uintptr_t ap_boot_init;
extern uintptr_t ap_boot_end;
extern uint32_t cpu_count;

volatile uint32_t cpu_ready = 0;

void ap_main() {
    cpu_ready++;
    kprintf("smp: hello from cpu %d, booted number %d\n", lapic_get_id(), cpu_ready);
}

bool smp_setup_trampoline() {
    uint32_t start = (uint32_t) &ap_boot_init;
    uint32_t end = (uint32_t) &ap_boot_end;

    uint32_t size = end - start;

    if ( size > 0x1000 ) {
        kprintf("smp: ap trampoline over a page (%x). skipping.\n", size);
        return false;
    }

    // Located in the 8th page
    uintptr_t ap_trampoline = 0x7000;

    // Get a virtual address for it
    void* virt = vasa_alloc(MEM_PCI, 4096, 0);

    if ( virt == NULL ) {
        kprintf("smp: failed to allocate virtual address for trampoline.\n");
        return false;
    }

    // Map it
    if ( !paging_map(ap_trampoline, (uintptr_t) virt, PAGE_PRESENT | PAGE_RW) ) {
        return false;
    }

    memcpy(virt, (void*) start, size);
    return true;
}

void smp_init() {
    kprintf("smp: enabling...\n");

    if ( !smp_setup_trampoline() ) {
        return;
    }

    kprintf("smp: trampoline setup...\n");

    for ( int i = 0; i < cpu_count; i++ ) {
        if ( i != lapic_get_id() ) {
            kprintf("smp: sending init to cpu %d\n", i);
            lapic_send_init(i);
        }
    }

    int a = 0;
    for ( int i = 0; i < 1000000; i++ ) { a++; }

    for ( int i = 0; i < cpu_count; i++ ) {
        if ( i != lapic_get_id() ) {
            kprintf("smp: sending startup to cpu %d\n", i);
            lapic_send_startup(i, 7);
        }
    }

    kprintf("smp: waiting on %d cpus\n", cpu_count - 1);
    while ( cpu_ready != (cpu_count - 1) ) {}
    kprintf("smp: all cpus are awake\n");
}
