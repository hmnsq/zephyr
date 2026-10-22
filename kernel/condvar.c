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

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_CONDVAR */

ساکن ساخت k_spinlock condvar_تالا;

صحیح z_impl_k_condvar_init(ساخت k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#اگر_ختم /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_پورٹ_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_init(ساخت k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	واپس z_impl_k_condvar_init(condvar);
}
#شامل <zephyr/syscalls/k_condvar_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_condvar_signal(ساخت k_condvar *condvar)
{
	k_spinlock_کلید_t کلید = k_spin_تالا(&condvar_تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_condvar, اشارہ, condvar);

	اگر (z_sched_wake(&condvar->wait_q, 0, تہی)) {
		SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_condvar, اشارہ, condvar, K_FOREVER);
		z_reschedule(&condvar_تالا, کلید);
	} ورنہ {
		k_spin_unlock(&condvar_تالا, کلید);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_condvar, اشارہ, condvar, 0);

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_signal(ساخت k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	واپس z_impl_k_condvar_signal(condvar);
}
#شامل <zephyr/syscalls/k_condvar_signal_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_condvar_نشر(ساخت k_condvar *condvar)
{
	k_spinlock_کلید_t کلید;
	صحیح woken = 0;

	کلید = k_spin_تالا(&condvar_تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_condvar, نشر, condvar);

	/* wake up any threads that are waiting to write */
	جب (z_sched_wake(&condvar->wait_q, 0, تہی)) {
		woken++;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_condvar, نشر, condvar, woken);


	اگر (woken == 0) {
		k_spin_unlock(&condvar_تالا, کلید);
	} ورنہ {
		z_reschedule(&condvar_تالا, کلید);
	}

	واپس woken;
}
#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_نشر(ساخت k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	واپس z_impl_k_condvar_نشر(condvar);
}
#شامل <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_condvar_wait(ساخت k_condvar *condvar, ساخت k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_کلید_t کلید;
	صحیح ret = -EAGAIN;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	اگر (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		واپس ret;
	}

	کلید = k_spin_تالا(&condvar_تالا);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_تالا, کلید, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_تالا(mutex, K_FOREVER);

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	واپس ret;
}
#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_condvar_wait(ساخت k_condvar *condvar, ساخت k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	واپس z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#شامل <zephyr/syscalls/k_condvar_wait_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */
