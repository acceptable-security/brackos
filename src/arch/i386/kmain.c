#include <arch/i386/gdt.h>
#include <arch/i386/map.h>
#include <arch/i386/paging.h>
#include <arch/i386/acpi.h>

#include <arch/i386/apic.h>
#include <arch/i386/pic.h>

#include <arch/i386/idt.h>
#include <arch/i386/irq.h>
#include <arch/i386/pit.h>

#include <drivers/ps2.h>
#include <drivers/vga.h>

#include <mem/frame.h>
#include <mem/early.h>
#include <mem/mmap.h>
#include <mem/pool.h>
#include <mem/vasa.h>

#include <multiboot.h>
#include <kprint.h>
#include <stdint.h>
#include <string.h>

extern uintptr_t virtual_end;
extern void* page_table_base;
unsigned long kernel_base = 0xC0000000;

void kernel_main(unsigned long multiboot_magic, multiboot_info_t* multiboot, unsigned long initial_pd, unsigned long kernel_heap_start, unsigned long kernel_heap_size) {
    // for (;;){}
    if ( multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC ) {
        // TODO - have an actual panic
        for ( ;; ) {}
    }

    vga_init();
    gdt_init();

    early_kmalloc_init((void*) kernel_heap_start, kernel_heap_size);
    frame_init();
    memmap_to_frames(multiboot);
    vasa_init((void*) 0xD0000000, (uintptr_t) page_table_base - 0xD0000000);

    // acpi_init();

    idt_init();
    idt_load();

    // TODO - I/O APIC support
    if ( false /* apic_supported() */ ) {
        pic_disable();
        apic_enable();
    }
    else {
        pic_enable(0x20, 0x28);
    }

    irq_init();
    nmi_init();
    pit_init();
    ps2_init();

    __asm__ volatile ("sti");
    kprintf("interrupts enabled\n");

    // memmap testing code:
    // void* test1 = memmap(NULL, 4096*3, MMAP_RW | MMAP_URGENT);
    // void* test2 = memmap(NULL, 4096, MMAP_RW | MMAP_URGENT);
    //
    // memunmap(test1, 4096 * 3);
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

    for ( ;; ) __asm__ ("hlt");
}
