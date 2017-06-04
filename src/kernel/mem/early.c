#include <stdint.h>
#include <stdbool.h>
#include <kprint.h>

uintptr_t early_memory_base = 0;
uintptr_t early_memory_end = 0;


// Returns whether or not early kmalloc is still active.
bool early_kmalloc_active() {
    return early_memory_base != 0;
}

void* early_kmalloc(unsigned long size) {
    if ( !early_kmalloc_active() ) {
        return (void*) 0;
    }

    if ( early_memory_base + size > early_memory_end ) {
        kprintf("out of kern mem");
        for (;;) {}
    }

    void* tmp = (void*) early_memory_base;
    early_memory_base += size;
    return tmp;

    return (void*) 0;
}

// Initialize the early kmalloc data
void early_kmalloc_init(void* start, unsigned long size) {
    early_memory_base = (uintptr_t) start;
    early_memory_end = (uintptr_t) start + size;

    kprintf("started early kmalloc (%p, %p)\n", early_memory_base, early_memory_end);
}

// End early kmalloc.
void* early_kmalloc_end() {
    void* end = (void*) early_memory_base;

    early_memory_base = 0;
    early_memory_end = 0;

    kprintf("early kmalloc is over.\n");

    return end;
}
