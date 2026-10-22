/*
 * Copyright (c) 2020 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/init.h>

#నిర్వచితమైతే CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_CONDVAR */

స్థిర నిర్మాణం k_spinlock condvar_తాళం;

పూర్ణాంకం z_impl_k_condvar_init(నిర్మాణం k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#నిర్వచితమైతే CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_పోర్టు_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_condvar_init(నిర్మాణం k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	తిరిగి z_impl_k_condvar_init(condvar);
}
#అన్తర్భావ <zephyr/syscalls/k_condvar_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_condvar_signal(నిర్మాణం k_condvar *condvar)
{
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(&condvar_తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_condvar, సంకేతం, condvar);

	ఒకవేళ (z_sched_wake(&condvar->wait_q, 0, ఖాళీ)) {
		SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_condvar, సంకేతం, condvar, K_FOREVER);
		z_reschedule(&condvar_తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(&condvar_తాళం, కీలకం);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_condvar, సంకేతం, condvar, 0);

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_condvar_signal(నిర్మాణం k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	తిరిగి z_impl_k_condvar_signal(condvar);
}
#అన్తర్భావ <zephyr/syscalls/k_condvar_signal_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_condvar_ప్రసారం(నిర్మాణం k_condvar *condvar)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం woken = 0;

	కీలకం = k_spin_తాళం(&condvar_తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_condvar, ప్రసారం, condvar);

	/* wake up any threads that are waiting to write */
	అయితే (z_sched_wake(&condvar->wait_q, 0, ఖాళీ)) {
		woken++;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_condvar, ప్రసారం, condvar, woken);


	ఒకవేళ (woken == 0) {
		k_spin_unlock(&condvar_తాళం, కీలకం);
	} లేకపోతే {
		z_reschedule(&condvar_తాళం, కీలకం);
	}

	తిరిగి woken;
}
#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_condvar_ప్రసారం(నిర్మాణం k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	తిరిగి z_impl_k_condvar_ప్రసారం(condvar);
}
#అన్తర్భావ <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_condvar_wait(నిర్మాణం k_condvar *condvar, నిర్మాణం k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_కీలకం_t కీలకం;
	పూర్ణాంకం ret = -EAGAIN;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_condvar, wait, condvar, timeout);

	ఒకవేళ (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);
		తిరిగి ret;
	}

	కీలకం = k_spin_తాళం(&condvar_తాళం);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_తాళం, కీలకం, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_తాళం(mutex, K_FOREVER);

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_condvar, wait, condvar, timeout, ret);

	తిరిగి ret;
}
#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_condvar_wait(నిర్మాణం k_condvar *condvar, నిర్మాణం k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	తిరిగి z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_condvar_wait_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
