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

#అన్తర్భావ <zephyr/kernel.h>

#అన్తర్భావ <zephyr/toolchain.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <zephyr/sys/dlist.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#ఒకవేళ defined(CONFIG_SMP) && (CONFIG_SEM_తాళం_STRIPES > 1)
స్థిర నిర్మాణం k_spinlock sem_locks[CONFIG_SEM_తాళం_STRIPES];
#లేకపోతే
స్థిర నిర్మాణం k_spinlock sem_తాళం;
#అయితే_ముగింపు

స్థిర అంతఃవరుస నిర్మాణం k_spinlock *sem_spinlock_get(నిర్మాణం k_sem *sem)
{
#ఒకవేళ defined(CONFIG_SMP) && (CONFIG_SEM_తాళం_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_యొక్క_TWO(CONFIG_SEM_తాళం_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	తిరిగి &sem_locks[((uintptr_t)sem / __alignof(నిర్మాణం k_sem)) %
			  CONFIG_SEM_తాళం_STRIPES];
#లేకపోతే
	ARG_వాడని(sem);
	తిరిగి &sem_తాళం;
#అయితే_ముగింపు
}

#నిర్వచితమైతే CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, ఖాళీ);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_SEM */

పూర్ణాంకం z_impl_k_sem_init(నిర్మాణం k_sem *sem, గుర్తులేని పూర్ణాంకం initial_లెక్క,
		      గుర్తులేని పూర్ణాంకం హద్దు)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(హద్దు == 0U || initial_లెక్క > హద్దు) {
		SYS_పోర్టు_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		తిరిగి -EINVAL;
	}

	sem->లెక్క = initial_లెక్క;
	sem->హద్దు = హద్దు;

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#ఒకవేళ defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#అయితే_ముగింపు /* CONFIG_POLL */
	k_object_init(sem);

#నిర్వచితమైతే CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#అయితే_ముగింపు /* CONFIG_OBJ_CORE_SEM */

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_USERSPACE
పూర్ణాంకం z_vrfy_k_sem_init(నిర్మాణం k_sem *sem, గుర్తులేని పూర్ణాంకం initial_లెక్క,
		      గుర్తులేని పూర్ణాంకం హద్దు)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	తిరిగి z_impl_k_sem_init(sem, initial_లెక్క, హద్దు);
}
#అన్తర్భావ <zephyr/syscalls/k_sem_init_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

స్థిర అంతఃవరుస bool sem_handle_poll_events(నిర్మాణం k_sem *sem)
{
#నిర్వచితమైతే CONFIG_POLL
	తిరిగి z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_అందుబాటు);
#లేకపోతే
	ARG_వాడని(sem);
	తిరిగి false;
#అయితే_ముగింపు /* CONFIG_POLL */
}

శూన్య z_impl_k_sem_give(నిర్మాణం k_sem *sem)
{
	నిర్మాణం k_spinlock *తాళం = sem_spinlock_get(sem);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(తాళం);
	bool resched;

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	ఒకవేళ (z_sched_wake(&sem->wait_q, 0, ఖాళీ)) {
		resched = true;
	} లేకపోతే {
		sem->లెక్క += (sem->లెక్క != sem->హద్దు) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	ఒకవేళ (unlikely(resched)) {
		z_reschedule(తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(తాళం, కీలకం);
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_sem_give(నిర్మాణం k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#అన్తర్భావ <zephyr/syscalls/k_sem_give_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */

పూర్ణాంకం z_impl_k_sem_take(నిర్మాణం k_sem *sem, k_timeout_t timeout)
{
	నిర్మాణం k_spinlock *తాళం = sem_spinlock_get(sem);
	పూర్ణాంకం ret;

	__ASSERT(((arch_is_లోపల_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(తాళం);

	SYS_పోర్టు_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	ఒకవేళ (likely(sem->లెక్క > 0U)) {
		sem->లెక్క--;
		k_spin_unlock(తాళం, కీలకం);
		ret = 0;
		వెళ్ళు బహిర్గతం;
	}

	ఒకవేళ (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(తాళం, కీలకం);
		ret = -EBUSY;
		వెళ్ళు బహిర్గతం;
	}

	SYS_పోర్టు_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(తాళం, కీలకం, &sem->wait_q, timeout);

బహిర్గతం:
	SYS_పోర్టు_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	తిరిగి ret;
}

శూన్య z_impl_k_sem_reset(నిర్మాణం k_sem *sem)
{
	నిర్మాణం k_spinlock *తాళం = sem_spinlock_get(sem);
	k_spinlock_కీలకం_t కీలకం = k_spin_తాళం(తాళం);
	bool resched = false;

	అయితే (z_sched_wake(&sem->wait_q, -EAGAIN, ఖాళీ)) {
		resched = true;
	}
	sem->లెక్క = 0;

	SYS_పోర్టు_TRACING_OBJ_FUNC(k_sem, reset, sem);

	ఒకవేళ (resched) {
		z_reschedule(తాళం, కీలకం);
	} లేకపోతే {
		k_spin_unlock(తాళం, కీలకం);
	}
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_sem_take(నిర్మాణం k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	తిరిగి z_impl_k_sem_take(sem, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_sem_take_mrsh.c>

స్థిర అంతఃవరుస శూన్య z_vrfy_k_sem_reset(నిర్మాణం k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#అన్తర్భావ <zephyr/syscalls/k_sem_reset_mrsh.c>

స్థిర అంతఃవరుస గుర్తులేని పూర్ణాంకం z_vrfy_k_sem_లెక్క_get(నిర్మాణం k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	తిరిగి z_impl_k_sem_లెక్క_get(sem);
}
#అన్తర్భావ <zephyr/syscalls/k_sem_count_get_mrsh.c>

#అయితే_ముగింపు /* CONFIG_USERSPACE */
