/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/toolchain.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>
#समावेश <wait_q.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/init.h>

#अगर_घोषित CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_CONDVAR */

अटल काष्ठा k_spinlock condvar_lock;

पूर्णांक z_impl_k_condvar_init(काष्ठा k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#अगर_घोषित CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	वापस 0;
}

#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_condvar_init(काष्ठा k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	वापस z_impl_k_condvar_init(condvar);
}
#समावेश <zephyr/syscalls/k_condvar_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_condvar_signal(काष्ठा k_condvar *condvar)
{
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, संकेत, condvar);

	अगर (z_sched_wake(&condvar->wait_q, 0, शून्य)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, संकेत, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(&condvar_lock, कुंजी);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, संकेत, condvar, 0);

	वापस 0;
}

#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_condvar_signal(काष्ठा k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	वापस z_impl_k_condvar_signal(condvar);
}
#समावेश <zephyr/syscalls/k_condvar_signal_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_condvar_broadcast(काष्ठा k_condvar *condvar)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक woken = 0;

	कुंजी = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	जबतक (z_sched_wake(&condvar->wait_q, 0, शून्य)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	अगर (woken == 0) {
		k_spin_unlock(&condvar_lock, कुंजी);
	} अन्यथा {
		z_reschedule(&condvar_lock, कुंजी);
	}

	वापस woken;
}
#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_condvar_broadcast(काष्ठा k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	वापस z_impl_k_condvar_broadcast(condvar);
}
#समावेश <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_condvar_wait(काष्ठा k_condvar *condvar, काष्ठा k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_कुंजी_t कुंजी;
	पूर्णांक ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, रुको, condvar, timeout);

	अगर (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, रुको, condvar, timeout, ret);
		वापस ret;
	}

	कुंजी = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, कुंजी, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, रुको, condvar, timeout, ret);

	वापस ret;
}
#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_condvar_wait(काष्ठा k_condvar *condvar, काष्ठा k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	वापस z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#समावेश <zephyr/syscalls/k_condvar_wait_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
