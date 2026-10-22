/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#define ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>

ବାହ୍ୟ ସଂରଚନା k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#ଯଦି (CONFIG_MP_MAX_ଅଙ୍କ_CPUS == 1)
#define LOCK_SCHED_SPINLOCK
#ଅନ୍ୟଥା
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
ସ୍ଥିର ALWAYS_INLINE bool z_is_sched_spinlock(ସଂରଚନା k_spinlock *lock)
{
	ଫେରନ୍ତୁ (lock == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_sched_spinlock_transfer_owner(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SPIN_VALIDATE
	z_spin_lock_transfer_owner(&_sched_spinlock, thread);
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
#endif
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_sched_spinlock_release(ଶୂନ୍ୟ)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
ସ୍ଥିର ALWAYS_INLINE k_spinlock_ଚାବି_t z_sched_spinlock_lock(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ k_spin_lock(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ z_sched_spinlock_unlock(k_spinlock_ଚାବି_t ଚାବି)
{
	k_spin_unlock(&_sched_spinlock, ଚାବି);
}

#endif /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
