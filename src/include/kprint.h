// Primitives that will only ever be used in the kernel.
#include <stdarg.h>
#include <stdbool.h>

void kputch(char c);
void kputch_hex(char c, bool capital, bool pad);
void kputs(const char* str);
void kprint_int(long n);
void kprint_uint(unsigned long n);
void kprint_hex_uint(unsigned long n, bool capital, bool pad);
void kprintf(const char* format, ...);
