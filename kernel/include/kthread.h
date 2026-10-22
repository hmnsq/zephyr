/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <timeout_q.h>


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
બાહ્ય સંરચના k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
બાહ્ય સંરચના k_thread z_idle_threads[CONFIG_MP_MAX_અંક_CPUS];
#endif /* CONFIG_MULTITHREADING */

શૂન્ય idle(શૂન્ય *unused1, શૂન્ય *unused2, શૂન્ય *unused3);

/* clean up when a thread is aborted */

#જો defined(CONFIG_THREAD_MONITOR)
શૂન્ય z_thread_monitor_exit(સંરચના k_thread *thread);
#નહીંતર
#define z_thread_monitor_exit(thread) \
	કરો {/* nothing */    \
	} જ્યારે (false)
#endif /* CONFIG_THREAD_MONITOR */

શૂન્ય z_thread_abort(સંરચના k_thread *thread);

સ્થિર અંતઃરેખ શૂન્ય thread_schedule_new(સંરચના k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	જો (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} નહીંતર {
		z_add_thread_timeout(thread, delay);
	}
#નહીંતર
	જો (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

સ્થિર અંતઃરેખ પૂર્ણાંક thread_is_preemptible(અચલ સંરચના k_thread *thread)
{
	/* explanation in kernel_struct.h */
	પાછા thread->base.preempt <= _PREEMPT_THRESHOLD;
}


સ્થિર અંતઃરેખ પૂર્ણાંક thread_is_metairq(અચલ સંરચના k_thread *thread)
{
#જો CONFIG_અંક_METAIRQ_PRIORITIES > 0
	પાછા (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_અંક_METAIRQ_PRIORITIES;
#નહીંતર
	ARG_UNUSED(thread);
	પાછા 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

સ્થિર અંતઃરેખ bool is_thread_dummy(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

સ્થિર અંતઃરેખ bool z_is_thread_suspended(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

સ્થિર અંતઃરેખ bool z_is_thread_pending(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

સ્થિર અંતઃરેખ bool z_is_thread_dead(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
સ્થિર અંતઃરેખ bool z_is_thread_aborting(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
સ્થિર અંતઃરેખ bool z_is_thread_halting(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


સ્થિર અંતઃરેખ bool z_is_thread_prevented_from_running(અચલ સંરચના k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	પાછા (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

સ્થિર અંતઃરેખ bool z_is_thread_timeout_active(અચલ સંરચના k_thread *thread)
{
	પાછા !z_is_inactive_timeout(&thread->base.timeout);
}

સ્થિર અંતઃરેખ bool z_is_thread_ready(અચલ સંરચના k_thread *thread)
{
	પાછા !z_is_thread_prevented_from_running(thread);
}

સ્થિર અંતઃરેખ bool z_is_thread_state_set(અચલ સંરચના k_thread *thread, uint32_t state)
{
	પાછા (thread->base.thread_state & state) != 0U;
}

સ્થિર અંતઃરેખ bool z_is_thread_queued(અચલ સંરચના k_thread *thread)
{
	પાછા z_is_thread_state_set(thread, _THREAD_QUEUED);
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_queued(સંરચના k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_not_queued(સંરચના k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_suspended(સંરચના k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_not_suspended(સંરચના k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_pending(સંરચના k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_not_pending(સંરચના k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

સ્થિર અંતઃરેખ bool z_is_thread_sleeping(સંરચના k_thread *thread)
{
	પાછા (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_sleeping(સંરચના k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

સ્થિર અંતઃરેખ શૂન્ય z_mark_thread_as_not_sleeping(સંરચના k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
સ્થિર અંતઃરેખ શૂન્ય z_thread_essential_set(સંરચના k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
સ્થિર અંતઃરેખ શૂન્ય z_thread_essential_clear(સંરચના k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
સ્થિર અંતઃરેખ bool z_is_thread_essential(અચલ સંરચના k_thread *thread)
{
	પાછા (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


સ્થિર ALWAYS_INLINE bool should_preempt(અચલ સંરચના k_thread *thread,
					 પૂર્ણાંક preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	જો (preempt_ok != 0) {
		પાછા true;
	}

	__ASSERT(_current != ખાલી, "");

	/* Or if we're pended/suspended/dummy (duh) */
	જો (z_is_thread_prevented_from_running(_current)) {
		પાછા true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	જો (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		પાછા true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	જો (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		પાછા true;
	}

	પાછા false;
}


સ્થિર અંતઃરેખ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	પાછા entry_point == idle;
}

સ્થિર અંતઃરેખ bool z_is_idle_thread_object(અચલ સંરચના k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	પાછા thread->base.is_idle;
#નહીંતર
	પાછા thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#નહીંતર
	પાછા false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
