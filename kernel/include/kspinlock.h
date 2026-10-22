/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#که_نه_تعریف_شوی ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#تعریف_کړه ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#ضمن <zephyr/spinlock.h>
#ضمن <stdbool.h>

خارجي بنية k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#إذا (CONFIG_MP_MAX_عدد_CPUS == 1)
#تعریف_کړه قلف_SCHED_SPINLOCK
#وإلا
#تعریف_کړه قلف_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#د_که_پای

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#تعریف_کړه Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
ساكن ALWAYS_INLINE bool z_is_sched_spinlock(بنية k_spinlock *قلف)
{
	أرجع (قلف == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
ساكن ALWAYS_INLINE فراغ z_sched_spinlock_transfer_owner(بنية k_thread *thread)
{
#که_تعریف_شوی CONFIG_SPIN_VALIDATE
	z_spin_قلف_transfer_owner(&_sched_spinlock, thread);
#وإلا
	ARG_ناکارول_شوی(thread);
#د_که_پای
}


/**
 * @brief Release the scheduler's spinlock without restoring interrupts
 */
ساكن ALWAYS_INLINE فراغ z_sched_spinlock_release(فراغ)
{
	k_spin_release(&_sched_spinlock);
}

/**
 * @brief Lock the scheduler's spinlock
 */
ساكن ALWAYS_INLINE k_spinlock_کلی_t z_sched_spinlock_قلف(فراغ)
{
	أرجع k_spin_قلف(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
ساكن ALWAYS_INLINE فراغ z_sched_spinlock_unlock(k_spinlock_کلی_t کلی)
{
	k_spin_unlock(&_sched_spinlock, کلی);
}

#د_که_پای /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
