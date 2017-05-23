#include <mem/mmap.h>
#include <mem/slab.h>
#include <stdlib.h>
#include <string.h>

#define SLAB_NEXT(SLAB) (mem_slab_t*)(((uintptr_t) (SLAB)->next_slab) & ~4095)
#define SLAB_COUNT(SLAB) (((uintptr_t) (SLAB)->next_slab) & 4095)
#define SLAB_CREATE_NEXT(NEXT, SLAB) (mem_slab_t*)((uintptr_t) slab | (uintptr_t) NEXT)

// Create the cache_cache in the data section.
mem_cache_t cache_cache = {
    .next_cache = NULL,

    .name = "cache_cache",
    .object_size = sizeof(mem_cache_t),

    .full  = NULL,
    .semi  = NULL,
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
    while ( head->next_cache != NULL ) {
        head = head->next_cache;
    }

    head->next_cache = cache;
    cache->next_cache = NULL;
}

// Allocate an object from a cache
void* mem_cache_alloc(const char* name) {
    mem_cache_t* cache = &cache_cache;

    // Find the cache
    while ( cache != NULL ) {
        if ( strcmp(cache->name, name) == 0 ) {
            break;
        }

        cache = cache->next_cache;
    }

    if ( cache == NULL ) {
        return NULL;
    }

    if ( cache->semi == NULL ) {
        if ( cache->empty == NULL ) {
            // Allocate a new slab
            mem_slab_t* slab = (mem_slab_t*) memmap(NULL, SLAB_SIZE, MMAP_URGENT);

            if ( slab == NULL ) {
                return NULL;
            }

            // Derive the object count and the end of the slab header
            unsigned int object_count = (SLAB_SIZE - sizeof(mem_slab_t)) / cache->object_size;
            uintptr_t* end = (uintptr_t*)(slab + 1);

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

            // Add the slab to the semi to be used.
            cache->semi = slab;
        }
        else {
            // Remove the cache from the empty
            mem_slab_t* slab = cache->empty;
            cache->empty = SLAB_NEXT(slab->next_slab);

            // Add it the semi list
            slab->next_slab = SLAB_CREATE_NEXT(NULL, SLAB_COUNT(slab));
            cache->semi = SLAB_NEXT(slab->next_slab);
        }
    }

    mem_slab_t* slab = cache->semi;

    // Get the next free object, and make the freelist head the next object.
    uintptr_t* object = slab->free_head;
    slab->free_head = *(uintptr_t**) object;

    // When we're out of free objects, move the slab to the used list.
    if ( slab->free_head == NULL ) {
        cache->semi = SLAB_NEXT(slab->next_slab);

        slab->next_slab = SLAB_CREATE_NEXT(cache->full, SLAB_COUNT(slab->next_slab));
        cache->full = slab;
    }

    if ( cache->construct ) {
        cache->construct((void*) object);
    }

    return object;
}

// Deallocate an object from a cache
void mem_cache_dealloc(const char* name, void* object) {
    mem_cache_t* cache = &cache_cache;

    // Find the cache
    while ( cache != NULL ) {
        if ( strcmp(cache->name, name) == 0 ) {
            break;
        }

        cache = cache->next_cache;
    }

    if ( cache == NULL ) {
        return;
    }

    // Locate the slab from the object
    mem_slab_t* slab = (mem_slab_t*) ((uintptr_t) object & ~0xFFF);

    // Object counts from the next_slab bottom 12 bits.
    unsigned int free_count = 0;
    unsigned int total_count = SLAB_COUNT(slab->next_slab);

    uintptr_t* free_prev = NULL;
    uintptr_t* free_obj = slab->free_head;

    // We go through the whole list - even once we insert the object - to get
    while ( free_obj != NULL ) {
        // Have we not inserted the object yet and found a location suitable for object (we want to keep order)
        if ( object != NULL && (uintptr_t) object > (uintptr_t) free_obj ) {
            // Insert the object here, or if there is no here at the head.
            if ( free_prev ) {
                *free_prev = (uintptr_t) object;
            }
            else {
                slab->free_head = (uintptr_t*) object;
            }

            // Set the current object to the object's next, and clear it so it isn't added again.
            *(uintptr_t*) object = (uintptr_t) free_obj;
            object = NULL;
        }

        // Incremenet the count, and move forward in the list
        free_count++;
        free_prev = free_obj;
        free_obj = (uintptr_t*) *free_obj;
    }

    // If we run off the end of the list we won't add, so add it at the end.
    if ( object ) {
        *free_prev = (uintptr_t) object;
    }

    // Take into account the just added object.
    free_count++;

    // Move this party to the empty list
    if ( free_count == total_count ) {

    }
}
