#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <kprint.h>

#include <net/ring.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

ring_t* ring_init(uintptr_t size) {
    ring_t* ring = (ring_t*) kmalloc(sizeof(ring_t));

    if ( ring == NULL ) {
        return NULL;
    }

    memset(ring, 0, sizeof(ring_t));

    ring->len = size;
    ring->base = (uintptr_t) kmalloc(size);

    if ( (void*) ring->base == NULL ) {
        ring_free(ring);
        return NULL;
    }

    ring->read_index = 0;
    ring->write_index = 0;

    memset((void*) ring->base, 0, size);

    return ring; 
}

static uintptr_t ring_writable_size(ring_t* ring) {
    if ( ring->read_index < ring->write_index ) {
        // Write to the end of buffer, then loop around to before read
        return (ring->len - ring->write_index) +
               (ring->read_index - 1);
    }
    else if ( ring->write_index < ring->read_index ) {
        // Gap from write to before the read
        return (ring->read_index - ring->write_index - 1);
    }
    else {
        // TODO - no space or all space?
        return ring->len;
    }
}

int ring_write(ring_t* ring, char* packet, uintptr_t len) {
    if ( len == 0 ) {
        kprintf("ring: can't handle 0 length packet.\n");
        return 0;
    }

    if ( len % sizeof(uintptr_t ) != 0 ) {
        kprintf("ring: length must be pointer aligned\n");
        return 0;
    }

    // Have storage space for size + data, and align to size of the
    // size field so we don't have any sizes split up at the end of
    // the buffer.
    uintptr_t write_size = len + sizeof(uintptr_t);

    if ( write_size > ring->len ) {
        kprintf("ring: not enough space for %d byte packet (buf is %d)\n\n", len, ring->len);
        return 0;
    }

    uintptr_t writable_size = ring_writable_size(ring);

    if ( write_size > writable_size ) {
        kprintf("ring: %d bytes would overflow %d writable space\n", write_size, writable_size);
        return 0;
    }

    // Calculate the pointer to the write location and copy
    // the length field into the buffer.
    uintptr_t* write_ptr = (uintptr_t*) (ring->base + ring->write_index);
    *write_ptr = len;

    // Move past the length field (ugly pointer arith here)
    write_ptr = write_ptr + 1;
    ring->write_index = (ring->write_index + sizeof(uintptr_t)) % ring->len;

    if ( ring->write_index < ring->read_index ) {
        // Get lengths for beginning and end of buffer writes
        uintptr_t end_len = ring->len - ring->write_index;
        uintptr_t start_len = len - end_len;

        // Copy to the end
        memcpy(write_ptr, packet, end_len);

        // Write the beginning section
        packet = (void*) ((uintptr_t) packet + end_len);
        memcpy((void*) ring->base, packet, start_len);

        ring->write_index = start_len;
    }
    else {
        // Just copy in
        memcpy(write_ptr, packet, len);
        ring->write_index = (ring->write_index + len) % ring->len;
    }

    return 1;
}

static uintptr_t ring_next_size(ring_t* ring) {
    if ( ring->read_index == ring->write_index ) {
        return 0;
    }

    uintptr_t* len_ptr = (uintptr_t*) (ring->base + ring->read_index);
    return *len_ptr;
}

uintptr_t ring_read(ring_t* ring, char* buff, uintptr_t len) {
    uintptr_t next_size = ring_next_size(ring);

    // No more data to be read
    if ( next_size == 0 ) {
        return 0;
    } 

    // Must read the next packet
    if ( len < next_size ) {
        return 0;
    }

    ring->read_index = (ring->read_index + sizeof(uintptr_t)) % ring->len; 

    if ( ring->read_index + next_size > ring->len ) {
        uintptr_t end_len = ring->len - next_size;
        uintptr_t start_len = len - end_len;

        memcpy(buff, (void*) (ring->base + ring->read_index), end_len);
        memcpy(buff, (void*) (ring->base), start_len);

        ring->read_index = start_len;
    }
    else {
        memcpy(buff, (void*) (ring->base + ring->read_index), len);
        ring->read_index = (ring->read_index + len) % ring->len;
    }

    return next_size;
}

void ring_free(ring_t* ring) {
    if ( (void*) ring->base != NULL ) {
        kfree((void*) ring->base);
        ring->base = 0;
    }

    kfree(ring);
}