// Primitives that will only ever be used in the kernel.
#include <stdarg.h>
#include <stdbool.h>

void kputch(char c);
void kputs(const char* str);
void kprintf(const char* format, ...);
