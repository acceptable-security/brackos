#include <stdbool.h>

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
};

// Memory is stored in a linked list of free and used chunks.
typedef struct {
    vasa_node_t* free_head; // head of the free list
    vasa_node_t* used_head; // head of the used list
} vasa_t;

void vasa_merge(bool used);
void vasa_dealloc(void* ptr);
void* vasa_alloc(vasa_memtype_t type, unsigned long size);
void vasa_print_state();
void vasa_init();
