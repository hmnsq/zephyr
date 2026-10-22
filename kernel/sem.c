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

#समावेश <zephyr/kernel.h>

#समावेश <zephyr/toolchain.h>
#समावेश <wait_q.h>
#समावेश <zephyr/sys/dlist.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>
#समावेश <zephyr/init.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/tracing/tracing.h>
#समावेश <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#अगर defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
अटल काष्ठा k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#अन्यथा
अटल काष्ठा k_spinlock sem_lock;
#पूर्ण_अगर

अटल अंतरभूत काष्ठा k_spinlock *sem_spinlock_get(काष्ठा k_sem *sem)
{
#अगर defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	वापस &sem_locks[((uintptr_t)sem / __alignof(काष्ठा k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#अन्यथा
	ARG_UNUSED(sem);
	वापस &sem_lock;
#पूर्ण_अगर
}

#अगर_घोषित CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, शून्य);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_SEM */

पूर्णांक z_impl_k_sem_init(काष्ठा k_sem *sem, अचिन्हित पूर्णांक initial_गिनती,
		      अचिन्हित पूर्णांक limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_गिनती > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		वापस -EINVAL;
	}

	sem->गिनती = initial_गिनती;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#अगर defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#पूर्ण_अगर /* CONFIG_POLL */
	k_object_init(sem);

#अगर_घोषित CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#पूर्ण_अगर /* CONFIG_OBJ_CORE_SEM */

	वापस 0;
}

#अगर_घोषित CONFIG_USERSPACE
पूर्णांक z_vrfy_k_sem_init(काष्ठा k_sem *sem, अचिन्हित पूर्णांक initial_गिनती,
		      अचिन्हित पूर्णांक limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	वापस z_impl_k_sem_init(sem, initial_गिनती, limit);
}
#समावेश <zephyr/syscalls/k_sem_init_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

अटल अंतरभूत bool sem_handle_poll_events(काष्ठा k_sem *sem)
{
#अगर_घोषित CONFIG_POLL
	वापस z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#अन्यथा
	ARG_UNUSED(sem);
	वापस false;
#पूर्ण_अगर /* CONFIG_POLL */
}

व्योम z_impl_k_sem_give(काष्ठा k_sem *sem)
{
	काष्ठा k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	अगर (z_sched_wake(&sem->wait_q, 0, शून्य)) {
		resched = true;
	} अन्यथा {
		sem->गिनती += (sem->गिनती != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	अगर (unlikely(resched)) {
		z_reschedule(lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(lock, कुंजी);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_sem_give(काष्ठा k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#समावेश <zephyr/syscalls/k_sem_give_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */

पूर्णांक z_impl_k_sem_take(काष्ठा k_sem *sem, k_timeout_t timeout)
{
	काष्ठा k_spinlock *lock = sem_spinlock_get(sem);
	पूर्णांक ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_कुंजी_t कुंजी = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	अगर (likely(sem->गिनती > 0U)) {
		sem->गिनती--;
		k_spin_unlock(lock, कुंजी);
		ret = 0;
		जाओ out;
	}

	अगर (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, कुंजी);
		ret = -EBUSY;
		जाओ out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, कुंजी, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	वापस ret;
}

व्योम z_impl_k_sem_reset(काष्ठा k_sem *sem)
{
	काष्ठा k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_कुंजी_t कुंजी = k_spin_lock(lock);
	bool resched = false;

	जबतक (z_sched_wake(&sem->wait_q, -EAGAIN, शून्य)) {
		resched = true;
	}
	sem->गिनती = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	अगर (resched) {
		z_reschedule(lock, कुंजी);
	} अन्यथा {
		k_spin_unlock(lock, कुंजी);
	}
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत पूर्णांक z_vrfy_k_sem_take(काष्ठा k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	वापस z_impl_k_sem_take(sem, timeout);
}
#समावेश <zephyr/syscalls/k_sem_take_mrsh.c>

अटल अंतरभूत व्योम z_vrfy_k_sem_reset(काष्ठा k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#समावेश <zephyr/syscalls/k_sem_reset_mrsh.c>

अटल अंतरभूत अचिन्हित पूर्णांक z_vrfy_k_sem_गिनती_get(काष्ठा k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	वापस z_impl_k_sem_गिनती_get(sem);
}
#समावेश <zephyr/syscalls/k_sem_count_get_mrsh.c>

#पूर्ण_अगर /* CONFIG_USERSPACE */
