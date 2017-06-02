#include <stdbool.h>

unsigned long strlen(const char* str);
bool strcmp(const char* a, const char* b);

void* memcpy(void* dest, const void* src, unsigned long count);
void* memset(void* dest, unsigned char c, unsigned long count);
void* memsetw(void* dest, unsigned short c, unsigned long count);
