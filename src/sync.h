#ifndef SRC_SYNC_H_
#define SRC_SYNC_H_

#include <stdatomic.h>
#include <stdbool.h>

typedef struct {
	volatile atomic_flag flag;
} SpinLock;

#define SPIN_LOCK_UNLOCKED_LIT { .flag = ATOMIC_FLAG_INIT }
#define SPIN_LOCK_UNLOCKED (SpinLock) SPIN_LOCK_UNLOCKED_LIT

// Returns true if the acquiry was successful
__attribute__((unused)) __attribute__((warn_unused_result))
inline static bool spin_lock_try_acquire(SpinLock *lock)
{
	return !atomic_flag_test_and_set(&lock->flag);
}

__attribute__((unused))
inline static void spin_lock_acquire(SpinLock *lock)
{
	while (atomic_flag_test_and_set(&lock->flag))
		;  // Busy wait
}

__attribute__((unused))
inline static void spin_lock_release(SpinLock *lock)
{
	atomic_flag_clear(&lock->flag);
}

#endif /* end of include guard: SRC_SYNC_H_ */
