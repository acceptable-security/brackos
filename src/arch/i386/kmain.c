#include <arch/i386/gdt.h>
#include <arch/i386/map.h>

#include <drivers/vga.h>

#include <mem/frame.h>
#include <mem/early.h>

#include <multiboot.h>
#include <kprint.h>
#include <stdint.h>

extern uintptr_t virtual_end;
unsigned long kernel_base = 0xC0000000;

void kernel_main(unsigned long multiboot_magic, multiboot_info_t* multiboot, unsigned long initial_pd) {
    if ( multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC ) {
        // TODO - have an actual panic
        for ( ;; ) {}
    }

	vga_init();
    kprintf("vga initialized...\n");

    gdt_init();
    kprintf("gdt initialized...\n");

    kprintf("kernel end: %x\n", &virtual_end);

    uintptr_t base = multiboot->mmap_addr + kernel_base;
    uintptr_t end = base + multiboot->mmap_length;

    // TODO - not this
    early_kmalloc_init(&virtual_end, 0x20000);
    frame_init();

    for ( ; base < end; base += ((multiboot_memory_map_t*) base)->size + sizeof(int) ) {
        multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

        if ( entry->type == MULTIBOOT_MEMORY_AVAILABLE ) {
            kprintf("found memory chunk @ %x (%m)\n", entry->addr_low, entry->len_low);
            frame_add_chunk(entry->addr_low, entry->len_low);
        }
    }

    frame_status();

    for(;;){}
}
