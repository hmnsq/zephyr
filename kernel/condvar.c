/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/toolchain.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <scheduler.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ଖାଲି);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

ସ୍ଥିର ସଂରଚନା k_spinlock condvar_lock;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_condvar_init(ସଂରଚନା k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_condvar_init(ସଂରଚନା k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	ଫେରନ୍ତୁ z_impl_k_condvar_init(condvar);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_condvar_signal(ସଂରଚନା k_condvar *condvar)
{
	k_spinlock_ଚାବି_t ଚାବି = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, ସଙ୍କେତ, condvar);

	ଯଦି (z_sched_wake(&condvar->wait_q, 0, ଖାଲି)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, ସଙ୍କେତ, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, ଚାବି);
	} ଅନ୍ୟଥା {
		k_spin_unlock(&condvar_lock, ଚାବି);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, ସଙ୍କେତ, condvar, 0);

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_condvar_signal(ସଂରଚନା k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ଫେରନ୍ତୁ z_impl_k_condvar_signal(condvar);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_condvar_broadcast(ସଂରଚନା k_condvar *condvar)
{
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା woken = 0;

	ଚାବି = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	ଯେତେବେଳେ (z_sched_wake(&condvar->wait_q, 0, ଖାଲି)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	ଯଦି (woken == 0) {
		k_spin_unlock(&condvar_lock, ଚାବି);
	} ଅନ୍ୟଥା {
		z_reschedule(&condvar_lock, ଚାବି);
	}

	ଫେରନ୍ତୁ woken;
}
#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_condvar_broadcast(ସଂରଚନା k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ଫେରନ୍ତୁ z_impl_k_condvar_broadcast(condvar);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_impl_k_condvar_wait(ସଂରଚନା k_condvar *condvar, ସଂରଚନା k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_ଚାବି_t ଚାବି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	ଯଦି (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		ଫେରନ୍ତୁ ret;
	}

	ଚାବି = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, ଚାବି, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	ଫେରନ୍ତୁ ret;
}
#ifdef CONFIG_USERSPACE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା z_vrfy_k_condvar_wait(ସଂରଚନା k_condvar *condvar, ସଂରଚନା k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ଫେରନ୍ତୁ z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
