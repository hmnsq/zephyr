/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeout_q.h>


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_DUMMY       "dummy"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_PENDING     "pending"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_SLEEPING    "sleeping"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_DEAD        "dead"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_SUSPENDED   "suspended"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_ABORTING    "aborting"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_SUSPENDING  "suspending"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_STATE_STR_QUEUED      "queued"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
ਬਾਹਰੀ ਬਣਤਰ k_spinlock z_thread_monitor_ਤਾਲਾ;
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_MONITOR */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
ਬਾਹਰੀ ਬਣਤਰ k_thread z_idle_ਧਾਗੇ[CONFIG_MP_MAX_ਅੰਕ_CPUS];
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */

ਖਾਲੀ idle(ਖਾਲੀ *unused1, ਖਾਲੀ *unused2, ਖਾਲੀ *unused3);

/* clean up when a thread is aborted */

#ਜੇ defined(CONFIG_THREAD_MONITOR)
ਖਾਲੀ z_thread_monitor_exit(ਬਣਤਰ k_thread *thread);
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ z_thread_monitor_exit(thread) \
	ਕਰੋ {/* nothing */    \
	} ਜਦੋਂ (false)
#ਜੇ_ਅੰਤ /* CONFIG_THREAD_MONITOR */

ਖਾਲੀ z_thread_abort(ਬਣਤਰ k_thread *thread);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ thread_ਸਮਾਂ_ਸਾਰਣੀ_new(ਬਣਤਰ k_thread *thread, k_timeout_t delay)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_CLOCK_EXISTS
	ਜੇ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_ਸ਼ੁਰੂ(thread);
	} ਨਹੀਂਤਾਂ {
		z_ਜੋੜੋ_thread_timeout(thread, delay);
	}
#ਨਹੀਂਤਾਂ
	ਜੇ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_ਸ਼ੁਰੂ(thread);
	}
#ਜੇ_ਅੰਤ /* CONFIG_SYS_CLOCK_EXISTS */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ thread_is_preemptible(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	/* explanation in kernel_struct.h */
	ਵਾਪਸ thread->ਅਧਾਰ.preempt <= PREEMPT_ਹੱਦ;
}


ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ thread_is_metairq(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
#ਜੇ CONFIG_ਅੰਕ_METAIRQ_PRIORITIES > 0
	ਵਾਪਸ (thread->ਅਧਾਰ.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_ਅੰਕ_METAIRQ_PRIORITIES;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool is_thread_dummy(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & _THREAD_DUMMY) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_suspended(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & _THREAD_SUSPENDED) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_pending(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & _THREAD_PENDING) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_dead(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_aborting(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_halting(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_prevented_from_running(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	uint8_t state = thread->ਅਧਾਰ.thread_state;

	ਵਾਪਸ (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_timeout_ਸਰਗਰਮ(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ !z_is_inactive_timeout(&thread->ਅਧਾਰ.timeout);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_ready(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ !z_is_thread_prevented_from_running(thread);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_state_ਸੈੱਟ(ਅਚਲ ਬਣਤਰ k_thread *thread, uint32_t state)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & state) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_queued(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ z_is_thread_state_ਸੈੱਟ(thread, _THREAD_QUEUED);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_queued(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state |= _THREAD_QUEUED;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_not_queued(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state &= ~_THREAD_QUEUED;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_suspended(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state |= _THREAD_SUSPENDED;

	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, sched_suspend, thread);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_not_suspended(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state &= ~_THREAD_SUSPENDED;

	SYS_ਪੋਰਟ_TRACING_FUNC(k_thread, sched_resume, thread);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_pending(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state |= _THREAD_PENDING;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_not_pending(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state &= ~_THREAD_PENDING;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_sleeping(ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.thread_state & _THREAD_SLEEPING) != 0U;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_sleeping(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state |= _THREAD_SLEEPING;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_mark_thread_as_not_sleeping(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_thread_essential_ਸੈੱਟ(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_thread_essential_clear(ਬਣਤਰ k_thread *thread)
{
	thread->ਅਧਾਰ.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_thread_essential(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ (thread->ਅਧਾਰ.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


ਸਥਿਰ ALWAYS_INLINE bool should_preempt(ਅਚਲ ਬਣਤਰ k_thread *thread,
					 ਪੂਰਨ preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ਜੇ (preempt_ok != 0) {
		ਵਾਪਸ true;
	}

	__ASSERT(_current != ਨਿਰਾ, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ਜੇ (z_is_thread_prevented_from_running(_current)) {
		ਵਾਪਸ true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ਜੇ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		ਵਾਪਸ true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ਜੇ (IS_ENABLED(CONFIG_ਅਦਲਾ_ਬਦਲੀ_NONATOMIC)
	    && z_is_thread_timeout_ਸਰਗਰਮ(thread)) {
		ਵਾਪਸ true;
	}

	ਵਾਪਸ false;
}


ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_idle_thread_ਇੰਦਰਾਜ(k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ_ਬਿੰਦੂ)
{
	ਵਾਪਸ ਇੰਦਰਾਜ_ਬਿੰਦੂ == idle;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_idle_thread_object(ਅਚਲ ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਵਾਪਸ thread->ਅਧਾਰ.is_idle;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ thread == &z_idle_ਧਾਗੇ[0];
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ false;
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */
}


#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
