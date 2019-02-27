// Physical Frame Allocator

#include <kernel/spinlock.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_FRAMES 20
#define PAGE_SIZE 0x1000

struct frame_node;
typedef struct frame_node frame_node_t;

struct frame_node {
    uintptr_t address;
    frame_node_t* next;
};

typedef struct {
    spinlock_t lock;
	uintptr_t base;
	frame_node_t* free_lists[MAX_FRAMES];
} frame_alloc_t;

void frame_init();
void frame_add_chunk(uintptr_t address, size_t size);
void* frame_alloc(size_t size);
void frame_dealloc(void* address, size_t size);
void frame_add_free_item(uintptr_t address, size_t order, bool new_item);
void frame_status();
size_t frame_free_count();