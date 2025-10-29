#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

void spin_lock(int64_t *lock);
void spin_unlock(int64_t *lock);

#endif // SPINLOCK_H