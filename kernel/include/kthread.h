/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 * Copyright (c) 2024 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_THREAD_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_THREAD_H_

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <timeout_q.h>


#నిర్వచించు Z_STATE_STR_DUMMY       "dummy"
#నిర్వచించు Z_STATE_STR_PENDING     "pending"
#నిర్వచించు Z_STATE_STR_SLEEPING    "sleeping"
#నిర్వచించు Z_STATE_STR_DEAD        "dead"
#నిర్వచించు Z_STATE_STR_SUSPENDED   "suspended"
#నిర్వచించు Z_STATE_STR_ABORTING    "aborting"
#నిర్వచించు Z_STATE_STR_SUSPENDING  "suspending"
#నిర్వచించు Z_STATE_STR_QUEUED      "queued"

#నిర్వచితమైతే CONFIG_THREAD_MONITOR
/* This lock protects the linked list of active threads; i.e. the
 * initial _kernel.threads pointer and the linked list made up of
 * thread->next_thread (until NULL)
 */
బాహ్య నిర్మాణం k_spinlock z_thread_monitor_తాళం;
#అయితే_ముగింపు /* CONFIG_THREAD_MONITOR */

#నిర్వచితమైతే CONFIG_MULTITHREADING
బాహ్య నిర్మాణం k_thread z_idle_దారాలు[CONFIG_MP_MAX_అంకె_CPUS];
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */

శూన్య idle(శూన్య *unused1, శూన్య *unused2, శూన్య *unused3);

/* clean up when a thread is aborted */

#ఒకవేళ defined(CONFIG_THREAD_MONITOR)
శూన్య z_thread_monitor_exit(నిర్మాణం k_thread *thread);
#లేకపోతే
#నిర్వచించు z_thread_monitor_exit(thread) \
	చేయి {/* nothing */    \
	} అయితే (false)
#అయితే_ముగింపు /* CONFIG_THREAD_MONITOR */

శూన్య z_thread_abort(నిర్మాణం k_thread *thread);

స్థిర అంతఃవరుస శూన్య thread_కాలపట్టిక_new(నిర్మాణం k_thread *thread, k_timeout_t delay)
{
#నిర్వచితమైతే CONFIG_SYS_CLOCK_EXISTS
	ఒకవేళ (K_TIMEOUT_EQ(delay, K_NO_WAIT)) {
		k_thread_ప్రారంభం(thread);
	} లేకపోతే {
		z_కూడు_thread_timeout(thread, delay);
	}
#లేకపోతే
	ఒకవేళ (!K_TIMEOUT_EQ(delay, K_FOREVER)) {
		k_thread_ప్రారంభం(thread);
	}
#అయితే_ముగింపు /* CONFIG_SYS_CLOCK_EXISTS */
}

స్థిర అంతఃవరుస పూర్ణాంకం thread_is_preemptible(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	/* explanation in kernel_struct.h */
	తిరిగి thread->పునాది.preempt <= PREEMPT_పరిమితి;
}


స్థిర అంతఃవరుస పూర్ణాంకం thread_is_metairq(స్థిరరాశి నిర్మాణం k_thread *thread)
{
#ఒకవేళ CONFIG_అంకె_METAIRQ_PRIORITIES > 0
	తిరిగి (thread->పునాది.prio - K_HIGHEST_THREAD_PRIO)
		< CONFIG_అంకె_METAIRQ_PRIORITIES;
#లేకపోతే
	ARG_వాడని(thread);
	తిరిగి 0;
#అయితే_ముగింపు /* CONFIG_NUM_METAIRQ_PRIORITIES */
}

స్థిర అంతఃవరుస bool is_thread_dummy(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state & _THREAD_DUMMY) != 0U;
}

స్థిర అంతఃవరుస bool z_is_thread_suspended(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state & _THREAD_SUSPENDED) != 0U;
}

స్థిర అంతఃవరుస bool z_is_thread_pending(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state & _THREAD_PENDING) != 0U;
}

స్థిర అంతఃవరుస bool z_is_thread_dead(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state & _THREAD_DEAD) != 0U;
}

/* Return true if the thread is aborting, else false */
స్థిర అంతఃవరుస bool z_is_thread_aborting(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state & _THREAD_ABORTING) != 0U;
}

/* Return true if the thread is aborting or suspending, else false */
స్థిర అంతఃవరుస bool z_is_thread_halting(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state &
		(_THREAD_ABORTING | _THREAD_SUSPENDING)) != 0U;
}


స్థిర అంతఃవరుస bool z_is_thread_prevented_from_running(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	uint8_t state = thread->పునాది.thread_state;

	తిరిగి (state & (_THREAD_PENDING | _THREAD_SLEEPING | _THREAD_DEAD |
			 _THREAD_DUMMY | _THREAD_SUSPENDED)) != 0U;
}

