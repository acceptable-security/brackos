#include <stdint.h>

unsigned long strlen(const char* str) {
    unsigned long i = 0;

    while ( *str ) {
        i++;
        str++;
    }

    return i;
}

void* memcpy(void* dest, const void* src, unsigned long count) {
    for ( int i = 0; i < count; i++ ) {
        *(unsigned char*) dest = *(unsigned char*) src;

        dest++;
        src++;
    }

    return dest;
}
