/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_KSCHED_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_KSCHED_H_

#అన్తర్భావ <zephyr/kernel_structs.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <kernel_internal.h>
#అన్తర్భావ <timeout_q.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <zephyr/tracing/tracing.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <priority_q.h>

BUILD_ASSERT(K_అత్యల్ప_APPLICATION_THREAD_PRIO
	     >= K_HIGHEST_APPLICATION_THREAD_PRIO);

#నిర్వచితమైతే CONFIG_MULTITHREADING
#నిర్వచించు Z_VALID_PRIO(prio, నమోదు_బిందువు)				     \
	(((prio) == K_IDLE_PRIO && z_is_idle_thread_నమోదు(నమోదు_బిందువు)) || \
	 ((K_అత్యల్ప_APPLICATION_THREAD_PRIO			     \
	   >= K_HIGHEST_APPLICATION_THREAD_PRIO)		     \
	  && (prio) >= K_HIGHEST_APPLICATION_THREAD_PRIO	     \
	  && (prio) <= K_అత్యల్ప_APPLICATION_THREAD_PRIO))

#నిర్వచించు Z_ASSERT_VALID_PRIO(prio, నమోదు_బిందువు) చేయి { \
	__ASSERT(Z_VALID_PRIO((prio), (నమోదు_బిందువు)), \
		 "invalid priority (%d); allowed range: %d to %d", \
		 (prio), \
		 K_అత్యల్ప_APPLICATION_THREAD_PRIO, \
		 K_HIGHEST_APPLICATION_THREAD_PRIO); \
	} అయితే (false)
#లేకపోతే
#నిర్వచించు Z_VALID_PRIO(prio, నమోదు_బిందువు) ((prio) == -1)
#నిర్వచించు Z_ASSERT_VALID_PRIO(prio, నమోదు_బిందువు) __ASSERT((prio) == -1, "")
#అయితే_ముగింపు /* CONFIG_MULTITHREADING */

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు

బాహ్య నిర్మాణం k_thread _thread_dummy;

/**
 * @brief Unpend thread, do not abort its timeout (if it exists).
 */
శూన్య z_unpend_thread_no_timeout(నిర్మాణం k_thread *thread);

/**
 * @brief Unpend thread and abort its timeout (if it exists).
 */
శూన్య z_unpend_thread(నిర్మాణం k_thread *thread);
నిర్మాణం k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q);
పూర్ణాంకం z_pend_curr(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం,
	       _wait_q_t *wait_q, k_timeout_t timeout);
శూన్య z_pend_thread(నిర్మాణం k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout);
శూన్య z_reschedule(నిర్మాణం k_spinlock *తాళం, k_spinlock_కీలకం_t కీలకం);
శూన్య z_reschedule_locked(k_spinlock_కీలకం_t కీలకం);
శూన్య z_reschedule_irqlock(uint32_t కీలకం);
పూర్ణాంకం z_unpend_all(_wait_q_t *wait_q);
bool z_thread_prio_సమితి(నిర్మాణం k_thread *thread, పూర్ణాంకం prio);
శూన్య *z_get_తదుపరి_switch_handle(శూన్య *interrupted);
శూన్య z_thread_suspend_current(నిర్మాణం k_thread *thread);

/* Wrapper around z_get_next_switch_handle() for the benefit of
 * non-SMP platforms that always pass a NULL interrupted handle.
 * Exposes the (extremely) common early exit case in a context that
 * can be (much) better optimized in surrounding C code.  Takes a
 * _current pointer that the outer scope surely already has to avoid
 * having to refetch it across a lock.
 *
 * Mystic arts for cycle nerds, basically.  Replace with
 * z_get_next_switch_handle() if this becomes a maintenance hassle.
 */
స్థిర ALWAYS_INLINE శూన్య *z_sched_తదుపరి_handle(నిర్మాణం k_thread *curr)
{
#నిర్వచితం_కాకపోతే CONFIG_SMP
	ఒకవేళ (curr == _kernel.ready_q.cache) {
		తిరిగి ఖాళీ;
	}
#అయితే_ముగింపు
	తిరిగి z_get_తదుపరి_switch_handle(ఖాళీ);
}

శూన్య z_sched_ప్రారంభం(నిర్మాణం k_thread *thread);
శూన్య z_ready_thread(నిర్మాణం k_thread *thread);
నిర్మాణం k_thread *z_మార్పిడి_తదుపరి_thread(శూన్య);
శూన్య move_current_గమ్యం_చివర_యొక్క_prio_q(శూన్య);

