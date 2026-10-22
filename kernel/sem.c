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

#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <wait_q.h>
#شامل <zephyr/sys/dlist.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/tracing/tracing.h>
#شامل <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#اگر defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
ایستا ساختار k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#وگرنه
ایستا ساختار k_spinlock sem_lock;
#endif

ایستا درونخطی ساختار k_spinlock *sem_spinlock_get(ساختار k_sem *sem)
{
#اگر defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	بازگشت &sem_locks[((uintptr_t)sem / __alignof(ساختار k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#وگرنه
	ARG_UNUSED(sem);
	بازگشت &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_SEM */

صحیح z_impl_k_sem_init(ساختار k_sem *sem, unsigned صحیح initial_شمار,
		      unsigned صحیح limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_شمار > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		بازگشت -EINVAL;
	}

	sem->شمار = initial_شمار;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#اگر defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	بازگشت 0;
}

#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_sem_init(ساختار k_sem *sem, unsigned صحیح initial_شمار,
		      unsigned صحیح limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	بازگشت z_impl_k_sem_init(sem, initial_شمار, limit);
}
#شامل <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ایستا درونخطی bool sem_handle_poll_events(ساختار k_sem *sem)
{
#ifdef CONFIG_POLL
	بازگشت z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#وگرنه
	ARG_UNUSED(sem);
	بازگشت false;
#endif /* CONFIG_POLL */
}

تهی z_impl_k_sem_give(ساختار k_sem *sem)
{
	ساختار k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_کلید_t کلید = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	اگر (z_sched_wake(&sem->wait_q, 0, پوچ)) {
		resched = true;
	} وگرنه {
		sem->شمار += (sem->شمار != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	اگر (unlikely(resched)) {
		z_reschedule(lock, کلید);
	} وگرنه {
		k_spin_unlock(lock, کلید);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی تهی z_vrfy_k_sem_give(ساختار k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#شامل <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_sem_take(ساختار k_sem *sem, k_timeout_t timeout)
{
	ساختار k_spinlock *lock = sem_spinlock_get(sem);
	صحیح ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_کلید_t کلید = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	اگر (likely(sem->شمار > 0U)) {
		sem->شمار--;
		k_spin_unlock(lock, کلید);
		ret = 0;
		برو out;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, کلید);
		ret = -EBUSY;
		برو out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, کلید, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	بازگشت ret;
}

تهی z_impl_k_sem_reset(ساختار k_sem *sem)
{
	ساختار k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_کلید_t کلید = k_spin_lock(lock);
	bool resched = false;

	وقتی (z_sched_wake(&sem->wait_q, -EAGAIN, پوچ)) {
		resched = true;
	}
	sem->شمار = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	اگر (resched) {
		z_reschedule(lock, کلید);
	} وگرنه {
		k_spin_unlock(lock, کلید);
	}
}

#ifdef CONFIG_USERSPACE
ایستا درونخطی صحیح z_vrfy_k_sem_take(ساختار k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	بازگشت z_impl_k_sem_take(sem, timeout);
}
#شامل <zephyr/syscalls/k_sem_take_mrsh.c>

ایستا درونخطی تهی z_vrfy_k_sem_reset(ساختار k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#شامل <zephyr/syscalls/k_sem_reset_mrsh.c>

ایستا درونخطی unsigned صحیح z_vrfy_k_sem_شمار_get(ساختار k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	بازگشت z_impl_k_sem_شمار_get(sem);
}
#شامل <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
