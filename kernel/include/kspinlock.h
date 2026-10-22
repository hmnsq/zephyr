/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#define ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <stdbool.h>

ബാഹ്യ ഘടന k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#എങ്കിൽ (CONFIG_MP_MAX_അക്കം_CPUS == 1)
#define LOCK_SCHED_SPINLOCK
#അല്ലെങ്കിൽ
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
സ്ഥിതം ALWAYS_INLINE bool z_is_sched_spinlock(ഘടന k_spinlock *lock)
{
	തിരികെ (lock == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_sched_spinlock_transfer_owner(ഘടന k_thread *thread)
{
#ifdef CONFIG_SPIN_VALIDATE
	z_spin_lock_transfer_owner(&_sched_spinlock, thread);
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
#endif
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_sched_spinlock_release(ശൂന്യം)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
സ്ഥിതം ALWAYS_INLINE k_spinlock_താക്കോൽ_t z_sched_spinlock_lock(ശൂന്യം)
{
	തിരികെ k_spin_lock(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
സ്ഥിതം ALWAYS_INLINE ശൂന്യം z_sched_spinlock_unlock(k_spinlock_താക്കോൽ_t താക്കോൽ)
{
	k_spin_unlock(&_sched_spinlock, താക്കോൽ);
}

#endif /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
