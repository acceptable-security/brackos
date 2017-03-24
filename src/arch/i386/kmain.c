#include <arch/i386/gdt.h>
#include <arch/i386/map.h>
#include <arch/i386/paging.h>
#include <mem/vasa.h>

#include <drivers/vga.h>

#include <mem/frame.h>
#include <mem/early.h>
#include <mem/mmap.h>

#include <multiboot.h>
#include <kprint.h>
#include <stdint.h>

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

    // TODO - not this
    early_kmalloc_init((void*) kernel_heap_start, kernel_heap_size);
    frame_init();
    memmap_to_frames(multiboot);
    vasa_init(0xD0000000, (uintptr_t) page_table_base - 0xD0000000);

    void* test = memmap(vasa_alloc(MEM_RAM, 4096*3), 4096*3, MMAP_RW | MMAP_URGENT);
    kprintf("%p\n", test);

    test = memmap(vasa_alloc(MEM_RAM, 4096), 4096, MMAP_RW | MMAP_URGENT);
    kprintf("%p\n", test);

    paging_print();

    // VASA Testing code
    // vasa_print_state();
    // void* test = vasa_alloc(MEM_RAM, 1);
    // kprintf("got %p\n", test);
    // vasa_print_state();
    // vasa_dealloc(test);
    // vasa_print_state();
    // vasa_merge(true);
    // vasa_merge(false);
    // vasa_print_state();

    // Paging testing code
    // paging_print();
    // paging_map(frame_alloc(1), (void*) 0x20000000, PAGE_PRESENT | PAGE_RW);
    // paging_print();
    // paging_unmap((void*) 0x20000000);
    // paging_print();
}
