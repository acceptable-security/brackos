#include <arch/i386/gdt.h>
#include <arch/i386/map.h>
#include <arch/i386/paging.h>
#include <arch/i386/acpi.h>

#include <arch/i386/apic.h>
#include <arch/i386/pic.h>

#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/pit.h>
#include <kernel/clock.h>

#include <arch/i386/tss.h>
#include <arch/i386/task.h>

#include <drivers/ps2.h>
#include <drivers/rs232.h>
#include <drivers/vga.h>

#include <mem/frame.h>
#include <mem/slab.h>
#include <mem/early.h>
#include <mem/mmap.h>
#include <mem/vasa.h>

#include <multiboot.h>
#include <kprint.h>
#include <stdint.h>
#include <string.h>

extern uintptr_t virtual_end;
extern void* page_table_base;
unsigned long kernel_base = 0xC0000000;

void late_kernel_main() {
    kprintf("Hello from late main!\n");

    for ( ;; ) {}
}

// Old tests from development
void old_tests() {
    // memmap testing code:
    // void* test1 = memmap(NULL, 4096*3, MMAP_RW | MMAP_URGENT);
    // void* test2 = memmap(NULL, 4096, MMAP_RW | MMAP_URGENT);
    // memunmap(test2, 4096);
    //
    // kprintf("%p ...?\n", test2);
    //
    // paging_print();

    // VASA testing code:
    // vasa_print_state();
    // kprintf("\n");
    // void* test = vasa_alloc(MEM_RAM, 1);
    // kprintf("got %p\n\n", test);
    // vasa_print_state();
    // kprintf("\n");
    // vasa_dealloc(test);
    // vasa_print_state();

    // Paging testing code:
    // paging_print();
    // paging_map(frame_alloc(1), (void*) 0x20000000, PAGE_PRESENT | PAGE_RW);
    // paging_print();
    // paging_unmap((void*) 0x20000000);
    // paging_print();
}

void kernel_main(unsigned long multiboot_magic, multiboot_info_t* multiboot, unsigned long initial_pd, unsigned long kernel_heap_start, unsigned long kernel_heap_size) {
    if ( multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC ) {
        // TODO - have an actual panic
        for ( ;; ) {}
    }

    gdt_init();         // Initialize the global descriptor table
    rs232_init();       // Enable RS232 serial i/o
    vga_init();         // Enable VGA output

    // Enable early stage kmalloc/memmap
    early_kmalloc_init((void*) kernel_heap_start, kernel_heap_size);
    memmap_to_frames(multiboot);
    vasa_init((void*) 0xD0000000, (uintptr_t) page_table_base - 0xD0000000);

    idt_init();         // Intialize the Interrupt Descriptor Table
    idt_load();         // Load the intiailized IDT

    bool acpi = acpi_init();

    if ( acpi ) {
        kprintf("using the apic\n");

        pic_disable();        // Disable the PIC
        lapic_enable();       // Enable the APIC

        ioapic_enable_irq(0); // Enable the clock
    }
    else {
        kprintf("using the pic\n");
        pic_enable(0x20, 0x28);
    }

    irq_init();         // Setup Interrupt Requests
    nmi_init();         // Setup Nonmaskable Interrupts
    pit_init();         // Setup the Programmable Interrupt Timer
    ps2_init();         // Setup PS/2 drivers

    // Initiate late stage memory
    frame_init();       // Setup the frame allocator
    kmalloc_init();     // Setup late stage kmalloc
    vasa_switch();      // Switch the virtual address space allocator into late stage mode
    kmem_swap();        // Switch to late stage memory

    tss_init();         // Setup the task segment selector
    clock_init();       // Setup the clock subsystem

    // Setup tasks with the first being the late kernel
    task_init((uintptr_t) late_kernel_main);

    kprintf("enabling interrupts...\n");
    __asm__ volatile ("sti");
    kprintf("... failed to enable interrupts!\n");

    for ( ;; ) {}
}
