#include <arch/i386/paging.h>
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

void paging_print(page_directory_t* current_pd) {

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

    if ( PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_SIZE) ) {
        // remapping an already mapped page
        // TODO - panic?
        return false;
    }

    // Don't remap the same page
    if ( PAGE_TABLE_TEST(table->entries[tab_ent].flags, PAGE_PRESENT) ) {
        // TODO - panic?
        return false;
    }

    uintptr_t ent = ((uintptr_t) physical & ~0xFFF) | flags;
    table->entries[tab_ent] = *(page_entry_t*)&ent;

    return true;
}

bool paging_unmap(void* virt) {
    uintptr_t dir_ent = ((uintptr_t) virt) >> 22;
    uintptr_t tab_ent = ((uintptr_t) virt) >> 12 & 0x03FF;

    if ( !PAGE_TABLE_TEST(page_directory_table->tables[dir_ent], PAGE_PRESENT) ) {
        // attempting to unmap a nonexistant page table
        // TODO - panic?
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
        uintptr_t page = ((uintptr_t) page_directory_table->tables[dir_ent]) & ~0xFFF;
        frame_dealloc((void*) page, 1);
        page_directory_table->tables[dir_ent] = 0;
    }

    return false;
}
