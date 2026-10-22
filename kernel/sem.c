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

#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <wait_q.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#אם defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
ساكن بنية k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#אחרת
ساكن بنية k_spinlock sem_lock;
#endif

ساكن مضمن بنية k_spinlock *sem_spinlock_get(بنية k_sem *sem)
{
#אם defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	החזר &sem_locks[((uintptr_t)sem / __alignof(بنية k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#אחרת
	ARG_UNUSED(sem);
	החזר &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, عدم);
#endif /* CONFIG_OBJ_CORE_SEM */

صحيح z_impl_k_sem_init(بنية k_sem *sem, unsigned صحيح initial_count,
		      unsigned صحيح limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_count > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		החזר -EINVAL;
	}

	sem->count = initial_count;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#אם defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	החזר 0;
}

#ifdef CONFIG_USERSPACE
صحيح z_vrfy_k_sem_init(بنية k_sem *sem, unsigned صحيح initial_count,
		      unsigned صحيح limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	החזר z_impl_k_sem_init(sem, initial_count, limit);
}
#ضمن <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ساكن مضمن bool sem_handle_poll_events(بنية k_sem *sem)
{
#ifdef CONFIG_POLL
	החזר z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#אחרת
	ARG_UNUSED(sem);
	החזר false;
#endif /* CONFIG_POLL */
}

فراغ z_impl_k_sem_give(بنية k_sem *sem)
{
	بنية k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_key_t key = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	אם (z_sched_wake(&sem->wait_q, 0, عدم)) {
		resched = true;
	} אחרת {
		sem->count += (sem->count != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	אם (unlikely(resched)) {
		z_reschedule(lock, key);
	} אחרת {
		k_spin_unlock(lock, key);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_sem_give(بنية k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#ضمن <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_sem_take(بنية k_sem *sem, k_timeout_t timeout)
{
	بنية k_spinlock *lock = sem_spinlock_get(sem);
	صحيح ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_key_t key = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	אם (likely(sem->count > 0U)) {
		sem->count--;
		k_spin_unlock(lock, key);
		ret = 0;
		اذهب out;
	}

	אם (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, key);
		ret = -EBUSY;
		اذهب out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, key, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	החזר ret;
}

فراغ z_impl_k_sem_reset(بنية k_sem *sem)
{
	بنية k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_key_t key = k_spin_lock(lock);
	bool resched = false;

	כלעוד (z_sched_wake(&sem->wait_q, -EAGAIN, عدم)) {
		resched = true;
	}
	sem->count = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	אם (resched) {
		z_reschedule(lock, key);
	} אחרת {
		k_spin_unlock(lock, key);
	}
}

#ifdef CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_sem_take(بنية k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	החזר z_impl_k_sem_take(sem, timeout);
}
#ضمن <zephyr/syscalls/k_sem_take_mrsh.c>

ساكن مضمن فراغ z_vrfy_k_sem_reset(بنية k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#ضمن <zephyr/syscalls/k_sem_reset_mrsh.c>

ساكن مضمن unsigned صحيح z_vrfy_k_sem_count_get(بنية k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	החזר z_impl_k_sem_count_get(sem);
}
#ضمن <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
