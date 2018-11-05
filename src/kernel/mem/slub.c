#include <kernel/config.h>

#include <arch/i386/apic.h>
#include <mem/mmap.h>
#include <mem/slub.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>

// TODO - not this
#define GET_CPU() lapic_get_id()

#ifdef BRACKOS_CONF_SLUB

#define SLAB_NEXT(SLAB) (mem_slab_t *) (((uintptr_t) (SLAB)->next_slab) & ~4095)
#define SLAB_COUNT(SLAB) (((uintptr_t)(SLAB)->next_slab) & 4095)
#define SLAB_CREATE_NEXT(NEXT, SLAB) (mem_slab_t*) ((uintptr_t) SLAB | (uintptr_t) NEXT)

// Create the cache_cache in the data section.
mem_cache_t cache_cache = {
    .next_cache = NULL,

    .name = "cache_cache",
    .object_size = sizeof(mem_cache_t),

    .cpu_caches = {}
};

// TODO - These numbers are kinda just off my head, should actually figure out good ones
mem_kmalloc_block_t kmalloc_sizes[] = {
    {.size = 16,   .name = "kmalloc16"},
    {.size = 32,   .name = "kmalloc32"},
    {.size = 64,   .name = "kmalloc64"},
    {.size = 92,   .name = "kmalloc92"},
    {.size = 128,  .name = "kmalloc128"},
    {.size = 196,  .name = "kmalloc196"},
    {.size = 256,  .name = "kmalloc256"},
    {.size = 384,  .name = "kmalloc384"},
    {.size = 512,  .name = "kmalloc512"},
    {.size = 768,  .name = "kmalloc768"},
    {.size = 1024, .name = "kmalloc1024"},
    {.size = 1363, .name = "kmalloc1363"},
    {.size = 2048, .name = "kmalloc2048"},
    {.size = 4096, .name = "kmalloc4096"},
};

// Create a new cache
mem_cache_t* mem_cache_new(const char* name,
                           unsigned int object_size,
                           unsigned int min,
                           mem_callback_t* construct,
                           mem_callback_t* destruct) {
    if ( strnlen(name, CACHE_NAME_MAXLEN) >= CACHE_NAME_MAXLEN ) {
        kprintf("slub: mem cache name to big!\n");
        return NULL;
    }

    if ( object_size >= SLAB_SIZE - sizeof(mem_cache_t) ) {
        kprintf("slub: object size is too big!\n");
        return NULL;
    }

    // Allocate the cache from the cache_cache cache :^)
    mem_cache_t* cache = mem_cache_alloc("cache_cache");

    if ( cache == NULL ) {
        kprintf("slub: failed to allocate new cache!\n");
        return NULL;
    }

    memcpy(cache->name, name, strlen(name) + 1);

    cache->object_size = object_size;
    cache->min = min;
    cache->construct = construct;
    cache->destruct = destruct;

    for ( int i = 0; i < CACHE_NUM_CPUS; i++ ) {
        cache->cpu_caches[i].full = NULL;
        cache->cpu_caches[i].semi = NULL;
        cache->cpu_caches[i].empty = NULL;
    }

    mem_cache_add(cache);

    for ( int i = 0; i < CACHE_NUM_CPUS; i++ ) {
        mem_slab_t *begin = mem_slab_new(cache);

        if ( begin ) {
            cache->cpu_caches[i].empty = begin;
        }
    }

    return cache;
}

// Add a cache to the cache list
void mem_cache_add(mem_cache_t* cache) {
    mem_cache_t* head = &cache_cache;

    // Find the end of the list in head.
    while ( head->next_cache != NULL ) {
        head = head->next_cache;
    }

    head->next_cache = cache;
    cache->next_cache = NULL;
}

// Allocate a new slab for a specific cache
mem_slab_t* mem_slab_new(mem_cache_t* cache) {
    // Allocate a new slab
    mem_slab_t* slab = (mem_slab_t*) memmap(NULL, SLAB_SIZE, MMAP_URGENT);

    if ( slab == NULL ) {
        return NULL;
    }

    // Derive the object count and the end of the slab header
    unsigned int object_count = (SLAB_SIZE - sizeof(mem_slab_t) - 1) / cache->object_size;
    uintptr_t* end = (uintptr_t*) (slab + 1);

    // Initialize the freelist
    slab->free_head = end;

    // Set the next_slab to NULL with object count
    slab->next_slab = SLAB_CREATE_NEXT(NULL, object_count);

    for ( int i = 0; i < object_count; i++ ) {
        // Get the address of the next object and set the current free object to point to it
        uintptr_t next = ((uintptr_t) end) + cache->object_size;
        *end = next;

        // Handle that object next.
        end = (uintptr_t*) next;
    }

    // Last object should be initialized to NULL
    *end = 0;

    return slab;
}

// Count the amount of free objects in a slab
uint32_t mem_slab_free_count(mem_slab_t* slab) {
    // Start at the head
    uint32_t count = 0;
    uintptr_t* object = slab->free_head;

    // Count the free objects
    while ( object != NULL ) {
        count++;
        object = *(uintptr_t**) object;
    }

    return count;
}

