/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_

#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>

ಬಾಹ್ಯ ರಚನೆ k_spinlock _sched_spinlock;

/**
 * The intent of LOCK_SCHED_SPINLOCK is to lock the scheduler's spinlock for
 * the scope that follows (e.g. LOCK_SCHED_SPINLOCK { ... }). However, on
 * single core systems, the scheduler's spinlock is not needed when the locked
 * region would otherwise be encapsulated by another spinlock (as that spinlock
 * would degenerate into an IRQ lock). For that degenerate case, this macro
 * will expand to nothing.
 */
#ಆದರೆ (CONFIG_MP_MAX_ಅಂಕೆ_CPUS == 1)
#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಗ_SCHED_SPINLOCK
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ಬೀಗ_SCHED_SPINLOCK   K_SPINLOCK(&_sched_spinlock)
#ಆದರೆ_ಅಂತ್ಯ

/**
 * @brief Always lock the scheduler's spinlock for the scope that follows
 *
 * Unlike LOCK_SCHED_SPINLOCK, this macro will always lock the scheduler's
 * spinlock--even on single core systems.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು Z_SCHED_SPINLOCK K_SPINLOCK(&_sched_spinlock)


/**
 * @brief Check if a spinlock is the scheduler's spinlock
 *
 * @retval true if is, false otherwise
 */
ಸ್ಥಿರ ALWAYS_INLINE bool z_is_sched_spinlock(ರಚನೆ k_spinlock *ಬೀಗ)
{
	ಹಿಂತಿರುಗಿ (ಬೀಗ == &_sched_spinlock);
}

/**
 * @brief Transfer ownership of the scheduler's spinlock
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_sched_spinlock_transfer_owner(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SPIN_VALIDATE
	z_spin_ಬೀಗ_transfer_owner(&_sched_spinlock, thread);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
#ಆದರೆ_ಅಂತ್ಯ
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
ಸ್ಥಿರ ALWAYS_INLINE k_spinlock_ಕೀಲಿ_t z_sched_spinlock_ಬೀಗ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ k_spin_ಬೀಗ(&_sched_spinlock);
}

/**
 * @brief Unlock the scheduler's spinlock
 */
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ z_sched_spinlock_unlock(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ)
{
	k_spin_unlock(&_sched_spinlock, ಕೀಲಿ);
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_KSPINLOCK_H_ */
