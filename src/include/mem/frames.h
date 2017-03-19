// Physical Frame Allocator
// Plan for allocator:
// Run length encoded linked list of free frames

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#define MIN_ORDER 12
#define MAX_ORDER 22

struct frame_node;
typedef struct frame_node frame_node_t;

struct frame_node {
    uintptr_t address;
    frame_node_t* next;
};

typedef struct {
	uintptr_t base;
	frame_node_t* free_lists[MAX_ORDER - MIN_ORDER];
} frame_alloc_t;

void frame_init();
void frame_add_chunk(uintptr_t address, size_t size);
void* frame_alloc(size_t size);
void frame_dealloc(uintptr_t address, size_t size);
void frame_add_free_item(uintptr_t address, size_t order, bool new_item);
