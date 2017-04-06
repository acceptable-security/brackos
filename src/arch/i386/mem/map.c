#include <arch/i386/map.h>
#include <arch/i386/paging.h>
#include <multiboot.h>

#include <mem/frame.h>
#include <mem/mmap.h>
#include <mem/vasa.h>

#include <stdint.h>
#include <kprint.h>
#include <math.h>

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
    // Find page aligned addresses
    uintptr_t virt_start = ((uintptr_t) start) & ~0xFFF;
    uintptr_t virt_end = ((uintptr_t) start + length) & ~0xFFF;
    uintptr_t virt_len = virt_end - virt_start;

    // If they ask for a memory address, give them one.
    if ( start == NULL ) {
        start = vasa_alloc(MEM_RAM, virt_len, flags);

        if ( start == NULL ) {
            return NULL;
        }
    }
    else {
        // Else try and make their given address as used
        if ( !vasa_mark(virt_start, virt_len, true) ) {
            // Failed to allocate the virtual address space.
            kprintf("Failed to allocate the vas\n");
            return NULL;
        }
    }

    // Amount of pages necessary for this allocation
    unsigned long page_cnt = max(1, (virt_end - virt_start) / PAGE_SIZE);

    // Flags to be passed to the pager
    unsigned long paging_flags = PAGE_RW | PAGE_PRESENT;//flags; // TODO - actual flags parsing

    kprintf("setting %p for %d pages\n", virt_start, page_cnt);

    if ( flags & MMAP_URGENT ) {
        if ( flags & MMAP_CONTINUOUS ) {
            // Immediately allocate page_cnt continuous pages
            void* pages = frame_alloc(page_cnt);

            if ( pages == NULL ) {
                kprintf("failed to allocate %d pages", page_cnt);
                return NULL;
            }

            for ( int i = 0; i < page_cnt; i++ ) {
                if ( !paging_map((uintptr_t) pages + (i * PAGE_SIZE), virt_start + (i * PAGE_SIZE), paging_flags) ) {
                    frame_dealloc(pages, page_cnt);
                    kprintf("Failed to map %p to %p\n", (uintptr_t) pages + (i * PAGE_SIZE), virt_start + (i * PAGE_SIZE));
                    // TODO - clean up the page dir/table.
                    return NULL;
                }
            }
        }
        else {
            // Immediately allocate page_cnt pages (by allocating 1 at a time, it won't necessarily be continuous)
            for ( int i = 0; i < page_cnt; i++ ) {
                void* page = frame_alloc(1);

                if ( page == NULL ) {
                    kprintf("failed to allocate 1 page\n");
                    frame_dealloc(page, 1);
                    // TODO - dealloc frames before and fix page dir/table
                    return NULL;
                }

                if ( !paging_map((uintptr_t) page, virt_start + (i * PAGE_SIZE), paging_flags) ) {
                    kprintf("failed to map %p to %p\n", page, (void*) virt_start + (page_cnt * PAGE_SIZE));
                    frame_dealloc(page, 1);
                    // TODO - dealloc frames before and fix page dir/table
                    return NULL;
                }

            }
        }

        return (void*) virt_start;
    }
    else {
        // TODO - mark in the VASA for allocation
    }

    // TODO
    return NULL;
}

void memunmap(void* start, unsigned long length) {

}
