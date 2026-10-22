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

#অন্তর্ভাব <zephyr/kernel.h>

#অন্তর্ভাব <zephyr/toolchain.h>
#অন্তর্ভাব <wait_q.h>
#অন্তর্ভাব <zephyr/sys/dlist.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <scheduler.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/tracing/tracing.h>
#অন্তর্ভাব <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#যদি defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
স্থির কাঠামো k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#অন্যথা
স্থির কাঠামো k_spinlock sem_lock;
#endif

স্থির অন্তঃরেখ কাঠামো k_spinlock *sem_spinlock_get(কাঠামো k_sem *sem)
{
#যদি defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	ফেরত &sem_locks[((uintptr_t)sem / __alignof(কাঠামো k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#অন্যথা
	ARG_UNUSED(sem);
	ফেরত &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ফাঁকা);
#endif /* CONFIG_OBJ_CORE_SEM */

পূর্ণসংখ্যা z_impl_k_sem_init(কাঠামো k_sem *sem, unsigned পূর্ণসংখ্যা initial_গুনতি,
		      unsigned পূর্ণসংখ্যা limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_গুনতি > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		ফেরত -EINVAL;
	}

	sem->গুনতি = initial_গুনতি;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#যদি defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	ফেরত 0;
}

#ifdef CONFIG_USERSPACE
পূর্ণসংখ্যা z_vrfy_k_sem_init(কাঠামো k_sem *sem, unsigned পূর্ণসংখ্যা initial_গুনতি,
		      unsigned পূর্ণসংখ্যা limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	ফেরত z_impl_k_sem_init(sem, initial_গুনতি, limit);
}
#অন্তর্ভাব <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

স্থির অন্তঃরেখ bool sem_handle_poll_events(কাঠামো k_sem *sem)
{
#ifdef CONFIG_POLL
	ফেরত z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#অন্যথা
	ARG_UNUSED(sem);
	ফেরত false;
#endif /* CONFIG_POLL */
}

শূন্য z_impl_k_sem_give(কাঠামো k_sem *sem)
{
	কাঠামো k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_চাবি_t চাবি = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	যদি (z_sched_wake(&sem->wait_q, 0, ফাঁকা)) {
		resched = true;
	} অন্যথা {
		sem->গুনতি += (sem->গুনতি != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	যদি (unlikely(resched)) {
		z_reschedule(lock, চাবি);
	} অন্যথা {
		k_spin_unlock(lock, চাবি);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ শূন্য z_vrfy_k_sem_give(কাঠামো k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#অন্তর্ভাব <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণসংখ্যা z_impl_k_sem_take(কাঠামো k_sem *sem, k_timeout_t timeout)
{
	কাঠামো k_spinlock *lock = sem_spinlock_get(sem);
	পূর্ণসংখ্যা ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_চাবি_t চাবি = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	যদি (likely(sem->গুনতি > 0U)) {
		sem->গুনতি--;
		k_spin_unlock(lock, চাবি);
		ret = 0;
		যাও out;
	}

	যদি (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, চাবি);
		ret = -EBUSY;
		যাও out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, চাবি, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	ফেরত ret;
}

শূন্য z_impl_k_sem_reset(কাঠামো k_sem *sem)
{
	কাঠামো k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_চাবি_t চাবি = k_spin_lock(lock);
	bool resched = false;

	যখন (z_sched_wake(&sem->wait_q, -EAGAIN, ফাঁকা)) {
		resched = true;
	}
	sem->গুনতি = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	যদি (resched) {
		z_reschedule(lock, চাবি);
	} অন্যথা {
		k_spin_unlock(lock, চাবি);
	}
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_sem_take(কাঠামো k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ফেরত z_impl_k_sem_take(sem, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_sem_take_mrsh.c>

স্থির অন্তঃরেখ শূন্য z_vrfy_k_sem_reset(কাঠামো k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#অন্তর্ভাব <zephyr/syscalls/k_sem_reset_mrsh.c>

স্থির অন্তঃরেখ unsigned পূর্ণসংখ্যা z_vrfy_k_sem_গুনতি_get(কাঠামো k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ফেরত z_impl_k_sem_গুনতি_get(sem);
}
#অন্তর্ভাব <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
