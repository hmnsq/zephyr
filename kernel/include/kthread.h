/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#define ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <timeout_q.h>


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
ಬಾಹ್ಯ ಸಂರಚನಾ k_spinlock z_thread_monitor_lock;
#endif /* CONFIG_THREAD_MONITOR */

#ifdef CONFIG_MULTITHREADING
ಬಾಹ್ಯ ಸಂರಚನಾ k_thread z_idle_threads[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];
#endif /* CONFIG_MULTITHREADING */

ಶೂನ್ಯ idle(ಶೂನ್ಯ *unused1, ಶೂನ್ಯ *unused2, ಶೂನ್ಯ *unused3);

/* clean up when a thread is aborted */

#ಯದಿ defined(CONFIG_THREAD_MONITOR)
ಶೂನ್ಯ z_thread_monitor_exit(ಸಂರಚನಾ k_thread *thread);
#ಅನ್ಯಥಾ
#define z_thread_monitor_exit(thread) \
	ಕುರು {/* nothing */    \
	} ಯಾವತ್ (false)
#endif /* CONFIG_THREAD_MONITOR */

ಶೂನ್ಯ z_thread_abort(ಸಂರಚನಾ k_thread *thread);

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ thread_schedule_new(ಸಂರಚನಾ k_thread *thread, k_timeout_t delay)
{
#ifdef CONFIG_SYS_CLOCK_EXISTS
	ಯದಿ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_start(thread);
	} ಅನ್ಯಥಾ {
		z_add_thread_timeout(thread, delay);
	}
#ಅನ್ಯಥಾ
	ಯದಿ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_start(thread);
	}
#endif /* CONFIG_SYS_CLOCK_EXISTS */
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ thread_is_preemptible(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ಪ್ರತ್ಯಾಗಮ thread->base.preempt <= _PREEMPT_THRESHOLD;
}


ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ thread_is_metairq(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
#ಯದಿ CONFIG_ಅಂಕೆ_METAIRQ_PRIORITIES > 0
	ಪ್ರತ್ಯಾಗಮ (thread->base.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_ಅಂಕೆ_METAIRQ_PRIORITIES;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ಪ್ರತ್ಯಾಗಮ 0;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool is_thread_dummy(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & _THREAD_DUMMY) != 0U;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_suspended(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & _THREAD_SUSPENDED) != 0U;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_pending(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & _THREAD_PENDING) != 0U;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_dead(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_aborting(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_halting(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_prevented_from_running(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	uint8_t state = thread->base.thread_state;

	ಪ್ರತ್ಯಾಗಮ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_timeout_active(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ !z_is_inactive_timeout(&thread->base.timeout);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_ready(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ !z_is_thread_prevented_from_running(thread);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_state_set(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread, uint32_t state)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & state) != 0U;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_queued(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ z_is_thread_state_set(thread, _THREAD_QUEUED);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_queued(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state |= _THREAD_QUEUED;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_not_queued(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_QUEUED;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_suspended(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_not_suspended(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SUSPENDED;

	SYS_PORT_TRACING_FUNC(k_thread, sched_resume, thread);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_pending(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state |= _THREAD_PENDING;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_not_pending(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_PENDING;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_sleeping(ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.thread_state & _THREAD_SLEEPING) != 0U;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_sleeping(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state |= _THREAD_SLEEPING;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_mark_thread_as_not_sleeping(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_thread_essential_set(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_thread_essential_clear(ಸಂರಚನಾ k_thread *thread)
{
	thread->base.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_thread_essential(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ (thread->base.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ಸ್ಥಿರ ALWAYS_INLINE bool should_preempt(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread,
					 ಪೂರ್ಣಾಙ್ಕ preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ಯದಿ (preempt_ok != 0) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	__ASSERT(_current != ರಿಕ್ತ, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ಯದಿ (z_is_thread_prevented_from_running(_current)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ಯದಿ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ಯದಿ (IS_ENABLED(CONFIG_SWAP_NONATOMIC)
	    && z_is_thread_timeout_active(thread)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಪ್ರತ್ಯಾಗಮ false;
}


ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_idle_thread_entry(k_thread_entry_t entry_point)
{
	ಪ್ರತ್ಯಾಗಮ entry_point == idle;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_idle_thread_object(ಧ್ರುವ ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_MULTITHREADING
#ifdef CONFIG_SMP
	ಪ್ರತ್ಯಾಗಮ thread->base.is_idle;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ thread == &z_idle_threads[0];
#endif /* CONFIG_SMP */
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ false;
#endif /* CONFIG_MULTITHREADING */
}


#endif /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
