#include <mem/early.h>
#include <mem/kmalloc.h>
#include <mem/slab.h>
#include <stdbool.h>
#include <kprint.h>

uintptr_t kernel_mem_end;
bool kern_mem_early = true;

void* kmalloc(unsigned long count) {
    if ( kern_mem_early ) {
        return early_kmalloc(count);
    }
    else {
        return _kmalloc(count);
    }
}

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
