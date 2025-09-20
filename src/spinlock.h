#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

// 条件编译：MSVC 使用 Windows 原子操作，否则使用 C11 _Atomic
#ifdef _MSC_VER
#include <windows.h>  // for InterlockedExchange64
typedef volatile LONG64 spinlock_t;  // 使用 Windows 的 LONG64（64-bit 原子类型）
#define SPINLOCK_EXCHANGE(ptr, val) InterlockedExchange64((ptr), (val))
#define SPINLOCK_STORE(ptr, val) InterlockedExchange64((ptr), (val))  // 简化实现，实际为交换
#else
#include <stdatomic.h>
typedef _Atomic int64_t spinlock_t;
#define SPINLOCK_EXCHANGE(ptr, val) atomic_exchange((ptr), (val))
#define SPINLOCK_STORE(ptr, val) atomic_store((ptr), (val))
#endif

// 自旋锁函数声明
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif // SPINLOCK_H