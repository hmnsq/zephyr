/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#define ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <stdbool.h>

வெளி கட்டமைப்பு k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#எனில் (CONFIG_MP_MAX_இலக்கம்_CPUS == 1)
#define LOCK_SCHED_SPINLOCK
#இல்லையெனில்
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
நிலையான ALWAYS_INLINE bool z_is_sched_spinlock(கட்டமைப்பு k_spinlock *lock)
{
	திரும்பு (lock == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
நிலையான ALWAYS_INLINE வெறுமை z_sched_spinlock_transfer_owner(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SPIN_VALIDATE
	z_spin_lock_transfer_owner(&_sched_spinlock, thread);
#இல்லையெனில்
	ARG_UNUSED(thread);
#endif
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
நிலையான ALWAYS_INLINE வெறுமை z_sched_spinlock_release(வெறுமை)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
நிலையான ALWAYS_INLINE k_spinlock_சாவி_t z_sched_spinlock_lock(வெறுமை)
{
	திரும்பு k_spin_lock(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
நிலையான ALWAYS_INLINE வெறுமை z_sched_spinlock_unlock(k_spinlock_சாவி_t சாவி)
{
	k_spin_unlock(&_sched_spinlock, சாவி);
}

#endif /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
