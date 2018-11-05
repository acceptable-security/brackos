#include <stdint.h>

void* large_mem_alloc(uintptr_t amount);
int large_mem_dealloc(void* ptr);
void large_mem_init();