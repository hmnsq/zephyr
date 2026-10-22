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
#यदि defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
स्थिर संरचना k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#अन्यथा
स्थिर संरचना k_spinlock sem_lock;
#endif

स्थिर अन्तःस्थ संरचना k_spinlock *sem_spinlock_get(संरचना k_sem *sem)
{
#यदि defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	फर्काऊ &sem_locks[((uintptr_t)sem / __alignof(संरचना k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#अन्यथा
	ARG_UNUSED(sem);
	फर्काऊ &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, रिक्त);
#endif /* CONFIG_OBJ_CORE_SEM */

पूर्णाङ्क z_impl_k_sem_init(संरचना k_sem *sem, अचिन्हित पूर्णाङ्क initial_गन्ती,
		      अचिन्हित पूर्णाङ्क limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_गन्ती > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		फर्काऊ -EINVAL;
	}

	sem->गन्ती = initial_गन्ती;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#यदि defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	फर्काऊ 0;
}

#ifdef CONFIG_USERSPACE
पूर्णाङ्क z_vrfy_k_sem_init(संरचना k_sem *sem, अचिन्हित पूर्णाङ्क initial_गन्ती,
		      अचिन्हित पूर्णाङ्क limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	फर्काऊ z_impl_k_sem_init(sem, initial_गन्ती, limit);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

स्थिर अन्तःस्थ bool sem_handle_poll_events(संरचना k_sem *sem)
{
#ifdef CONFIG_POLL
	फर्काऊ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#अन्यथा
	ARG_UNUSED(sem);
	फर्काऊ false;
#endif /* CONFIG_POLL */
}

शून्य z_impl_k_sem_give(संरचना k_sem *sem)
{
	संरचना k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_साँचो_t साँचो = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	यदि (z_sched_wake(&sem->wait_q, 0, रिक्त)) {
		resched = true;
	} अन्यथा {
		sem->गन्ती += (sem->गन्ती != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	यदि (unlikely(resched)) {
		z_reschedule(lock, साँचो);
	} अन्यथा {
		k_spin_unlock(lock, साँचो);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ शून्य z_vrfy_k_sem_give(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

पूर्णाङ्क z_impl_k_sem_take(संरचना k_sem *sem, k_timeout_t timeout)
{
	संरचना k_spinlock *lock = sem_spinlock_get(sem);
	पूर्णाङ्क ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_साँचो_t साँचो = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	यदि (likely(sem->गन्ती > 0U)) {
		sem->गन्ती--;
		k_spin_unlock(lock, साँचो);
		ret = 0;
		गच्छ out;
	}

	यदि (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, साँचो);
		ret = -EBUSY;
		गच्छ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, साँचो, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	फर्काऊ ret;
}

शून्य z_impl_k_sem_reset(संरचना k_sem *sem)
{
	संरचना k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_साँचो_t साँचो = k_spin_lock(lock);
	bool resched = false;

	जबसम्म (z_sched_wake(&sem->wait_q, -EAGAIN, रिक्त)) {
		resched = true;
	}
	sem->गन्ती = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	यदि (resched) {
		z_reschedule(lock, साँचो);
	} अन्यथा {
		k_spin_unlock(lock, साँचो);
	}
}

#ifdef CONFIG_USERSPACE
स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_sem_take(संरचना k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	फर्काऊ z_impl_k_sem_take(sem, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_take_mrsh.c>

स्थिर अन्तःस्थ शून्य z_vrfy_k_sem_reset(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_reset_mrsh.c>

स्थिर अन्तःस्थ अचिन्हित पूर्णाङ्क z_vrfy_k_sem_गन्ती_get(संरचना k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	फर्काऊ z_impl_k_sem_गन्ती_get(sem);
}
#अन्तर्भाव <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
