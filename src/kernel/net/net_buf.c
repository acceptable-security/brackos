#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <kprint.h>

#include <net/net_buf.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

net_buf_t* net_buf_init(uintptr_t size) {
    net_buf_t* net_buf = (net_buf_t*) kmalloc(sizeof(net_buf_t));

    if ( net_buf == NULL ) {
        return NULL;
    }

    memset(net_buf, 0, sizeof(net_buf_t));

    net_buf->len = size;
    net_buf->base = (uintptr_t) kmalloc(size);

    if ( (void*) net_buf->base == NULL ) {
        net_buf_free(net_buf);
        return NULL;
    }

    net_buf->read_index = 0;
    net_buf->write_index = 0;

    memset((void*) net_buf->base, 0, size);

    return net_buf; 
}

static uintptr_t net_buf_writable_size(net_buf_t* net_buf) {
    if ( net_buf->read_index < net_buf->write_index ) {
        // Write to the end of buffer, then loop around to before read
        return (net_buf->len - net_buf->write_index) +
               (net_buf->read_index - 1);
    }
    else if ( net_buf->write_index < net_buf->read_index ) {
        // Gap from write to before the read
        return (net_buf->read_index - net_buf->write_index - 1);
    }
    else {
        // TODO - no space or all space?
        return net_buf->len;
    }
}

int net_buf_write(net_buf_t* net_buf, char* packet, uintptr_t len) {
    if ( len == 0 ) {
        kprintf("net_buf: can't handle 0 length packet.\n");
        return 0;
    }

    if ( len % sizeof(uintptr_t ) != 0 ) {
        kprintf("net_buf: length must be pointer aligned\n");
        return 0;
    }

    // Have storage space for size + data, and align to size of the
    // size field so we don't have any sizes split up at the end of
    // the buffer.
    uintptr_t write_size = len + sizeof(uintptr_t);

    if ( write_size > net_buf->len ) {
        kprintf("net_buf: not enough space for %d byte packet (buf is %d)\n\n", len, net_buf->len);
        return 0;
    }

    uintptr_t writable_size = net_buf_writable_size(net_buf);

    if ( write_size > writable_size ) {
        kprintf("net_buf: %d bytes would overflow %d writable space\n", write_size, writable_size);
        return 0;
    }

    // Calculate the pointer to the write location and copy
    // the length field into the buffer.
    uintptr_t* write_ptr = (uintptr_t*) (net_buf->base + net_buf->write_index);
    *write_ptr = len;

    // Move past the length field (ugly pointer arith here)
    write_ptr = write_ptr + 1;
    net_buf->write_index = (net_buf->write_index + sizeof(uintptr_t)) % net_buf->len;

    if ( net_buf->write_index < net_buf->read_index ) {
        // Get lengths for beginning and end of buffer writes
        uintptr_t end_len = net_buf->len - net_buf->write_index;
        uintptr_t start_len = len - end_len;

        // Copy to the end
        memcpy(write_ptr, packet, end_len);

        // Write the beginning section
        packet = (void*) ((uintptr_t) packet + end_len);
        memcpy((void*) net_buf->base, packet, start_len);

        net_buf->write_index = start_len;
    }
    else {
        // Just copy in
        memcpy(write_ptr, packet, len);
        net_buf->write_index = (net_buf->write_index + len) % net_buf->len;
    }

    return 1;
}

static uintptr_t net_buf_next_size(net_buf_t* net_buf) {
    if ( net_buf->read_index == net_buf->write_index ) {
        return 0;
    }

    uintptr_t* len_ptr = (uintptr_t*) (net_buf->base + net_buf->read_index);
    return *len_ptr;
}

uintptr_t net_buf_read(net_buf_t* net_buf, char* buff, uintptr_t len) {
    uintptr_t next_size = net_buf_next_size(net_buf);

    // No more data to be read
    if ( next_size == 0 ) {
        return 0;
    } 

    // Must read the next packet
    if ( len < next_size ) {
        return 0;
    }

    net_buf->read_index = (net_buf->read_index + sizeof(uintptr_t)) % net_buf->len; 

    if ( net_buf->read_index + next_size > net_buf->len ) {
        uintptr_t end_len = net_buf->len - next_size;
        uintptr_t start_len = len - end_len;

        memcpy(buff, (void*) (net_buf->base + net_buf->read_index), end_len);
        memcpy(buff, (void*) (net_buf->base), start_len);

        net_buf->read_index = start_len;
    }
    else {
        memcpy(buff, (void*) (net_buf->base + net_buf->read_index), len);
        net_buf->read_index = (net_buf->read_index + len) % net_buf->len;
    }

    return next_size;
}

void net_buf_free(net_buf_t* net_buf) {
    if ( (void*) net_buf->base != NULL ) {
        kfree((void*) net_buf->base);
        net_buf->base = 0;
    }

    kfree(net_buf);
}