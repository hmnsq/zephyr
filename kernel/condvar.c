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

#यदि_परिभाषितम् CONFIG_OBJ_CORE_CONDVAR
K_OBJ_TYPE_DEFINE(obj_type_condvar, k_condvar, K_OBJ_TYPE_CONDVAR_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_CONDVAR */

स्थिर संरचना k_spinlock condvar_तालकम्;

पूर्णाङ्क z_impl_k_condvar_init(संरचना k_condvar *condvar)
{
	z_waitq_init(&condvar->wait_q);
	k_object_init(condvar);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_CONDVAR
	k_obj_core_init_and_link(K_OBJ_CORE(condvar), &obj_type_condvar);
#यदि_अन्तः /* CONFIG_OBJ_CORE_CONDVAR */

	SYS_पत्तनम्_TRACING_OBJ_INIT(k_condvar, condvar, 0);

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_init(संरचना k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(condvar, K_OBJ_CONDVAR));
	प्रत्यागम z_impl_k_condvar_init(condvar);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_init_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_condvar_signal(संरचना k_condvar *condvar)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(&condvar_तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_condvar, सङ्केत, condvar);

	यदि (z_sched_wake(&condvar->wait_q, 0, रिक्त)) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_condvar, सङ्केत, condvar, K_FOREVER);
		z_reschedule(&condvar_तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(&condvar_तालकम्, कुञ्चिका);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_condvar, सङ्केत, condvar, 0);

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_signal(संरचना k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	प्रत्यागम z_impl_k_condvar_signal(condvar);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_signal_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_condvar_प्रसारणम्(संरचना k_condvar *condvar)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क woken = 0;

	कुञ्चिका = k_spin_तालकम्(&condvar_तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_condvar, प्रसारणम्, condvar);

	/* wake up any threads that are waiting to write */
	यावत् (z_sched_wake(&condvar->wait_q, 0, रिक्त)) {
		woken++;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_condvar, प्रसारणम्, condvar, woken);


	यदि (woken == 0) {
		k_spin_unlock(&condvar_तालकम्, कुञ्चिका);
	} अन्यथा {
		z_reschedule(&condvar_तालकम्, कुञ्चिका);
	}

	प्रत्यागम woken;
}
#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_प्रसारणम्(संरचना k_condvar *condvar)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	प्रत्यागम z_impl_k_condvar_प्रसारणम्(condvar);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_broadcast_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_condvar_wait(संरचना k_condvar *condvar, संरचना k_mutex *mutex,
			  k_timeout_t timeout)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	पूर्णाङ्क ret = -EAGAIN;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_condvar, प्रतीक्षस्व, condvar, timeout);

	यदि (unlikely(K_TIMEOUT_EQ(timeout, K_NO_WAIT))) {
		/* No wait: per POSIX semantics, the mutex must remain locked when
		 * k_condvar_wait() returns. Returning -EAGAIN immediately without
		 * touching the mutex preserves the calling-thread-holds-mutex
		 * invariant.
		 */
		SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_condvar, प्रतीक्षस्व, condvar, timeout, ret);
		प्रत्यागम ret;
	}

	कुञ्चिका = k_spin_तालकम्(&condvar_तालकम्);
	k_mutex_unlock(mutex);

	ret = z_pend_curr(&condvar_तालकम्, कुञ्चिका, &condvar->wait_q, timeout);

	/* Always re-acquire the mutex before returning, even on timeout.
	 * This matches POSIX semantics: the mutex must be locked by the
	 * calling thread when pthread_cond_wait() returns, regardless of
	 * whether it was signaled or timed out.
	 */
	k_mutex_तालकम्(mutex, K_FOREVER);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_condvar, प्रतीक्षस्व, condvar, timeout, ret);

	प्रत्यागम ret;
}
#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_condvar_wait(संरचना k_condvar *condvar, संरचना k_mutex *mutex,
			  k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(condvar, K_OBJ_CONDVAR));
	K_OOPS(K_SYSCALL_OBJ(mutex, K_OBJ_MUTEX));
	प्रत्यागम z_impl_k_condvar_wait(condvar, mutex, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_condvar_wait_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */
