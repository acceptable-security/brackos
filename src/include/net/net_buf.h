#include <stdint.h>

typedef struct {
    // Whole buffer
    uintptr_t base;
    uintptr_t len;

    // Individual packets
    uintptr_t read_index;
    uintptr_t write_index;
} net_buf_t;

net_buf_t* net_buf_init(uintptr_t size);
int net_buf_write(net_buf_t* net_buf, char* packet, uintptr_t len);
uintptr_t net_buf_read(net_buf_t* net_buf, char* buff, uintptr_t len);
void net_buf_free(net_buf_t* net_buf);
