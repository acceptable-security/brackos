#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <stdint.h>
typedef uint32_t spinlock_t;

extern void _spinlock_lock(spinlock_t* lock);
extern void _spinlock_unlock(spinlock_t* lock);

#define spinlock_lock(LOCK) _spinlock_lock(&(LOCK));
#define spinlock_unlock(LOCK) _spinlock_unlock(&(LOCK));

#endif
