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

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>

#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/dlist.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/tracing/tracing.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#ଯଦି defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
ସ୍ଥିର ସଂରଚନା k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#ଅନ୍ୟଥା
ସ୍ଥିର ସଂରଚନା k_spinlock sem_lock;
#endif

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ସଂରଚନା k_spinlock *sem_spinlock_get(ସଂରଚନା k_sem *sem)
{
#ଯଦି defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	ଫେରନ୍ତୁ &sem_locks[((uintptr_t)sem / __alignof(ସଂରଚନା k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#ଅନ୍ୟଥା
	ARG_UNUSED(sem);
	ଫେରନ୍ତୁ &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ଖାଲି);
#endif /* CONFIG_OBJ_CORE_SEM */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_sem_init(ସଂରଚନା k_sem *sem, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା initial_ଗଣନା,
		      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_ଗଣନା > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		ଫେରନ୍ତୁ -EINVAL;
	}

	sem->ଗଣନା_2 = initial_ଗଣନା;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#ଯଦି defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_sem_init(ସଂରଚନା k_sem *sem, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା initial_ଗଣନା,
		      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	ଫେରନ୍ତୁ z_impl_k_sem_init(sem, initial_ଗଣନା, limit);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool sem_handle_poll_events(ସଂରଚନା k_sem *sem)
{
#ifdef CONFIG_POLL
	ଫେରନ୍ତୁ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#ଅନ୍ୟଥା
	ARG_UNUSED(sem);
	ଫେରନ୍ତୁ false;
#endif /* CONFIG_POLL */
}

ଶୂନ୍ୟ z_impl_k_sem_give(ସଂରଚନା k_sem *sem)
{
	ସଂରଚନା k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	ଯଦି (z_sched_wake(&sem->wait_q, 0, ଖାଲି)) {
		resched = true;
	} ଅନ୍ୟଥା {
		sem->ଗଣନା_2 += (sem->ଗଣନା_2 != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	ଯଦି (unlikely(resched)) {
		z_reschedule(lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(lock, ଚାବି);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_sem_give(ସଂରଚନା k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_sem_take(ସଂରଚନା k_sem *sem, k_timeout_t timeout)
{
	ସଂରଚନା k_spinlock *lock = sem_spinlock_get(sem);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	ଯଦି (likely(sem->ଗଣନା_2 > 0U)) {
		sem->ଗଣନା_2--;
		k_spin_unlock(lock, ଚାବି);
		ret = 0;
		ଯାଅ out;
	}

	ଯଦି (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, ଚାବି);
		ret = -EBUSY;
		ଯାଅ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, ଚାବି, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ z_impl_k_sem_reset(ସଂରଚନା k_sem *sem)
{
	ସଂରଚନା k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(lock);
	bool resched = false;

	ଯେତେବେଳେ (z_sched_wake(&sem->wait_q, -EAGAIN, ଖାଲି)) {
		resched = true;
	}
	sem->ଗଣନା_2 = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	ଯଦି (resched) {
		z_reschedule(lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(lock, ଚାବି);
	}
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_sem_take(ସଂରଚନା k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ଫେରନ୍ତୁ z_impl_k_sem_take(sem, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_sem_take_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_sem_reset(ସଂରଚନା k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_sem_reset_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_sem_ଗଣନା_get(ସଂରଚନା k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ଫେରନ୍ତୁ z_impl_k_sem_ଗଣନା_get(sem);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
