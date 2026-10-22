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

#include <zephyr/kernel.h>

#include <zephyr/toolchain.h>
#include <wait_q.h>
#include <zephyr/sys/dlist.h>
#include <ksched.h>
#include <scheduler.h>
#include <zephyr/init.h>
#include <zephyr/internal/syscall_handler.h>
#include <zephyr/tracing/tracing.h>
#include <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#ᮚᮦᮔ᮪ defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
static sa_nracanaa k_spinlock sem_locks[CONFIG_SEM_LOCK_STRIPES];
#ᮜᮤᮚ
static sa_nracanaa k_spinlock sem_lock;
#endif

static anta_hstha sa_nracanaa k_spinlock *sem_spinlock_get(sa_nracanaa k_sem *sem)
{
#ᮚᮦᮔ᮪ defined(CONFIG_SMP) && (CONFIG_SEM_LOCK_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_OF_TWO(CONFIG_SEM_LOCK_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	ᮝᮜᮤ &sem_locks[((uintptr_t)sem / __alignof(sa_nracanaa k_sem)) %
			  CONFIG_SEM_LOCK_STRIPES];
#ᮜᮤᮚ
	ARG_UNUSED(sem);
	ᮝᮜᮤ &sem_lock;
#endif
}

#ifdef CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, NULL);
#endif /* CONFIG_OBJ_CORE_SEM */

puur_naa_nka z_impl_k_sem_init(sa_nracanaa k_sem *sem, unsigned puur_naa_nka initial_count,
		      unsigned puur_naa_nka limit)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(limit == 0U || initial_count > limit) {
		SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		ᮝᮜᮤ -EINVAL;
	}

	sem->count = initial_count;
	sem->limit = limit;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#ᮚᮦᮔ᮪ defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#endif /* CONFIG_POLL */
	k_object_init(sem);

#ifdef CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#endif /* CONFIG_OBJ_CORE_SEM */

	ᮝᮜᮤ 0;
}

#ifdef CONFIG_USERSPACE
puur_naa_nka z_vrfy_k_sem_init(sa_nracanaa k_sem *sem, unsigned puur_naa_nka initial_count,
		      unsigned puur_naa_nka limit)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	ᮝᮜᮤ z_impl_k_sem_init(sem, initial_count, limit);
}
#include <zephyr/syscalls/k_sem_init_mrsh.c>
#endif /* CONFIG_USERSPACE */

static anta_hstha bool sem_handle_poll_events(sa_nracanaa k_sem *sem)
{
#ifdef CONFIG_POLL
	ᮝᮜᮤ z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_AVAILABLE);
#ᮜᮤᮚ
	ARG_UNUSED(sem);
	ᮝᮜᮤ false;
#endif /* CONFIG_POLL */
}

void z_impl_k_sem_give(sa_nracanaa k_sem *sem)
{
	sa_nracanaa k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_key_t key = k_spin_lock(lock);
	bool resched;

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	ᮚᮦᮔ᮪ (z_sched_wake(&sem->wait_q, 0, NULL)) {
		resched = true;
	} ᮜᮤᮚ {
		sem->count += (sem->count != sem->limit) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	ᮚᮦᮔ᮪ (unlikely(resched)) {
		z_reschedule(lock, key);
	} ᮜᮤᮚ {
		k_spin_unlock(lock, key);
	}

	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#ifdef CONFIG_USERSPACE
static anta_hstha void z_vrfy_k_sem_give(sa_nracanaa k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#include <zephyr/syscalls/k_sem_give_mrsh.c>
#endif /* CONFIG_USERSPACE */

puur_naa_nka z_impl_k_sem_take(sa_nracanaa k_sem *sem, k_timeout_t timeout)
{
	sa_nracanaa k_spinlock *lock = sem_spinlock_get(sem);
	puur_naa_nka ret;

	__ASSERT(((arch_is_in_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_key_t key = k_spin_lock(lock);

	SYS_PORT_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	ᮚᮦᮔ᮪ (likely(sem->count > 0U)) {
		sem->count--;
		k_spin_unlock(lock, key);
		ret = 0;
		goto out;
	}

	ᮚᮦᮔ᮪ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(lock, key);
		ret = -EBUSY;
		goto out;
	}

	SYS_PORT_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(lock, key, &sem->wait_q, timeout);

out:
	SYS_PORT_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	ᮝᮜᮤ ret;
}

void z_impl_k_sem_reset(sa_nracanaa k_sem *sem)
{
	sa_nracanaa k_spinlock *lock = sem_spinlock_get(sem);
	k_spinlock_key_t key = k_spin_lock(lock);
	bool resched = false;

	ᮞᮨᮓᮨᮜ (z_sched_wake(&sem->wait_q, -EAGAIN, NULL)) {
		resched = true;
	}
	sem->count = 0;

	SYS_PORT_TRACING_OBJ_FUNC(k_sem, reset, sem);

	ᮚᮦᮔ᮪ (resched) {
		z_reschedule(lock, key);
	} ᮜᮤᮚ {
		k_spin_unlock(lock, key);
	}
}

#ifdef CONFIG_USERSPACE
static anta_hstha puur_naa_nka z_vrfy_k_sem_take(sa_nracanaa k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ᮝᮜᮤ z_impl_k_sem_take(sem, timeout);
}
#include <zephyr/syscalls/k_sem_take_mrsh.c>

static anta_hstha void z_vrfy_k_sem_reset(sa_nracanaa k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#include <zephyr/syscalls/k_sem_reset_mrsh.c>

static anta_hstha unsigned puur_naa_nka z_vrfy_k_sem_count_get(sa_nracanaa k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	ᮝᮜᮤ z_impl_k_sem_count_get(sem);
}
#include <zephyr/syscalls/k_sem_count_get_mrsh.c>

#endif /* CONFIG_USERSPACE */
