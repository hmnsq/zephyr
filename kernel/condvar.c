/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <zephyr/toolchain.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <scheduler.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ശൂന്യ);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

സ്ഥിതം ഘടന k_spinlock condvar_lock;

പൂർണ്ണസംഖ്യ z_impl_k_condvar_init(ഘടന k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	തിരികെ 0;
}

#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_condvar_init(ഘടന k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	തിരികെ z_impl_k_condvar_init(condvar);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_condvar_signal(ഘടന k_condvar *condvar)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, സൂചന, condvar);

	എങ്കിൽ (z_sched_wake(&condvar->wait_q, 0, ശൂന്യ)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, സൂചന, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		k_spin_unlock(&condvar_lock, താക്കോൽ);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, സൂചന, condvar, 0);

	തിരികെ 0;
}

#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_condvar_signal(ഘടന k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	തിരികെ z_impl_k_condvar_signal(condvar);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_condvar_broadcast(ഘടന k_condvar *condvar)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ woken = 0;

	താക്കോൽ = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	ആകുമ്പോൾ (z_sched_wake(&condvar->wait_q, 0, ശൂന്യ)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	എങ്കിൽ (woken == 0) {
		k_spin_unlock(&condvar_lock, താക്കോൽ);
	} അല്ലെങ്കിൽ {
		z_reschedule(&condvar_lock, താക്കോൽ);
	}

	തിരികെ woken;
}
#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_condvar_broadcast(ഘടന k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	തിരികെ z_impl_k_condvar_broadcast(condvar);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

പൂർണ്ണസംഖ്യ z_impl_k_condvar_wait(ഘടന k_condvar *condvar, ഘടന k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_താക്കോൽ_t താക്കോൽ;
	പൂർണ്ണസംഖ്യ ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	എങ്കിൽ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		തിരികെ ret;
	}

	താക്കോൽ = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, താക്കോൽ, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	തിരികെ ret;
}
#ifdef CONFIG_USERSPACE
പൂർണ്ണസംഖ്യ z_vrfy_k_condvar_wait(ഘടന k_condvar *condvar, ഘടന k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	തിരികെ z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
