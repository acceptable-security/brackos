#include <mem/frame.h>
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
	return (lower + size) == upper;
}

// Add a free chunk into a certain free list
void frame_add_free_item(uintptr_t address, size_t count, bool new_item) {
	if ( count > MAX_FRAMES ) {
		kprintf("%d > %d\n", count, MAX_FRAMES);
		return;
	}


	frame_node_t* head = buddy_alloc->free_lists[count - 1];

	if ( !new_item && head != 0) {
		// Old address with a nonempty list, lets find some buddies!
		frame_node_t* prev = 0;

		// Kind of ugly but w\e
		while ( true ) {
			// Check for buddies
			if ( count < MAX_FRAMES && frame_buddy_check(head->address, address, count * PAGE_SIZE) ) {
				// We found buddies
				// Remove this item from it's old list
				if ( prev != 0 ) {
					prev->next = head->next;
				}
				else {
					buddy_alloc->free_lists[count - 1] = head->next;
				}

				kfree(head);
				frame_add_free_item(min(head->address, address), count + 1, false);
				return;
			}

			if ( head->next == 0 ) {
				// We reached the end of the list, add it here.
                frame_node_t* next = kmalloc(sizeof(frame_node_t));

				next->address = address;
                next->next = 0;

                head->next = next;
				return;
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
		buddy_alloc->free_lists[count - 1] = next;
	}
}

// Allocate a chunk from a buddy allocator
void* frame_alloc(size_t want_count) {
	// Not enough frames for this request;
	if ( want_count < 1 && want_count >= MAX_FRAMES ) {
		return NULL;
	}

	// Find the smallest frame count with space
	for ( int count = want_count; count <= MAX_FRAMES; count++ ) {
		if ( buddy_alloc->free_lists[count - 1] != NULL ) {
			// Pop the head
            frame_node_t* next = buddy_alloc->free_lists[count - 1];
			uintptr_t address = next->address;
			buddy_alloc->free_lists[count - 1] = next->next;

			// Try to return it's buddies
			// Keep halving the found size, returning the second parts of the halves into
			// the free list as we go, until we find the right size.
			while ( count > want_count ) {
				count--;
				frame_add_free_item(address + (count * PAGE_SIZE), count, true);
			}

			return (void*) address;
		}
	}

	// Didin't find anything :(
	return NULL;
}

// Return a chunk of memory back into the buddy allocator
//   address - starting address of the allocation
//   count   - amount of frames to return
void frame_dealloc(void* address, size_t count) {
	frame_add_free_item((uintptr_t) address, count, false);
}

// Prints out the current status of the allocator
void frame_status() {
	for ( int count = 0; count < MAX_FRAMES; count++ ) {

		frame_node_t* head = buddy_alloc->free_lists[count];

		if ( head != 0 ) {
			kprintf("Count %d\n", count + 1);

			while ( head != 0 ) {
				kprintf("| %p\n", head->address);
				head = head->next;
			}
		}
	}
}

void frame_add_chunk(uintptr_t address, size_t size) {
	// If we don't have enough space for a single frame, ignore.
	if ( size <= PAGE_SIZE ) {
		return;
	}

	// TODO - not this
	buddy_alloc->base = address;

	// Carve out as many chunks of space as possible.
	while ( size >= PAGE_SIZE ) {
		for ( int count = MAX_FRAMES; count >= 1; count-- ) {
			size_t byte_count = count * PAGE_SIZE;

			// Find the largest frame count to cut out of the size.
			if ( size >= byte_count ) {
				frame_add_free_item(address, count, true);

				address += byte_count;
				size -= byte_count;
				break;
			}
		}
	}
}

// Initialize a buddy allocator
void frame_init() {
	// Allocate a header.
	buddy_alloc = (frame_alloc_t*) kmalloc(sizeof(frame_alloc_t));

	// Setup the header
	memset(buddy_alloc, 0, sizeof(frame_alloc_t));
}
