#include <stdint.h>

#define MEMPOOL_MAGIC 0xDEDA110C // it took a ridiculous amount of time to come up with that. pls no make fun :(
#define MEMPOOL_MAX_SPACES 256

// Header at beginning of memory pool
typedef struct {
    unsigned long max_elements;
    unsigned long element_size;
} mempool_hdr_t;

// Header at beginning of memory pool location
typedef struct {
    uint32_t magic;

    unsigned long total_space;
    unsigned long remaining_space;
    uintptr_t space_head;

    mempool_hdr_t* pool_address[MEMPOOL_MAX_SPACES];
} mempool_global_hdr_t;

typedef unsigned int mempool_t;

bool mempool_initialize(void* mempool_base, unsigned long length);
mempool_t mempool_register(void* mempool_base, unsigned long max_elements, unsigned long element_size);
void* mempool_alloc(void* mempool_base, mempool_t pool);
void mempool_dealloc(void* mempool_base, mempool_t pool, void* addr);
