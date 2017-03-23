#define MMAP_USER       1 << 0  // User memory
#define MMAP_RW         1 << 1  // Read/write memory
#define MMAP_URGENT     1 << 2  // No on-demand paging
#define MMAP_CONTINOUS  1 << 3  // Contious memory necessary

void* memmap(void* start, unsigned long length, unsigned long flags);
