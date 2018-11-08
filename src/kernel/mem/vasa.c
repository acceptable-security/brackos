// Virtual Address Space Allocator.
#include <mem/early.h>
#include <mem/slab.h>
#include <mem/slub.h>
#include <mem/vasa.h>
#include <mem/mmap.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <kprint.h>
#include <math.h>

#define APPLY_PAD(VAL, PAD) (((VAL) + (PAD) - 1) & ~(PAD - 1))

vasa_t global_asa;

// Checks if a is directly next to b (and can be merged)
static inline bool vasa_congruent(vasa_node_t* a, vasa_node_t* b) {
    // If B is infront of A, start at A's base
    if ( (uintptr_t) b->base > (uintptr_t) a->base ) {
        return ((uintptr_t) a->base) + a->length == ((uintptr_t) b->base);
    }
    else {
        return ((uintptr_t) b->base) + b->length == ((uintptr_t) a->base);
    }
}

// Given two nodes, merge them. Second node is always freed
void vasa_merge_node(vasa_node_t* a, vasa_node_t* b) {
    // Go to the lowest base and add the lengths.
    a->base = min(a->base, b->base);
    a->length += b->length;
    kfree(b);
}

// Allocates a single vasa_node_t
vasa_node_t* vasa_node_alloc() {
    if ( early_kmalloc_active() ) {
        return (vasa_node_t*) kmalloc(sizeof(vasa_node_t));
    }
    else {
        return mem_cache_alloc("cache_vasa");
    }
}

// Add a node to the linked lists.
static void vasa_add_node(vasa_node_t* node, bool used) {
    if ( node == NULL ) {
        return;
    }

    // Clear flags on things that aren't used.
    if ( !used ) {
        node->flags = 0;
    }

    vasa_node_t* prev = NULL;
    vasa_node_t* head = used ? global_asa.used_head : global_asa.free_head;

    while ( head ) {
        // If we find a congruent chun and we're freeing, merge it.
        // Can't merge in used because not all mappings are equal
        if ( vasa_congruent(node, head) && !used) {
            vasa_merge_node(head, node);
            return;
        }

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

    if ( prev == NULL ) {
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
        prev->next = node;
        node->next = NULL;
    }
}

// Attempt to mark a specific area of memory used
bool vasa_mark(uintptr_t base, unsigned long length, bool used, unsigned long flags) {
    // Make sure everything is merged so we don't have any misproper finds.
    vasa_node_t* prev = NULL;
    vasa_node_t* head;

    // If we're setting used, we look in free, and if we're freeing, we look in used.
    if ( used ) {
        head = global_asa.free_head;
    }
    else {
        head = global_asa.used_head;
    }


    uintptr_t end = base + length;

    while ( head != NULL ) {
        uintptr_t their_base = (uintptr_t) head->base;
        uintptr_t their_end = their_base + head->length;

        // List is sorted, if we're past then we don't need to look anymore.
        if ( (uintptr_t) head->base >= their_end ) {
            break;
        }

        // If the chunk of memory is in range.
        if ( base >= their_base && end <= their_end ) {
            if ( base == their_base && end == their_end ) {
                // We found an exact size, remove from the list and add to the other side
                if ( prev ) {
                    prev->next = head->next;
                }
                else {
                    if ( used ) {
                        global_asa.used_head = head->next;
                    }
                    else {
                        global_asa.free_head = head->next;
                    }
                }

                head->flags = flags;
                vasa_add_node(head, used);

                return true;
            }
            else {
                // tb           te
                // |   b  e   |
                //     |  |
                //
                // tb - b: stays
                // b - e: other side
                // e - te: stays
                // Cut the new chunk out of the chunk we found and move it onto the other side

                head->length = base - their_base;
                // head is now s - a
                // leave it in it's current position as it's correct

                // put AB onto the other side
                vasa_node_t* be_node = vasa_node_alloc();
                be_node->base = (void*) base;
                be_node->length = length;
                be_node->flags = flags;
                vasa_add_node(be_node, used);

                // allocate b - e and keep on this side
                vasa_node_t* end_node = vasa_node_alloc();
                end_node->base = (void*) end;
                end_node->length = their_end - end;
                end_node->flags = head->flags;
                vasa_add_node(end_node, !used);

                return true;
            }
        }

        prev = head;
        head = head->next;
    }

    return false;
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

// Return the flags given a base
unsigned long vasa_get_flags(uintptr_t base) {
    vasa_node_t* head = global_asa.used_head;

    if ( head == NULL ) {
        return 0;
    }

    while ( head != NULL ) {
        kprintf("%p\n", head->base);
        if ( head->base == (void*) base ) {
            return head->flags;
        }

        head = head->next;
    }

    return 0;
}

// Deallocate a virtual address space ptr.
void vasa_dealloc(void* ptr) {
    spinlock_lock(global_asa.lock);

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
            spinlock_unlock(global_asa.lock);
            return;
        }

        prev = head;
        head = head->next;
    }

    // Didn't find the pointer in the freelist, ignore it.
    // TODO - PANIC?
    spinlock_unlock(global_asa.lock);
}

