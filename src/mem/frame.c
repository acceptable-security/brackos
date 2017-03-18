#include <mem/frames.h>
#include <kprint.h>
#include <stdint.h>

frame_alloc_t* list_head;

// uintptr_t free_stack[1024 * 1024];
// uintptr_t stack_head;
//
// void frame_add_address(void* address) {
//     free_stack[stack_head++] = (uintptr_t) address;
// }
//
// void* frame_get_frame(void* )

void frame_add_chunk(void* base, unsigned int size) {
    kprintf("adding memory chunk @ 0x%p (%m)\n", base, size);
}
