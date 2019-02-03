#include <mem/kmalloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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

char* strdup(const char* str) {
    size_t len = strlen(str);
    char* data = (char*) kmalloc(len + 1);

    if ( data == NULL ) {
        return NULL;
    }

    memcpy(data, str, len + 1);

    return data;
}

ssize_t strcmp(const char* a, const char* b) {
    while ( *a && *a == *b ) {
        a++;
        b++;
    }

    return *a - *b;
}

void strcpy(const char* a, const char* b) {
    memcpy((void*) a, (void*) b, strlen(b) + 1);
}

void* memcpy(void* dest, const void* src, unsigned long count) {
    for ( int i = 0; i < count; i++ ) {
        *(unsigned char*) dest = *(unsigned char*) src;

        dest++;
        src++;
    }

    return dest;
}

void* memset(void* dest, unsigned char c, unsigned long count) {
    unsigned char* _dest = (unsigned char*) dest;

    for ( int i = 0; i < count; i++ ) {
        _dest[i] = c;
    }

    return dest;
}

void* memsetw(void* dest, unsigned short c, unsigned long count) {
    unsigned short* _dest = (unsigned short*) dest;

    for ( int i = 0; i < count; i++ ) {
        _dest[i] = c;
    }

    return dest;
}
