// Virtual Address Space Allocator.
#include <arch/i386/vasa.h>
#include <stdlib.h>
#include <stdint.h>

extern void* page_table_base;
vasa_t global_asa;

// The VASA system will take up from the end of the kernel to the start of the page_table_base
void vasa_init(void* start) {
    // Split the virtual address space neatly amongst all of the
    unsigned long total_space = ((uintptr_t) page_table_base) - (uintptr_t) start;
    unsigned long individual_space = total_space / (_MEM_CAP - 1);

    uintptr_t index = (uintptr_t) start;

    for ( int i = 0; i < _MEM_CAP - 1; i++ ) {
        global_asa.types_start[i] = (void*) index;
        global_asa.types_end[i] = (void*) index + individual_space;

        vasa_node_t* type_head = (vasa_node_t*) kmalloc(sizeof(vasa_node_t));
        type_head->next = NULL;
        type_head->base = (void*) index;
        type_head->length = individual_space;

        global_asa.free_head[i] = type_head;
        global_asa.used_head[i] = NULL;

        index += individual_space;
    }
}
