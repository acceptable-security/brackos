#define MMAP_USER       1 << 0  // User memory
#define MMAP_RW         1 << 1  // Read/write memory
#define MMAP_URGENT     1 << 2  // No on-demand paging
#define MMAP_CONTINUOUS 1 << 3  // Contious memory necessary
#define MMAP_VALLOC     1 << 4  // Virtual address space needs to be allocated.

void* memmap(void* start, unsigned long length, unsigned long flags);
