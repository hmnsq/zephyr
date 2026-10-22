/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#परिभाषय ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <stdbool.h>

बाह्य संरचना k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#यदि (CONFIG_MP_MAX_अङ्कः_CPUS == 1)
#परिभाषय तालकम्_SCHED_SPINLOCK
#अन्यथा
#परिभाषय तालकम्_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#यदि_अन्तः

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#परिभाषय Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
स्थिर ALWAYS_INLINE bool z_is_sched_spinlock(संरचना k_spinlock *तालकम्)
{
	प्रत्यागम (तालकम् == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
स्थिर ALWAYS_INLINE शून्य z_sched_spinlock_transfer_owner(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SPIN_VALIDATE
	z_spin_तालकम्_transfer_owner(&_sched_spinlock, thread);
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
#यदि_अन्तः
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
स्थिर ALWAYS_INLINE शून्य z_sched_spinlock_release(शून्य)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
स्थिर ALWAYS_INLINE k_spinlock_कुञ्चिका_t z_sched_spinlock_तालकम्(शून्य)
{
	प्रत्यागम k_spin_तालकम्(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
स्थिर ALWAYS_INLINE शून्य z_sched_spinlock_unlock(k_spinlock_कुञ्चिका_t कुञ्चिका)
{
	k_spin_unlock(&_sched_spinlock, कुञ्चिका);
}

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
