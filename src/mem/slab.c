#include <mem/mmap.h>
#include <mem/slab.h>
#include <stdlib.h>
#include <string.h>

// Create the cache_cache in the data section.
mem_cache_t cache_cache = (mem_cache_t) {
    .next = NULL,

    .name = "cache_cache",
    .object_size = sizeof(mem_cache_t),

    .full = NULL,
    .semi = NULL,
    .empty = NULL
};

// Create a new cache
mem_cache_t* mem_cache_new(const char* name, unsigned int object_size, mem_callback_t* construct,
                                                                       mem_callback_t* destruct) {
    if ( strlen(name) >= CACHE_NAME_MAXLEN ) {
        return NULL;
    }

    // Allocate the cache from the cache_cache cache :^)
    mem_cache_t* cache = mem_cache_alloc("cache_cache");

    if ( cache == NULL ) {
        return NULL;
    }

    memcpy(cache->name, name, sizeof(cache->name));

    cache->object_size = object_size;
    cache->construct = construct;
    cache->destruct = destruct;

    mem_cache_add(cache);

    return cache;
}

// Add a cache to the cache list
void mem_cache_add(mem_cache_t* cache) {
    mem_cache_t* head = &cache_cache;

    // Find the end of the list in head.
    while ( head->next != NULL ) {
        head = head->next;
    }

    head->next = cache;
    cache->next = NULL;
}

void* mem_cache_alloc(const char* name) {
    mem_cache_t* cache = &cache_cache;

    // Find the cache
    while ( cache != NULL ) {
        if ( strcmp(cache->name, name) == 0 ) {
            break;
        }

        cache = cache->next;
    }

    if ( cache == NULL ) {
        return NULL;
    }

    if ( cache->semi == NULL ) {
        if ( cache->empty == NULL ) {
            // Create a new slab
            mem_slab_t* slab = (mem_slab_t*) memmap(NULL, SLAB_SIZE, MMAP_URGENT);

            if ( slab == NULL ) {
                return NULL;
            }

            slab->next = NULL;

            // Derive the object count and the end of the slab header
            unsigned int object_count = (SLAB_SIZE - sizeof(mem_slab_t)) / cache->object_size;
            uintptr_t* end = (uintptr_t*)(slab + 1);

            // Initialize the freelist
            slab->free_head = end;

            for ( int i = 0; i < object_count; i++ ) {
                // Get the address of the next object and set the current free object to point to it
                uintptr_t next = ((uintptr_t) end) + cache->object_size;
                *end = next;

                // Handle that object next.
                end = (uintptr_t*) next;
            }

            // Add the slab to the semi to be used.
            cache->semi = slab;
        }
        else {
            // Remove the cache from the empty
            mem_slab_t* slab = cache->empty;
            cache->empty = slab->next;

            // Add it the semi list
            slab->next = NULL;
            cache->semi = slab->next;
        }
    }

    mem_slab_t* slab = cache->semi;

    // Get the next free object
    uintptr_t* object = slab->free_head;

    // Get the next pointer to the head
    slab->free_head = *(uintptr_t**) object;

    if ( slab->free_head == NULL ) {
        // Remove the slab from the semi list
        cache->semi = slab->next;

        // Add it to the beginning of the used list
        slab->next = cache->full;
        cache->full = slab;
    }

    if ( cache->construct ) {
        cache->construct((void*) object);
    }

    return object;
}
