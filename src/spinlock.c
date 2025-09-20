#include "spinlock.h"

// 自旋锁实现
void spin_lock(spinlock_t *lock)
{
    while (SPINLOCK_EXCHANGE(lock, 1))
    {
        // 自旋等待，直到锁被释放
    }
}

void spin_unlock(spinlock_t *lock)
{
    SPINLOCK_STORE(lock, 0); // 释放锁
}