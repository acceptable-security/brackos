// Virtual Address Space Allocator.

#include <arch/i386/vasa.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern void* page_table_base;
vasa_t global_asa;

// Add a node to the linked lists.
void vasa_add_node(vasa_node_t* node, bool used) {
    if ( node == NULL ) {
        // TODO - no node
        return;
    }

    vasa_node_t* head = used ? global_asa.used_head : global_asa.free_head;

    while ( head ) {
        if ( (uintptr_t) node->base > (uintptr_t) head->base ) {
            node->next = head->next;
            head->next = node;

            return;
        }

        head = head->next;
    }

    if ( head == NULL ) {
        if ( used ) {
            global_asa.used_head = node;
        }
        else {
            global_asa.free_head = node;
        }
    }
    else {
        head->next = node;
    }
}

// Allocate virtual address space for a given memory type
void* vasa_alloc(vasa_memtype_t type, unsigned long size) {
    vasa_node_t* prev = NULL;
    vasa_node_t* head = global_asa.free_head;

    while ( head != NULL ) {
        if ( head->length >= size ) {
            void* ptr = head->base;
            vasa_node_t* node;

            if ( head->length > size ) {
                // Cut our chunk out of the start of the node.
                head->length -= size;
                head->base += size;

                node = kmalloc(sizeof(vasa_node_t));
            }
            else {
                // If all space is used up, free the node.
                if ( prev ) {
                    prev->next = head->next;
                }

                node = head;
            }

            node->next = NULL;
            node->type = type;
            node->base = ptr;
            node->length = size;

            vasa_add_node(node, true);

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
