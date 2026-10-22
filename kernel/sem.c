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
#ಯದಿ defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock sem_lock;
#endif

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಸಂರಚನಾ k_spinlock *sem_spinlock_get(ಸಂರಚನಾ k_sem *sem)
{
#ಯದಿ defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	ಪ್ರತ್ಯಾಗಮ &sem_locks[((uintptr_t)sem / __alignof(ಸಂರಚನಾ k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#ಅನ್ಯಥಾ
	ARG_UNUSED(sem);
	ಪ್ರತ್ಯಾಗಮ &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_SEM */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_sem_init(ಸಂರಚನಾ k_sem *sem, unsigned ಪೂರ್ಣಾಙ್ಕ initial_ಎಣಿಕೆ,
		      unsigned ಪೂರ್ಣಾಙ್ಕ limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_ಎಣಿಕೆ > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	sem->ಎಣಿಕೆ = initial_ಎಣಿಕೆ;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#ಯದಿ defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_sem_init(ಸಂರಚನಾ k_sem *sem, unsigned ಪೂರ್ಣಾಙ್ಕ initial_ಎಣಿಕೆ,
		      unsigned ಪೂರ್ಣಾಙ್ಕ limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_sem_init(sem, initial_ಎಣಿಕೆ, limit);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool sem_handle_poll_events(ಸಂರಚನಾ k_sem *sem)
{
#ifdef CONFIG_POLL
	ಪ್ರತ್ಯಾಗಮ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#ಅನ್ಯಥಾ
	ARG_UNUSED(sem);
	ಪ್ರತ್ಯಾಗಮ false;
#endif /* CONFIG_POLL */
}

ಶೂನ್ಯ z_impl_k_sem_give(ಸಂರಚನಾ k_sem *sem)
{
	ಸಂರಚನಾ k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	ಯದಿ (z_sched_wake(&sem->wait_q, 0, ರಿಕ್ತ)) {
		resched = true;
	} ಅನ್ಯಥಾ {
		sem->ಎಣಿಕೆ += (sem->ಎಣಿಕೆ != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	ಯದಿ (unlikely(resched)) {
		z_reschedule(lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(lock, ಕೀಲಿ);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_sem_give(ಸಂರಚನಾ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_sem_take(ಸಂರಚನಾ k_sem *sem, k_timeout_t timeout)
{
	ಸಂರಚನಾ k_spinlock *lock = sem_spinlock_get(sem);
	ಪೂರ್ಣಾಙ್ಕ ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	ಯದಿ (likely(sem->ಎಣಿಕೆ > 0U)) {
		sem->ಎಣಿಕೆ--;
		k_spin_unlock(lock, ಕೀಲಿ);
		ret = 0;
		ಗಚ್ಛ out;
	}

	ಯದಿ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, ಕೀಲಿ);
		ret = -EBUSY;
		ಗಚ್ಛ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, ಕೀಲಿ, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ z_impl_k_sem_reset(ಸಂರಚನಾ k_sem *sem)
{
	ಸಂರಚನಾ k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(lock);
	bool resched = false;

	ಯಾವತ್ (z_sched_wake(&sem->wait_q, -EAGAIN, ರಿಕ್ತ)) {
		resched = true;
	}
	sem->ಎಣಿಕೆ = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	ಯದಿ (resched) {
		z_reschedule(lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(lock, ಕೀಲಿ);
	}
}

#ifdef CONFIG_USERSPACE
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_sem_take(ಸಂರಚನಾ k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_sem_take(sem, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_take_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_vrfy_k_sem_reset(ಸಂರಚನಾ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_reset_mrsh.c>

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ unsigned ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_sem_ಎಣಿಕೆ_get(ಸಂರಚನಾ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_sem_ಎಣಿಕೆ_get(sem);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