/*
 * Internal scheduler functions exposed for use by thread lifecycle code
 * (thread.c). These operate under the scheduler's spinlock and must not be
 * called without holding it.
 */

/**
 * @brief Halt a thread, suspending or terminating it.
 *
 * Shared implementation for k_thread_suspend() and k_thread_abort(). The
 * caller must hold the scheduler's spinlock; this function releases it before
 * returning (possibly after a context switch).
 *
 * @param thread  Thread to halt.
 * @param key     Scheduler spinlock key held by the caller.
 * @param terminate true to abort (kill) the thread, false to suspend it.
 */
శూన్య z_thread_halt(నిర్మాణం k_thread *thread, k_spinlock_కీలకం_t కీలకం, bool terminate);

/**
 * @brief Ready a thread while the scheduler spinlock is already held.
 *
 * Equivalent to the internal ready_thread() helper. Callers must hold the
 * scheduler's spinlock.
 *
 * @param thread Thread to make ready.
 */
శూన్య z_sched_ready_locked(నిర్మాణం k_thread *thread);

/**
 * @brief Add a thread to a wait queue while the scheduler spinlock is held.
 *
 * Moves the thread out of the run queue and onto the specified wait queue.
 * Callers must hold the scheduler's spinlock.
 *
 * @param thread  Thread to pend.
 * @param wait_q  Wait queue to add the thread to (may be NULL).
 */
శూన్య z_sched_కూడు_గమ్యం_waitq_locked(నిర్మాణం k_thread *thread, _wait_q_t *wait_q);

/**
 * @brief Remove a thread from the run queue (scheduler spinlock must be held).
 *
 * Called by sleep.c to unready the current thread before arming its wakeup
 * timeout.  Callers must already hold the scheduler's spinlock.
 *
 * @param thread Thread to remove from the run queue.
 */
శూన్య z_sched_unready_locked(నిర్మాణం k_thread *thread);

/**
 * @brief Yield the current thread's remaining time slice.
 *
 * Moves _current to the end of its priority group in the run queue, updates
 * the scheduler cache, and context-switches away.  Equivalent to the body
 * of k_yield(); exposed so that thread.c can implement k_yield() without
 * depending on sched.c internals.
 */
శూన్య z_sched_yield(శూన్య);

స్థిర అంతఃవరుస శూన్య z_reschedule_unlocked(శూన్య)
{
	(శూన్య) z_reschedule_irqlock(arch_irq_తాళం());
}

స్థిర అంతఃవరుస bool z_is_under_prio_ceiling(పూర్ణాంకం prio)
{
	తిరిగి prio >= CONFIG_ప్రాధాన్యత_CEILING;
}

స్థిర అంతఃవరుస పూర్ణాంకం z_get_new_prio_with_ceiling(పూర్ణాంకం prio)
{
	తిరిగి z_is_under_prio_ceiling(prio) ? prio : CONFIG_ప్రాధాన్యత_CEILING;
}

స్థిర అంతఃవరుస bool z_is_prio1_higher_than_or_equal_గమ్యం_prio2(పూర్ణాంకం prio1, పూర్ణాంకం prio2)
{
	తిరిగి prio1 <= prio2;
}

స్థిర అంతఃవరుస bool z_is_prio_higher_or_equal(పూర్ణాంకం prio1, పూర్ణాంకం prio2)
{
	తిరిగి z_is_prio1_higher_than_or_equal_గమ్యం_prio2(prio1, prio2);
}

స్థిర అంతఃవరుస bool z_is_prio1_క్రింది_than_or_equal_గమ్యం_prio2(పూర్ణాంకం prio1, పూర్ణాంకం prio2)
{
	తిరిగి prio1 >= prio2;
}

స్థిర అంతఃవరుస bool z_is_prio1_higher_than_prio2(పూర్ణాంకం prio1, పూర్ణాంకం prio2)
{
	తిరిగి prio1 < prio2;
}

స్థిర అంతఃవరుస bool z_is_prio_higher(పూర్ణాంకం prio, పూర్ణాంకం test_prio)
{
	తిరిగి z_is_prio1_higher_than_prio2(prio, test_prio);
}

స్థిర అంతఃవరుస bool z_is_prio_క్రింది_or_equal(పూర్ణాంకం prio1, పూర్ణాంకం prio2)
{
	తిరిగి z_is_prio1_క్రింది_than_or_equal_గమ్యం_prio2(prio1, prio2);
}

