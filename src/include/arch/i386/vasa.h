typedef enum {
    MEM_USABLE,
    MEM_MMIO,
    MEM_PCI,
    MEM_BIOS,
    _MEM_CAP // end position to use for arrays
} vasa_memtype_t;

struct vasa_node;
typedef struct vasa_node vasa_node_t;

struct vasa_node {
    vasa_node_t* next;

    void* base;
    unsigned long length;
};

// Each memory type has a linked list of free and used address spaces,
// which are used for allocating and recycling continous pieces of
// address space which can be used by those respective types.
typedef struct {
    vasa_node_t* free_head[_MEM_CAP - 1]; // head of the free lists
    vasa_node_t* used_head[_MEM_CAP - 1]; // head of the used lists
} vasa_t;

void vasa_dealloc(void* ptr);
void* vasa_alloc(vasa_memtype_t type, unsigned long size);
void vasa_init();
