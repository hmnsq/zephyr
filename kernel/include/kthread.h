/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#شامل <zephyr/kernel.h>
#شامل <kernel_internal.h>
#شامل <timeout_q.h>


#define Z_STATE_STR_DUMMY       "dummy"
#define Z_STATE_STR_PENDING     "pending"
#define Z_STATE_STR_SLEEPING    "sleeping"
#define Z_STATE_STR_DEAD        "dead"
#define Z_STATE_STR_SUSPENDED   "suspended"
#define Z_STATE_STR_ABORTING    "aborting"
#define Z_STATE_STR_SUSPENDING  "suspending"
#define Z_STATE_STR_QUEUED      "queued"

#ifdef CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
بیرونی ساختار k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
بیرونی ساختار k_thread z_idle_threads[CONFIG_MP_MAX_رقم_CPUS];
#endif /* CONFIG_MULTITHREADING */

تهی idle(تهی *unused1, تهی *unused2, تهی *unused3);

/* clean up when a thread is aborted */

#اگر defined(CONFIG_THREAD_MONITOR)
تهی z_thread_monitor_exit(ساختار k_thread *thread);
#وگرنه
#define z_thread_monitor_exit(thread) \
	انجامبده {/* nothing */    \
	} وقتی (false)
#endif /* CONFIG_THREAD_MONITOR */

تهی z_thread_abort(ساختار k_thread *thread);

ایستا درونخطی تهی thread_schedule_new(ساختار k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	اگر (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} وگرنه {
		z_add_thread_timeout(thread, delay);
	}
#وگرنه
	اگر (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

ایستا درونخطی صحیح thread_is_preemptible(ثابت ساختار k_thread *thread)
{
	/* explanation in kernel_struct.h */
	بازگشت thread->base.preempt <= _PREEMPT_THRESHOLD;
}


ایستا درونخطی صحیح thread_is_metairq(ثابت ساختار k_thread *thread)
{
#اگر CONFIG_رقم_METAIRQ_PRIORITIES > 0
	بازگشت (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_رقم_METAIRQ_PRIORITIES;
#وگرنه
	ARG_UNUSED(thread);
	بازگشت 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ایستا درونخطی bool is_thread_dummy(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

ایستا درونخطی bool z_is_thread_suspended(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

ایستا درونخطی bool z_is_thread_pending(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

ایستا درونخطی bool z_is_thread_dead(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ایستا درونخطی bool z_is_thread_aborting(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ایستا درونخطی bool z_is_thread_halting(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ایستا درونخطی bool z_is_thread_prevented_from_running(ثابت ساختار k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	بازگشت (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ایستا درونخطی bool z_is_thread_timeout_active(ثابت ساختار k_thread *thread)
{
	بازگشت !z_is_inactive_timeout(&thread->base.timeout);
}

ایستا درونخطی bool z_is_thread_ready(ثابت ساختار k_thread *thread)
{
	بازگشت !z_is_thread_prevented_from_running(thread);
}

ایستا درونخطی bool z_is_thread_state_set(ثابت ساختار k_thread *thread, uint32_t state)
{
	بازگشت (thread->base.thread_state & state) != 0U;
}

ایستا درونخطی bool z_is_thread_queued(ثابت ساختار k_thread *thread)
{
	بازگشت z_is_thread_state_set(thread, _THREAD_QUEUED);
}

ایستا درونخطی تهی z_mark_thread_as_queued(ساختار k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

ایستا درونخطی تهی z_mark_thread_as_not_queued(ساختار k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

ایستا درونخطی تهی z_mark_thread_as_suspended(ساختار k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ایستا درونخطی تهی z_mark_thread_as_not_suspended(ساختار k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

ایستا درونخطی تهی z_mark_thread_as_pending(ساختار k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

ایستا درونخطی تهی z_mark_thread_as_not_pending(ساختار k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

ایستا درونخطی bool z_is_thread_sleeping(ساختار k_thread *thread)
{
	بازگشت (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

ایستا درونخطی تهی z_mark_thread_as_sleeping(ساختار k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

ایستا درونخطی تهی z_mark_thread_as_not_sleeping(ساختار k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ایستا درونخطی تهی z_thread_essential_set(ساختار k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ایستا درونخطی تهی z_thread_essential_clear(ساختار k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ایستا درونخطی bool z_is_thread_essential(ثابت ساختار k_thread *thread)
{
	بازگشت (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ایستا ALWAYS_INLINE bool should_preempt(ثابت ساختار k_thread *thread,
					 صحیح preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	اگر (preempt_ok != 0) {
		بازگشت true;
	}

	__ASSERT(_current != پوچ, "");

	/* Or if we're pended/suspended/dummy (duh) */
	اگر (z_is_thread_prevented_from_running(_current)) {
		بازگشت true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	اگر (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		بازگشت true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	اگر (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		بازگشت true;
	}

	بازگشت false;
}


ایستا درونخطی bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	بازگشت entry_point == idle;
}

ایستا درونخطی bool z_is_idle_thread_object(ثابت ساختار k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	بازگشت thread->base.is_idle;
#وگرنه
	بازگشت thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#وگرنه
	بازگشت false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
