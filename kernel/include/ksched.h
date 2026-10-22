/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_KSCHED_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_KSCHED_H_

#شامل <zephyr/kernel_structs.h>
#شامل <kspinlock.h>
#شامل <kernel_internal.h>
#شامل <timeout_q.h>
#شامل <kthread.h>
#شامل <zephyr/tracing/tracing.h>
#شامل <stdbool.h>
#شامل <priority_q.h>

BUILD_ASSERT(K_سب_سے_نیچا_APPLICATION_THREAD_PRIO
	     >= K_HIGHEST_APPLICATION_THREAD_PRIO);

#اگر_تعریف_شدہ CONFIG_MULTITHREADING
#تعریف_کرو Z_VALID_PRIO(prio, اندراج_نقطہ)				     \
	(((prio) == K_IDLE_PRIO && z_is_idle_thread_اندراج(اندراج_نقطہ)) || \
	 ((K_سب_سے_نیچا_APPLICATION_THREAD_PRIO			     \
	   >= K_HIGHEST_APPLICATION_THREAD_PRIO)		     \
	  && (prio) >= K_HIGHEST_APPLICATION_THREAD_PRIO	     \
	  && (prio) <= K_سب_سے_نیچا_APPLICATION_THREAD_PRIO))

#تعریف_کرو Z_ASSERT_VALID_PRIO(prio, اندراج_نقطہ) کرو { \
	__ASSERT(Z_VALID_PRIO((prio), (اندراج_نقطہ)), \
		 "invalid priority (%d); allowed range: %d to %d", \
		 (prio), \
		 K_سب_سے_نیچا_APPLICATION_THREAD_PRIO, \
		 K_HIGHEST_APPLICATION_THREAD_PRIO); \
	} جب (false)
#ورنہ
#تعریف_کرو Z_VALID_PRIO(prio, اندراج_نقطہ) ((prio) == -1)
#تعریف_کرو Z_ASSERT_VALID_PRIO(prio, اندراج_نقطہ) __ASSERT((prio) == -1, "")
#اگر_ختم /* CONFIG_MULTITHREADING */

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم

خارجی ساخت k_thread _thread_dummy;

/**
 * @brief Unpend thread, do not abort its timeout (if it exists).
 */
خالی z_unpend_thread_no_timeout(ساخت k_thread *thread);

/**
 * @brief Unpend thread and abort its timeout (if it exists).
 */
خالی z_unpend_thread(ساخت k_thread *thread);
ساخت k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q);
صحیح z_pend_curr(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید,
	       _wait_q_t *wait_q, k_timeout_t timeout);
خالی z_pend_thread(ساخت k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout);
خالی z_reschedule(ساخت k_spinlock *تالا, k_spinlock_کلید_t کلید);
خالی z_reschedule_locked(k_spinlock_کلید_t کلید);
خالی z_reschedule_irqlock(uint32_t کلید);
صحیح z_unpend_all(_wait_q_t *wait_q);
bool z_thread_prio_سیٹ(ساخت k_thread *thread, صحیح prio);
خالی *z_get_اگلا_switch_handle(خالی *interrupted);
خالی z_thread_suspend_current(ساخت k_thread *thread);

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
ساکن ALWAYS_INLINE خالی *z_sched_اگلا_handle(ساخت k_thread *curr)
{
#اگر_غیر_تعریف_شدہ CONFIG_SMP
	اگر (curr == _kernel.ready_q.cache) {
		واپس تہی;
	}
#اگر_ختم
	واپس z_get_اگلا_switch_handle(تہی);
}

خالی z_sched_آغاز(ساخت k_thread *thread);
خالی z_ready_thread(ساخت k_thread *thread);
ساخت k_thread *z_تبادلہ_اگلا_thread(خالی);
خالی move_current_مقصد_انتہا_کا_prio_q(خالی);

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
خالی z_thread_halt(ساخت k_thread *thread, k_spinlock_کلید_t کلید, bool terminate);

/**
 * @brief Ready a thread while the scheduler spinlock is already held.
 *
 * Equivalent to the internal ready_thread() helper. Callers must hold the
 * scheduler's spinlock.
 *
 * @param thread Thread to make ready.
 */
خالی z_sched_ready_locked(ساخت k_thread *thread);

/**
 * @brief Add a thread to a wait queue while the scheduler spinlock is held.
 *
 * Moves the thread out of the run queue and onto the specified wait queue.
 * Callers must hold the scheduler's spinlock.
 *
 * @param thread  Thread to pend.
 * @param wait_q  Wait queue to add the thread to (may be NULL).
 */
خالی z_sched_جوڑو_مقصد_waitq_locked(ساخت k_thread *thread, _wait_q_t *wait_q);

/**
 * @brief Remove a thread from the run queue (scheduler spinlock must be held).
 *
 * Called by sleep.c to unready the current thread before arming its wakeup
 * timeout.  Callers must already hold the scheduler's spinlock.
 *
 * @param thread Thread to remove from the run queue.
 */
