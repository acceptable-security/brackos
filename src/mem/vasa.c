// Virtual Address Space Allocator.

#include <mem/vasa.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <kprint.h>

vasa_t global_asa;

bool vasa_mark(uintptr_t base, unsigned long length, bool used) {
    // Make sure everything is merged so we odn't have any misproper finds.
    vasa_merge(used);

    vasa_node_t* prev = NULL;
    vasa_node_t* head;

    if ( used ) {
        head = global_asa.used_head;
    }
    else {
        head = global_asa.free_head;
    }


    uintptr_t end = base + length;

    while ( head != NULL ) {
        uintptr_t their_base = (uintptr_t) head->base;
        uintptr_t their_end = their_base + head->length;

        if ( base >= their_base && end <= their_end ) {
            if ( base == their_base && end == their_end ) {
                // We found an exact size, remove from the list.
                prev->next = head->next;
                kfree(head);

                return true;
            }
            else {
                // Cut the new chunk out of the chunk we found.
                head->length = base - their_base;

                vasa_node_t* new_node = (vasa_node_t*) kmalloc(sizeof(vasa_node_t));
                new_node->base = (void*) end;
                new_node->length = their_end - end;
                new_node->next = head->next;
                head->next = new_node;

                return true;
            }
        }

        prev = head;
        head = head->next;
    }

    return false;
}

// Add a node to the linked lists.
void vasa_add_node(vasa_node_t* node, bool used) {
    if ( node == NULL ) {
        // TODO - no node
        return;
    }

    vasa_node_t* prev = NULL;
    vasa_node_t* head = used ? global_asa.used_head : global_asa.free_head;

    while ( head ) {
        // Add the node into the first place before where the head's base is greater than us.
        if ( (uintptr_t) head->base > (uintptr_t) node->base ) {
            if ( prev ) {
                // Add it inline
                prev->next = node;
                node->next = head;
            }
            else {
                // Nothing before, add it in the beginning
                node->next = head;

                if ( used ) {
                    global_asa.used_head = node;
                }
                else {
                    global_asa.free_head = node;
                }
            }


            return;
        }

        prev = head;
        head = head->next;
    }

    if ( head == NULL ) {
        // No head, just add it in.
        if ( used ) {
            global_asa.used_head = node;
        }
        else {
            global_asa.free_head = node;
        }
    }
    else {
        // Ran to the end, add to the end.
        head->next = node;
    }
}

// Take the free or used lists and merge the chunks of memory if they're continuous
void vasa_merge(bool used) {
    vasa_node_t* head;

    if ( used ) {
        head = global_asa.used_head;
    }
    else {
        head = global_asa.free_head;
    }

    while ( head != NULL && head->next != NULL ) {
        // Is the node continuous?
        if ( head->base + head->length == head->next->base ) {
            // Merge the nodes and correct the linked list.
            vasa_node_t* mergeable = head->next;
            head->length += head->next->length;
            head->next = head->next->next;
            kfree(mergeable);

            continue;
        }

        head = head->next;
    }
}

// Print the current state of the global VASA
void vasa_print_state() {
    kprintf("free list:");

    vasa_node_t* head = global_asa.free_head;

    if ( head ) {
        kprintf("\n")
        ;
        while ( head != NULL ) {
            kprintf(" %p (%d)\n", head->base, head->length);

            head = head->next;
        }
    }
    else {
        kprintf(" empty\n");
    }

    head = global_asa.used_head;
    kprintf("used list: ");

    if ( head ) {
        kprintf("\n")
        ;
        while ( head != NULL ) {
            kprintf(" %p (%d)\n", head->base, head->length);

            head = head->next;
        }
    }
    else {
        kprintf(" empty\n");
    }
}

// Deallocate a virtual address space ptr.
void vasa_dealloc(void* ptr) {
    vasa_node_t* prev = NULL;
    vasa_node_t* head = global_asa.used_head;

    uintptr_t iptr = (uintptr_t) ptr;

    while ( head != NULL ) {
        // Find the block of space we fall into in the used list.
        uintptr_t start = (uintptr_t) head->base;
        uintptr_t end = start + head->length;

        if ( iptr >= start && iptr <= end ) {
            // Found our free block :D. Add it to the free list C:
            if ( prev ) {
                prev->next = head->next;

            }
            else {
                global_asa.used_head = head->next;
            }

            vasa_add_node(head, false);
            return;
        }

        prev = head;
        head = head->next;
    }

    // Didn't find the pointer in the freelist, ignore it.
    // TODO - PANIC?
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
void vasa_init(void* start, unsigned long length) {
    vasa_node_t* type_head = (vasa_node_t*) kmalloc(sizeof(vasa_node_t));
    type_head->next = NULL;
    type_head->base = start;
    type_head->length = length;

    global_asa.free_head = type_head;
    global_asa.used_head = NULL;
}
