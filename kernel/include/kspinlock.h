/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#شامل <zephyr/spinlock.h>
#شامل <stdbool.h>

خارجی ساخت k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#اگر (CONFIG_MP_MAX_عدد_CPUS == 1)
#تعریف_کرو تالا_SCHED_SPINLOCK
#ورنہ
#تعریف_کرو تالا_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#اگر_ختم

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#تعریف_کرو Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
ساکن ALWAYS_INLINE bool z_is_sched_spinlock(ساخت k_spinlock *تالا)
{
	واپس (تالا == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
ساکن ALWAYS_INLINE خالی z_sched_spinlock_transfer_owner(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SPIN_VALIDATE
	z_spin_تالا_transfer_owner(&_sched_spinlock, thread);
#ورنہ
	ARG_غیر_مستعمل(thread);
#اگر_ختم
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
ساکن ALWAYS_INLINE خالی z_sched_spinlock_release(خالی)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
ساکن ALWAYS_INLINE k_spinlock_کلید_t z_sched_spinlock_تالا(خالی)
{
	واپس k_spin_تالا(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
ساکن ALWAYS_INLINE خالی z_sched_spinlock_unlock(k_spinlock_کلید_t کلید)
{
	k_spin_unlock(&_sched_spinlock, کلید);
}

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
