#include <arch/i386/paging.h>

void paging_clone_table(page_table_t* source, page_table_t* target) {
    for ( int i = 0; i < 1024; i++ ) {
        if ( !IS_PAGE_EMPTY(source->entries[i]) ) {
            target->entries[i] = source->entries[i];
        }
    }
}

void paging_clone_directory(page_directory_t* source, page_directory_t* target) {
    for ( int i = 0; i < 1024; i++ ) {
        paging_clone_table(&source->tables[i], &target->tables[i]);
    }
}

void paging_init(page_directory_t* initial_pd) {

}
