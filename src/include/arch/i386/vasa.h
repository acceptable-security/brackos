typedef enum {
    MEM_USABLE,
    MEM_MMIO,
    MEM_PCI,
    MEM_BIOS
} vasa_memtype_t;

struct vasa_node;
typedef struct vasa_node vasa_node_t;

struct vasa_node {
    vasa_node_t* next;
    vasa_memtype_t type;

    void* base;
    unsigned long length;
};

typedef struct {
    vasa_node_t* free_head; // head of the free list
    vasa_node_t* used_head; // head of the used list.
} vasa_t;

void vasa_dealloc(void* ptr);
void* vasa_alloc(vasa_memtype_t type, unsigned long size);
void vasa_init();
