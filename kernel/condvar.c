/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <zephyr/toolchain.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <scheduler.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ರಿಕ್ತ);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

ಸ್ಥಿರ ಸಂರಚನಾ k_spinlock condvar_lock;

ಪೂರ್ಣಾಙ್ಕ z_impl_k_condvar_init(ಸಂರಚನಾ k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_condvar_init(ಸಂರಚನಾ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_condvar_init(condvar);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_condvar_signal(ಸಂರಚನಾ k_condvar *condvar)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, ಸಙ್ಕೇತ, condvar);

	ಯದಿ (z_sched_wake(&condvar->wait_q, 0, ರಿಕ್ತ)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, ಸಙ್ಕೇತ, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		k_spin_unlock(&condvar_lock, ಕೀಲಿ);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, ಸಙ್ಕೇತ, condvar, 0);

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_condvar_signal(ಸಂರಚನಾ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_condvar_signal(condvar);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_condvar_broadcast(ಸಂರಚನಾ k_condvar *condvar)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ woken = 0;

	ಕೀಲಿ = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	ಯಾವತ್ (z_sched_wake(&condvar->wait_q, 0, ರಿಕ್ತ)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	ಯದಿ (woken == 0) {
		k_spin_unlock(&condvar_lock, ಕೀಲಿ);
	} ಅನ್ಯಥಾ {
		z_reschedule(&condvar_lock, ಕೀಲಿ);
	}

	ಪ್ರತ್ಯಾಗಮ woken;
}
#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_condvar_broadcast(ಸಂರಚನಾ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_condvar_broadcast(condvar);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಙ್ಕ z_impl_k_condvar_wait(ಸಂರಚನಾ k_condvar *condvar, ಸಂರಚನಾ k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಙ್ಕ ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	ಯದಿ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಕೀಲಿ = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, ಕೀಲಿ, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	ಪ್ರತ್ಯಾಗಮ ret;
}
#ifdef CONFIG_USERSPACE
ಪೂರ್ಣಾಙ್ಕ z_vrfy_k_condvar_wait(ಸಂರಚನಾ k_condvar *condvar, ಸಂರಚನಾ k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ಪ್ರತ್ಯಾಗಮ z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
