#define MMAP_USER       1 << 0  // User memory
#define MMAP_RW         1 << 1  // Read/write memory
#define MMAP_URGENT     1 << 2  // No on-demand paging
#define MMAP_CONTINUOUS 1 << 3  // Contious memory necessary
#define MMAP_ALLOCATED  1 << 4  // The memory has been allocated. Set by kernel.

void* memmap(void* start, unsigned long length, unsigned long flags);
void memunmap(void* start, unsigned long length);
