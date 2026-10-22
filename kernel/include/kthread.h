/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kernel_internal.h>
#ଅନ୍ତର୍ଭାଵ <timeout_q.h>


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
ବାହ୍ୟ ସଂରଚନା k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
ବାହ୍ୟ ସଂରଚନା k_thread z_idle_threads[CONFIG_MP_MAX_ଅଙ୍କ_CPUS];
#endif /* CONFIG_MULTITHREADING */

ଶୂନ୍ୟ idle(ଶୂନ୍ୟ *unused1, ଶୂନ୍ୟ *unused2, ଶୂନ୍ୟ *unused3);

/* clean up when a thread is aborted */

#ଯଦି defined(CONFIG_THREAD_MONITOR)
ଶୂନ୍ୟ z_thread_monitor_exit(ସଂରଚନା k_thread *thread);
#ଅନ୍ୟଥା
#define z_thread_monitor_exit(thread) \
	କର {/* nothing */    \
	} ଯେତେବେଳେ (false)
#endif /* CONFIG_THREAD_MONITOR */

ଶୂନ୍ୟ z_thread_abort(ସଂରଚନା k_thread *thread);

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ thread_schedule_new(ସଂରଚନା k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	ଯଦି (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} ଅନ୍ୟଥା {
		z_add_thread_timeout(thread, delay);
	}
#ଅନ୍ୟଥା
	ଯଦି (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା thread_is_preemptible(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ଫେରନ୍ତୁ thread->base.preempt <= _PREEMPT_THRESHOLD;
}


ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା thread_is_metairq(ଅଚଳ ସଂରଚନା k_thread *thread)
{
#ଯଦି CONFIG_ଅଙ୍କ_METAIRQ_PRIORITIES > 0
	ଫେରନ୍ତୁ (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_ଅଙ୍କ_METAIRQ_PRIORITIES;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ଫେରନ୍ତୁ 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool is_thread_dummy(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_suspended(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_pending(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_dead(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_aborting(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_halting(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_prevented_from_running(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	ଫେରନ୍ତୁ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_timeout_active(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ !z_is_inactive_timeout(&thread->base.timeout);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_ready(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ !z_is_thread_prevented_from_running(thread);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_state_set(ଅଚଳ ସଂରଚନା k_thread *thread, uint32_t state)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & state) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_queued(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ z_is_thread_state_set(thread, _THREAD_QUEUED);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_queued(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_not_queued(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_suspended(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_not_suspended(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_pending(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_not_pending(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_sleeping(ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_sleeping(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_mark_thread_as_not_sleeping(ସଂରଚନା k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_thread_essential_set(ସଂରଚନା k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_thread_essential_clear(ସଂରଚନା k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_thread_essential(ଅଚଳ ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ସ୍ଥିର ALWAYS_INLINE bool should_preempt(ଅଚଳ ସଂରଚନା k_thread *thread,
					 ପୂର୍ଣ୍ଣସଂଖ୍ୟା preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ଯଦି (preempt_ok != 0) {
		ଫେରନ୍ତୁ true;
	}

	__ASSERT(_current != ଖାଲି, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ଯଦି (z_is_thread_prevented_from_running(_current)) {
		ଫେରନ୍ତୁ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ଯଦି (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ଫେରନ୍ତୁ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ଯଦି (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		ଫେରନ୍ତୁ true;
	}

	ଫେରନ୍ତୁ false;
}


ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	ଫେରନ୍ତୁ entry_point == idle;
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool z_is_idle_thread_object(ଅଚଳ ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	ଫେରନ୍ତୁ thread->base.is_idle;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
