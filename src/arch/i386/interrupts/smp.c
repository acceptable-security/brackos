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
uint32_t ap_stack_size = 0x4000;

// Virtual address of AP Trampoline code
void* trampoline_virt;

volatile uint32_t cpu_init_bitmap = 0;

// TODO - Not this.
void smp_wait() {
    int a = 0;
    for ( int i = 0; i < 1000000; i++ ) { a++; }
}

void ap_main() {
    cpu_init_bitmap |= (1 << lapic_get_id());

    // Validate that the stack was loaded correctly
    uintptr_t stack = 0;
    __asm__ volatile ("movl %%ebp, %0" : "=r" (stack) );

    if ( stack != ap_stack_list[lapic_get_id()] ) {
        kprintf("smp: failed to properly set stack\n");
        kprintf("smp: expected %p got %p\n", ap_stack_list[lapic_get_id()], stack);
        while(1){}
    }

    kprintf("smp: hello from cpu %d\n", lapic_get_id());
    while(1) {}
}

// Allocate the AP stacks
bool smp_alloc_stack() {
    kprintf("smp: allocating stacks of size %d...\n", ap_stack_size);

    ap_stack_list = kmalloc(sizeof(void*) * cpu_count);

    if ( ap_stack_list == NULL ) {
        kprintf("smp: failed to allocate smp stack list\n");
        return false;
    }

    for ( int i = 0; i < cpu_count; i++ ) {
        if ( i == lapic_get_id() ) {
            ap_stack_list[i] = 0;
        }
        else {
            ap_stack_list[i] = (uintptr_t) memmap(NULL, ap_stack_size, MMAP_URGENT);

            if ( ap_stack_list[i] == 0 ) {
                kprintf("smp: failed to alloc stack for cpu #%d\n", i);
                kfree(ap_stack_list);
                // TODO - free other stacks

                return false;
            }

            kprintf("smp: cpu #%d stack at %p\n", i, ap_stack_list[i]);
        }
    }

    return true;
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
    // Ignore uniprocessor setups
    if ( cpu_count == 1 ) {
        kprintf("smp: only 1 cpu found\n");
        return;
    }

    kprintf("smp: enabling...\n");

    // Allocate/copy AP boot trampoline to 0x7000
    if ( !smp_setup_trampoline() ) {
        return;
    }

    // Allocate 16KB stacks for each AP
    if ( !smp_alloc_stack() ) {
        return;
    }

    kprintf("smp: trampoline setup...\n");

    // BSP is already booted.
    cpu_init_bitmap |= (1 << lapic_get_id());

    for ( int i = 0; i < cpu_count; i++ ) {
        // Ignore BSP
        if ( i == lapic_get_id() ) {
            continue;
        }

        // Send INIT IPI
        kprintf("smp: sending init to cpu %d\n", i);
        lapic_send_init(i);

        smp_wait();

        // Send STARTUP IPI
        kprintf("smp: sending startup to cpu %d\n", i);
        lapic_send_startup(i, 7);

        /// Wait for boot flag to be set
        kprintf("smp: waiting on cpu %d\n", i);
        while ( !(cpu_init_bitmap & (1 << i)) );

        // TODO - Set timeout for faulty processors to be ignored
    }

    kprintf("smp: all cpus are awake\n");
    // smp_destroy_trampoline();
}
