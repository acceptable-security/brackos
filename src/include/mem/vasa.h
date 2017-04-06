#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MEM_RAM,
    MEM_MMIO,
    MEM_PCI
} vasa_memtype_t;

struct vasa_node;
typedef struct vasa_node vasa_node_t;

struct vasa_node {
    vasa_node_t* next;
    vasa_memtype_t type;

    void* base;
    unsigned long length;
    unsigned long flags;
};

// Memory is stored in a linked list of free and used chunks.
typedef struct {
    vasa_node_t* free_head; // head of the free list
    vasa_node_t* used_head; // head of the used list
} vasa_t;

bool vasa_mark(uintptr_t base, unsigned long length, bool used);
void vasa_dealloc(void* ptr);
void* vasa_alloc(vasa_memtype_t type, unsigned long size, unsigned long flags);
void vasa_print_state();
void vasa_init(void* start, unsigned long length);
