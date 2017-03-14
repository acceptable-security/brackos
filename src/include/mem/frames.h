// Physical Frame Allocator
// Plan for allocator:
// Run length encoded linked list of free frames

struct frame_alloc;
typedef struct frame_alloc frame_alloc_t;

struct frame_alloc {
    void* base;          // Base physical address of the chunk
    unsigned int count;  // Amount of page frames accessable
    frame_alloc_t* next; // Next chunk, NULL if none.
};

void frame_dealloc(void* base, unsigned int count);
void* frame_alloc(unsigned int count);

void frame_add_chunk(void* base, unsigned int count);
