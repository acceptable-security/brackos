#include <stdint.h>
#include <stdbool.h>

unsigned long strlen(const char* str);
unsigned long strnlen(const char* str, unsigned long n);
ssize_t strcmp(const char* a, const char* b);
void strcpy(const char* a, const char* b);
char* strdup(const char* a);

void* memcpy(void* dest, const void* src, unsigned long count);
void* memset(void* dest, unsigned char c, unsigned long count);
void* memsetw(void* dest, unsigned short c, unsigned long count);
