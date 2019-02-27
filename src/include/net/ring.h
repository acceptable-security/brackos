#include <stdint.h>

typedef struct {
    // Whole buffer
    uintptr_t base;
    uintptr_t len;

    // Individual packets
    uintptr_t read_index;
    uintptr_t write_index;
} ring_t;

ring_t* ring_init(uintptr_t size);
int ring_write(ring_t* ring, char* packet, uintptr_t len);
uintptr_t ring_read(ring_t* ring, char* buff, uintptr_t len);
void ring_free(ring_t* ring);
