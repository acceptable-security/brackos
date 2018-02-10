#include <arch/i386/paging.h>
#include <kernel/spinlock.h>
#include <kprint.h>
#include <mem/frame.h>
#include <stdint.h>

spinlock_t page_lock;

#define PAGE_NO_FLAGS(OBJ) (((uintptr_t) OBJ) & ~0xFFF)
#define PAGE_GET_FLAGS(OBJ) (((uintptr_t) OBJ) & 0xFFF)
#define PAGE_TABLE_FLAGS(TABLE, FLAGS) ((page_table_t*)((uintptr_t)(TABLE) | (FLAGS)))
#define PAGE_TABLE_TEST(TABLE, FLAG) ((uintptr_t)(TABLE) & FLAG)

page_directory_t* page_directory_table = (page_directory_t*) 0xFFFFF000;
page_table_t* page_table_base = (page_table_t*) 0xFFC00000;

void paging_print_page(uintptr_t dir) {
    if ( dir & PAGE_PRESENT ) {
        kprintf("pres ");
    }
    if ( dir & PAGE_RW ) {
        kprintf("rw ");
    }
    if ( dir & PAGE_USER ) {
        kprintf("user ");
    }
    if ( dir & PAGE_WT ) {
        kprintf("writethrough ");
    }
    if ( dir & PAGE_CACHE ) {
        kprintf("cached ");
    }
    if ( dir & PAGE_ACCESSED ) {
        kprintf("accessed ");
    }
    if ( dir & PAGE_PSE ) {
        kprintf("pse ");
    }
    if ( dir & PAGE_GLOBAL ) {
        kprintf("global ");
    }
}

void page_directory_copy(page_directory_t* dest, page_directory_t* src) {
    spinlock_lock(page_lock);

    for ( int dir_ent = 0; dir_ent < 1023; dir_ent++ ) {
        if ( PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PRESENT) ) {
            page_table_t* dst_table = (page_table_t*) frame_alloc(1);
            dest->tables[dir_ent] = PAGE_TABLE_FLAGS(dst_table, PAGE_GET_FLAGS(src->tables[dir_ent]));

            page_table_t* src_table = (page_table_t*) PAGE_NO_FLAGS(src->tables[dir_ent]);

            for ( int tab_ent = 0; tab_ent < 1024; tab_ent++ ) {
                dst_table->entries[tab_ent] = src_table->entries[tab_ent];
            }
        }
        else {
            dest->tables[dir_ent] = PAGE_TABLE_FLAGS(NULL, PAGE_GET_FLAGS(src->tables[dir_ent]));
        }
    }

    spinlock_unlock(page_lock);
}

void paging_print() {
    kprintf("\n== page mapping ==\n");
    // Don't print the recursive page :P
    for ( int dir_ent = 0; dir_ent < 1023; dir_ent++ ) {
        uintptr_t dir = (uintptr_t) page_directory_table->tables[dir_ent];
        uint32_t size = 0x1000;
        uintptr_t address = dir & ~0xFFF;

        if ( dir == 0 ) {
            continue;
        }

        kprintf("page dir %d: ", dir_ent);
        paging_print_page(dir);

        if ( dir & PAGE_PSE ) {
            size = 0x100000;
            kprintf("range: [0x%p, 0x%p]", address, address + size);
            kprintf("\n");
        }
        else if ( !(dir & PAGE_PSE) ) {
            kprintf("\n");

            for ( int tab_ent = 0; tab_ent < 1024; tab_ent++ ) {
                page_table_t* table = (page_table_t*) (((uintptr_t) page_table_base) + (dir_ent * sizeof(page_table_t)));
                uintptr_t addr = *(uintptr_t*)&table->entries[tab_ent];

                if ( addr == 0 || !(addr & PAGE_PRESENT) ) {
                    continue;
                }

                kprintf("  page table %d: ", tab_ent);
                paging_print_page(addr);
                addr = addr & ~0xFFF;
                kprintf("range: [0x%p, 0x%p]\n", addr, addr + size);
            }
        }
    }

    kprintf("page dir 1023 recursive directory\n");
    kprintf("== page mapping ==\n\n");
}

