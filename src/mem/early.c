#include <stdint.h>

uintptr_t early_memory_base = 0;
uintptr_t early_memory_end = 0;

// no early kfree. any early kmallocs should be *necessary* structures only.

void* early_kmalloc(unsigned long size) {
    // Make usre we aren't done with early kmallocs/they've been setup
    if ( early_memory_base == 0 ) {
        return (void*) 0;
    }

    if ( early_memory_base + size <= early_memory_end ) {
        void* tmp = (void*) early_memory_base;
        early_memory_base += size;
        return tmp;
    }

    return (void*) 0;
}

// Initialize the early kmalloc data
void early_kmalloc_init(uintptr_t start, unsigned long size) {
    early_memory_base = start;
    early_memory_end = start + size;
}

// End early kmalloc.
void* early_kmalloc_end() {
    void* end = (void*) early_memory_base;

    early_memory_base = 0;
    early_memory_end = 0;
    
    return end;
}
