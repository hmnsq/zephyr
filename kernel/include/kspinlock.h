/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>

ਬਾਹਰੀ ਬਣਤਰ k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#ਜੇ (CONFIG_MP_MAX_ਅੰਕ_CPUS == 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਤਾਲਾ_SCHED_SPINLOCK
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਤਾਲਾ_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#ਜੇ_ਅੰਤ

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
ਸਥਿਰ ALWAYS_INLINE bool z_is_sched_spinlock(ਬਣਤਰ k_spinlock *ਤਾਲਾ)
{
	ਵਾਪਸ (ਤਾਲਾ == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_sched_spinlock_transfer_owner(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SPIN_VALIDATE
	z_spin_ਤਾਲਾ_transfer_owner(&_sched_spinlock, thread);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਅੰਤ
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_sched_spinlock_release(ਖਾਲੀ)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
ਸਥਿਰ ALWAYS_INLINE k_spinlock_ਕੁੰਜੀ_t z_sched_spinlock_ਤਾਲਾ(ਖਾਲੀ)
{
	ਵਾਪਸ k_spin_ਤਾਲਾ(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ z_sched_spinlock_unlock(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ)
{
	k_spin_unlock(&_sched_spinlock, ਕੁੰਜੀ);
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
