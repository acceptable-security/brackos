#include <arch/i386/gdt.h>
#include <arch/i386/map.h>
#include <arch/i386/paging.h>

#include <drivers/vga.h>

#include <mem/frame.h>
#include <mem/early.h>

#include <multiboot.h>
#include <kprint.h>
#include <stdint.h>

extern uintptr_t virtual_end;
unsigned long kernel_base = 0xC0000000;

void kernel_main(unsigned long multiboot_magic, multiboot_info_t* multiboot, unsigned long initial_pd) {
    // for (;;){}
    if ( multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC ) {
        // TODO - have an actual panic
        for ( ;; ) {}
    }

    vga_init();
    gdt_init();

    uintptr_t base = multiboot->mmap_addr + kernel_base;
    uintptr_t end = base + multiboot->mmap_length;

    // TODO - not this
    early_kmalloc_init(&virtual_end, 0x10000);
    frame_init();

    for ( ; base < end; base += ((multiboot_memory_map_t*) base)->size + sizeof(int) ) {
        multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

        if ( entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->addr_low != 0 ) {
            kprintf("found memory chunk @ %x (%m)\n", entry->addr_low, entry->len_low);
            frame_add_chunk(entry->addr_low, entry->len_low);
            break;
        }
    }


    void* frame = frame_alloc(1);
    kprintf("got %p\n", frame);

    paging_map(frame, (void*) 12, PAGE_PRESENT | PAGE_RW);

    // int test = 12;
    // *(int*) test = 1;

    for( ;; ) {}
}
