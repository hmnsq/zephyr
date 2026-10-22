/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <zephyr/toolchain.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <scheduler.h>
#অন্তর্ভাৱ <wait_q.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <zephyr/init.h>

#ifdef CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, রিক্ত);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

স্থির সংরচনা k_spinlock condvar_lock;

পূর্ণাঙ্ক z_impl_k_condvar_init(সংরচনা k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#ifdef CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#endif /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_PORT_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	ঘূৰাই 0;
}

#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_condvar_init(সংরচনা k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	ঘূৰাই z_impl_k_condvar_init(condvar);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_condvar_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_condvar_signal(সংরচনা k_condvar *condvar)
{
	k_spinlock_চাবি_t চাবি = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, সঙ্কেত, condvar);

	যদি (z_sched_wake(&condvar->wait_q, 0, রিক্ত)) {
		SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_condvar, সঙ্কেত, condvar, K_FOREVER);
		z_reschedule(&condvar_lock, চাবি);
	} অন্যথা {
		k_spin_unlock(&condvar_lock, চাবি);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, সঙ্কেত, condvar, 0);

	ঘূৰাই 0;
}

#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_condvar_signal(সংরচনা k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ঘূৰাই z_impl_k_condvar_signal(condvar);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_condvar_broadcast(সংরচনা k_condvar *condvar)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক woken = 0;

	চাবি = k_spin_lock(&condvar_lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, broadcast, condvar);

	/* wake up any threads that are waiting to write */
	যেতিয়ালৈ (z_sched_wake(&condvar->wait_q, 0, রিক্ত)) {
		woken++;
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, broadcast, condvar, woken);


	যদি (woken == 0) {
		k_spin_unlock(&condvar_lock, চাবি);
	} অন্যথা {
		z_reschedule(&condvar_lock, চাবি);
	}

	ঘূৰাই woken;
}
#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_condvar_broadcast(সংরচনা k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	ঘূৰাই z_impl_k_condvar_broadcast(condvar);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#endif /* CONFIG_USERSPACE */

পূর্ণাঙ্ক z_impl_k_condvar_wait(সংরচনা k_condvar *condvar, সংরচনা k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_চাবি_t চাবি;
	পূর্ণাঙ্ক ret = -EAGAIN;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	যদি (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		ঘূৰাই ret;
	}

	চাবি = k_spin_lock(&condvar_lock);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_lock, চাবি, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_lock(mutex, K_FOREVER);

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	ঘূৰাই ret;
}
#ifdef CONFIG_USERSPACE
পূর্ণাঙ্ক z_vrfy_k_condvar_wait(সংরচনা k_condvar *condvar, সংরচনা k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	ঘূৰাই z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#endif /* CONFIG_USERSPACE */
