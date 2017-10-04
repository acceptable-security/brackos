#include <mem/early.h>
#include <mem/kmalloc.h>
#include <mem/slab.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>

uintptr_t kernel_mem_end;
bool kern_mem_early = true;

// Kernel malloc
void* kmalloc(unsigned long count) {
    if ( kern_mem_early ) {
        return early_kmalloc(count);
    }
    else {
        return _kmalloc(count);
    }
}

// Kernel calloc
void* kcalloc(unsigned long count, unsigned long size) {
    void* ptr = kmalloc(count * size);

    if ( ptr == NULL ) {
        return NULL;
    }

    memset(ptr, 0, count * size);

    return ptr;
}

// Kernel realloc
void* krealloc(void* addr, unsigned long size) {
    // Early kernel memory can't be realloc
    if ( kern_mem_early && (uintptr_t) addr < kernel_mem_end ) {
        return NULL;
    }
    else {
        return _krealloc(addr, size);
    }
}

// Kernel free
void kfree(void* addr) {
    // Early kernel memory is forever :)
    if ( kern_mem_early && (uintptr_t) addr < kernel_mem_end ) {
        return;
    }
    else {
        _kfree(addr);
    }
}

void kmem_swap() {
    void* end = early_kmalloc_end();
    kprintf("Ended early kernel mem at %p\n", end);
    kernel_mem_end = (uintptr_t) end;
    kern_mem_early = false;
}