స్థిర అంతఃవరుస bool _is_valid_prio(పూర్ణాంకం prio, k_thread_నమోదు_t నమోదు_బిందువు)
{
	ఒకవేళ ((prio == K_IDLE_PRIO) && z_is_idle_thread_నమోదు(నమోదు_బిందువు)) {
		తిరిగి true;
	}

	ఒకవేళ (!z_is_prio_higher_or_equal(prio,
				       K_అత్యల్ప_APPLICATION_THREAD_PRIO)) {
		తిరిగి false;
	}

	ఒకవేళ (!z_is_prio_క్రింది_or_equal(prio,
				      K_HIGHEST_APPLICATION_THREAD_PRIO)) {
		తిరిగి false;
	}

	తిరిగి true;
}

స్థిర ALWAYS_INLINE _wait_q_t *pended_on_thread(నిర్మాణం k_thread *thread)
{
	__ASSERT_NO_MSG(thread->పునాది.pended_on);

	తిరిగి thread->పునాది.pended_on;
}


స్థిర అంతఃవరుస శూన్య unpend_thread_no_timeout(నిర్మాణం k_thread *thread)
{
	_priq_wait_remove(&pended_on_thread(thread)->waitq, thread);
	z_mark_thread_as_not_pending(thread);
	thread->పునాది.pended_on = ఖాళీ;
}

/*
 * Pick the best-priority waiter from @p wait_q, unpend it, and abort any
 * timeout it had pending. Returns the thread, or NULL if the wait_q was
 * empty.
 *
 * Caller MUST hold the scheduler's spinlock and MUST complete the wake (set
 * return value, ready the thread) under the same lock acquisition before
 * releasing it. Otherwise a racing in-flight timeout handler can ready the
 * thread before the caller's wake state is in place, exposing the woken thread
 * to a stale swap_retval. The pre-1b8c7a3 dticks-cancel check used to
 * close this window; doing all the wake work atomically under the sched
 * lock is now the way.
 *
 * z_sched_wake() is the convenient wrapper for the common case of
 * "wake one waiter with this retval and this swap_data".
 */
స్థిర ALWAYS_INLINE నిర్మాణం k_thread *z_unpend_మొదటి_thread_locked(_wait_q_t *wait_q)
{
	నిర్మాణం k_thread *thread = priq_wait_ఉత్తమ(&wait_q->waitq);

	ఒకవేళ (unlikely(thread != ఖాళీ)) {
		unpend_thread_no_timeout(thread);
		/* Abort the thread's timeout. If its handler is in flight on
		 * another CPU it is blocked on the sched lock; the abort flags
		 * it superseded, and z_thread_timeout() bails on that flag when
		 * it finally runs -- so it won't wake the thread from whatever
		 * it has since re-pended on. We don't wait for it here.
		 */
		(శూన్య)z_try_abort_thread_timeout(thread);
	}
	తిరిగి thread;
}

/**
 * Wake up a thread pending on the provided wait queue
 *
 * Given a wait_q, wake up the highest priority thread on the queue. If the
 * queue was empty just return false.
 *
 * Otherwise, do the following, in order,  holding the scheduler's spinlock the
 *  entire time so that the thread state is guaranteed not to change:
 * - Set the thread's swap return values to swap_retval and swap_data
 * - un-pend and ready the thread, but do not invoke the scheduler.
 *
 * Repeated calls to this function until it returns false is a suitable
 * way to wake all threads on the queue.
 *
 * It is up to the caller to implement locking such that the return value of
 * this function (whether a thread was woken up or not) does not immediately
 * become stale. Calls to wait and wake on the same wait_q object must have
 * synchronization. Calling this without holding any spinlock is a sign that
 * this API is not being used properly.
 *
 * @param wait_q Wait queue to wake up the highest prio thread
 * @param swap_retval Swap return value for woken thread
 * @param swap_data Data return value to supplement swap_retval. May be NULL.
 * @retval true If a thread was woken up
 * @retval false If the wait_q was empty
 */
స్థిర ALWAYS_INLINE bool z_sched_wake(_wait_q_t *wait_q, పూర్ణాంకం మార్పిడి_retval, శూన్య *మార్పిడి_దత్తాంశం)
{
	నిర్మాణం k_thread *thread;
	bool ret = false;

	తాళం_SCHED_SPINLOCK {
		thread = z_unpend_మొదటి_thread_locked(wait_q);
		ఒకవేళ (thread != ఖాళీ) {
			z_thread_return_విలువ_సమితి_with_దత్తాంశం(thread,
							    మార్పిడి_retval,
							    మార్పిడి_దత్తాంశం);
			z_sched_ready_locked(thread);
			ret = true;
		}
	}

	తిరిగి ret;
}

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_KSCHED_H_ */
