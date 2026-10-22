/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <kernel_internal.h>
#अन्तर्भाव <timeout_q.h>


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
बाह्य संरचना k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
बाह्य संरचना k_thread z_idle_threads[CONFIG_MP_MAX_अङ्कः_CPUS];
#endif /* CONFIG_MULTITHREADING */

शून्य idle(शून्य *unused1, शून्य *unused2, शून्य *unused3);

/* clean up when a thread is aborted */

#सचे defined(CONFIG_THREAD_MONITOR)
शून्य z_thread_monitor_exit(संरचना k_thread *thread);
#अञ्ञथा
#define z_thread_monitor_exit(thread) \
	करोहि {/* nothing */    \
	} याव (false)
#endif /* CONFIG_THREAD_MONITOR */

शून्य z_thread_abort(संरचना k_thread *thread);

स्थिर अन्तःस्थ शून्य thread_schedule_new(संरचना k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	सचे (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} अञ्ञथा {
		z_add_thread_timeout(thread, delay);
	}
#अञ्ञथा
	सचे (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

स्थिर अन्तःस्थ पूर्णाङ्क thread_is_preemptible(ध्रुव संरचना k_thread *thread)
{
	/* explanation in kernel_struct.h */
	पटिनिवत्त thread->base.preempt <= _PREEMPT_THRESHOLD;
}


स्थिर अन्तःस्थ पूर्णाङ्क thread_is_metairq(ध्रुव संरचना k_thread *thread)
{
#सचे CONFIG_अङ्कः_METAIRQ_PRIORITIES > 0
	पटिनिवत्त (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_अङ्कः_METAIRQ_PRIORITIES;
#अञ्ञथा
	ARG_UNUSED(thread);
	पटिनिवत्त 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

स्थिर अन्तःस्थ bool is_thread_dummy(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_suspended(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_pending(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_dead(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
स्थिर अन्तःस्थ bool z_is_thread_aborting(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
स्थिर अन्तःस्थ bool z_is_thread_halting(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


स्थिर अन्तःस्थ bool z_is_thread_prevented_from_running(ध्रुव संरचना k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	पटिनिवत्त (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_timeout_active(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त !z_is_inactive_timeout(&thread->base.timeout);
}

स्थिर अन्तःस्थ bool z_is_thread_ready(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त !z_is_thread_prevented_from_running(thread);
}

स्थिर अन्तःस्थ bool z_is_thread_state_set(ध्रुव संरचना k_thread *thread, uint32_t state)
{
	पटिनिवत्त (thread->base.thread_state & state) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_queued(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त z_is_thread_state_set(thread, _THREAD_QUEUED);
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_queued(संरचना k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_not_queued(संरचना k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_suspended(संरचना k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_not_suspended(संरचना k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_pending(संरचना k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_not_pending(संरचना k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

स्थिर अन्तःस्थ bool z_is_thread_sleeping(संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_sleeping(संरचना k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

स्थिर अन्तःस्थ शून्य z_mark_thread_as_not_sleeping(संरचना k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
स्थिर अन्तःस्थ शून्य z_thread_essential_set(संरचना k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
स्थिर अन्तःस्थ शून्य z_thread_essential_clear(संरचना k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
स्थिर अन्तःस्थ bool z_is_thread_essential(ध्रुव संरचना k_thread *thread)
{
	पटिनिवत्त (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


स्थिर ALWAYS_INLINE bool should_preempt(ध्रुव संरचना k_thread *thread,
					 पूर्णाङ्क preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	सचे (preempt_ok != 0) {
		पटिनिवत्त true;
	}

	__ASSERT(_current != रिक्त, "");

	/* Or if we're pended/suspended/dummy (duh) */
	सचे (z_is_thread_prevented_from_running(_current)) {
		पटिनिवत्त true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	सचे (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		पटिनिवत्त true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	सचे (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		पटिनिवत्त true;
	}

	पटिनिवत्त false;
}


स्थिर अन्तःस्थ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	पटिनिवत्त entry_point == idle;
}

स्थिर अन्तःस्थ bool z_is_idle_thread_object(ध्रुव संरचना k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	पटिनिवत्त thread->base.is_idle;
#अञ्ञथा
	पटिनिवत्त thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#अञ्ञथा
	पटिनिवत्त false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
