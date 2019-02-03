#include <mem/kmalloc.h>

#define NULL ((void*) 0)
#define MEMZERO(X) (memset((X), 0, sizeof(__typeof__(*(X)))))