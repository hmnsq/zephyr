/*
 * Copyright (c) 2010-2016 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 *
 * @brief Kernel semaphore object.
 *
 * The semaphores are of the 'counting' type, i.e. each 'give' operation will
 * increment the internal count by 1, if no thread is pending on it. The 'init'
 * call initializes the count to 'initial_count'. Following multiple 'give'
 * operations, the same number of 'take' operations can be performed without
 * the calling thread having to pend on the semaphore, or the calling task
 * having to poll.
 */

#अन्तर्भाव <zephyr/kernel.h>

#अन्तर्भाव <zephyr/toolchain.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <zephyr/sys/dlist.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/tracing/tracing.h>
#अन्तर्भाव <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#यदि defined(CONFIG_SMP) && (CONFIG_SEM_तालकम्_STRIPES > 1)
स्थिर संरचना k_spinlock sem_locks[CONFIG_SEM_तालकम्_STRIPES];
#अन्यथा
स्थिर संरचना k_spinlock sem_तालकम्;
#यदि_अन्तः

स्थिर अन्तःस्थ संरचना k_spinlock *sem_spinlock_get(संरचना k_sem *sem)
{
#यदि defined(CONFIG_SMP) && (CONFIG_SEM_तालकम्_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_सम्बन्धि_TWO(CONFIG_SEM_तालकम्_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	प्रत्यागम &sem_locks[((uintptr_t)sem / __alignof(संरचना k_sem)) %
			  CONFIG_SEM_तालकम्_STRIPES];
#अन्यथा
	ARG_अप्रयुक्तम्(sem);
	प्रत्यागम &sem_तालकम्;
#यदि_अन्तः
}

#यदि_परिभाषितम् CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, रिक्त);
#यदि_अन्तः /* CONFIG_OBJ_CORE_SEM */

पूर्णाङ्क z_impl_k_sem_init(संरचना k_sem *sem, अचिह्नित पूर्णाङ्क initial_गणनम्,
		      अचिह्नित पूर्णाङ्क सीमा)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(सीमा == 0U || initial_गणनम् > सीमा) {
		SYS_पत्तनम्_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		प्रत्यागम -EINVAL;
	}

	sem->गणनम् = initial_गणनम्;
	sem->सीमा = सीमा;

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#यदि defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#यदि_अन्तः /* CONFIG_POLL */
	k_object_init(sem);

#यदि_परिभाषितम् CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#यदि_अन्तः /* CONFIG_OBJ_CORE_SEM */

	प्रत्यागम 0;
}

#यदि_परिभाषितम् CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_sem_init(संरचना k_sem *sem, अचिह्नित पूर्णाङ्क initial_गणनम्,
		      अचिह्नित पूर्णाङ्क सीमा)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	प्रत्यागम z_impl_k_sem_init(sem, initial_गणनम्, सीमा);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_init_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

स्थिर अन्तःस्थ bool sem_handle_poll_events(संरचना k_sem *sem)
{
#यदि_परिभाषितम् CONFIG_POLL
	प्रत्यागम z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_उपलब्धम्);
#अन्यथा
	ARG_अप्रयुक्तम्(sem);
	प्रत्यागम false;
#यदि_अन्तः /* CONFIG_POLL */
}

शून्य z_impl_k_sem_give(संरचना k_sem *sem)
{
	संरचना k_spinlock *तालकम् = sem_spinlock_get(sem);
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(तालकम्);
	bool resched;

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	यदि (z_sched_wake(&sem->wait_q, 0, रिक्त)) {
		resched = true;
	} अन्यथा {
		sem->गणनम् += (sem->गणनम् != sem->सीमा) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	यदि (unlikely(resched)) {
		z_reschedule(तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(तालकम्, कुञ्चिका);
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_sem_give(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_give_mrsh.c>
#यदि_अन्तः /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_sem_take(संरचना k_sem *sem, k_timeout_t timeout)
{
	संरचना k_spinlock *तालकम् = sem_spinlock_get(sem);
	पूर्णाङ्क ret;

	__ASSERT(((arch_is_अन्तर्_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(तालकम्);

	SYS_पत्तनम्_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	यदि (likely(sem->गणनम् > 0U)) {
		sem->गणनम्--;
		k_spin_unlock(तालकम्, कुञ्चिका);
		ret = 0;
		गच्छ बहिर्गमनम्;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(तालकम्, कुञ्चिका);
		ret = -EBUSY;
		गच्छ बहिर्गमनम्;
	}

	SYS_पत्तनम्_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(तालकम्, कुञ्चिका, &sem->wait_q, timeout);

बहिर्गमनम्:
	SYS_पत्तनम्_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	प्रत्यागम ret;
}

शून्य z_impl_k_sem_reset(संरचना k_sem *sem)
{
	संरचना k_spinlock *तालकम् = sem_spinlock_get(sem);
	k_spinlock_कुञ्चिका_t कुञ्चिका = k_spin_तालकम्(तालकम्);
	bool resched = false;

	यावत् (z_sched_wake(&sem->wait_q, -EAGAIN, रिक्त)) {
		resched = true;
	}
	sem->गणनम् = 0;

	SYS_पत्तनम्_TRACING_OBJ_FUNC(k_sem, reset, sem);

	यदि (resched) {
		z_reschedule(तालकम्, कुञ्चिका);
	} अन्यथा {
		k_spin_unlock(तालकम्, कुञ्चिका);
	}
}

#यदि_परिभाषितम् CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_sem_take(संरचना k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	प्रत्यागम z_impl_k_sem_take(sem, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_take_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_sem_reset(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_reset_mrsh.c>

स्थिर अन्तःस्थ अचिह्नित पूर्णाङ्क z_vrfy_k_sem_गणनम्_get(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	प्रत्यागम z_impl_k_sem_गणनम्_get(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_count_get_mrsh.c>

#यदि_अन्तः /* CONFIG_USERSPACE */
