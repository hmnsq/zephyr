/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief Kernel semaphore object.
 *
 * The semaphores are of the 'counting' type, i.e. each 'give' operation will
 * increment the internal count by 1, if no thread is pending on it. The 'init'
 * call initializes the count to 'initial_count'. Following multiple 'give'
 * operations, the same number of 'take' operations can be performed without
 * the calling thread having to pend on the semaphore, or the calling task
 * having to poll.
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/dlist.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#ಆದರೆ defined(CONFIG_SMP) && (CONFIG_SEM_ಬೀಗ_STRIPES > 1)
ಸ್ಥಿರ ರಚನೆ k_spinlock sem_locks[CONFIG_SEM_ಬೀಗ_STRIPES];
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ರಚನೆ k_spinlock sem_ಬೀಗ;
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ರಚನೆ k_spinlock *sem_spinlock_get(ರಚನೆ k_sem *sem)
{
#ಆದರೆ defined(CONFIG_SMP) && (CONFIG_SEM_ಬೀಗ_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_ದ_TWO(CONFIG_SEM_ಬೀಗ_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	ಹಿಂತಿರುಗಿ &sem_locks[((uintptr_t)sem / __alignof(ರಚನೆ k_sem)) %
			  CONFIG_SEM_ಬೀಗ_STRIPES];
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(sem);
	ಹಿಂತಿರುಗಿ &sem_ಬೀಗ;
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_SEM */

ಪೂರ್ಣಾಂಕ z_impl_k_sem_init(ರಚನೆ k_sem *sem, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ initial_ಎಣಿಕೆ,
		      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(ಎಲ್ಲೆ == 0U || initial_ಎಣಿಕೆ > ಎಲ್ಲೆ) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	sem->ಎಣಿಕೆ_2 = initial_ಎಣಿಕೆ;
	sem->ಎಲ್ಲೆ = ಎಲ್ಲೆ;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#ಆದರೆ defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */
	k_object_init(sem);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_SEM */

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_sem_init(ರಚನೆ k_sem *sem, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ initial_ಎಣಿಕೆ,
		      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	ಹಿಂತಿರುಗಿ z_impl_k_sem_init(sem, initial_ಎಣಿಕೆ, ಎಲ್ಲೆ);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool sem_handle_poll_events(ರಚನೆ k_sem *sem)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POLL
	ಹಿಂತಿರುಗಿ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_ಲಭ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(sem);
	ಹಿಂತಿರುಗಿ false;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_POLL */
}

ಶೂನ್ಯ z_impl_k_sem_give(ರಚನೆ k_sem *sem)
{
	ರಚನೆ k_spinlock *ಬೀಗ = sem_spinlock_get(sem);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(ಬೀಗ);
	bool resched;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	ಆದರೆ (z_sched_wake(&sem->wait_q, 0, ಖಾಲಿ)) {
		resched = true;
	} ಇಲ್ಲದಿದ್ದರೆ {
		sem->ಎಣಿಕೆ_2 += (sem->ಎಣಿಕೆ_2 != sem->ಎಲ್ಲೆ) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	ಆದರೆ (unlikely(resched)) {
		z_reschedule(ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(ಬೀಗ, ಕೀಲಿ);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_sem_give(ರಚನೆ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_give_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_sem_take(ರಚನೆ k_sem *sem, k_timeout_t timeout)
{
	ರಚನೆ k_spinlock *ಬೀಗ = sem_spinlock_get(sem);
	ಪೂರ್ಣಾಂಕ ret;

	__ASSERT(((arch_is_ಒಳ_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	ಆದರೆ (likely(sem->ಎಣಿಕೆ_2 > 0U)) {
		sem->ಎಣಿಕೆ_2--;
		k_spin_unlock(ಬೀಗ, ಕೀಲಿ);
		ret = 0;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	ಆದರೆ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(ಬೀಗ, ಕೀಲಿ);
		ret = -EBUSY;
		ಹೋಗು ಹೊರಹೋಗು;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(ಬೀಗ, ಕೀಲಿ, &sem->wait_q, timeout);

ಹೊರಹೋಗು:
	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ z_impl_k_sem_reset(ರಚನೆ k_sem *sem)
{
	ರಚನೆ k_spinlock *ಬೀಗ = sem_spinlock_get(sem);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(ಬೀಗ);
	bool resched = false;

	ಆಗ (z_sched_wake(&sem->wait_q, -EAGAIN, ಖಾಲಿ)) {
		resched = true;
	}
	sem->ಎಣಿಕೆ_2 = 0;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC(k_sem, reset, sem);

	ಆದರೆ (resched) {
		z_reschedule(ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(ಬೀಗ, ಕೀಲಿ);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ z_vrfy_k_sem_take(ರಚನೆ k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ಹಿಂತಿರುಗಿ z_impl_k_sem_take(sem, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_take_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_sem_reset(ರಚನೆ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_reset_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ z_vrfy_k_sem_ಎಣಿಕೆ_get(ರಚನೆ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ಹಿಂತಿರುಗಿ z_impl_k_sem_ಎಣಿಕೆ_get(sem);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