// Count the amount of free objects in a cache
uint32_t mem_cache_free_count(mem_cache_t* cache) {
    uint32_t count = 0;

    // Count the semi list
    mem_slab_t* head = cache->cpu_caches[GET_CPU()].semi;

    while ( head != NULL ) {
        count += mem_slab_free_count(head);
        head = SLAB_NEXT(head);
    }

    // Count empty list.
    head = cache->cpu_caches[GET_CPU()].empty;

    while ( head != NULL ) {
        count += SLAB_COUNT(head);
        head = SLAB_NEXT(head);
    }

    return count;
}

// Allocate an object from a cache
void* mem_cache_alloc(const char* name) {
    mem_cache_t* cache = &cache_cache;

    // Find the cache
    while ( cache != NULL ) {
        if ( strcmp(cache->name, name) ) {
            break;
        }

        cache = cache->next_cache;
    }

    if ( cache == NULL ) {
        kprintf("slub: couldn't find cache %s!\n", name);
        return NULL;
    }

    if ( cache->cpu_caches[GET_CPU()].semi == NULL ) {
        if ( cache->cpu_caches[GET_CPU()].empty == NULL ) {
            mem_slab_t* slab = mem_slab_new(cache);

            if ( slab == NULL ) {
                return NULL;
            }

            // Add the slab to the semi to be used.
            cache->cpu_caches[GET_CPU()].semi = slab;
        }
        else {
            // Remove the cache from the empty
            mem_slab_t* slab = cache->cpu_caches[GET_CPU()].empty;
            cache->cpu_caches[GET_CPU()].empty = SLAB_NEXT(slab);

            // Add it the semi list
            slab->next_slab = SLAB_CREATE_NEXT(NULL, SLAB_COUNT(slab));
            cache->cpu_caches[GET_CPU()].semi = slab;
        }
    }

    mem_slab_t* slab = cache->cpu_caches[GET_CPU()].semi;

    // Get the next free object, and make the freelist head the next object.
    uintptr_t* object = slab->free_head;
    slab->free_head = *(uintptr_t**) object;

    // When we're out of free objects, move the slab to the used list.
    if ( slab->free_head == NULL ) {
        cache->cpu_caches[GET_CPU()].semi = SLAB_NEXT(slab);

        slab->next_slab = SLAB_CREATE_NEXT(cache->cpu_caches[GET_CPU()].full, SLAB_COUNT(slab));
        cache->cpu_caches[GET_CPU()].full = slab;
    }

    if ( cache->construct ) {
        cache->construct((void*) object);
    }

    // If there is a minimum to worry about, make sure we don't hit it.
    if ( cache->min > 0 ) {
        uint32_t free_count = mem_cache_free_count(cache);

        if ( free_count < cache->min ) {

            // If we did, allocate into the empty list. If it fails, don't worry.
            mem_slab_t* slab = mem_slab_new(cache);

            if ( slab != NULL ) {
                slab->next_slab = SLAB_CREATE_NEXT(cache->cpu_caches[GET_CPU()].empty, SLAB_COUNT(slab));
                cache->cpu_caches[GET_CPU()].empty = slab;
            }
        }
    }

    return object;
}

mem_slab_t* mem_cache_find_slab(mem_cache_t* cache, void* object) {
    uintptr_t obj_n = (uintptr_t) object;

    // Check the semi list
    mem_slab_t* head = cache->cpu_caches[GET_CPU()].semi;

    while ( head != NULL ) {
        uintptr_t start = (uintptr_t) head;
        uintptr_t end = start + SLAB_SIZE;

        if ( obj_n >= start && obj_n <= end ) {
            return head;
        }

        head = SLAB_NEXT(head);
    }

    // Check the full list
    head = cache->cpu_caches[GET_CPU()].full;

    while ( head != NULL ) {
        uintptr_t start = (uintptr_t) head;
        uintptr_t end = start + SLAB_SIZE;

        kprintf("> bounds %p - %p\n", start, end);

        if ( obj_n >= start && obj_n <= end ) {
            return head;
        }

        head = SLAB_NEXT(head);
    }

    return NULL;
}

