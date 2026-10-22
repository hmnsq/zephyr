/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kernel_internal.h>
#അന്തര്ഭാവ <timeout_q.h>


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
ബാഹ്യ ഘടന k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
ബാഹ്യ ഘടന k_thread z_idle_threads[CONFIG_MP_MAX_അക്കം_CPUS];
#endif /* CONFIG_MULTITHREADING */

ശൂന്യം idle(ശൂന്യം *unused1, ശൂന്യം *unused2, ശൂന്യം *unused3);

/* clean up when a thread is aborted */

#എങ്കിൽ defined(CONFIG_THREAD_MONITOR)
ശൂന്യം z_thread_monitor_exit(ഘടന k_thread *thread);
#അല്ലെങ്കിൽ
#define z_thread_monitor_exit(thread) \
	ചെയ്യുക {/* nothing */    \
	} ആകുമ്പോൾ (false)
#endif /* CONFIG_THREAD_MONITOR */

ശൂന്യം z_thread_abort(ഘടന k_thread *thread);

സ്ഥിതം വരിയിൽ ശൂന്യം thread_schedule_new(ഘടന k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	എങ്കിൽ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} അല്ലെങ്കിൽ {
		z_add_thread_timeout(thread, delay);
	}
#അല്ലെങ്കിൽ
	എങ്കിൽ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ thread_is_preemptible(സ്ഥിരം ഘടന k_thread *thread)
{
	/* explanation in kernel_struct.h */
	തിരികെ thread->base.preempt <= _PREEMPT_THRESHOLD;
}


സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ thread_is_metairq(സ്ഥിരം ഘടന k_thread *thread)
{
#എങ്കിൽ CONFIG_അക്കം_METAIRQ_PRIORITIES > 0
	തിരികെ (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_അക്കം_METAIRQ_PRIORITIES;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	തിരികെ 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

സ്ഥിതം വരിയിൽ bool is_thread_dummy(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

സ്ഥിതം വരിയിൽ bool z_is_thread_suspended(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

സ്ഥിതം വരിയിൽ bool z_is_thread_pending(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

സ്ഥിതം വരിയിൽ bool z_is_thread_dead(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
സ്ഥിതം വരിയിൽ bool z_is_thread_aborting(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
സ്ഥിതം വരിയിൽ bool z_is_thread_halting(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


സ്ഥിതം വരിയിൽ bool z_is_thread_prevented_from_running(സ്ഥിരം ഘടന k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	തിരികെ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

സ്ഥിതം വരിയിൽ bool z_is_thread_timeout_active(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ !z_is_inactive_timeout(&thread->base.timeout);
}

സ്ഥിതം വരിയിൽ bool z_is_thread_ready(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ !z_is_thread_prevented_from_running(thread);
}

സ്ഥിതം വരിയിൽ bool z_is_thread_state_set(സ്ഥിരം ഘടന k_thread *thread, uint32_t state)
{
	തിരികെ (thread->base.thread_state & state) != 0U;
}

സ്ഥിതം വരിയിൽ bool z_is_thread_queued(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ z_is_thread_state_set(thread, _THREAD_QUEUED);
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_queued(ഘടന k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_not_queued(ഘടന k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_suspended(ഘടന k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_not_suspended(ഘടന k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_pending(ഘടന k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_not_pending(ഘടന k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

സ്ഥിതം വരിയിൽ bool z_is_thread_sleeping(ഘടന k_thread *thread)
{
	തിരികെ (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_sleeping(ഘടന k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

സ്ഥിതം വരിയിൽ ശൂന്യം z_mark_thread_as_not_sleeping(ഘടന k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
സ്ഥിതം വരിയിൽ ശൂന്യം z_thread_essential_set(ഘടന k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
സ്ഥിതം വരിയിൽ ശൂന്യം z_thread_essential_clear(ഘടന k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
സ്ഥിതം വരിയിൽ bool z_is_thread_essential(സ്ഥിരം ഘടന k_thread *thread)
{
	തിരികെ (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


സ്ഥിതം ALWAYS_INLINE bool should_preempt(സ്ഥിരം ഘടന k_thread *thread,
					 പൂർണ്ണസംഖ്യ preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	എങ്കിൽ (preempt_ok != 0) {
		തിരികെ true;
	}

	__ASSERT(_current != ശൂന്യ, "");

	/* Or if we're pended/suspended/dummy (duh) */
	എങ്കിൽ (z_is_thread_prevented_from_running(_current)) {
		തിരികെ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	എങ്കിൽ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		തിരികെ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	എങ്കിൽ (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		തിരികെ true;
	}

	തിരികെ false;
}


സ്ഥിതം വരിയിൽ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	തിരികെ entry_point == idle;
}

സ്ഥിതം വരിയിൽ bool z_is_idle_thread_object(സ്ഥിരം ഘടന k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	തിരികെ thread->base.is_idle;
#അല്ലെങ്കിൽ
	തിരികെ thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#അല്ലെങ്കിൽ
	തിരികെ false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
