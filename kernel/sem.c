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

#അന്തര്ഭാവ <zephyr/kernel.h>

#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <zephyr/sys/dlist.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <zephyr/init.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/tracing/tracing.h>
#അന്തര്ഭാവ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#എങ്കിൽ defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
സ്ഥിതം ഘടന k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#അല്ലെങ്കിൽ
സ്ഥിതം ഘടന k_spinlock sem_lock;
#endif

സ്ഥിതം വരിയിൽ ഘടന k_spinlock *sem_spinlock_get(ഘടന k_sem *sem)
{
#എങ്കിൽ defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	തിരികെ &sem_locks[((uintptr_t)sem / __alignof(ഘടന k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#അല്ലെങ്കിൽ
	ARG_UNUSED(sem);
	തിരികെ &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_SEM */

പൂർണ്ണസംഖ്യ z_impl_k_sem_init(ഘടന k_sem *sem, unsigned പൂർണ്ണസംഖ്യ initial_എണ്ണം,
		      unsigned പൂർണ്ണസംഖ്യ limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_എണ്ണം > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		തിരികെ -EINVAL;
	}

	sem->എണ്ണം = initial_എണ്ണം;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#എങ്കിൽ defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	തിരികെ 0;
}

#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_sem_init(ഘടന k_sem *sem, unsigned പൂർണ്ണസംഖ്യ initial_എണ്ണം,
		      unsigned പൂർണ്ണസംഖ്യ limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	തിരികെ z_impl_k_sem_init(sem, initial_എണ്ണം, limit);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

സ്ഥിതം വരിയിൽ bool sem_handle_poll_events(ഘടന k_sem *sem)
{
#ifdef CONFIG_POLL
	തിരികെ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#അല്ലെങ്കിൽ
	ARG_UNUSED(sem);
	തിരികെ false;
#endif /* CONFIG_POLL */
}

ശൂന്യം z_impl_k_sem_give(ഘടന k_sem *sem)
{
	ഘടന k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	എങ്കിൽ (z_sched_wake(&sem->wait_q, 0, ശൂന്യ)) {
		resched = true;
	} അല്ലെങ്കിൽ {
		sem->എണ്ണം += (sem->എണ്ണം != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	എങ്കിൽ (unlikely(resched)) {
		z_reschedule(lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(lock, താക്കോൽ);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_sem_give(ഘടന k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_sem_take(ഘടന k_sem *sem, k_timeout_t timeout)
{
	ഘടന k_spinlock *lock = sem_spinlock_get(sem);
	പൂർണ്ണസംഖ്യ ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	എങ്കിൽ (likely(sem->എണ്ണം > 0U)) {
		sem->എണ്ണം--;
		k_spin_unlock(lock, താക്കോൽ);
		ret = 0;
		പോകുക out;
	}

	എങ്കിൽ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, താക്കോൽ);
		ret = -EBUSY;
		പോകുക out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, താക്കോൽ, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	തിരികെ ret;
}

ശൂന്യം z_impl_k_sem_reset(ഘടന k_sem *sem)
{
	ഘടന k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(lock);
	bool resched = false;

	ആകുമ്പോൾ (z_sched_wake(&sem->wait_q, -EAGAIN, ശൂന്യ)) {
		resched = true;
	}
	sem->എണ്ണം = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	എങ്കിൽ (resched) {
		z_reschedule(lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(lock, താക്കോൽ);
	}
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ z_vrfy_k_sem_take(ഘടന k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	തിരികെ z_impl_k_sem_take(sem, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sem_take_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_sem_reset(ഘടന k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sem_reset_mrsh.c>

സ്ഥിതം വരിയിൽ unsigned പൂർണ്ണസംഖ്യ z_vrfy_k_sem_എണ്ണം_get(ഘടന k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	തിരികെ z_impl_k_sem_എണ്ണം_get(sem);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
