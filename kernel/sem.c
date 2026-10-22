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
#जर defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
स्थिर संरचना k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#अन्यथा
स्थिर संरचना k_spinlock sem_lock;
#endif

स्थिर ओळीत संरचना k_spinlock *sem_spinlock_get(संरचना k_sem *sem)
{
#जर defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	परत &sem_locks[((uintptr_t)sem / __alignof(संरचना k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#अन्यथा
	ARG_UNUSED(sem);
	परत &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_SEM */

पूर्णांक z_impl_k_sem_init(संरचना k_sem *sem, अचिन्हित पूर्णांक initial_मोजणी,
		      अचिन्हित पूर्णांक limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_मोजणी > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		परत -EINVAL;
	}

	sem->मोजणी = initial_मोजणी;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#जर defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	परत 0;
}

#ifdef CONFIG_USERSPACE
पूर्णांक z_vrfy_k_sem_init(संरचना k_sem *sem, अचिन्हित पूर्णांक initial_मोजणी,
		      अचिन्हित पूर्णांक limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	परत z_impl_k_sem_init(sem, initial_मोजणी, limit);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिर ओळीत bool sem_handle_poll_events(संरचना k_sem *sem)
{
#ifdef CONFIG_POLL
	परत z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#अन्यथा
	ARG_UNUSED(sem);
	परत false;
#endif /* CONFIG_POLL */
}

शून्य z_impl_k_sem_give(संरचना k_sem *sem)
{
	संरचना k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	जर (z_sched_wake(&sem->wait_q, 0, रिक्त)) {
		resched = true;
	} अन्यथा {
		sem->मोजणी += (sem->मोजणी != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	जर (unlikely(resched)) {
		z_reschedule(lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(lock, किल्ली);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत शून्य z_vrfy_k_sem_give(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_sem_take(संरचना k_sem *sem, k_timeout_t timeout)
{
	संरचना k_spinlock *lock = sem_spinlock_get(sem);
	पूर्णांक ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_किल्ली_t किल्ली = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	जर (likely(sem->मोजणी > 0U)) {
		sem->मोजणी--;
		k_spin_unlock(lock, किल्ली);
		ret = 0;
		जा out;
	}

	जर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, किल्ली);
		ret = -EBUSY;
		जा out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, किल्ली, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	परत ret;
}

शून्य z_impl_k_sem_reset(संरचना k_sem *sem)
{
	संरचना k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_किल्ली_t किल्ली = k_spin_lock(lock);
	bool resched = false;

	पर्यंत (z_sched_wake(&sem->wait_q, -EAGAIN, रिक्त)) {
		resched = true;
	}
	sem->मोजणी = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	जर (resched) {
		z_reschedule(lock, किल्ली);
	} अन्यथा {
		k_spin_unlock(lock, किल्ली);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर ओळीत पूर्णांक z_vrfy_k_sem_take(संरचना k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	परत z_impl_k_sem_take(sem, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_take_mrsh.c>

स्थिर ओळीत शून्य z_vrfy_k_sem_reset(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_reset_mrsh.c>

स्थिर ओळीत अचिन्हित पूर्णांक z_vrfy_k_sem_मोजणी_get(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	परत z_impl_k_sem_मोजणी_get(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
