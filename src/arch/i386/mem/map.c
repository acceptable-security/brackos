#include <arch/i386/map.h>
#include <arch/i386/paging.h>
#include <multiboot.h>

#include <mem/frame.h>
#include <mem/mmap.h>
#include <mem/vasa.h>

#include <stdint.h>
#include <kprint.h>
#include <math.h>

#define MEM_TYPE(x) \
            ((x) == MULTIBOOT_MEMORY_AVAILABLE ? "AVAILABLE" : "RESERVED")

extern unsigned long kernel_base;

void print_mem_map(void* _multiboot) {
    multiboot_info_t* multiboot = _multiboot;

    uintptr_t base = multiboot->mmap_addr + kernel_base;
    uintptr_t end = base + multiboot->mmap_length;

    multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

    kprintf("== GRUB Memory Map ==\n");
    kprintf("range: [%x, %x]\n", base, end);
    kprintf("Start  End   Type\n");

    for ( ; base < end; base += entry->size + sizeof(int) ) {
        entry = (multiboot_memory_map_t*) base;

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

    multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

    for ( ; base < end; base += entry->size + sizeof(int) ) {
        entry = (multiboot_memory_map_t*) base;

        uintptr_t addr = entry->addr_low;
        uint32_t len = entry->len_low;

        // Don't use lower memory.
        if ( entry->type == MULTIBOOT_MEMORY_AVAILABLE && addr != 0 ) {
            kprintf("found memory chunk @ 0x%x (%m)\n", addr, len);
            frame_add_chunk(addr, len);
            break;
        }
    }
}

// Mark a certain section of memory as used.
void* memmap(void* start, unsigned long length, unsigned long flags) {
    // Find page aligned addresses
    uintptr_t virt_start = ((uintptr_t) start) & ~0xFFF;
    uintptr_t virt_end = ((uintptr_t) start + length) & ~0xFFF;
    uintptr_t virt_len = virt_end - virt_start;

    // If we're going to allocate the page, flag it.
    if ( flags & MMAP_URGENT ) {
        flags |= MMAP_ALLOCATED;
    }

    // If they ask for a memory address, give them one.
    if ( start == NULL ) {
        virt_start = (uintptr_t) vasa_alloc(MEM_RAM, virt_len, flags);

        if ( virt_start == 0 ) {
            return NULL;
        }

        virt_end = virt_start + virt_len;
    }
    else {
        // Else try and make their given address as used
        if ( !vasa_mark(virt_start, virt_len, true, flags) ) {
            // Failed to allocate the virtual address space.
            kprintf("Failed to allocate the vas\n");
            return NULL;
        }
    }

    // Amount of pages necessary for this allocation
    unsigned long page_cnt = max(1, (virt_end - virt_start) / PAGE_SIZE);

    // Flags to be passed to the pager
    // TODO - actual flags parsing
    unsigned long page_flags = PAGE_RW | PAGE_PRESENT;

    if ( flags & MMAP_URGENT ) {
        if ( flags & MMAP_CONTINUOUS ) {
            // Immediately allocate page_cnt continuous pages
            void* pages = frame_alloc(page_cnt);

            if ( pages == NULL ) {
                kprintf("failed to allocate %d pages", page_cnt);
                return NULL;
            }

            for ( int i = 0; i < page_cnt; i++ ) {
                uintptr_t page = (uintptr_t) pages + (i * PAGE_SIZE);
                uintptr_t virt =  virt_start + (i * PAGE_SIZE);

                if ( !paging_map(page, virt, page_flags) ) {
                    frame_dealloc(pages, page_cnt);
                    kprintf("Failed to map %p to %p\n", page, virt);

                    // TODO - clean up the page dir/table.
                    return NULL;
                }
            }
        }
        else {
            // Immediately allocate page_cnt pages (by allocating 1 at a time,
            // it won't necessarily be continuous)
            for ( int i = 0; i < page_cnt; i++ ) {
                void* page = frame_alloc(1);

                if ( page == NULL ) {
                    kprintf("failed to allocate 1 page\n");
                    frame_dealloc(page, 1);
                    // TODO - dealloc frames before and fix page dir/table
                    return NULL;
                }

                uintptr_t virt = virt_start + (i * PAGE_SIZE);

                if ( !paging_map((uintptr_t) page, virt, page_flags) ) {
                    kprintf("failed to map %p to %p\n", page, virt);
                    frame_dealloc(page, 1);
                    // TODO - dealloc frames before and fix page dir/table

                    return NULL;
                }
            }
        }

        return (void*) virt_start;
    }

    // The allocations have been made in the virtual address space.
    // If/when a page fault occurs, the page fault handler can handle
    // it with the flags.

    return (void*) virt_start;
}

// Mark a section of memory as no longer used.
// This is also responsible for the deallocation of page frames.
void memunmap(void* start, unsigned long length) {
    if ( start == NULL ) {
        kprintf("can't unmap null\n");
        return;
    }

    uintptr_t virt_start = ((uintptr_t) start) & ~0xFFF;
    uintptr_t virt_end = ((uintptr_t) start + length) & ~0xFFF;
    uintptr_t virt_size = virt_end - virt_start;

    // Acquire the flags.
    unsigned long flags = vasa_get_flags(virt_start);

    // Handle the allocated memory segments
    if ( flags & MMAP_ALLOCATED ) {
        if ( flags & MMAP_CONTINUOUS ) {
            // Find the physical frame and deallocate it
            uintptr_t physical = paging_find_physical(virt_start);
            frame_dealloc((void*) physical, virt_size / PAGE_SIZE);

            // Unmap all the pages
            for ( int page = virt_start; page < virt_end; page += PAGE_SIZE ) {
                if ( !paging_unmap(page) ) {
                    // Failed to unmap page!
                    // PANIC!!
                    return;
                }
            }
        }
        else {
            // Find each physical frame, deallocate it, and unmap the memory.
            for ( int page = virt_start; page < virt_end; page += PAGE_SIZE ) {
                uintptr_t physical = paging_find_physical(page);
                frame_dealloc((void*) physical, 1);

                if ( !paging_unmap(page) ) {
                    kprintf("failed to unmap page\n");
                    // Failed to unmap page!
                    // PANIC!!
                    return;
                }
            }
        }
    }

    // Mark the virtul memory as unused.
    vasa_mark(virt_start, length, false, 0);
}
