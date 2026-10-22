/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/toolchain.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ਨਿਰਾ);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_CONDVAR */

ਸਥਿਰ ਬਣਤਰ k_spinlock condvar_ਤਾਲਾ;

ਪੂਰਨ z_impl_k_condvar_init(ਬਣਤਰ k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#ਜੇ_ਅੰਤ /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_ਪੋਰਟ_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_condvar_init(ਬਣਤਰ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	ਵਾਪਸ z_impl_k_condvar_init(condvar);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_condvar_init_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_condvar_signal(ਬਣਤਰ k_condvar *condvar)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&condvar_ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_condvar, ਸੰਕੇਤ, condvar);

	ਜੇ (z_sched_wake(&condvar->wait_q, 0, ਨਿਰਾ)) {
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_BLOCKING(k_condvar, ਸੰਕੇਤ, condvar, K_FOREVER);
		z_reschedule(&condvar_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		k_spin_unlock(&condvar_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_condvar, ਸੰਕੇਤ, condvar, 0);

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_condvar_signal(ਬਣਤਰ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ਵਾਪਸ z_impl_k_condvar_signal(condvar);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_condvar_ਪ੍ਰਸਾਰਣ(ਬਣਤਰ k_condvar *condvar)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ woken = 0;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&condvar_ਤਾਲਾ);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_condvar, ਪ੍ਰਸਾਰਣ, condvar);

	/* wake up any threads that are waiting to write */
	ਜਦੋਂ (z_sched_wake(&condvar->wait_q, 0, ਨਿਰਾ)) {
		woken++;
	}

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_condvar, ਪ੍ਰਸਾਰਣ, condvar, woken);


	ਜੇ (woken == 0) {
		k_spin_unlock(&condvar_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		z_reschedule(&condvar_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ woken;
}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_condvar_ਪ੍ਰਸਾਰਣ(ਬਣਤਰ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ਵਾਪਸ z_impl_k_condvar_ਪ੍ਰਸਾਰਣ(condvar);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */

ਪੂਰਨ z_impl_k_condvar_wait(ਬਣਤਰ k_condvar *condvar, ਬਣਤਰ k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਪੂਰਨ ret = -EAGAIN;

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	ਜੇ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		ਵਾਪਸ ret;
	}

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&condvar_ਤਾਲਾ);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_ਤਾਲਾ, ਕੁੰਜੀ, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_ਤਾਲਾ(mutex, K_FOREVER);

	SYS_ਪੋਰਟ_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	ਵਾਪਸ ret;
}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_USERSPACE
ਪੂਰਨ z_vrfy_k_condvar_wait(ਬਣਤਰ k_condvar *condvar, ਬਣਤਰ k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ਵਾਪਸ z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#ਜੇ_ਅੰਤ /* CONFIG_USERSPACE */
