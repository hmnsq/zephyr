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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_CONDVAR */

ಸ್ಥಿರ ರಚನೆ k_spinlock condvar_ಬೀಗ;

ಪೂರ್ಣಾಂಕ z_impl_k_condvar_init(ರಚನೆ k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_condvar_init(ರಚನೆ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	ಹಿಂತಿರುಗಿ z_impl_k_condvar_init(condvar);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_init_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_condvar_signal(ರಚನೆ k_condvar *condvar)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ = k_spin_ಬೀಗ(&condvar_ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_condvar, ಸಂಕೇತ, condvar);

	ಆದರೆ (z_sched_wake(&condvar->wait_q, 0, ಖಾಲಿ)) {
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_BLOCKING(k_condvar, ಸಂಕೇತ, condvar, K_FOREVER);
		z_reschedule(&condvar_ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		k_spin_unlock(&condvar_ಬೀಗ, ಕೀಲಿ);
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_condvar, ಸಂಕೇತ, condvar, 0);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_condvar_signal(ರಚನೆ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ಹಿಂತಿರುಗಿ z_impl_k_condvar_signal(condvar);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_condvar_ಪ್ರಸಾರ(ರಚನೆ k_condvar *condvar)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ woken = 0;

	ಕೀಲಿ = k_spin_ಬೀಗ(&condvar_ಬೀಗ);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_condvar, ಪ್ರಸಾರ, condvar);

	/* wake up any threads that are waiting to write */
	ಆಗ (z_sched_wake(&condvar->wait_q, 0, ಖಾಲಿ)) {
		woken++;
	}

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_condvar, ಪ್ರಸಾರ, condvar, woken);


	ಆದರೆ (woken == 0) {
		k_spin_unlock(&condvar_ಬೀಗ, ಕೀಲಿ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		z_reschedule(&condvar_ಬೀಗ, ಕೀಲಿ);
	}

	ಹಿಂತಿರುಗಿ woken;
}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_condvar_ಪ್ರಸಾರ(ರಚನೆ k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ಹಿಂತಿರುಗಿ z_impl_k_condvar_ಪ್ರಸಾರ(condvar);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */

ಪೂರ್ಣಾಂಕ z_impl_k_condvar_wait(ರಚನೆ k_condvar *condvar, ರಚನೆ k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_ಕೀಲಿ_t ಕೀಲಿ;
	ಪೂರ್ಣಾಂಕ ret = -EAGAIN;

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	ಆದರೆ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಕೀಲಿ = k_spin_ಬೀಗ(&condvar_ಬೀಗ);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_ಬೀಗ, ಕೀಲಿ, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_ಬೀಗ(mutex, K_FOREVER);

	SYS_ಪೋರ್ಟ್_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	ಹಿಂತಿರುಗಿ ret;
}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಪೂರ್ಣಾಂಕ z_vrfy_k_condvar_wait(ರಚನೆ k_condvar *condvar, ರಚನೆ k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ಹಿಂತಿರುಗಿ z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
