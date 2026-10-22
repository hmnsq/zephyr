/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <timeout_q.h>


#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_DUMMY       "dummy"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_PENDING     "pending"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_SLEEPING    "sleeping"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_DEAD        "dead"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_SUSPENDED   "suspended"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_ABORTING    "aborting"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_SUSPENDING  "suspending"
#ವ್ಯಾಖ್ಯಾನಿಸು Z_STATE_STR_QUEUED      "queued"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
ಬಾಹ್ಯ ರಚನೆ k_spinlock z_thread_monitor_ಬೀಗ;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_MONITOR */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MULTITHREADING
ಬಾಹ್ಯ ರಚನೆ k_thread z_idle_ಎಳೆಗಳು[CONFIG_MP_MAX_ಅಂಕೆ_CPUS];
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MULTITHREADING */

ಶೂನ್ಯ idle(ಶೂನ್ಯ *unused1, ಶೂನ್ಯ *unused2, ಶೂನ್ಯ *unused3);

/* clean up when a thread is aborted */

#ಆದರೆ defined(CONFIG_THREAD_MONITOR)
ಶೂನ್ಯ z_thread_monitor_exit(ರಚನೆ k_thread *thread);
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು z_thread_monitor_exit(thread) \
	ಮಾಡು {/* nothing */    \
	} ಆಗ (false)
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_THREAD_MONITOR */

ಶೂನ್ಯ z_thread_abort(ರಚನೆ k_thread *thread);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ thread_ವೇಳಾಪಟ್ಟಿ_new(ರಚನೆ k_thread *thread, k_timeout_t delay)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_CLOCK_EXISTS
	ಆದರೆ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_ಆರಂಭ(thread);
	} ಇಲ್ಲದಿದ್ದರೆ {
		z_ಕೂಡಿಸು_thread_timeout(thread, delay);
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_ಆರಂಭ(thread);
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_CLOCK_EXISTS */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ thread_is_preemptible(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ಹಿಂತಿರುಗಿ thread->ತಳ.preempt <= PREEMPT_ಮಿತಿ;
}


ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ thread_is_metairq(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
#ಆದರೆ CONFIG_ಅಂಕೆ_METAIRQ_PRIORITIES > 0
	ಹಿಂತಿರುಗಿ (thread->ತಳ.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_ಅಂಕೆ_METAIRQ_PRIORITIES;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool is_thread_dummy(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & _THREAD_DUMMY) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_suspended(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & _THREAD_SUSPENDED) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_pending(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & _THREAD_PENDING) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_dead(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_aborting(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_halting(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_prevented_from_running(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	uint8_t state = thread->ತಳ.thread_state;

	ಹಿಂತಿರುಗಿ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_timeout_ಸಕ್ರಿಯ(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ !z_is_inactive_timeout(&thread->ತಳ.timeout);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_ready(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ !z_is_thread_prevented_from_running(thread);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_state_ಗಣ(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread, uint32_t state)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & state) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_queued(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ z_is_thread_state_ಗಣ(thread, _THREAD_QUEUED);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_queued(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state |= _THREAD_QUEUED;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_not_queued(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state &= ~_THREAD_QUEUED;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_suspended(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state |= _THREAD_SUSPENDED;

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_not_suspended(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state &= ~_THREAD_SUSPENDED;

	SYS_ಪೋರ್ಟ್_TRACING_FUNC(k_thread, sched_resume, thread);
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_pending(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state |= _THREAD_PENDING;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_not_pending(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state &= ~_THREAD_PENDING;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_sleeping(ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.thread_state & _THREAD_SLEEPING) != 0U;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_sleeping(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state |= _THREAD_SLEEPING;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_mark_thread_as_not_sleeping(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_thread_essential_ಗಣ(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_thread_essential_clear(ರಚನೆ k_thread *thread)
{
	thread->ತಳ.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_thread_essential(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ (thread->ತಳ.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ಸ್ಥಿರ ALWAYS_INLINE bool should_preempt(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread,
					 ಪೂರ್ಣಾಂಕ preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ಆದರೆ (preempt_ok != 0) {
		ಹಿಂತಿರುಗಿ true;
	}

	__ASSERT(_current != ಖಾಲಿ, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ಆದರೆ (z_is_thread_prevented_from_running(_current)) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ಆದರೆ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ಹಿಂತಿರುಗಿ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_ಅದಲುಬದಲು_NONATOMIC)
	    && z_is_thread_timeout_ಸಕ್ರಿಯ(thread)) {
		ಹಿಂತಿರುಗಿ true;
	}

	ಹಿಂತಿರುಗಿ false;
}


ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_idle_thread_ನಮೂದು(k_thread_ನಮೂದು_t ನಮೂದು_ಬಿಂದು)
{
	ಹಿಂತಿರುಗಿ ನಮೂದು_ಬಿಂದು == idle;
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool z_is_idle_thread_object(ಸ್ಥಿರಾಂಕ ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MULTITHREADING
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಹಿಂತಿರುಗಿ thread->ತಳ.is_idle;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ thread == &z_idle_ಎಳೆಗಳು[0];
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ false;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_MULTITHREADING */
}


#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
