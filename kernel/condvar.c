/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/toolchain.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <wait_q.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, عدم);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

ساكن بنية k_spinlock condvar_lock;

صحيح z_impl_k_condvar_init(بنية k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_init(بنية k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	أرجع z_impl_k_condvar_init(condvar);
}
#ضمن <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_condvar_signal(بنية k_condvar *condvar)
{
	k_spinlock_key_t key = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, إشارة, condvar);

	𐤀𐤌 (z_sched_wake(&condvar->wait_q, 0, عدم)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, إشارة, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, key);
	} وإلا {
		k_spin_unlock(&condvar_lock, key);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, إشارة, condvar, 0);

	أرجع 0;
}

#ifdef CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_signal(بنية k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	أرجع z_impl_k_condvar_signal(condvar);
}
#ضمن <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_condvar_broadcast(بنية k_condvar *condvar)
{
	k_spinlock_key_t key;
	صحيح woken = 0;

	key = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&condvar->wait_q, 0, عدم)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	𐤀𐤌 (woken == 0) {
		k_spin_unlock(&condvar_lock, key);
	} وإلا {
		z_reschedule(&condvar_lock, key);
	}

	أرجع woken;
}
#ifdef CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_broadcast(بنية k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	أرجع z_impl_k_condvar_broadcast(condvar);
}
#ضمن <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحيح z_impl_k_condvar_wait(بنية k_condvar *condvar, بنية k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_key_t key;
	صحيح ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	𐤀𐤌 (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		أرجع ret;
	}

	key = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, key, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	أرجع ret;
}
#ifdef CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_wait(بنية k_condvar *condvar, بنية k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	أرجع z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#ضمن <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
