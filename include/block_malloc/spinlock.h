#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

#if defined(__x86_64__) || defined(__i386__)
#define SPINLOCK_CPU_RELAX() __asm__ __volatile__("pause" ::: "memory")
#elif defined(__aarch64__) || defined(__arm__)
#define SPINLOCK_CPU_RELAX() __asm__ __volatile__("yield" ::: "memory")
#else
#define SPINLOCK_CPU_RELAX() ((void)0)
#endif

#ifdef _MSC_VER
#include <windows.h>
typedef volatile LONG64 spinlock_t;
#define SPINLOCK_EXCHANGE(ptr, val) InterlockedExchange64((ptr), (val))
#define SPINLOCK_STORE(ptr, val) InterlockedExchange64((ptr), (val))
#else
typedef volatile int64_t spinlock_t;
#define SPINLOCK_EXCHANGE(ptr, val) __atomic_exchange_n((ptr), (val), __ATOMIC_ACQ_REL)
#define SPINLOCK_STORE(ptr, val) __atomic_store_n((ptr), (val), __ATOMIC_RELEASE)
#endif

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif // SPINLOCK_H