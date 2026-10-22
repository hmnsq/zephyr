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

#અન્તર્ભાવ <zephyr/kernel.h>

#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <zephyr/sys/dlist.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/tracing/tracing.h>
#અન્તર્ભાવ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#જો defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
સ્થિર સંરચના k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#નહીંતર
સ્થિર સંરચના k_spinlock sem_lock;
#endif

સ્થિર અંતઃરેખ સંરચના k_spinlock *sem_spinlock_get(સંરચના k_sem *sem)
{
#જો defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	પાછા &sem_locks[((uintptr_t)sem / __alignof(સંરચના k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#નહીંતર
	ARG_UNUSED(sem);
	પાછા &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_SEM */

પૂર્ણાંક z_impl_k_sem_init(સંરચના k_sem *sem, unsigned પૂર્ણાંક initial_ગણતરી,
		      unsigned પૂર્ણાંક limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_ગણતરી > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		પાછા -EINVAL;
	}

	sem->ગણતરી = initial_ગણતરી;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#જો defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	પાછા 0;
}

#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_sem_init(સંરચના k_sem *sem, unsigned પૂર્ણાંક initial_ગણતરી,
		      unsigned પૂર્ણાંક limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	પાછા z_impl_k_sem_init(sem, initial_ગણતરી, limit);
}
#અન્તર્ભાવ <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

સ્થિર અંતઃરેખ bool sem_handle_poll_events(સંરચના k_sem *sem)
{
#ifdef CONFIG_POLL
	પાછા z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#નહીંતર
	ARG_UNUSED(sem);
	પાછા false;
#endif /* CONFIG_POLL */
}

શૂન્ય z_impl_k_sem_give(સંરચના k_sem *sem)
{
	સંરચના k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	જો (z_sched_wake(&sem->wait_q, 0, ખાલી)) {
		resched = true;
	} નહીંતર {
		sem->ગણતરી += (sem->ગણતરી != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	જો (unlikely(resched)) {
		z_reschedule(lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(lock, ચાવી);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_sem_give(સંરચના k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#અન્તર્ભાવ <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_sem_take(સંરચના k_sem *sem, k_timeout_t timeout)
{
	સંરચના k_spinlock *lock = sem_spinlock_get(sem);
	પૂર્ણાંક ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_ચાવી_t ચાવી = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	જો (likely(sem->ગણતરી > 0U)) {
		sem->ગણતરી--;
		k_spin_unlock(lock, ચાવી);
		ret = 0;
		જાઓ out;
	}

	જો (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, ચાવી);
		ret = -EBUSY;
		જાઓ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, ચાવી, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	પાછા ret;
}

શૂન્ય z_impl_k_sem_reset(સંરચના k_sem *sem)
{
	સંરચના k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(lock);
	bool resched = false;

	જ્યારે (z_sched_wake(&sem->wait_q, -EAGAIN, ખાલી)) {
		resched = true;
	}
	sem->ગણતરી = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	જો (resched) {
		z_reschedule(lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(lock, ચાવી);
	}
}

#ifdef CONFIG_USERSPACE
સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_sem_take(સંરચના k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	પાછા z_impl_k_sem_take(sem, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_sem_take_mrsh.c>

સ્થિર અંતઃરેખ શૂન્ય z_vrfy_k_sem_reset(સંરચના k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#અન્તર્ભાવ <zephyr/syscalls/k_sem_reset_mrsh.c>

સ્થિર અંતઃરેખ unsigned પૂર્ણાંક z_vrfy_k_sem_ગણતરી_get(સંરચના k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	પાછા z_impl_k_sem_ગણતરી_get(sem);
}
#અન્તર્ભાવ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
