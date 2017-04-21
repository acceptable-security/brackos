#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <mem/pool.h>

// Initialize the memory pool allocator at a certain address with a certain length
bool mempool_initialize(void* mempool_base, unsigned long length) {
    // Can't if fit any data
    if ( length <= sizeof(mempool_global_hdr_t) + sizeof(mempool_hdr_t) ) {
        return false;
    }

    mempool_global_hdr_t* global_hdr = (mempool_global_hdr_t*) mempool_base;

    global_hdr->magic = MEMPOOL_MAGIC;
    global_hdr->total_space = length - sizeof(mempool_global_hdr_t);
    global_hdr->remaining_space = global_hdr->total_space;
    global_hdr->space_head = ((uintptr_t) mempool_base) + sizeof(mempool_global_hdr_t);

    for ( int i = 0; i < MEMPOOL_MAX_SPACES; i++ ) {
        global_hdr->pool_address[i] = NULL;
    }

    return true;
}

// Attempts to register a new memory pool in the memory pool space.
mempool_t mempool_register(void* mempool_base, unsigned long max_elements, unsigned long element_size) {
    mempool_global_hdr_t* global_hdr = (mempool_global_hdr_t*) mempool_base;

    // Check magic
    if ( global_hdr->magic != MEMPOOL_MAGIC ) {
        return 0;
    }

    unsigned long total_space = sizeof(mempool_hdr_t) +                     // Header size
                                (max_elements / sizeof(unsigned long)) +    // Usage bitmap
                                max_elements * element_size;                // Memory pool

    // Not enough space for allocation
    if ( global_hdr->remaining_space <= total_space ) {
        return 0;
    }

    mempool_hdr_t* pool_hdr = (mempool_hdr_t*) global_hdr->space_head;

    pool_hdr->max_elements = max_elements;
    pool_hdr->element_size = element_size;

    for ( int i = 0; i < MEMPOOL_MAX_SPACES; i++ ) {
        if ( global_hdr->pool_address[i] == 0 ) {
            // Make state changes once confirmed can insert.
            global_hdr->remaining_space -= total_space;
            global_hdr->space_head += total_space;

            // Insert
            global_hdr->pool_address[i] = pool_hdr;
            return i + 1;
        }
    }

    // Failed to find an available space in the global header.
    return 0;
}

// Allocate from a memory pool
void* mempool_alloc(void* mempool_base, mempool_t pool) {
    if ( pool == 0 ) {
        return NULL;
    }

    mempool_global_hdr_t* global_hdr = (mempool_global_hdr_t*) mempool_base;

    // Check magic
    if ( global_hdr->magic != MEMPOOL_MAGIC ) {
        return 0;
    }

    mempool_hdr_t* pool_hdr = global_hdr->pool_address[pool - 1];

    // Make sure there is a pool where they're asking.
    if ( pool_hdr == NULL ) {
        return NULL;
    }

    unsigned long usage_bitmap_size = pool_hdr->max_elements / sizeof(unsigned long);
    unsigned long usage_bitmap_length = usage_bitmap_size / sizeof(unsigned long);
    uintptr_t usage_bitmap_base = (uintptr_t) pool_hdr + sizeof(mempool_hdr_t);

    for ( int i = 0; i < usage_bitmap_length; i++ ) {
        // Get the i-th unsigned long in the bitmap
        unsigned long* usage_bitmap_element = (unsigned long*)(usage_bitmap_base + (i * sizeof(unsigned long)));

        // Try to find a bit that isn't set
        for ( int j = 0; j < sizeof(unsigned long); j++ ) {
            if ( !(*usage_bitmap_element & (1 << j)) ) {
                // Calculate address
                unsigned long element_num = j * sizeof(unsigned long) + i;
                uintptr_t addr = usage_bitmap_base + usage_bitmap_size + (pool_hdr->element_size * element_num);

                // Mark as used
                *usage_bitmap_element = *usage_bitmap_element | (1 << j);

                return (void*) addr;
            }
        }
    }

    return NULL;
}

// Deallocate from a memory pool
void mempool_dealloc(void* mempool_base, mempool_t pool, void* addr) {
    if ( pool == 0 ) {
        return;
    }

    mempool_global_hdr_t* global_hdr = (mempool_global_hdr_t*) mempool_base;

    // Check magic
    if ( global_hdr->magic != MEMPOOL_MAGIC ) {
        return;
    }

    mempool_hdr_t* pool_hdr = global_hdr->pool_address[pool - 1];

    // Make sure there is a pool where they're asking.
    if ( pool_hdr == NULL ) {
        return;
    }

    unsigned long usage_bitmap_size = pool_hdr->max_elements / sizeof(unsigned long);
    uintptr_t usage_bitmap_base = (uintptr_t) pool_hdr + sizeof(mempool_hdr_t);

    // First, calculate which element within the entire memory pool the address is (it's linear so just subtract the
    // junk and divide by element size). Then calculate where in the usage bitmap that is, and what bit in the element
    // it uses.
    unsigned long element_num = ((uintptr_t) addr - usage_bitmap_base - usage_bitmap_size) / pool_hdr->element_size;
    unsigned long usage_element_num = element_num / sizeof(unsigned long);
    unsigned long usage_bit_num = element_num % sizeof(unsigned long);

    // Mark it as no longer used.
    unsigned long* usage_element = (unsigned long*)(usage_bitmap_base + usage_element_num * sizeof(unsigned long));

    *usage_element = *usage_element & ~(1 << usage_bit_num);
}
