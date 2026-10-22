/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#ضمن <zephyr/kernel.h>
#ضمن <kernel_internal.h>
#ضمن <timeout_q.h>


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
خارجي بنية k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
خارجي بنية k_thread z_idle_threads[CONFIG_MP_MAX_NUM_CPUS];
#endif /* CONFIG_MULTITHREADING */

فراغ idle(فراغ *unused1, فراغ *unused2, فراغ *unused3);

/* clean up when a thread is aborted */

#ܐܢ defined(CONFIG_THREAD_MONITOR)
فراغ z_thread_monitor_exit(بنية k_thread *thread);
#ܐܚܪܢܐ
#define z_thread_monitor_exit(thread) \
	ܥܒܕ {/* nothing */    \
	} ܟܡܐ (false)
#endif /* CONFIG_THREAD_MONITOR */

فراغ z_thread_abort(بنية k_thread *thread);

ساكن مضمن فراغ thread_schedule_new(بنية k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	ܐܢ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} ܐܚܪܢܐ {
		z_add_thread_timeout(thread, delay);
	}
#ܐܚܪܢܐ
	ܐܢ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

ساكن مضمن صحيح thread_is_preemptible(ثابت بنية k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ܦܢܐ thread->base.preempt <= _PREEMPT_THRESHOLD;
}


ساكن مضمن صحيح thread_is_metairq(ثابت بنية k_thread *thread)
{
#ܐܢ CONFIG_NUM_METAIRQ_PRIORITIES > 0
	ܦܢܐ (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_NUM_METAIRQ_PRIORITIES;
#ܐܚܪܢܐ
	ARG_UNUSED(thread);
	ܦܢܐ 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ساكن مضمن bool is_thread_dummy(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

ساكن مضمن bool z_is_thread_suspended(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

ساكن مضمن bool z_is_thread_pending(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

ساكن مضمن bool z_is_thread_dead(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ساكن مضمن bool z_is_thread_aborting(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ساكن مضمن bool z_is_thread_halting(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ساكن مضمن bool z_is_thread_prevented_from_running(ثابت بنية k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	ܦܢܐ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ساكن مضمن bool z_is_thread_timeout_active(ثابت بنية k_thread *thread)
{
	ܦܢܐ !z_is_inactive_timeout(&thread->base.timeout);
}

ساكن مضمن bool z_is_thread_ready(ثابت بنية k_thread *thread)
{
	ܦܢܐ !z_is_thread_prevented_from_running(thread);
}

ساكن مضمن bool z_is_thread_state_set(ثابت بنية k_thread *thread, uint32_t state)
{
	ܦܢܐ (thread->base.thread_state & state) != 0U;
}

ساكن مضمن bool z_is_thread_queued(ثابت بنية k_thread *thread)
{
	ܦܢܐ z_is_thread_state_set(thread, _THREAD_QUEUED);
}

ساكن مضمن فراغ z_mark_thread_as_queued(بنية k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

ساكن مضمن فراغ z_mark_thread_as_not_queued(بنية k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

ساكن مضمن فراغ z_mark_thread_as_suspended(بنية k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ساكن مضمن فراغ z_mark_thread_as_not_suspended(بنية k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

ساكن مضمن فراغ z_mark_thread_as_pending(بنية k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

ساكن مضمن فراغ z_mark_thread_as_not_pending(بنية k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

ساكن مضمن bool z_is_thread_sleeping(بنية k_thread *thread)
{
	ܦܢܐ (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

ساكن مضمن فراغ z_mark_thread_as_sleeping(بنية k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

ساكن مضمن فراغ z_mark_thread_as_not_sleeping(بنية k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ساكن مضمن فراغ z_thread_essential_set(بنية k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ساكن مضمن فراغ z_thread_essential_clear(بنية k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ساكن مضمن bool z_is_thread_essential(ثابت بنية k_thread *thread)
{
	ܦܢܐ (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ساكن ALWAYS_INLINE bool should_preempt(ثابت بنية k_thread *thread,
					 صحيح preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ܐܢ (preempt_ok != 0) {
		ܦܢܐ true;
	}

	__ASSERT(_current != عدم, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ܐܢ (z_is_thread_prevented_from_running(_current)) {
		ܦܢܐ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ܐܢ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ܦܢܐ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ܐܢ (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		ܦܢܐ true;
	}

	ܦܢܐ false;
}


ساكن مضمن bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	ܦܢܐ entry_point == idle;
}

ساكن مضمن bool z_is_idle_thread_object(ثابت بنية k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	ܦܢܐ thread->base.is_idle;
#ܐܚܪܢܐ
	ܦܢܐ thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#ܐܚܪܢܐ
	ܦܢܐ false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
