/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <stdbool.h>

బాహ్య నిర్మాణం k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#ఒకవేళ (CONFIG_MP_MAX_అంకె_CPUS == 1)
#నిర్వచించు తాళం_SCHED_SPINLOCK
#లేకపోతే
#నిర్వచించు తాళం_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#అయితే_ముగింపు

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#నిర్వచించు Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
స్థిర ALWAYS_INLINE bool z_is_sched_spinlock(నిర్మాణం k_spinlock *తాళం)
{
	తిరిగి (తాళం == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
స్థిర ALWAYS_INLINE శూన్య z_sched_spinlock_transfer_owner(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SPIN_VALIDATE
	z_spin_తాళం_transfer_owner(&_sched_spinlock, thread);
#లేకపోతే
	ARG_వాడని(thread);
#అయితే_ముగింపు
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
స్థిర ALWAYS_INLINE శూన్య z_sched_spinlock_release(శూన్య)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
స్థిర ALWAYS_INLINE k_spinlock_కీలకం_t z_sched_spinlock_తాళం(శూన్య)
{
	తిరిగి k_spin_తాళం(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
స్థిర ALWAYS_INLINE శూన్య z_sched_spinlock_unlock(k_spinlock_కీలకం_t కీలకం)
{
	k_spin_unlock(&_sched_spinlock, కీలకం);
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
