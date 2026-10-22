/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/toolchain.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <wait_q.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, پوچ);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

ایستا ساختار k_spinlock condvar_lock;

صحیح z_impl_k_condvar_init(ساختار k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	بازگشت 0;
}

#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_init(ساختار k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	بازگشت z_impl_k_condvar_init(condvar);
}
#شامل <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_condvar_signal(ساختار k_condvar *condvar)
{
	k_spinlock_کلید_t کلید = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, سیگنال, condvar);

	اگر (z_sched_wake(&condvar->wait_q, 0, پوچ)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, سیگنال, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, کلید);
	} وگرنه {
		k_spin_unlock(&condvar_lock, کلید);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, سیگنال, condvar, 0);

	بازگشت 0;
}

#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_signal(ساختار k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	بازگشت z_impl_k_condvar_signal(condvar);
}
#شامل <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_condvar_broadcast(ساختار k_condvar *condvar)
{
	k_spinlock_کلید_t کلید;
	صحیح woken = 0;

	کلید = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	وقتی (z_sched_wake(&condvar->wait_q, 0, پوچ)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	اگر (woken == 0) {
		k_spin_unlock(&condvar_lock, کلید);
	} وگرنه {
		z_reschedule(&condvar_lock, کلید);
	}

	بازگشت woken;
}
#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_broadcast(ساختار k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	بازگشت z_impl_k_condvar_broadcast(condvar);
}
#شامل <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

صحیح z_impl_k_condvar_wait(ساختار k_condvar *condvar, ساختار k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحیح ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	اگر (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		بازگشت ret;
	}

	کلید = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, کلید, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	بازگشت ret;
}
#ifdef CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_wait(ساختار k_condvar *condvar, ساختار k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	بازگشت z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#شامل <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
