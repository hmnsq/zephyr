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

#ضمن <zephyr/kernel.h>

#ضمن <zephyr/toolchain.h>
#ضمن <wait_q.h>
#ضمن <zephyr/sys/dlist.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>
#ضمن <zephyr/init.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/tracing/tracing.h>
#ضمن <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#إذا defined(CONFIG_SMP) && (CONFIG_SEM_قلف_STRIPES > 1)
ساكن بنية k_spinlock sem_locks[CONFIG_SEM_قلف_STRIPES];
#وإلا
ساكن بنية k_spinlock sem_قلف;
#د_که_پای

ساكن مضمن بنية k_spinlock *sem_spinlock_get(بنية k_sem *sem)
{
#إذا defined(CONFIG_SMP) && (CONFIG_SEM_قلف_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_د_TWO(CONFIG_SEM_قلف_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	أرجع &sem_locks[((uintptr_t)sem / __alignof(بنية k_sem)) %
			  CONFIG_SEM_قلف_STRIPES];
#وإلا
	ARG_ناکارول_شوی(sem);
	أرجع &sem_قلف;
#د_که_پای
}

#که_تعریف_شوی CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, عدم);
#د_که_پای /* CONFIG_OBJ_CORE_SEM */

صحيح z_impl_k_sem_init(بنية k_sem *sem, بینښه صحيح initial_شمېر,
		      بینښه صحيح حد)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(حد == 0U || initial_شمېر > حد) {
		SYS_پورټ_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		أرجع -EINVAL;
	}

	sem->شمېر = initial_شمېر;
	sem->حد = حد;

	SYS_پورټ_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#إذا defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#د_که_پای /* CONFIG_POLL */
	k_object_init(sem);

#که_تعریف_شوی CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#د_که_پای /* CONFIG_OBJ_CORE_SEM */

	أرجع 0;
}

#که_تعریف_شوی CONFIG_USERSPACE
صحيح z_vrfy_k_sem_init(بنية k_sem *sem, بینښه صحيح initial_شمېر,
		      بینښه صحيح حد)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	أرجع z_impl_k_sem_init(sem, initial_شمېر, حد);
}
#ضمن <zephyr/syscalls/k_sem_init_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

ساكن مضمن bool sem_handle_poll_events(بنية k_sem *sem)
{
#که_تعریف_شوی CONFIG_POLL
	أرجع z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_شته);
#وإلا
	ARG_ناکارول_شوی(sem);
	أرجع false;
#د_که_پای /* CONFIG_POLL */
}

فراغ z_impl_k_sem_give(بنية k_sem *sem)
{
	بنية k_spinlock *قلف = sem_spinlock_get(sem);
	k_spinlock_کلی_t کلی = k_spin_قلف(قلف);
	bool resched;

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	إذا (z_sched_wake(&sem->wait_q, 0, عدم)) {
		resched = true;
	} وإلا {
		sem->شمېر += (sem->شمېر != sem->حد) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	إذا (unlikely(resched)) {
		z_reschedule(قلف, کلی);
	} وإلا {
		k_spin_unlock(قلف, کلی);
	}

	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن فراغ z_vrfy_k_sem_give(بنية k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#ضمن <zephyr/syscalls/k_sem_give_mrsh.c>
#د_که_پای /* CONFIG_USERSPACE */

صحيح z_impl_k_sem_take(بنية k_sem *sem, k_timeout_t timeout)
{
	بنية k_spinlock *قلف = sem_spinlock_get(sem);
	صحيح ret;

	__ASSERT(((arch_is_دننه_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_کلی_t کلی = k_spin_قلف(قلف);

	SYS_پورټ_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	إذا (likely(sem->شمېر > 0U)) {
		sem->شمېر--;
		k_spin_unlock(قلف, کلی);
		ret = 0;
		اذهب بهر;
	}

	إذا (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(قلف, کلی);
		ret = -EBUSY;
		اذهب بهر;
	}

	SYS_پورټ_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(قلف, کلی, &sem->wait_q, timeout);

بهر:
	SYS_پورټ_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	أرجع ret;
}

فراغ z_impl_k_sem_reset(بنية k_sem *sem)
{
	بنية k_spinlock *قلف = sem_spinlock_get(sem);
	k_spinlock_کلی_t کلی = k_spin_قلف(قلف);
	bool resched = false;

	طالما (z_sched_wake(&sem->wait_q, -EAGAIN, عدم)) {
		resched = true;
	}
	sem->شمېر = 0;

	SYS_پورټ_TRACING_OBJ_FUNC(k_sem, reset, sem);

	إذا (resched) {
		z_reschedule(قلف, کلی);
	} وإلا {
		k_spin_unlock(قلف, کلی);
	}
}

#که_تعریف_شوی CONFIG_USERSPACE
ساكن مضمن صحيح z_vrfy_k_sem_take(بنية k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	أرجع z_impl_k_sem_take(sem, timeout);
}
#ضمن <zephyr/syscalls/k_sem_take_mrsh.c>

ساكن مضمن فراغ z_vrfy_k_sem_reset(بنية k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#ضمن <zephyr/syscalls/k_sem_reset_mrsh.c>

ساكن مضمن بینښه صحيح z_vrfy_k_sem_شمېر_get(بنية k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	أرجع z_impl_k_sem_شمېر_get(sem);
}
#ضمن <zephyr/syscalls/k_sem_count_get_mrsh.c>

#د_که_پای /* CONFIG_USERSPACE */
