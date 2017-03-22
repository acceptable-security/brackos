#include <arch/i386/paging.h>
#include <kprint.h>
#include <mem/frame.h>
#include <stdint.h>

#define PAGE_TABLE_FLAGS(table, flags) ((page_table_t*)((uintptr_t)(table) | (flags)))
#define PAGE_TABLE_TEST(table, flag) ((uintptr_t)(table) & flag)

page_directory_t* page_directory_table = (page_directory_t*) 0xFFFFF000;
page_table_t* page_table_base = (page_table_t*) 0xFFC00000;

// void paging_clone_table(page_table_t* source, page_table_t* target) {
//     for ( int i = 0; i < 1024; i++ ) {
//         if ( !IS_PAGE_EMPTY(source->entries[i]) ) {
//             target->entries[i] = source->entries[i];
//         }
//     }
// }
//
// void paging_clone_directory(page_directory_t* source, page_directory_t* target) {
//     for ( int i = 0; i < 1024; i++ ) {
//         paging_clone_table(&source->tables[i], &target->tables[i]);
//     }
// }

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
                page_table_t* table = page_table_base + (dir_ent * sizeof(page_table_t));
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

bool paging_map(void* physical, void* virt, unsigned short flags) {
    uintptr_t dir_ent = ((uintptr_t) virt) >> 22;
    uintptr_t tab_ent = ((uintptr_t) virt) >> 12 & 0x03FF;

    page_table_t* table = page_table_base + (dir_ent * sizeof(page_table_t));

    if ( !PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PRESENT) ) {
        // Allocate a new table and insert it to the proper location with the proper flags.
        page_table_t* new_table = (page_table_t*) frame_alloc(1);
        new_table = PAGE_TABLE_FLAGS(new_table, PAGE_PRESENT | PAGE_RW);
        page_directory_table->tables[dir_ent] = new_table;
    }

    if ( PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PSE) ) {
        // remapping an already mapped page
        // TODO - panic?
        kprintf("this page is already mapped in a PSE page");
        return false;
    }

    if ( PAGE_TABLE_TEST(table->entries[tab_ent].flags, PAGE_PRESENT) ) {
        // Don't remap the same page
        // TODO - panic?
        kprintf("this page is already mapped");
        return false;
    }

    uintptr_t ent = ((uintptr_t) physical & ~0xFFF) | flags;
    table->entries[tab_ent] = *(page_entry_t*)&ent;

    kprintf("mapped %d:%d to %p\n", dir_ent, tab_ent, ent);

    return true;
}

bool paging_unmap(void* virt) {
    uintptr_t dir_ent = ((uintptr_t) virt) >> 22;
    uintptr_t tab_ent = ((uintptr_t) virt) >> 12 & 0x03FF;

    if ( !PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PRESENT) ) {
        // attempting to unmap a nonexistant page table
        // TODO - panic?
        kprintf("attempted to unmap from a nonexistant page table.");
        return false;
    }

    page_table_t* table = page_table_base + (dir_ent * sizeof(page_table_t));

    table->entries[tab_ent].address = 0;

    if ( table->entries[tab_ent].flags & PAGE_PRESENT ) {
        // Remove the page_present flag
        table->entries[tab_ent].flags &= ~PAGE_PRESENT;
    }

    unsigned int present_count = 0;

    for ( int i = 0; i < 1024; i++ ) {
        if ( table->entries[tab_ent].flags & PAGE_PRESENT ) {
            present_count++;
        }
    }

    // If the page table is now empty, release it.
    if ( present_count == 0 ) {
        kprintf("page table empty, releasing...\n");
        uintptr_t page = ((uintptr_t) page_directory_table->tables[dir_ent]) & ~0xFFF;
        frame_dealloc((void*) page, 1);
        page_directory_table->tables[dir_ent] = 0;
    }


    return false;
}
