#include <arch/i386/apic.h>
#include <arch/i386/paging.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <mem/mmap.h>
#include <mem/vasa.h>
#include <kprint.h>

// Location of the ap trampoline code
extern uintptr_t ap_boot_init;
extern uintptr_t ap_boot_end;

extern uint32_t cpu_count;

// Used for allocating stacks for each AP
void** ap_stacks;
extern uint32_t stack_size;

volatile uint32_t cpu_ready = 0;

void ap_main() {
    void* test = NULL;
    cpu_ready++;
    kprintf("smp: hello from cpu %d, booted number %d\n", lapic_get_id(), cpu_ready);
    kprintf("smp: %x\n", (void*) &test);
    while(1) {}
}

// Allocate the AP stacks
void smp_alloc_stack() {
    kprintf("smp: allocating stacks...\n");

    ap_stacks = kmalloc(sizeof(void*) * cpu_count);

    for ( int i = 0; i < cpu_count; i++ ) {
        if ( i == lapic_get_id() ) {
            ap_stacks[i] = NULL;
        }
        else {
            ap_stacks[i] = memmap(NULL, stack_size, MMAP_URGENT);
        }
    }
}

bool smp_setup_trampoline() {
    // Get the start and end of the ap boot code
    uint32_t start = (uint32_t) &ap_boot_init;
    uint32_t end = (uint32_t) &ap_boot_end;

    // Calculate the size
    uint32_t size = end - start;

    // Make sure it's a single page
    if ( size > 0x1000 ) {
        kprintf("smp: ap trampoline over a page (%x). skipping.\n", size);
        return false;
    }

    // Located in the 7th page
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

    // Copy the code
    memcpy(virt, (void*) start, size);
    return true;
}

// TODO - deallocate the ap trampoline after all APs are initialized

void smp_init() {
    kprintf("smp: enabling...\n");

    if ( !smp_setup_trampoline() ) {
        return;
    }

    smp_alloc_stack();

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
