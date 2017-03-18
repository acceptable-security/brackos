#include <arch/i386/map.h>
#include <multiboot.h>
#include <stdint.h>
#include <kprint.h>

#define MEM_TYPE(x) ((x) == MULTIBOOT_MEMORY_AVAILABLE ? "AVAILABLE" : "RESERVED" )
extern unsigned long kernel_base;

void print_mem_map(void* _multiboot) {
    multiboot_info_t* multiboot = _multiboot;
    uintptr_t base = multiboot->mmap_addr + kernel_base;
    uintptr_t end = base + multiboot->mmap_length;

    kprintf("== GRUB Memory Map ==\n");
    kprintf("range: [%x, %x]\n", base, end);
    kprintf("Start  End   Type\n");
    for ( ; base < end; base += ((multiboot_memory_map_t*) base)->size + sizeof(int) ) {
        multiboot_memory_map_t* entry = (multiboot_memory_map_t*) base;

        kprintf("%x | %x | %s (%m)\n", entry->addr_low,
                                  entry->addr_low + entry->len_low - 1,
                                  MEM_TYPE(entry->type),
                                  entry->len_low);
    }
}
