// Virtual Address Space Allocator.
#include <arch/i386/vasa.h>

#include <stdlib.h>

vasa_t global_asa;

void vasa_init() {
    for (int i = 0; i < _MEM_CAP - 1; i++ ) {
        global_asa.free_head[i] = NULL;
        global_asa.used_head[i] = NULL;
    }
}
