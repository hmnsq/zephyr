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

#شامل <zephyr/kernel.h>

#شامل <zephyr/toolchain.h>
#شامل <wait_q.h>
#شامل <zephyr/sys/dlist.h>
#شامل <ksched.h>
#شامل <scheduler.h>
#شامل <zephyr/init.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/tracing/tracing.h>
#شامل <zephyr/sys/check.h>

/* Optional striped locks reduce the performance impact of a system-wide
 * semaphore lock without the significant extra RAM required by per-object
 * locks (semaphores are *very* widely used). A properly spin-aware semaphore
 * implementation would spin on atomic access to the count variable,
 * and not a spinlock per se. Useful optimization for the future...
 */
/* A k_spinlock can be zero-sized on uniprocessor configurations. Keep the
 * default lock scalar because arrays of empty structures are not portable.
 */
#اگر defined(CONFIG_SMP) && (CONFIG_SEM_تالا_STRIPES > 1)
ساکن ساخت k_spinlock sem_locks[CONFIG_SEM_تالا_STRIPES];
#ورنہ
ساکن ساخت k_spinlock sem_تالا;
#اگر_ختم

ساکن اندرونی ساخت k_spinlock *sem_spinlock_get(ساخت k_sem *sem)
{
#اگر defined(CONFIG_SMP) && (CONFIG_SEM_تالا_STRIPES > 1)
	BUILD_ASSERT(IS_POWER_کا_TWO(CONFIG_SEM_تالا_STRIPES),
		     "CONFIG_SEM_LOCK_STRIPES must be a power of two");

	/* Hash the naturally aligned semaphore address into a lock stripe. */
	واپس &sem_locks[((uintptr_t)sem / __alignof(ساخت k_sem)) %
			  CONFIG_SEM_تالا_STRIPES];
#ورنہ
	ARG_غیر_مستعمل(sem);
	واپس &sem_تالا;
#اگر_ختم
}

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_SEM
K_OBJ_TYPE_DEFINE(obj_type_sem, k_sem, K_OBJ_TYPE_SEM_ID, تہی);
#اگر_ختم /* CONFIG_OBJ_CORE_SEM */

