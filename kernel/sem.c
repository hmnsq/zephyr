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

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/dlist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/tracing/tracing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#ਜੇ defined(CONFIG_SMP) && (CONFIG_SEM_ਤਾਲਾ_STRIPES > 1)
ਸਥਿਰ ਬਣਤਰ k_spinlock sem_locks[CONFIG_SEM_ਤਾਲਾ_STRIPES];
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਬਣਤਰ k_spinlock sem_ਤਾਲਾ;
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਣਤਰ k_spinlock *sem_spinlock_get(ਬਣਤਰ k_sem *sem)
{
#ਜੇ defined(CONFIG_SMP) && (CONFIG_SEM_ਤਾਲਾ_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_ਦਾ_TWO(CONFIG_SEM_ਤਾਲਾ_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	ਵਾਪਸ &sem_locks[((uintptr_t)sem / __alignof(ਬਣਤਰ k_sem)) %
			  CONFIG_SEM_ਤਾਲਾ_STRIPES];
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(sem);
	ਵਾਪਸ &sem_ਤਾਲਾ;
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_SEM */

ਪੂਰਨ z_impl_k_sem_init(ਬਣਤਰ k_sem *sem, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ initial_ਗਿਣਤੀ,
		      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਹੱਦਬੰਦੀ)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(ਹੱਦਬੰਦੀ == 0U || initial_ਗਿਣਤੀ > ਹੱਦਬੰਦੀ) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		ਵਾਪਸ -EINVAL;
	}

	sem->ਗਿਣਤੀ_2 = initial_ਗਿਣਤੀ;
	sem->ਹੱਦਬੰਦੀ = ਹੱਦਬੰਦੀ;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#ਜੇ defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#ਜੇ_ਅੰਤ /* CONFIG_POLL */
	k_object_init(sem);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_SEM */

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_sem_init(ਬਣਤਰ k_sem *sem, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ initial_ਗਿਣਤੀ,
		      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਹੱਦਬੰਦੀ)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	ਵਾਪਸ z_impl_k_sem_init(sem, initial_ਗਿਣਤੀ, ਹੱਦਬੰਦੀ);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_sem_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਸਥਿਰ ਸਤਰਅੰਦਰ bool sem_handle_poll_events(ਬਣਤਰ k_sem *sem)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POLL
	ਵਾਪਸ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_ਉਪਲਬਧ);
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(sem);
	ਵਾਪਸ false;
#ਜੇ_ਅੰਤ /* CONFIG_POLL */
}

ਖਾਲੀ z_impl_k_sem_give(ਬਣਤਰ k_sem *sem)
{
	ਬਣਤਰ k_spinlock *ਤਾਲਾ = sem_spinlock_get(sem);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(ਤਾਲਾ);
	bool resched;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	ਜੇ (z_sched_wake(&sem->wait_q, 0, ਨਿਰਾ)) {
		resched = true;
	} ਨਹੀਂਤਾਂ {
		sem->ਗਿਣਤੀ_2 += (sem->ਗਿਣਤੀ_2 != sem->ਹੱਦਬੰਦੀ) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	ਜੇ (unlikely(resched)) {
		z_reschedule(ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(ਤਾਲਾ, ਕੁੰਜੀ);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_sem_give(ਬਣਤਰ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_sem_give_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_sem_take(ਬਣਤਰ k_sem *sem, k_timeout_t timeout)
{
	ਬਣਤਰ k_spinlock *ਤਾਲਾ = sem_spinlock_get(sem);
	ਪੂਰਨ ret;

	__ASSERT(((arch_is_ਵਿੱਚ_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	ਜੇ (likely(sem->ਗਿਣਤੀ_2 > 0U)) {
		sem->ਗਿਣਤੀ_2--;
		k_spin_unlock(ਤਾਲਾ, ਕੁੰਜੀ);
		ret = 0;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	ਜੇ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(ਤਾਲਾ, ਕੁੰਜੀ);
		ret = -EBUSY;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(ਤਾਲਾ, ਕੁੰਜੀ, &sem->wait_q, timeout);

ਬਾਹਰਲਾ:
	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	ਵਾਪਸ ret;
}

ਖਾਲੀ z_impl_k_sem_reset(ਬਣਤਰ k_sem *sem)
{
	ਬਣਤਰ k_spinlock *ਤਾਲਾ = sem_spinlock_get(sem);
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(ਤਾਲਾ);
	bool resched = false;

	ਜਦੋਂ (z_sched_wake(&sem->wait_q, -EAGAIN, ਨਿਰਾ)) {
		resched = true;
	}
	sem->ਗਿਣਤੀ_2 = 0;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC(k_sem, reset, sem);

	ਜੇ (resched) {
		z_reschedule(ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(ਤਾਲਾ, ਕੁੰਜੀ);
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_sem_take(ਬਣਤਰ k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ਵਾਪਸ z_impl_k_sem_take(sem, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_sem_take_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_vrfy_k_sem_reset(ਬਣਤਰ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_sem_reset_mrsh.c>

ਸਥਿਰ ਸਤਰਅੰਦਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ z_vrfy_k_sem_ਗਿਣਤੀ_get(ਬਣਤਰ k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ਵਾਪਸ z_impl_k_sem_ਗਿਣਤੀ_get(sem);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
