// Virtual Address Space Allocator.

#include <arch/i386/vasa.h>
#include <stdlib.h>
#include <stdint.h>

extern void* page_table_base;
vasa_t global_asa;

void* vasa_alloc(vasa_memtype_t type, unsigned long size) {
    vasa_node_t* prev = NULL;
    vasa_node_t* head = global_asa.free_head;

    while ( head != NULL ) {
        if ( head->length >= size ) {
            void* ptr = head->base;

            if ( head->length > size ) {
                // Cut our chunk out of the start of the node.
                head->length -= size;
                head->base += size;
            }
            else {
                // If all space is used up, free the node.
                if ( prev ) {
                    prev->next = head->next;
                }

                kfree(head);
            }

            return ptr;
        }

        prev = head;
        head = head->next;
    }

    return NULL;
}

// The VASA system will take up from the end of the kernel to the start of the page_table_base
void vasa_init(void* start) {
    unsigned long total_space = ((uintptr_t) page_table_base) - (uintptr_t) start;

    vasa_node_t* type_head = (vasa_node_t*) kmalloc(sizeof(vasa_node_t));
    type_head->next = NULL;
    type_head->base = start;
    type_head->length = total_space;

    global_asa.free_head = type_head;
    global_asa.used_head = NULL;
}