// Allocate virtual address space for a given memory type
// Iterate through the VASA free list and find the first chunk of space that has enough to give.
void* vasa_alloc(vasa_memtype_t type, unsigned long size, unsigned long flags) {
    if ( size == 0 ) {
        kprintf("vasa: cannot allocate size 0\n");
        return NULL;
    }

    spinlock_lock(global_asa.lock);

    vasa_node_t* prev = NULL;
    vasa_node_t* head = global_asa.free_head;

    // Get padding from flags
    unsigned long pad = HAS_FLAG(flags, MMAP_PG_ALIGN)  ? 0x1000 :
                        HAS_FLAG(flags, MMAP_2PG_ALIGN) ? 0x2000 :
                                                          0x0000;

    unsigned long search = size + pad;

    while ( head != NULL ) {
        if ( head->length >= search ) {
            // We found space, allocate from it.
            uintptr_t base = (uintptr_t) head->base;

            // Align and calculate waste
            void* ptr;
            unsigned long waste;
            vasa_node_t* node;
            vasa_node_t* waste_node = NULL;

            // Handle padding
            if ( pad > 0 ) {
                ptr = (void*) APPLY_PAD((uintptr_t) base, pad);
                waste = (uintptr_t) ptr - base;

                // If theres waste allocate a waste node
                if ( waste > 0 ) {
                    kprintf("vasa: allocating waste node of %d bytes\n", waste);
                    waste_node = vasa_node_alloc();

                    // Goes behind the head node
                    waste_node->next = head;
                    waste_node->type = type;
                    waste_node->base = head->base;
                    waste_node->length = waste;
                    waste_node->flags = flags;

                    head->base = (void*) base;
                    head->length = head->length - waste;                    
                }
            }
            else {
                ptr = (void*) base;
                waste = 0;
            }

            if ( head->length > size ) {
                // Cut our chunk out of the start of the node.
                head->length -= size;
                head->base += size;

                // Create a new node
                node = vasa_node_alloc();
            }
            else {
                // If all space is used up, free the node.
                if ( prev ) {
                    prev->next = head->next;
                }
                else {
                    global_asa.free_head = head->next;
                }

                // Use old node
                node = head;
            }

            // Repurpose old/setup new node by removing links
            // and setting eveything up
            node->next = NULL;
            node->type = type;
            node->base = ptr;
            node->length = size;
            node->flags = flags;

            vasa_add_node(node, true);

            if ( waste_node != NULL ) {
                vasa_add_node(waste_node, false);
            }

            spinlock_unlock(global_asa.lock);

            return ptr;
        }

        prev = head;
        head = head->next;
    }

    spinlock_unlock(global_asa.lock);
    return NULL;
}

// The VASA system will take up from the end of the kernel to the start of the page_table_base
void vasa_init(void* start, unsigned long length) {
    vasa_node_t* type_head = vasa_node_alloc();
    type_head->next = NULL;
    type_head->base = start;
    type_head->length = length;

    global_asa.free_head = type_head;
    global_asa.used_head = NULL;
}

// Switch the VASA to using a cache
void vasa_switch() {
    if ( mem_cache_new("cache_vasa", sizeof(vasa_node_t), 1, NULL, NULL) != NULL ) {
        kprintf("vasa: initiated the cache_vasa\n");
    }
}
