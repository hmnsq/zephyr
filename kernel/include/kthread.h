/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <kernel_internal.h>
#অন্তর্ভাৱ <timeout_q.h>


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
বাহ্য সংরচনা k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
বাহ্য সংরচনা k_thread z_idle_threads[CONFIG_MP_MAX_অঙ্ক_CPUS];
#endif /* CONFIG_MULTITHREADING */

শূন্য idle(শূন্য *unused1, শূন্য *unused2, শূন্য *unused3);

/* clean up when a thread is aborted */

#যদি defined(CONFIG_THREAD_MONITOR)
শূন্য z_thread_monitor_exit(সংরচনা k_thread *thread);
#অন্যথা
#define z_thread_monitor_exit(thread) \
	কৰা {/* nothing */    \
	} যেতিয়ালৈ (false)
#endif /* CONFIG_THREAD_MONITOR */

শূন্য z_thread_abort(সংরচনা k_thread *thread);

স্থির অন্তঃস্থ শূন্য thread_schedule_new(সংরচনা k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	যদি (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} অন্যথা {
		z_add_thread_timeout(thread, delay);
	}
#অন্যথা
	যদি (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক thread_is_preemptible(ধ্রুৱ সংরচনা k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ঘূৰাই thread->base.preempt <= _PREEMPT_THRESHOLD;
}


স্থির অন্তঃস্থ পূর্ণাঙ্ক thread_is_metairq(ধ্রুৱ সংরচনা k_thread *thread)
{
#যদি CONFIG_অঙ্ক_METAIRQ_PRIORITIES > 0
	ঘূৰাই (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_অঙ্ক_METAIRQ_PRIORITIES;
#অন্যথা
	ARG_UNUSED(thread);
	ঘূৰাই 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

স্থির অন্তঃস্থ bool is_thread_dummy(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

স্থির অন্তঃস্থ bool z_is_thread_suspended(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

স্থির অন্তঃস্থ bool z_is_thread_pending(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

স্থির অন্তঃস্থ bool z_is_thread_dead(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
স্থির অন্তঃস্থ bool z_is_thread_aborting(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
স্থির অন্তঃস্থ bool z_is_thread_halting(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


স্থির অন্তঃস্থ bool z_is_thread_prevented_from_running(ধ্রুৱ সংরচনা k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	ঘূৰাই (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

স্থির অন্তঃস্থ bool z_is_thread_timeout_active(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই !z_is_inactive_timeout(&thread->base.timeout);
}

স্থির অন্তঃস্থ bool z_is_thread_ready(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই !z_is_thread_prevented_from_running(thread);
}

স্থির অন্তঃস্থ bool z_is_thread_state_set(ধ্রুৱ সংরচনা k_thread *thread, uint32_t state)
{
	ঘূৰাই (thread->base.thread_state & state) != 0U;
}

স্থির অন্তঃস্থ bool z_is_thread_queued(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই z_is_thread_state_set(thread, _THREAD_QUEUED);
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_queued(সংরচনা k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_not_queued(সংরচনা k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_suspended(সংরচনা k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_not_suspended(সংরচনা k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_pending(সংরচনা k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_not_pending(সংরচনা k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

স্থির অন্তঃস্থ bool z_is_thread_sleeping(সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_sleeping(সংরচনা k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

স্থির অন্তঃস্থ শূন্য z_mark_thread_as_not_sleeping(সংরচনা k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
স্থির অন্তঃস্থ শূন্য z_thread_essential_set(সংরচনা k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
স্থির অন্তঃস্থ শূন্য z_thread_essential_clear(সংরচনা k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
স্থির অন্তঃস্থ bool z_is_thread_essential(ধ্রুৱ সংরচনা k_thread *thread)
{
	ঘূৰাই (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


স্থির ALWAYS_INLINE bool should_preempt(ধ্রুৱ সংরচনা k_thread *thread,
					 পূর্ণাঙ্ক preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	যদি (preempt_ok != 0) {
		ঘূৰাই true;
	}

	__ASSERT(_current != রিক্ত, "");

	/* Or if we're pended/suspended/dummy (duh) */
	যদি (z_is_thread_prevented_from_running(_current)) {
		ঘূৰাই true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	যদি (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ঘূৰাই true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	যদি (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		ঘূৰাই true;
	}

	ঘূৰাই false;
}


স্থির অন্তঃস্থ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	ঘূৰাই entry_point == idle;
}

স্থির অন্তঃস্থ bool z_is_idle_thread_object(ধ্রুৱ সংরচনা k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	ঘূৰাই thread->base.is_idle;
#অন্যথা
	ঘূৰাই thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#অন্যথা
	ঘূৰাই false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
