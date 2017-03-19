#include <mem/early.h>
#include <mem/kmalloc.h>
#include <stdbool.h>
#include <kprint.h>

bool kern_mem_early = true;

void* kmalloc(unsigned long count) {
    if ( kern_mem_early ) {
        return early_kmalloc(count);
    }
    else {
        // TODO - oh lord
        return (void*) 0;
    }
}

void kfree(void* addr) {
    // Early kernel memory is forever :)
    if ( kern_mem_early ) {
        return;
    }
}

void kmem_swap() {
    void* end = early_kmalloc_end();
    kprintf("Ended early kernel mem at %p\n", end);
    kern_mem_early = false;
}
