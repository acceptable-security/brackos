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
uintptr_t* ap_stack_list;
extern uint32_t stack_size;

// Virtual address of AP Trampoline code
void* trampoline_virt;

volatile uint32_t cpu_ready = 0;

void ap_main() {
    cpu_ready++;

    uintptr_t stack = 0;
    __asm__ volatile ("movl %%ebp, %0" : "=r" (stack) );

    if ( stack != ap_stack_list[lapic_get_id()] ) {
        kprintf("smp: failed to properly set stack\n");
        kprintf("smp: expected %p got %p\n", ap_stack_list[lapic_get_id()], stack);
        while(1){}
    }

    kprintf("smp: hello from cpu %d, booted number %d\n", lapic_get_id(), cpu_ready);
    kprintf("smp: stack at %p\n", stack);
    while(1) {}
}

// Allocate the AP stacks
void smp_alloc_stack() {
    kprintf("smp: allocating stacks of size %d...\n", stack_size);

    ap_stack_list = kmalloc(sizeof(void*) * cpu_count);

    for ( int i = 0; i < cpu_count; i++ ) {
        if ( i == lapic_get_id() ) {
            ap_stack_list[i] = 0;
        }
        else {
            ap_stack_list[i] = (uintptr_t) memmap(NULL, stack_size, MMAP_URGENT);
            kprintf("cpu #%d stack at %p\n", i, ap_stack_list[i]);
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
    trampoline_virt = vasa_alloc(MEM_PCI, 4096, 0);

    if ( trampoline_virt == NULL ) {
        kprintf("smp: failed to allocate virtual address for trampoline.\n");
        return false;
    }

    // Map it
    if ( !paging_map(ap_trampoline, (uintptr_t) trampoline_virt, PAGE_PRESENT | PAGE_RW) ) {
        return false;
    }

    // Copy the code
    memcpy(trampoline_virt, (void*) start, size);
    return true;
}

// Deallocate the ap trampoline after all APs are initialized
void smp_destroy_trampoline() {
    if ( trampoline_virt != NULL ) {
        // Remove allocations and null pointer
        paging_unmap((uintptr_t) trampoline_virt);
        vasa_dealloc(trampoline_virt);
        trampoline_virt = NULL;

        kprintf("smp: trampoline removed.\n");
    }
}

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

    smp_destroy_trampoline();
}
