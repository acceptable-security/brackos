#include <mem/frames.h>
#include <mem/kmalloc.h>

#include <kprint.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

frame_alloc_t* buddy_alloc;

// Determines whether or not a and b are buddies of size size.
bool frame_buddy_check(uintptr_t a, uintptr_t b, size_t size) {
	uintptr_t lower = min(a, b) - buddy_alloc->base;
	uintptr_t upper = max(a, b) - buddy_alloc->base;

	// Calculate upper from lower and check.
	return (lower ^ size) == upper;
}

// Add a free chunk into a certain order
void frame_add_free_item(uintptr_t address, size_t order, bool new_item) {
	frame_node_t* head = buddy_alloc->free_lists[order - MIN_ORDER];
	size_t size = 1 << order;

	if ( !new_item && head != 0) {
		// Old address with a nonempty list, lets find some buddies!
		frame_node_t* prev = 0;

		// Kind of ugly but w\e
		while ( true ) {
			// Check for buddies
			if ( frame_buddy_check(head->address, address, size) ) {
				// We found buddies

				if ( prev != 0 ) {
					// Remove this item from it's old list
					prev->next = head->next;
				}
				else {
					buddy_alloc->free_lists[order - MIN_ORDER] = head;
				}

				frame_add_free_item(min(head->address, address), order + 1, false);
				return;
			}

			if ( head->next == 0 ) {
				// We reached the end of the list, add it here.
                frame_node_t* next = kmalloc(sizeof(frame_node_t));

                next->address = address;
                next->next = 0;
                head->next = next;
				break;
			}

			prev = head;
			head = head->next;
		}
	}
	else {
		// Fresh address, push to the head.
        frame_node_t* next = kmalloc(sizeof(frame_node_t));

        next->address = address;
        next->next = head;
		buddy_alloc->free_lists[order - MIN_ORDER] = next;
	}
}

// Allocate a chunk from a buddy allocator
void* frame_alloc(size_t size) {
	// Round up by getting highest order and calculating the size form that
	int original_order = (32 - __builtin_clz(size - 1));

	// Not enough size for this request;
	if ( original_order >= MAX_ORDER ) {
		return NULL;
	}

	size_t want_size = 1 << original_order;

	// Find the smallest order with space
	for ( int order = original_order; order < MAX_ORDER; order++ ) {
		if ( buddy_alloc->free_lists[order - MIN_ORDER] != 0 ) {
			// Pop the head
            frame_node_t* next = buddy_alloc->free_lists[order - MIN_ORDER];
			uintptr_t address = next->address;
			buddy_alloc->free_lists[order - MIN_ORDER] = next->next;

			// Try to return it's buddies
			size_t found_size = 1 << order;

			if ( found_size != want_size ) {
				// Keep halving the found size, returning the second parts of the halves into
				// the free list as we go, until we find the right size.

				while ( found_size > want_size ) {
					found_size = found_size >> 1;
					frame_add_free_item(address + found_size, 32 - __builtin_clz(found_size - 1), true);
				}
			}

			return (void*) address;
		}
	}

	// Didin't find anything :(
	return NULL;
}

// Return a chunk of memory ack into the buddy allocator
void frame_dealloc(uintptr_t address, size_t size) {
	int order = 32 - __builtin_clz(size - 1);
	frame_add_free_item(address, order, false);
}

// Prints out the current status of the allocator
void frame_status() {
	for ( int order = MIN_ORDER; order < MAX_ORDER; order++ ) {
		kprintf("Order %d\n", order);

		frame_node_t* head = buddy_alloc->free_lists[order - MIN_ORDER];

		while ( head != 0 ) {
			kprintf("| %p\n", head->address);
			head = head->next;
		}
	}
}

void frame_add_chunk(uintptr_t address, size_t size) {
	// If we don't have enough space for the smallest chunk of space, ignore.
	if ( size <= (1 << MIN_ORDER) ) {
		return;
	}

	// TODO - not this
	buddy_alloc->base = address;

	// Carve out as many chunks of space as possible.
	while ( size > (1 << MIN_ORDER) ) {
		for ( int order = MAX_ORDER - 1; order >= MIN_ORDER; order-- ) {
			// Find the largest order to cut out of the size.
			if ( size >= (1 << order) ) {
				frame_add_free_item(address, order, true);

				address += 1 << order;
				size -= 1 << order;
				break;
			}
		}
	}
}

// Initialize a buddy allocator
void frame_init() {
	// Take the first chunk of space for the header
	buddy_alloc = (frame_alloc_t*) kmalloc(sizeof(frame_alloc_t));

	// Setup the header
	memset(buddy_alloc, 0, sizeof(frame_alloc_t));
}
