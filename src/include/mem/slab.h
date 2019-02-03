#include <kernel/config.h>

#include <stdint.h>
#include <stdbool.h>

#ifdef BRACKOS_CONF_SLAB

#define CACHE_NAME_MAXLEN 32
#define SLAB_SIZE 0x2000 // slab size == page size for now.

typedef void (mem_callback_t)(void* object);

struct mem_slab_s;
typedef struct mem_slab_s mem_slab_t;

struct mem_slab_s {
    mem_slab_t* next_slab; // next slab, with the lower 12 bits representing object count of the current slab.
    uintptr_t* free_head; // free item
};

struct mem_cache_s;
typedef struct mem_cache_s mem_cache_t;

struct mem_cache_s {
    mem_cache_t* next_cache;

    char name[CACHE_NAME_MAXLEN];
    unsigned int object_size;
    unsigned int min;

    // slab lists
    mem_slab_t* full;  // Completely used
    mem_slab_t* semi;  // Partially used
    mem_slab_t* empty; // Not used at all

    // initialization methods
    mem_callback_t* construct;
    mem_callback_t* destruct;
};

typedef struct {
    unsigned int size;
    const char* name;
} mem_kmalloc_block_t;

mem_cache_t* mem_cache_new(const char* name, unsigned int object_size, unsigned int minimum,
                                                                       mem_callback_t* construct,
                                                                       mem_callback_t* destruct);
void mem_cache_add(mem_cache_t* cache);
mem_slab_t* mem_slab_new(mem_cache_t* cache);

void* mem_cache_alloc(const char* name);
void mem_cache_dealloc(const char* name, void* object);

void kmalloc_init();
void* _kmalloc(unsigned int size);
bool _kfree(void* ptr);
void* _krealloc(void* addr, unsigned long size);


#endif