#include <stdbool.h>

#define PAGE_PRESENT  1 << 0
#define PAGE_RW       1 << 1
#define PAGE_USER     1 << 2
#define PAGE_WT       1 << 3
#define PAGE_CACHE    1 << 4
#define PAGE_ACCESSED 1 << 5
#define PAGE_RESERVED 1 << 6
#define PAGE_SIZE     1 << 7
#define PAGE_GLOBAL   1 << 8

typedef struct {
    unsigned int address        : 20;
    unsigned int empty          : 3;
    unsigned int flags          : 9;
} __attribute__((packed)) page_entry_t;

typedef struct {
    page_entry_t entries[1024];
} __attribute__((packed)) page_table_t;

typedef struct {
    page_table_t* tables[1024];
} __attribute__((packed)) page_directory_t;

bool paging_unmap(void* virt);
bool paging_map(void* physical, void* virt, unsigned short flags);

void paging_clone_table(page_table_t* source, page_table_t* target);
void paging_clone_directory(page_directory_t* source, page_directory_t* target);

void paging_init(page_directory_t* initial_pd);
