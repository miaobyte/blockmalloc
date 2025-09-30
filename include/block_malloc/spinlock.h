#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

#ifdef _MSC_VER
#include <windows.h>
typedef volatile LONG64 spinlock_t;
#else
typedef volatile int64_t spinlock_t;
#endif

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif // SPINLOCK_H