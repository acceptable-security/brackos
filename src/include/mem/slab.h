#include <stdint.h>

#define CACHE_NAME_MAXLEN 32
#define SLAB_SIZE 0x1000 // slab size == page size for now.

typedef void (mem_callback_t)(void* object);

struct mem_slab_s;
typedef struct mem_slab_s mem_slab_t;

struct mem_slab_s {
    mem_slab_t* next; // next slab
    uintptr_t* free_head; // free item
};

struct mem_cache_s;
typedef struct mem_cache_s mem_cache_t;

struct mem_cache_s {
    mem_cache_t* next;

    char name[CACHE_NAME_MAXLEN];
    unsigned int object_size;

    // slab lists
    mem_slab_t* full;  // Completely used
    mem_slab_t* semi;  // Partially used
    mem_slab_t* empty; // Not used at all

    // initialization methods
    mem_callback_t* construct;
    mem_callback_t* destruct;
};

mem_cache_t* mem_cache_new(const char* name, unsigned int object_size, mem_callback_t* construct,
                                                                       mem_callback_t* destruct);
void mem_cache_add(mem_cache_t* cache);

void* mem_cache_alloc(const char* name);
void mem_cache_dealloc(const char* name, void* object);
