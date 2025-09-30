#include "block_malloc/spinlock.h"

void spin_lock(spinlock_t *lock)
{
    while (SPINLOCK_EXCHANGE(lock, 1))
    {
        SPINLOCK_CPU_RELAX();
    }
}

void spin_unlock(spinlock_t *lock)
{
    SPINLOCK_STORE(lock, 0);
}