صحیح z_impl_k_sem_init(ساخت k_sem *sem, غیرعلامتی صحیح initial_گنتی,
		      غیرعلامتی صحیح حد_بندی)
{
	/*
	 * Limit cannot be zero and count cannot be greater than limit
	 */
	CHECKIF(حد_بندی == 0U || initial_گنتی > حد_بندی) {
		SYS_پورٹ_TRACING_OBJ_FUNC(k_sem, init, sem, -EINVAL);

		واپس -EINVAL;
	}

	sem->گنتی = initial_گنتی;
	sem->حد_بندی = حد_بندی;

	SYS_پورٹ_TRACING_OBJ_FUNC(k_sem, init, sem, 0);

	z_waitq_init(&sem->wait_q);
#اگر defined(CONFIG_POLL)
	sys_dlist_init(&sem->poll_events);
#اگر_ختم /* CONFIG_POLL */
	k_object_init(sem);

#اگر_تعریف_شدہ CONFIG_OBJ_CORE_SEM
	k_obj_core_init_and_link(K_OBJ_CORE(sem), &obj_type_sem);
#اگر_ختم /* CONFIG_OBJ_CORE_SEM */

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
صحیح z_vrfy_k_sem_init(ساخت k_sem *sem, غیرعلامتی صحیح initial_گنتی,
		      غیرعلامتی صحیح حد_بندی)
{
	K_OOPS(K_SYSCALL_OBJ_INIT(sem, K_OBJ_SEM));
	واپس z_impl_k_sem_init(sem, initial_گنتی, حد_بندی);
}
#شامل <zephyr/syscalls/k_sem_init_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

ساکن اندرونی bool sem_handle_poll_events(ساخت k_sem *sem)
{
#اگر_تعریف_شدہ CONFIG_POLL
	واپس z_handle_obj_poll_events(&sem->poll_events, K_POLL_STATE_SEM_دستیاب);
#ورنہ
	ARG_غیر_مستعمل(sem);
	واپس false;
#اگر_ختم /* CONFIG_POLL */
}

خالی z_impl_k_sem_give(ساخت k_sem *sem)
{
	ساخت k_spinlock *تالا = sem_spinlock_get(sem);
	k_spinlock_کلید_t کلید = k_spin_تالا(تالا);
	bool resched;

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_sem, give, sem);

	اگر (z_sched_wake(&sem->wait_q, 0, تہی)) {
		resched = true;
	} ورنہ {
		sem->گنتی += (sem->گنتی != sem->حد_بندی) ? 1U : 0U;
		resched = sem_handle_poll_events(sem);
	}

	اگر (unlikely(resched)) {
		z_reschedule(تالا, کلید);
	} ورنہ {
		k_spin_unlock(تالا, کلید);
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_sem, give, sem);
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی خالی z_vrfy_k_sem_give(ساخت k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_give(sem);
}
#شامل <zephyr/syscalls/k_sem_give_mrsh.c>
#اگر_ختم /* CONFIG_USERSPACE */

صحیح z_impl_k_sem_take(ساخت k_sem *sem, k_timeout_t timeout)
{
	ساخت k_spinlock *تالا = sem_spinlock_get(sem);
	صحیح ret;

	__ASSERT(((arch_is_اندر_isr() == false) ||
		  K_TIMEOUT_EQ(timeout, K_NO_WAIT)), "");

	k_spinlock_کلید_t کلید = k_spin_تالا(تالا);

	SYS_پورٹ_TRACING_OBJ_FUNC_ENTER(k_sem, take, sem, timeout);

	اگر (likely(sem->گنتی > 0U)) {
		sem->گنتی--;
		k_spin_unlock(تالا, کلید);
		ret = 0;
		جاؤ برآمد;
	}

	اگر (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
		k_spin_unlock(تالا, کلید);
		ret = -EBUSY;
		جاؤ برآمد;
	}

	SYS_پورٹ_TRACING_OBJ_FUNC_BLOCKING(k_sem, take, sem, timeout);

	ret = z_pend_curr(تالا, کلید, &sem->wait_q, timeout);

برآمد:
	SYS_پورٹ_TRACING_OBJ_FUNC_EXIT(k_sem, take, sem, timeout, ret);

	واپس ret;
}

خالی z_impl_k_sem_reset(ساخت k_sem *sem)
{
	ساخت k_spinlock *تالا = sem_spinlock_get(sem);
	k_spinlock_کلید_t کلید = k_spin_تالا(تالا);
	bool resched = false;

	جب (z_sched_wake(&sem->wait_q, -EAGAIN, تہی)) {
		resched = true;
	}
	sem->گنتی = 0;

	SYS_پورٹ_TRACING_OBJ_FUNC(k_sem, reset, sem);

	اگر (resched) {
		z_reschedule(تالا, کلید);
	} ورنہ {
		k_spin_unlock(تالا, کلید);
	}
}

#اگر_تعریف_شدہ CONFIG_USERSPACE
ساکن اندرونی صحیح z_vrfy_k_sem_take(ساخت k_sem *sem, k_timeout_t timeout)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	واپس z_impl_k_sem_take(sem, timeout);
}
#شامل <zephyr/syscalls/k_sem_take_mrsh.c>

ساکن اندرونی خالی z_vrfy_k_sem_reset(ساخت k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	z_impl_k_sem_reset(sem);
}
#شامل <zephyr/syscalls/k_sem_reset_mrsh.c>

ساکن اندرونی غیرعلامتی صحیح z_vrfy_k_sem_گنتی_get(ساخت k_sem *sem)
{
	K_OOPS(K_SYSCALL_OBJ(sem, K_OBJ_SEM));
	واپس z_impl_k_sem_گنتی_get(sem);
}
#شامل <zephyr/syscalls/k_sem_count_get_mrsh.c>

#اگر_ختم /* CONFIG_USERSPACE */
