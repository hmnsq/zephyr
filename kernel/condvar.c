/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

स्थिर संरचना k_spinlock condvar_lock;

पूर्णाङ्क z_impl_k_condvar_init(संरचना k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	पटिनिवत्त 0;
}

#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_init(संरचना k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	पटिनिवत्त z_impl_k_condvar_init(condvar);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_condvar_signal(संरचना k_condvar *condvar)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, सङ्केत, condvar);

	सचे (z_sched_wake(&condvar->wait_q, 0, रिक्त)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, सङ्केत, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, कुञ्चिका);
	} अञ्ञथा {
		k_spin_unlock(&condvar_lock, कुञ्चिका);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, सङ्केत, condvar, 0);

	पटिनिवत्त 0;
}

#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_signal(संरचना k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	पटिनिवत्त z_impl_k_condvar_signal(condvar);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_condvar_broadcast(संरचना k_condvar *condvar)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क woken = 0;

	कुञ्चिका = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	याव (z_sched_wake(&condvar->wait_q, 0, रिक्त)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	सचे (woken == 0) {
		k_spin_unlock(&condvar_lock, कुञ्चिका);
	} अञ्ञथा {
		z_reschedule(&condvar_lock, कुञ्चिका);
	}

	पटिनिवत्त woken;
}
#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_broadcast(संरचना k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	पटिनिवत्त z_impl_k_condvar_broadcast(condvar);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_condvar_wait(संरचना k_condvar *condvar, संरचना k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	सचे (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		पटिनिवत्त ret;
	}

	कुञ्चिका = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, कुञ्चिका, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	पटिनिवत्त ret;
}
#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_wait(संरचना k_condvar *condvar, संरचना k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	पटिनिवत्त z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
