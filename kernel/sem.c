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

#அந்தர்பாவ <zephyr/kernel.h>

#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <zephyr/sys/dlist.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <zephyr/init.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/tracing/tracing.h>
#அந்தர்பாவ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#எனில் defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
நிலையான கட்டமைப்பு k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#இல்லையெனில்
நிலையான கட்டமைப்பு k_spinlock sem_lock;
#endif

நிலையான உள்வரி கட்டமைப்பு k_spinlock *sem_spinlock_get(கட்டமைப்பு k_sem *sem)
{
#எனில் defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	திரும்பு &sem_locks[((uintptr_t)sem / __alignof(கட்டமைப்பு k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#இல்லையெனில்
	ARG_UNUSED(sem);
	திரும்பு &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_SEM */

முழு z_impl_k_sem_init(கட்டமைப்பு k_sem *sem, unsigned முழு initial_மொத்தம்,
		      unsigned முழு limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_மொத்தம் > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		திரும்பு -EINVAL;
	}

	sem->மொத்தம் = initial_மொத்தம்;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#எனில் defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	திரும்பு 0;
}

#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_sem_init(கட்டமைப்பு k_sem *sem, unsigned முழு initial_மொத்தம்,
		      unsigned முழு limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	திரும்பு z_impl_k_sem_init(sem, initial_மொத்தம், limit);
}
#அந்தர்பாவ <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

நிலையான உள்வரி bool sem_handle_poll_events(கட்டமைப்பு k_sem *sem)
{
#ifdef CONFIG_POLL
	திரும்பு z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#இல்லையெனில்
	ARG_UNUSED(sem);
	திரும்பு false;
#endif /* CONFIG_POLL */
}

வெறுமை z_impl_k_sem_give(கட்டமைப்பு k_sem *sem)
{
	கட்டமைப்பு k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_சாவி_t சாவி = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	எனில் (z_sched_wake(&sem->wait_q, 0, வெற்று)) {
		resched = true;
	} இல்லையெனில் {
		sem->மொத்தம் += (sem->மொத்தம் != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	எனில் (unlikely(resched)) {
		z_reschedule(lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(lock, சாவி);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி வெறுமை z_vrfy_k_sem_give(கட்டமைப்பு k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#அந்தர்பாவ <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_sem_take(கட்டமைப்பு k_sem *sem, k_timeout_t timeout)
{
	கட்டமைப்பு k_spinlock *lock = sem_spinlock_get(sem);
	முழு ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_சாவி_t சாவி = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	எனில் (likely(sem->மொத்தம் > 0U)) {
		sem->மொத்தம்--;
		k_spin_unlock(lock, சாவி);
		ret = 0;
		செல் out;
	}

	எனில் (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, சாவி);
		ret = -EBUSY;
		செல் out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, சாவி, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	திரும்பு ret;
}

வெறுமை z_impl_k_sem_reset(கட்டமைப்பு k_sem *sem)
{
	கட்டமைப்பு k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_சாவி_t சாவி = k_spin_lock(lock);
	bool resched = false;

	போது (z_sched_wake(&sem->wait_q, -EAGAIN, வெற்று)) {
		resched = true;
	}
	sem->மொத்தம் = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	எனில் (resched) {
		z_reschedule(lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(lock, சாவி);
	}
}

#ifdef CONFIG_USERSPACE
நிலையான உள்வரி முழு z_vrfy_k_sem_take(கட்டமைப்பு k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	திரும்பு z_impl_k_sem_take(sem, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_sem_take_mrsh.c>

நிலையான உள்வரி வெறுமை z_vrfy_k_sem_reset(கட்டமைப்பு k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#அந்தர்பாவ <zephyr/syscalls/k_sem_reset_mrsh.c>

நிலையான உள்வரி unsigned முழு z_vrfy_k_sem_மொத்தம்_get(கட்டமைப்பு k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	திரும்பு z_impl_k_sem_மொத்தம்_get(sem);
}
#அந்தர்பாவ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
