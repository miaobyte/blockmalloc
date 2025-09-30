#include "block_malloc/spinlock.h"

#if defined(_MSC_VER)
#include <windows.h>
#include <intrin.h>
#endif

/* architecture-specific CPU-relax / pause */
#if defined(__x86_64__) || defined(__i386__)
static inline void cpu_relax(void) { __asm__ __volatile__("pause" ::: "memory"); }
#elif defined(__aarch64__) || defined(__arm__)
static inline void cpu_relax(void) { __asm__ __volatile__("yield" ::: "memory"); }
#elif defined(_MSC_VER)
static inline void cpu_relax(void) { YieldProcessor(); }
#else
static inline void cpu_relax(void) { (void)0; }
#endif

/* atomic exchange / store helpers (platform specific) */
#ifdef _MSC_VER
static inline LONG64 spin_atomic_exchange(spinlock_t *ptr, LONG64 val)
{
    return InterlockedExchange64((volatile LONG64 *)ptr, val);
}
static inline void spin_atomic_store(spinlock_t *ptr, LONG64 val)
{
    /* Use InterlockedExchange64 as a store (returns previous, ignore it) */
    InterlockedExchange64((volatile LONG64 *)ptr, val);
}
#else
static inline int64_t spin_atomic_exchange(spinlock_t *ptr, int64_t val)
{
    return __atomic_exchange_n((int64_t *)ptr, val, __ATOMIC_ACQ_REL);
}
static inline void spin_atomic_store(spinlock_t *ptr, int64_t val)
{
    __atomic_store_n((int64_t *)ptr, val, __ATOMIC_RELEASE);
}
#endif

void spin_lock(spinlock_t *lock)
{
    while (spin_atomic_exchange(lock, 1))
    {
        cpu_relax();
    }
}

void spin_unlock(spinlock_t *lock)
{
    spin_atomic_store(lock, 0);
}