bool paging_map(uintptr_t physical, uintptr_t virt, unsigned short flags) {
    // Only mapping 4KiB aligned addresses
    // TODO - account for PAE
    physical = physical & ~0xFFF;

    uintptr_t dir_ent = virt >> 22;
    uintptr_t tab_ent = virt >> 12 & 0x03FF;

    page_table_t* table = page_table_base + dir_ent;

    spinlock_lock(page_lock);

    if ( !PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PRESENT) ) {
        // Allocate a new table and insert it to the proper location with the proper flags.
        page_table_t* new_table = (page_table_t*) frame_alloc(1);
        new_table = PAGE_TABLE_FLAGS(new_table, PAGE_PRESENT | PAGE_RW);
        page_directory_table->tables[dir_ent] = new_table;
    }

    if ( PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PSE) ) {
        // remapping an already mapped page
        // TODO - panic?
        kprintf("paging: this page is already mapped in a PSE page\n");
        spinlock_unlock(page_lock);
        return false;
    }

    if ( PAGE_TABLE_TEST(table->entries[tab_ent].flags, PAGE_PRESENT) ) {
        kprintf("%p v. %p\n", *(uintptr_t*)&table->entries[tab_ent], physical);
        if ( table->entries[tab_ent].address == physical && table->entries[tab_ent].flags == flags ) {
            // Already mapped at the same address
            spinlock_unlock(page_lock);
            return true;
        }

        // Don't remap the same page
        kprintf("paging: this page is already mapped\n");
        spinlock_unlock(page_lock);
        return false;
    }

    uintptr_t ent = (physical & ~0xFFF) | flags;
    table->entries[tab_ent] = *(page_entry_t*)&ent;

    spinlock_unlock(page_lock);

    return true;
}

bool paging_unmap(uintptr_t virt) {
    uintptr_t dir_ent = virt >> 22;
    uintptr_t tab_ent = virt >> 12 & 0x03FF;

    spinlock_lock(page_lock);

    if ( !PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PRESENT) ) {
        // attempting to unmap a nonexistant page table
        // TODO - panic?
        kprintf("paging: attempted to unmap from a nonexistant page table.\n");

        spinlock_unlock(page_lock);
        return false;
    }

    page_table_t* table = page_table_base + dir_ent;

    table->entries[tab_ent].address = 0;

    if ( table->entries[tab_ent].flags & PAGE_PRESENT ) {
        // Remove the page_present flag
        table->entries[tab_ent].flags &= (unsigned) ~PAGE_PRESENT;
    }

    unsigned int page_count = 0;

    for ( int i = 0; i < 1024; i++ ) {
        if ( table->entries[i].flags != 0 &&
             table->entries[i].address != 0 ) {
            page_count++;
        }
    }

    // If the page table is now empty, release it.
    if ( page_count == 0 ) {
        kprintf("paging: page table empty, releasing...\n");
        uintptr_t page = ((uintptr_t) page_directory_table->tables[dir_ent]) & ~0xFFF;
        frame_dealloc((void*) page, 1);
        page_directory_table->tables[dir_ent] = 0;
    }

    spinlock_unlock(page_lock);
    return true;
}

// Get the physical address of a given virtual address
uintptr_t paging_find_physical(uintptr_t virt) {
    uintptr_t dir_ent = virt >> 22;
    uintptr_t tab_ent = virt >> 12 & 0x03FF;

    page_table_t* table = page_table_base + dir_ent;
    uintptr_t addr = *(uintptr_t*)&table->entries[tab_ent];
    return addr & ~0xFFF;
}

void paging_load_directory(page_directory_t* dir) {
    __asm__ volatile("mov %%cr3, %%eax":"=a"(dir));
}
