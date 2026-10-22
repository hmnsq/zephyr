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

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, عدم);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_CONDVAR */

ساكن بنية k_spinlock condvar_تالو;

صحيح z_impl_k_condvar_init(بنية k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#جيڪڏهن_وصف_ٿيل CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#جيڪڏهن_پورو /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_پورٽ_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_init(بنية k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	أرجع z_impl_k_condvar_init(condvar);
}
#ضمن <zephyr/syscalls/k_condvar_init_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_condvar_signal(بنية k_condvar *condvar)
{
	k_spinlock_ڪنجي_t ڪنجي = k_spin_تالو(&condvar_تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_condvar, إشارة, condvar);

	إذا (z_sched_wake(&condvar->wait_q, 0, عدم)) {
		SYS_پورٽ_TRACING_OBJ_FUNC_BLOCKING(k_condvar, إشارة, condvar, K_FOREVER);
		z_reschedule(&condvar_تالو, ڪنجي);
	} وإلا {
		k_spin_unlock(&condvar_تالو, ڪنجي);
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_condvar, إشارة, condvar, 0);

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_signal(بنية k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	أرجع z_impl_k_condvar_signal(condvar);
}
#ضمن <zephyr/syscalls/k_condvar_signal_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_condvar_نشر(بنية k_condvar *condvar)
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح woken = 0;

	ڪنجي = k_spin_تالو(&condvar_تالو);

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_condvar, نشر, condvar);

	/* wake up any threads that are waiting to write */
	طالما (z_sched_wake(&condvar->wait_q, 0, عدم)) {
		woken++;
	}

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_condvar, نشر, condvar, woken);


	إذا (woken == 0) {
		k_spin_unlock(&condvar_تالو, ڪنجي);
	} وإلا {
		z_reschedule(&condvar_تالو, ڪنجي);
	}

	أرجع woken;
}
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_نشر(بنية k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	أرجع z_impl_k_condvar_نشر(condvar);
}
#ضمن <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */

صحيح z_impl_k_condvar_wait(بنية k_condvar *condvar, بنية k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_ڪنجي_t ڪنجي;
	صحيح ret = -EAGAIN;

	SYS_پورٽ_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	إذا (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		أرجع ret;
	}

	ڪنجي = k_spin_تالو(&condvar_تالو);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_تالو, ڪنجي, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_تالو(mutex, K_FOREVER);

	SYS_پورٽ_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	أرجع ret;
}
#جيڪڏهن_وصف_ٿيل CONFIG_USERSPACE
صحيح z_vrfy_k_condvar_wait(بنية k_condvar *condvar, بنية k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	أرجع z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#ضمن <zephyr/syscalls/k_condvar_wait_mrsh.c>
#جيڪڏهن_پورو /* CONFIG_USERSPACE */
