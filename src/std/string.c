#include <stdbool.h>
#include <stdint.h>

unsigned long strlen(const char* str) {
    unsigned long i = 0;

    while ( *str ) {
        i++;
        str++;
    }

    return i;
}

unsigned long strnlen(const char* str, unsigned long n) {
    unsigned long i = 0;

    while ( *str ) {
        i++;
        str++;

        if ( i >= n ) {
            return n;
        }
    }

    return i;
}

bool strcmp(const char* a, const char* b) {
    int len = strlen(a);

    if ( !a || !b || len != strlen(b) ) {
        return false;
    }

    for ( int i = 0; i < len; i++ ) {
        if ( a[i] != b[i] ) {
            return false;
        }
    }

    return true;
}

void* memcpy(void* dest, const void* src, unsigned long count) {
    for ( int i = 0; i < count; i++ ) {
        *(unsigned char*) dest = *(unsigned char*) src;

        dest++;
        src++;
    }

    return dest;
}

void* memset(void* dest, unsigned char c, unsigned int count) {
    unsigned char* _dest = (unsigned char*) dest;

    for ( int i = 0; i < count; i++ ) {
        _dest[i] = c;
    }

    return dest;
}

void* memsetw(void* dest, unsigned short c, unsigned int count) {
    unsigned short* _dest = (unsigned short*) dest;

    for ( int i = 0; i < count; i++ ) {
        _dest[i] = c;
    }

    return dest;
}
