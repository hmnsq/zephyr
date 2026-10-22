/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/toolchain.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <scheduler.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ખાલી);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

સ્થિર સંરચના k_spinlock condvar_lock;

પૂર્ણાંક z_impl_k_condvar_init(સંરચના k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	પાછા 0;
}

#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_condvar_init(સંરચના k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	પાછા z_impl_k_condvar_init(condvar);
}
#અન્તર્ભાવ <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_condvar_signal(સંરચના k_condvar *condvar)
{
	k_spinlock_ચાવી_t ચાવી = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, સંકેત, condvar);

	જો (z_sched_wake(&condvar->wait_q, 0, ખાલી)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, સંકેત, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, ચાવી);
	} નહીંતર {
		k_spin_unlock(&condvar_lock, ચાવી);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, સંકેત, condvar, 0);

	પાછા 0;
}

#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_condvar_signal(સંરચના k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	પાછા z_impl_k_condvar_signal(condvar);
}
#અન્તર્ભાવ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_condvar_broadcast(સંરચના k_condvar *condvar)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક woken = 0;

	ચાવી = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	જ્યારે (z_sched_wake(&condvar->wait_q, 0, ખાલી)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	જો (woken == 0) {
		k_spin_unlock(&condvar_lock, ચાવી);
	} નહીંતર {
		z_reschedule(&condvar_lock, ચાવી);
	}

	પાછા woken;
}
#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_condvar_broadcast(સંરચના k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	પાછા z_impl_k_condvar_broadcast(condvar);
}
#અન્તર્ભાવ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

પૂર્ણાંક z_impl_k_condvar_wait(સંરચના k_condvar *condvar, સંરચના k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_ચાવી_t ચાવી;
	પૂર્ણાંક ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	જો (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		પાછા ret;
	}

	ચાવી = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, ચાવી, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	પાછા ret;
}
#ifdef CONFIG_USERSPACE
પૂર્ણાંક z_vrfy_k_condvar_wait(સંરચના k_condvar *condvar, સંરચના k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	પાછા z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
