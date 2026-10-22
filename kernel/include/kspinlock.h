/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#define ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>

ಬಾಹ್ಯ ಸಂರಚನಾ k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#ಯದಿ (CONFIG_MP_MAX_ಅಂಕೆ_CPUS == 1)
#define LOCK_SCHED_SPINLOCK
#ಅನ್ಯಥಾ
#define LOCK_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#endif

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#define Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
ಸ್ಥಿರ ALWAYS_INLINE bool z_is_sched_spinlock(ಸಂರಚನಾ k_spinlock *lock)
{
	ಪ್ರತ್ಯಾಗಮ (lock == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_sched_spinlock_transfer_owner(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SPIN_VALIDATE
	z_spin_lock_transfer_owner(&_sched_spinlock, thread);
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
#endif
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_sched_spinlock_release(ಶೂನ್ಯ)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
ಸ್ಥಿರ ALWAYS_INLINE k_spinlock_ಕೀಲಿ_t z_sched_spinlock_lock(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ k_spin_lock(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_sched_spinlock_unlock(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	k_spin_unlock(&_sched_spinlock, ಕೀಲಿ);
}

#endif /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