// Deallocate an object from a cache
void mem_cache_dealloc(const char* name, void* object) {
    mem_cache_t* cache = &cache_cache;

    // Find the cache
    while ( cache != NULL ) {
        if ( strcmp(cache->name, name) == true ) {
            break;
        }

        cache = cache->next_cache;
    }

    if ( cache == NULL ) {
        return;
    }

    // Locate the slab from the object
    
    mem_slab_t* slab = NULL;
    
    switch ( SLAB_SIZE ) {
        case 0x1000: slab = (mem_slab_t*) ((uintptr_t) object & ~0x1FFF);
        case 0x2000: slab = (mem_slab_t*) ((uintptr_t) object & ~0xFFF);
        default:     slab = mem_cache_find_slab(cache, object);
    }    

    if ( slab == NULL ) {
        kprintf("failed to find slab for %p\n", object);
        return;
    }

    // Object counts from the next_slab bottom 12 bits.
    unsigned int free_count = 1; // Start at one for the object we're adding
    unsigned int total_count = SLAB_COUNT(slab);

    uintptr_t* free_prev = NULL;
    uintptr_t* free_obj = slab->free_head;

    // We go through the whole list - even once we insert the object - to get free count
    while ( free_obj != NULL ) {
        // Have we not inserted the object yet and found a location suitable for object (we want to keep order)
        if ( object != NULL && ((uintptr_t) object) < ((uintptr_t) free_obj) ) {
            // Insert the object here, or if there is no here at the head then
            // set the current object to the object's next, and clear it so it isn't added again.
            if ( free_prev != NULL ) {
                *free_prev = (uintptr_t) object;
            }
            else {
                slab->free_head = (uintptr_t*) object;
            }

            *(uintptr_t*) object = (uintptr_t) free_obj;
            object = NULL;
        }

        // Increment the count, and move forward in the list
        free_count++;
        free_prev = free_obj;
        free_obj = (uintptr_t*) *free_obj;
    }

    // If we run off the end of the list we won't add, so add it at the end.
    if ( object ) {
        kprintf("ran off end, adding %p at %p", object, free_prev);
        *free_prev = (uintptr_t) object;
    }

    // Move this party to the empty list
    if ( free_count == total_count ) {
        // TODO - Move into the empty list
        // Cycle through the used list, find the slab before us, cut ourselves out, and put ourselves at the head of
        // the free list.
    }
}

// Initialize the kmalloc caches
void kmalloc_init() {
    // Create a new cache for each kmalloc chunk
    for ( int i = 0; i < sizeof(kmalloc_sizes) / sizeof(mem_kmalloc_block_t); i++ ) {
        mem_cache_new(kmalloc_sizes[i].name, kmalloc_sizes[i].size, 0, NULL, NULL);
    }
}

mem_kmalloc_block_t _kmalloc_get_block(void *object) {
    // Locate the slab from the object
    mem_slab_t* slab = (mem_slab_t*) ((uintptr_t) object & ~0x1FFF);

    // Get the total amount of objects in the slab
    unsigned int object_count = SLAB_COUNT(slab);

    // Get the total amount of space used by the slab
    unsigned int total_object_space = SLAB_SIZE - sizeof(mem_slab_t);

    // Estimate the size of the objects. This will be wrong due to rounding.
    int estimate = total_object_space / object_count;

    // Find the first highest object then go one lower (aka round down)
    for ( int i = 0; i < sizeof(kmalloc_sizes) / sizeof(mem_kmalloc_block_t); i++ ) {
        if ( kmalloc_sizes[i].size > estimate ) {
            if ( i == 0 ) {
                return kmalloc_sizes[i];
            }
            else {
                return kmalloc_sizes[i - 1];
            }
        }
    }

    return (mem_kmalloc_block_t) {
        .size = 0,
        .name = ""
    };
}

// Get a kmalloc block for a given size
mem_kmalloc_block_t _kmalloc_block_for_size(unsigned long size) {
    // Start with a NULL block
    mem_kmalloc_block_t block = {
        .size = 0,
        .name = ""
    };

    for ( int i = 0; i < sizeof(kmalloc_sizes) / sizeof(mem_kmalloc_block_t); i++ ) {
        // Find the first size we're less than and use that.
        if ( size < kmalloc_sizes[i].size ) {
            block = kmalloc_sizes[i];
            break;
        }
    }

    return block;
}

// kmalloc for small pointers
void* _kmalloc(unsigned int size) {
    if ( size == 0 ) {
        return NULL;
    }

    mem_kmalloc_block_t block = _kmalloc_block_for_size(size);

    if ( block.size == 0 ) {
        return NULL;
    }

    return mem_cache_alloc(block.name);
}

// krealloc for small pointers
void* _krealloc(void* ptr, unsigned long size) {
    // Get the current block size of the pointer
    mem_kmalloc_block_t current_block = _kmalloc_get_block(ptr);

    if ( current_block.size == 0 ) {
        return NULL;
    }

    mem_kmalloc_block_t next_block = _kmalloc_block_for_size(size);

    if ( next_block.size == current_block.size ) {
        // We can continue using the same block of memory since we're within the
        // same chunk that were in before.
        return ptr;
    }
    else {
        void *next_ptr = mem_cache_alloc(next_block.name);

        if ( next_ptr == NULL ) {
            return NULL;
        }

        // Copy the old contents
        memcpy(next_ptr, ptr, current_block.size);

        // Release the old chunk of memory
        mem_cache_dealloc(current_block.name, ptr);

        return next_ptr;
    }
}

// kfree for small pointers
bool _kfree(void* ptr) {
    mem_kmalloc_block_t block = _kmalloc_get_block(ptr);

    if ( block.size == 0 ) {
        return false;
    }

    mem_cache_dealloc(block.name, ptr);
    return true;
}

#endif
