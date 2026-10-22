/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <kernel_internal.h>
#அந்தர்பாவ <timeout_q.h>


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
வெளி கட்டமைப்பு k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
வெளி கட்டமைப்பு k_thread z_idle_threads[CONFIG_MP_MAX_இலக்கம்_CPUS];
#endif /* CONFIG_MULTITHREADING */

வெறுமை idle(வெறுமை *unused1, வெறுமை *unused2, வெறுமை *unused3);

/* clean up when a thread is aborted */

#எனில் defined(CONFIG_THREAD_MONITOR)
வெறுமை z_thread_monitor_exit(கட்டமைப்பு k_thread *thread);
#இல்லையெனில்
#define z_thread_monitor_exit(thread) \
	செய் {/* nothing */    \
	} போது (false)
#endif /* CONFIG_THREAD_MONITOR */

வெறுமை z_thread_abort(கட்டமைப்பு k_thread *thread);

நிலையான உள்வரி வெறுமை thread_schedule_new(கட்டமைப்பு k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	எனில் (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} இல்லையெனில் {
		z_add_thread_timeout(thread, delay);
	}
#இல்லையெனில்
	எனில் (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

நிலையான உள்வரி முழு thread_is_preemptible(மாறிலி கட்டமைப்பு k_thread *thread)
{
	/* explanation in kernel_struct.h */
	திரும்பு thread->base.preempt <= _PREEMPT_THRESHOLD;
}


நிலையான உள்வரி முழு thread_is_metairq(மாறிலி கட்டமைப்பு k_thread *thread)
{
#எனில் CONFIG_இலக்கம்_METAIRQ_PRIORITIES > 0
	திரும்பு (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_இலக்கம்_METAIRQ_PRIORITIES;
#இல்லையெனில்
	ARG_UNUSED(thread);
	திரும்பு 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

நிலையான உள்வரி bool is_thread_dummy(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

நிலையான உள்வரி bool z_is_thread_suspended(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

நிலையான உள்வரி bool z_is_thread_pending(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

நிலையான உள்வரி bool z_is_thread_dead(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
நிலையான உள்வரி bool z_is_thread_aborting(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
நிலையான உள்வரி bool z_is_thread_halting(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


நிலையான உள்வரி bool z_is_thread_prevented_from_running(மாறிலி கட்டமைப்பு k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	திரும்பு (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

நிலையான உள்வரி bool z_is_thread_timeout_active(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு !z_is_inactive_timeout(&thread->base.timeout);
}

நிலையான உள்வரி bool z_is_thread_ready(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு !z_is_thread_prevented_from_running(thread);
}

நிலையான உள்வரி bool z_is_thread_state_set(மாறிலி கட்டமைப்பு k_thread *thread, uint32_t state)
{
	திரும்பு (thread->base.thread_state & state) != 0U;
}

நிலையான உள்வரி bool z_is_thread_queued(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு z_is_thread_state_set(thread, _THREAD_QUEUED);
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_queued(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_not_queued(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_suspended(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_not_suspended(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_pending(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_not_pending(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

நிலையான உள்வரி bool z_is_thread_sleeping(கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_sleeping(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

நிலையான உள்வரி வெறுமை z_mark_thread_as_not_sleeping(கட்டமைப்பு k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
நிலையான உள்வரி வெறுமை z_thread_essential_set(கட்டமைப்பு k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
நிலையான உள்வரி வெறுமை z_thread_essential_clear(கட்டமைப்பு k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
நிலையான உள்வரி bool z_is_thread_essential(மாறிலி கட்டமைப்பு k_thread *thread)
{
	திரும்பு (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


நிலையான ALWAYS_INLINE bool should_preempt(மாறிலி கட்டமைப்பு k_thread *thread,
					 முழு preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	எனில் (preempt_ok != 0) {
		திரும்பு true;
	}

	__ASSERT(_current != வெற்று, "");

	/* Or if we're pended/suspended/dummy (duh) */
	எனில் (z_is_thread_prevented_from_running(_current)) {
		திரும்பு true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	எனில் (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		திரும்பு true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	எனில் (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		திரும்பு true;
	}

	திரும்பு false;
}


நிலையான உள்வரி bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	திரும்பு entry_point == idle;
}

நிலையான உள்வரி bool z_is_idle_thread_object(மாறிலி கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	திரும்பு thread->base.is_idle;
#இல்லையெனில்
	திரும்பு thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#இல்லையெனில்
	திரும்பு false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
