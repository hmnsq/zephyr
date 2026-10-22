/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <zephyr/toolchain.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <scheduler.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <zephyr/internal/syscall_handler.h>
#அந்தர்பாவ <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, வெற்று);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

நிலையான கட்டமைப்பு k_spinlock condvar_lock;

முழு z_impl_k_condvar_init(கட்டமைப்பு k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	திரும்பு 0;
}

#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_condvar_init(கட்டமைப்பு k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	திரும்பு z_impl_k_condvar_init(condvar);
}
#அந்தர்பாவ <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_condvar_signal(கட்டமைப்பு k_condvar *condvar)
{
	k_spinlock_சாவி_t சாவி = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, சமிக்ஞை, condvar);

	எனில் (z_sched_wake(&condvar->wait_q, 0, வெற்று)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, சமிக்ஞை, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, சாவி);
	} இல்லையெனில் {
		k_spin_unlock(&condvar_lock, சாவி);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, சமிக்ஞை, condvar, 0);

	திரும்பு 0;
}

#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_condvar_signal(கட்டமைப்பு k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	திரும்பு z_impl_k_condvar_signal(condvar);
}
#அந்தர்பாவ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_condvar_broadcast(கட்டமைப்பு k_condvar *condvar)
{
	k_spinlock_சாவி_t சாவி;
	முழு woken = 0;

	சாவி = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	போது (z_sched_wake(&condvar->wait_q, 0, வெற்று)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	எனில் (woken == 0) {
		k_spin_unlock(&condvar_lock, சாவி);
	} இல்லையெனில் {
		z_reschedule(&condvar_lock, சாவி);
	}

	திரும்பு woken;
}
#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_condvar_broadcast(கட்டமைப்பு k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	திரும்பு z_impl_k_condvar_broadcast(condvar);
}
#அந்தர்பாவ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

முழு z_impl_k_condvar_wait(கட்டமைப்பு k_condvar *condvar, கட்டமைப்பு k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_சாவி_t சாவி;
	முழு ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	எனில் (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		திரும்பு ret;
	}

	சாவி = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, சாவி, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	திரும்பு ret;
}
#ifdef CONFIG_USERSPACE
முழு z_vrfy_k_condvar_wait(கட்டமைப்பு k_condvar *condvar, கட்டமைப்பு k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	திரும்பு z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#அந்தர்பாவ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
