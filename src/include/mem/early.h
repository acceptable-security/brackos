#include <stdbool.h>

bool early_kmalloc_active();
void* early_kmalloc(unsigned long size);
void early_kmalloc_init(void* start, unsigned long size);
void* early_kmalloc_end();
