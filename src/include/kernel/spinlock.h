#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <stdint.h>
typedef uint32_t* spinlock_t;

extern void spinlock_lock(spinlock_t lock);
extern void spinlock_unlock(spinlock_t lock);

#endif
