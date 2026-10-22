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
बाह्य संरचना k_thread z_idle_threads[CONFIG_MP_MAX_अङ्क_CPUS];
#endif /* CONFIG_MULTITHREADING */

शून्य idle(शून्य *unused1, शून्य *unused2, शून्य *unused3);

/* clean up when a thread is aborted */

#यदि defined(CONFIG_THREAD_MONITOR)
शून्य z_thread_monitor_exit(संरचना k_thread *thread);
#अन्यथा
#define z_thread_monitor_exit(thread) \
	गर {/* nothing */    \
	} जबसम्म (false)
#endif /* CONFIG_THREAD_MONITOR */

शून्य z_thread_abort(संरचना k_thread *thread);

स्थिर अन्तःस्थ शून्य thread_schedule_new(संरचना k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	यदि (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} अन्यथा {
		z_add_thread_timeout(thread, delay);
	}
#अन्यथा
	यदि (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

स्थिर अन्तःस्थ पूर्णाङ्क thread_is_preemptible(अचर संरचना k_thread *thread)
{
	/* explanation in kernel_struct.h */
	फर्काऊ thread->base.preempt <= _PREEMPT_THRESHOLD;
}


स्थिर अन्तःस्थ पूर्णाङ्क thread_is_metairq(अचर संरचना k_thread *thread)
{
#यदि CONFIG_अङ्क_METAIRQ_PRIORITIES > 0
	फर्काऊ (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_अङ्क_METAIRQ_PRIORITIES;
#अन्यथा
	ARG_UNUSED(thread);
	फर्काऊ 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

स्थिर अन्तःस्थ bool is_thread_dummy(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_suspended(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_pending(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_dead(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
स्थिर अन्तःस्थ bool z_is_thread_aborting(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
स्थिर अन्तःस्थ bool z_is_thread_halting(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


स्थिर अन्तःस्थ bool z_is_thread_prevented_from_running(अचर संरचना k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	फर्काऊ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_timeout_active(अचर संरचना k_thread *thread)
{
	फर्काऊ !z_is_inactive_timeout(&thread->base.timeout);
}

स्थिर अन्तःस्थ bool z_is_thread_ready(अचर संरचना k_thread *thread)
{
	फर्काऊ !z_is_thread_prevented_from_running(thread);
}

स्थिर अन्तःस्थ bool z_is_thread_state_set(अचर संरचना k_thread *thread, uint32_t state)
{
	फर्काऊ (thread->base.thread_state & state) != 0U;
}

स्थिर अन्तःस्थ bool z_is_thread_queued(अचर संरचना k_thread *thread)
{
	फर्काऊ z_is_thread_state_set(thread, _THREAD_QUEUED);
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
	फर्काऊ (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
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
स्थिर अन्तःस्थ bool z_is_thread_essential(अचर संरचना k_thread *thread)
{
	फर्काऊ (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


स्थिर ALWAYS_INLINE bool should_preempt(अचर संरचना k_thread *thread,
					 पूर्णाङ्क preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	यदि (preempt_ok != 0) {
		फर्काऊ true;
	}

	__ASSERT(_current != रिक्त, "");

	/* Or if we're pended/suspended/dummy (duh) */
	यदि (z_is_thread_prevented_from_running(_current)) {
		फर्काऊ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	यदि (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		फर्काऊ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	यदि (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		फर्काऊ true;
	}

	फर्काऊ false;
}


स्थिर अन्तःस्थ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	फर्काऊ entry_point == idle;
}

स्थिर अन्तःस्थ bool z_is_idle_thread_object(अचर संरचना k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	फर्काऊ thread->base.is_idle;
#अन्यथा
	फर्काऊ thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#अन्यथा
	फर्काऊ false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
