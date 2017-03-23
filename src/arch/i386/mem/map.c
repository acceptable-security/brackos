#include <arch/i386/map.h>
#include <arch/i386/paging.h>
#include <multiboot.h>

#include <mem/frame.h>
#include <mem/mmap.h>

#include <stdint.h>
#include <kprint.h>

#define MEM_TYPE(x) ((x) == MULTIBOOT_MEMORY_AVAILABLE ? "AVAILABLE" : "RESERVED" )
extern unsigned long kernel_base;

void print_mem_map(void* _multiboot) {
    multiboot_info_t* multiboot = _multiboot;
    uintptr_t base = multiboot->mmap_addr + kernel_base;
    uintptr_t end = base + multiboot->mmap_length;

    kprintf("== GRUB Memory Map ==\n");
    kprintf("range: [%x, %x]\n", base, end);
    kprintf("Start  End   Type\n");
    for ( ; base < end; base += ((multiboot_memory_map_t*) base)->size + sizeof(int) ) {
        multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

        kprintf("%x | %x | %s (%m)\n", entry->addr_low,
                                  entry->addr_low + entry->len_low - 1,
                                  MEM_TYPE(entry->type),
                                  entry->len_low);
    }
}

void memmap_to_frames(void* _multiboot) {
    multiboot_info_t* multiboot = _multiboot;

    uintptr_t base = multiboot->mmap_addr + kernel_base;
    uintptr_t end = base + multiboot->mmap_length;

    for ( ; base < end; base += ((multiboot_memory_map_t*) base)->size + sizeof(int) ) {
        multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

        // Don't use lower memory.
        if ( entry->type == MULTIBOOT_MEMORY_AVAILABLE && entry->addr_low != 0 ) {
            kprintf("found memory chunk @ %x (%m)\n", entry->addr_low, entry->len_low);
            frame_add_chunk(entry->addr_low, entry->len_low);
            break;
        }
    }

}

void* memmap(void* start, unsigned long length, unsigned long flags) {
    if ( flags & MMAP_URGENT ) {
        uintptr_t virt_start = ((uintptr_t) start) & ~0xFFF;
        uintptr_t virt_end = ((uintptr_t) start + length) & ~0xFFF;
        unsigned long page_cnt = (virt_end - virt_start) / PAGE_SIZE;

        void* pages = frame_alloc(page_cnt);

        if ( pages == NULL ) {
            return NULL;
        }

        for ( int i = 0; i < page_cnt; i++ ) {
            if ( !paging_map(pages, (void*) virt_start + (page_cnt * PAGE_SIZE), PAGE_PRESENT | PAGE_RW) ) {
                frame_dealloc(pages, page_cnt);
                return NULL;
            }
        }
    }
    else {

    }

    // TODO
    return NULL;
}
