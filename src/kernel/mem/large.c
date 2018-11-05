#include <arch/i386/cas.h>
#include <mem/large.h>
#include <mem/mmap.h>
#include <mem/frame.h>
#include <stdlib.h>
#include <string.h>
#include <kprint.h>

// Large Memory Allocation Structure
struct large_mem_s;
typedef struct large_mem_s large_mem_t;

struct large_mem_s {
	large_mem_t* next;

	void* base;
	uintptr_t length;
};

// Global Large Alloc
static large_mem_t* g_lmem;

// Atomically prepend lmem
void large_mem_add_block(large_mem_t* lmem) {
	while ( 1 ) {
		large_mem_t* old = g_lmem;
		lmem->next = old;

		if ( cas(&g_lmem, old, lmem) ) {
			break;
		}		
	}
}

// Atomically remove lmem
int large_mem_rem_block(large_mem_t* lmem, large_mem_t* prev) {
	if ( prev == NULL ) {
		if ( cas(&g_lmem, lmem, lmem->next) ) {
			return 0;
		}
	}
	else {
		if ( prev->next != lmem ) {
			return 0;
		}

		if ( cas(&prev->next, lmem, lmem->next) ) {
			return 0;
		}
	}

	return 1;
}

void* large_mem_alloc(uintptr_t amount) {
	// Hopefully this doesn't get optimized out :)
	volatile uintptr_t pages = (amount * PAGE_SIZE) / PAGE_SIZE;

	if ( pages < 1 ) {
		kprintf("lmem: can't alloc 0 pages\n");
		return NULL;
	}

	void* addr = memmap(NULL, pages, MMAP_RW | MMAP_URGENT);

	if ( addr == NULL ) {
		return NULL;
	}

	large_mem_t* lmem = (large_mem_t*) kmalloc(sizeof(large_mem_t));

	if ( lmem == NULL ) {
		memunmap(addr, pages);
		return NULL;
	}

	lmem->base = addr;
	lmem->length = amount;

	large_mem_add_block(lmem);

	return addr;
}

int large_mem_dealloc(void* ptr) {
	large_mem_t* prev = NULL;
	large_mem_t* curr = g_lmem;

	while ( curr != NULL ) {
		if ( curr->base == ptr ) {
			break;
		}

		prev = NULL;
		curr = curr->next;
	}

	if ( curr == NULL ) {
		return 0;
	}

	if ( large_mem_rem_block(curr, prev) == 0 ) {
		// TODO - panic?
		return 0;
	}

	return 1;
}

void large_mem_init() {
	g_lmem = NULL;
	kprintf("lmem: init\n");
}