خالی z_sched_unready_locked(ساخت k_thread *thread);

/**
 * @brief Yield the current thread's remaining time slice.
 *
 * Moves _current to the end of its priority group in the run queue, updates
 * the scheduler cache, and context-switches away.  Equivalent to the body
 * of k_yield(); exposed so that thread.c can implement k_yield() without
 * depending on sched.c internals.
 */
خالی z_sched_yield(خالی);

ساکن اندرونی خالی z_reschedule_unlocked(خالی)
{
	(خالی) z_reschedule_irqlock(arch_irq_تالا());
}

ساکن اندرونی bool z_is_under_prio_ceiling(صحیح prio)
{
	واپس prio >= CONFIG_ترجیح_CEILING;
}

ساکن اندرونی صحیح z_get_new_prio_with_ceiling(صحیح prio)
{
	واپس z_is_under_prio_ceiling(prio) ? prio : CONFIG_ترجیح_CEILING;
}

ساکن اندرونی bool z_is_prio1_higher_than_or_equal_مقصد_prio2(صحیح prio1, صحیح prio2)
{
	واپس prio1 <= prio2;
}

ساکن اندرونی bool z_is_prio_higher_or_equal(صحیح prio1, صحیح prio2)
{
	واپس z_is_prio1_higher_than_or_equal_مقصد_prio2(prio1, prio2);
}

ساکن اندرونی bool z_is_prio1_زیریں_than_or_equal_مقصد_prio2(صحیح prio1, صحیح prio2)
{
	واپس prio1 >= prio2;
}

ساکن اندرونی bool z_is_prio1_higher_than_prio2(صحیح prio1, صحیح prio2)
{
	واپس prio1 < prio2;
}

ساکن اندرونی bool z_is_prio_higher(صحیح prio, صحیح test_prio)
{
	واپس z_is_prio1_higher_than_prio2(prio, test_prio);
}

ساکن اندرونی bool z_is_prio_زیریں_or_equal(صحیح prio1, صحیح prio2)
{
	واپس z_is_prio1_زیریں_than_or_equal_مقصد_prio2(prio1, prio2);
}

ساکن اندرونی bool _is_valid_prio(صحیح prio, k_thread_اندراج_t اندراج_نقطہ)
{
	اگر ((prio == K_IDLE_PRIO) && z_is_idle_thread_اندراج(اندراج_نقطہ)) {
		واپس true;
	}

	اگر (!z_is_prio_higher_or_equal(prio,
				       K_سب_سے_نیچا_APPLICATION_THREAD_PRIO)) {
		واپس false;
	}

	اگر (!z_is_prio_زیریں_or_equal(prio,
				      K_HIGHEST_APPLICATION_THREAD_PRIO)) {
		واپس false;
	}

	واپس true;
}

ساکن ALWAYS_INLINE _wait_q_t *pended_on_thread(ساخت k_thread *thread)
{
	__ASSERT_NO_MSG(thread->بنیاد.pended_on);

	واپس thread->بنیاد.pended_on;
}


ساکن اندرونی خالی unpend_thread_no_timeout(ساخت k_thread *thread)
{
	_priq_wait_remove(&pended_on_thread(thread)->waitq, thread);
	z_mark_thread_as_not_pending(thread);
	thread->بنیاد.pended_on = تہی;
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
ساکن ALWAYS_INLINE ساخت k_thread *z_unpend_پہلا_thread_locked(_wait_q_t *wait_q)
{
	ساخت k_thread *thread = priq_wait_بہترین(&wait_q->waitq);

	اگر (unlikely(thread != تہی)) {
		unpend_thread_no_timeout(thread);
		/* Abort the thread's timeout. If its handler is in flight on
		 * another CPU it is blocked on the sched lock; the abort flags
		 * it superseded, and z_thread_timeout() bails on that flag when
		 * it finally runs -- so it won't wake the thread from whatever
		 * it has since re-pended on. We don't wait for it here.
		 */
		(خالی)z_try_abort_thread_timeout(thread);
	}
	واپس thread;
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
ساکن ALWAYS_INLINE bool z_sched_wake(_wait_q_t *wait_q, صحیح تبادلہ_retval, خالی *تبادلہ_ڈیٹا)
{
	ساخت k_thread *thread;
	bool ret = false;

	تالا_SCHED_SPINLOCK {
		thread = z_unpend_پہلا_thread_locked(wait_q);
		اگر (thread != تہی) {
			z_thread_return_قدر_سیٹ_with_ڈیٹا(thread,
							    تبادلہ_retval,
							    تبادلہ_ڈیٹا);
			z_sched_ready_locked(thread);
			ret = true;
		}
	}

	واپس ret;
}

#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_KSCHED_H_ */