స్థిర అంతఃవరుస bool z_is_thread_timeout_చురుకైన(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి !z_is_inactive_timeout(&thread->పునాది.timeout);
}

స్థిర అంతఃవరుస bool z_is_thread_ready(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి !z_is_thread_prevented_from_running(thread);
}

స్థిర అంతఃవరుస bool z_is_thread_state_సమితి(స్థిరరాశి నిర్మాణం k_thread *thread, uint32_t state)
{
	తిరిగి (thread->పునాది.thread_state & state) != 0U;
}

స్థిర అంతఃవరుస bool z_is_thread_queued(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి z_is_thread_state_సమితి(thread, _THREAD_QUEUED);
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_queued(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state |= _THREAD_QUEUED;
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_not_queued(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state &= ~_THREAD_QUEUED;
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_suspended(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state |= _THREAD_SUSPENDED;

	SYS_పోర్టు_TRACING_FUNC(k_thread, sched_suspend, thread);
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_not_suspended(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state &= ~_THREAD_SUSPENDED;

	SYS_పోర్టు_TRACING_FUNC(k_thread, sched_resume, thread);
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_pending(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state |= _THREAD_PENDING;
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_not_pending(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state &= ~_THREAD_PENDING;
}

స్థిర అంతఃవరుస bool z_is_thread_sleeping(నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.thread_state & _THREAD_SLEEPING) != 0U;
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_sleeping(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state |= _THREAD_SLEEPING;
}

స్థిర అంతఃవరుస శూన్య z_mark_thread_as_not_sleeping(నిర్మాణం k_thread *thread)
{
	thread->పునాది.thread_state &= ~_THREAD_SLEEPING;
}

/*
 * This function tags the current thread as essential to system operation.
 * Exceptions raised by this thread will be treated as a fatal system error.
 */
స్థిర అంతఃవరుస శూన్య z_thread_essential_సమితి(నిర్మాణం k_thread *thread)
{
	thread->పునాది.user_options |= K_ESSENTIAL;
}

/*
 * This function tags the current thread as not essential to system operation.
 * Exceptions raised by this thread may be recoverable.
 * (This is the default tag for a thread.)
 */
స్థిర అంతఃవరుస శూన్య z_thread_essential_clear(నిర్మాణం k_thread *thread)
{
	thread->పునాది.user_options &= ~K_ESSENTIAL;
}

/*
 * This routine indicates if the current thread is an essential system thread.
 *
 * Returns true if current thread is essential, false if it is not.
 */
స్థిర అంతఃవరుస bool z_is_thread_essential(స్థిరరాశి నిర్మాణం k_thread *thread)
{
	తిరిగి (thread->పునాది.user_options & K_ESSENTIAL) == K_ESSENTIAL;
}


స్థిర ALWAYS_INLINE bool should_preempt(స్థిరరాశి నిర్మాణం k_thread *thread,
					 పూర్ణాంకం preempt_ok)
{
	/* Preemption is OK if it's being explicitly allowed by
	 * software state (e.g. the thread called k_yield())
	 */
	ఒకవేళ (preempt_ok != 0) {
		తిరిగి true;
	}

	__ASSERT(_current != ఖాళీ, "");

	/* Or if we're pended/suspended/dummy (duh) */
	ఒకవేళ (z_is_thread_prevented_from_running(_current)) {
		తిరిగి true;
	}

	/* Otherwise we have to be running a preemptible thread or
	 * switching to a metairq
	 */
	ఒకవేళ (thread_is_preemptible(_current) || thread_is_metairq(thread)) {
		తిరిగి true;
	}

	/* Edge case on ARM where a thread can be pended out of an
	 * interrupt handler before the "synchronous" swap starts
	 * context switching.  Platforms with atomic swap can never
	 * hit this.
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_మార్పిడి_NONATOMIC)
	    && z_is_thread_timeout_చురుకైన(thread)) {
		తిరిగి true;
	}

	తిరిగి false;
}


స్థిర అంతఃవరుస bool z_is_idle_thread_నమోదు(k_thread_నమోదు_t నమోదు_బిందువు)
{
	తిరిగి నమోదు_బిందువు == idle;
}

స్థిర అంతఃవరుస bool z_is_idle_thread_object(స్థిరరాశి నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_MULTITHREADING
#నిర్వచితమైతే CONFIG_SMP
	తిరిగి thread->పునాది.is_idle;
#లేకపోతే
	తిరిగి thread == &z_idle_దారాలు[0];
#అయితే_ముగింపు /* CONFIG_SMP */
#లేకపోతే
	తిరిగి false;
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */
}


#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_THREAD_H_ */
