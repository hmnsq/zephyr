/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#समावेश <zephyr/spinlock.h>
#समावेश <stdbool.h>

बाह्य काष्ठा k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#अगर (CONFIG_MP_MAX_अंक_CPUS == 1)
#घोषणा LOCK_SCHED_SPINLOCK
#अन्यथा
#घोषणा LOCK_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#पूर्ण_अगर

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#घोषणा Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
अटल ALWAYS_INLINE bool z_is_sched_spinlock(काष्ठा k_spinlock *lock)
{
	वापस (lock == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
अटल ALWAYS_INLINE व्योम z_sched_spinlock_transfer_owner(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SPIN_VALIDATE
	z_spin_lock_transfer_owner(&_sched_spinlock, thread);
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
अटल ALWAYS_INLINE व्योम z_sched_spinlock_release(व्योम)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
अटल ALWAYS_INLINE k_spinlock_कुंजी_t z_sched_spinlock_lock(व्योम)
{
	वापस k_spin_lock(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
अटल ALWAYS_INLINE व्योम z_sched_spinlock_unlock(k_spinlock_कुंजी_t कुंजी)
{
	k_spin_unlock(&_sched_spinlock, कुंजी);
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
