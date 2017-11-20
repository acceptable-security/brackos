#include <arch/i386/apic.h>
#include <arch/i386/paging.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mem/vasa.h>
#include <kprint.h>

extern uintptr_t ap_boot_init;
extern uintptr_t ap_boot_end;

bool smp_setup_trampoline() {
    uint32_t start = (uint32_t) &ap_boot_init;
    uint32_t end = (uint32_t) &ap_boot_end;

    uint32_t size = end - start;

    if ( size > 0x1000 ) {
        kprintf("ap trampoline over a page (%x). skipping.\n", size);
        return false;
    }

    // Located in the 8th page
    uintptr_t ap_trampoline = 0x8000;

    // Get a virtual address for it
    void* virt = vasa_alloc(MEM_PCI, 4096, 0);

    if ( virt == NULL ) {
        kprintf("failed to allocate virtual address for trampoline.\n");
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
    kprintf("enabling smp...\n");

    if ( !smp_setup_trampoline() ) {
        return;
    }

    kprintf("smp trampoline setup...\n");
}
