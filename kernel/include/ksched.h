/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_KSCHED_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_KSCHED_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel_structs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kspinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kernel_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timeout_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/tracing/tracing.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <priority_q.h>

BUILD_ASSERT(K_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_APPLICATION_THREAD_PRIO
	     >= K_HIGHEST_APPLICATION_THREAD_PRIO);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MULTITHREADING
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_VALID_PRIO(prio, ਇੰਦਰਾਜ_ਬਿੰਦੂ)				     \
	(((prio) == K_IDLE_PRIO && z_is_idle_thread_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ_ਬਿੰਦੂ)) || \
	 ((K_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_APPLICATION_THREAD_PRIO			     \
	   >= K_HIGHEST_APPLICATION_THREAD_PRIO)		     \
	  && (prio) >= K_HIGHEST_APPLICATION_THREAD_PRIO	     \
	  && (prio) <= K_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_APPLICATION_THREAD_PRIO))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_ASSERT_VALID_PRIO(prio, ਇੰਦਰਾਜ_ਬਿੰਦੂ) ਕਰੋ { \
	__ASSERT(Z_VALID_PRIO((prio), (ਇੰਦਰਾਜ_ਬਿੰਦੂ)), \
		 "invalid priority (%d); allowed range: %d to %d", \
		 (prio), \
		 K_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_APPLICATION_THREAD_PRIO, \
		 K_HIGHEST_APPLICATION_THREAD_PRIO); \
	} ਜਦੋਂ (false)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_VALID_PRIO(prio, ਇੰਦਰਾਜ_ਬਿੰਦੂ) ((prio) == -1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Z_ASSERT_VALID_PRIO(prio, ਇੰਦਰਾਜ_ਬਿੰਦੂ) __ASSERT((prio) == -1, "")
#ਜੇ_ਅੰਤ /* CONFIG_MULTITHREADING */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਬਣਤਰ k_thread _thread_dummy;

/**
 * @brief Unpend thread, do not abort its timeout (if it exists).
 */
ਖਾਲੀ z_unpend_thread_no_timeout(ਬਣਤਰ k_thread *thread);

/**
 * @brief Unpend thread and abort its timeout (if it exists).
 */
ਖਾਲੀ z_unpend_thread(ਬਣਤਰ k_thread *thread);
ਬਣਤਰ k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q);
ਪੂਰਨ z_pend_curr(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ,
	       _wait_q_t *wait_q, k_timeout_t timeout);
ਖਾਲੀ z_pend_thread(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout);
ਖਾਲੀ z_reschedule(ਬਣਤਰ k_spinlock *ਤਾਲਾ, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ);
ਖਾਲੀ z_reschedule_locked(k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ);
ਖਾਲੀ z_reschedule_irqlock(uint32_t ਕੁੰਜੀ);
ਪੂਰਨ z_unpend_all(_wait_q_t *wait_q);
bool z_thread_prio_ਸੈੱਟ(ਬਣਤਰ k_thread *thread, ਪੂਰਨ prio);
ਖਾਲੀ *z_get_ਅਗਲਾ_switch_handle(ਖਾਲੀ *interrupted);
ਖਾਲੀ z_thread_suspend_current(ਬਣਤਰ k_thread *thread);

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
ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ *z_sched_ਅਗਲਾ_handle(ਬਣਤਰ k_thread *curr)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਜੇ (curr == _kernel.ready_q.cache) {
		ਵਾਪਸ ਨਿਰਾ;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ z_get_ਅਗਲਾ_switch_handle(ਨਿਰਾ);
}

ਖਾਲੀ z_sched_ਸ਼ੁਰੂ(ਬਣਤਰ k_thread *thread);
ਖਾਲੀ z_ready_thread(ਬਣਤਰ k_thread *thread);
ਬਣਤਰ k_thread *z_ਅਦਲਾ_ਬਦਲੀ_ਅਗਲਾ_thread(ਖਾਲੀ);
ਖਾਲੀ move_current_ਮੰਜ਼ਿਲ_ਸਿਰਾ_ਦਾ_prio_q(ਖਾਲੀ);

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
ਖਾਲੀ z_thread_halt(ਬਣਤਰ k_thread *thread, k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ, bool terminate);

/**
 * @brief Ready a thread while the scheduler spinlock is already held.
 *
 * Equivalent to the internal ready_thread() helper. Callers must hold the
 * scheduler's spinlock.
 *
 * @param thread Thread to make ready.
 */
ਖਾਲੀ z_sched_ready_locked(ਬਣਤਰ k_thread *thread);

/**
 * @brief Add a thread to a wait queue while the scheduler spinlock is held.
 *
 * Moves the thread out of the run queue and onto the specified wait queue.
 * Callers must hold the scheduler's spinlock.
 *
 * @param thread  Thread to pend.
 * @param wait_q  Wait queue to add the thread to (may be NULL).
 */
ਖਾਲੀ z_sched_ਜੋੜੋ_ਮੰਜ਼ਿਲ_waitq_locked(ਬਣਤਰ k_thread *thread, _wait_q_t *wait_q);

/**
 * @brief Remove a thread from the run queue (scheduler spinlock must be held).
 *
 * Called by sleep.c to unready the current thread before arming its wakeup
 * timeout.  Callers must already hold the scheduler's spinlock.
 *
 * @param thread Thread to remove from the run queue.
 */
ਖਾਲੀ z_sched_unready_locked(ਬਣਤਰ k_thread *thread);

/**
 * @brief Yield the current thread's remaining time slice.
 *
 * Moves _current to the end of its priority group in the run queue, updates
 * the scheduler cache, and context-switches away.  Equivalent to the body
 * of k_yield(); exposed so that thread.c can implement k_yield() without
 * depending on sched.c internals.
 */
ਖਾਲੀ z_sched_yield(ਖਾਲੀ);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_reschedule_unlocked(ਖਾਲੀ)
{
	(ਖਾਲੀ) z_reschedule_irqlock(arch_irq_ਤਾਲਾ());
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_under_prio_ceiling(ਪੂਰਨ prio)
{
	ਵਾਪਸ prio >= CONFIG_ਤਰਜੀਹ_CEILING;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_get_new_prio_with_ceiling(ਪੂਰਨ prio)
{
	ਵਾਪਸ z_is_under_prio_ceiling(prio) ? prio : CONFIG_ਤਰਜੀਹ_CEILING;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_prio1_higher_than_or_equal_ਮੰਜ਼ਿਲ_prio2(ਪੂਰਨ prio1, ਪੂਰਨ prio2)
{
	ਵਾਪਸ prio1 <= prio2;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_prio_higher_or_equal(ਪੂਰਨ prio1, ਪੂਰਨ prio2)
{
	ਵਾਪਸ z_is_prio1_higher_than_or_equal_ਮੰਜ਼ਿਲ_prio2(prio1, prio2);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_prio1_ਹੇਠਲਾ_than_or_equal_ਮੰਜ਼ਿਲ_prio2(ਪੂਰਨ prio1, ਪੂਰਨ prio2)
{
	ਵਾਪਸ prio1 >= prio2;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_prio1_higher_than_prio2(ਪੂਰਨ prio1, ਪੂਰਨ prio2)
{
	ਵਾਪਸ prio1 < prio2;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_prio_higher(ਪੂਰਨ prio, ਪੂਰਨ test_prio)
{
	ਵਾਪਸ z_is_prio1_higher_than_prio2(prio, test_prio);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool z_is_prio_ਹੇਠਲਾ_or_equal(ਪੂਰਨ prio1, ਪੂਰਨ prio2)
{
	ਵਾਪਸ z_is_prio1_ਹੇਠਲਾ_than_or_equal_ਮੰਜ਼ਿਲ_prio2(prio1, prio2);
}

ਸਥਿਰ ਸਤਰਅੰਦਰ bool _is_valid_prio(ਪੂਰਨ prio, k_thread_ਇੰਦਰਾਜ_t ਇੰਦਰਾਜ_ਬਿੰਦੂ)
{
	ਜੇ ((prio == K_IDLE_PRIO) && z_is_idle_thread_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ_ਬਿੰਦੂ)) {
		ਵਾਪਸ true;
	}

	ਜੇ (!z_is_prio_higher_or_equal(prio,
				       K_ਸਭ_ਤੋਂ_ਨੀਵਾਂ_APPLICATION_THREAD_PRIO)) {
		ਵਾਪਸ false;
	}

	ਜੇ (!z_is_prio_ਹੇਠਲਾ_or_equal(prio,
				      K_HIGHEST_APPLICATION_THREAD_PRIO)) {
		ਵਾਪਸ false;
	}

	ਵਾਪਸ true;
}

ਸਥਿਰ ALWAYS_INLINE _wait_q_t *pended_on_thread(ਬਣਤਰ k_thread *thread)
{
	__ASSERT_NO_MSG(thread->ਅਧਾਰ.pended_on);

	ਵਾਪਸ thread->ਅਧਾਰ.pended_on;
}


ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ unpend_thread_no_timeout(ਬਣਤਰ k_thread *thread)
{
	_priq_wait_remove(&pended_on_thread(thread)->waitq, thread);
	z_mark_thread_as_not_pending(thread);
	thread->ਅਧਾਰ.pended_on = ਨਿਰਾ;
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
ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *z_unpend_ਪਹਿਲਾ_thread_locked(_wait_q_t *wait_q)
{
	ਬਣਤਰ k_thread *thread = priq_wait_ਉੱਤਮ(&wait_q->waitq);

	ਜੇ (unlikely(thread != ਨਿਰਾ)) {
		unpend_thread_no_timeout(thread);
		/* Abort the thread's timeout. If its handler is in flight on
		 * another CPU it is blocked on the sched lock; the abort flags
		 * it superseded, and z_thread_timeout() bails on that flag when
		 * it finally runs -- so it won't wake the thread from whatever
		 * it has since re-pended on. We don't wait for it here.
		 */
		(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
	}
	ਵਾਪਸ thread;
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
ਸਥਿਰ ALWAYS_INLINE bool z_sched_wake(_wait_q_t *wait_q, ਪੂਰਨ ਅਦਲਾ_ਬਦਲੀ_retval, ਖਾਲੀ *ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ)
{
	ਬਣਤਰ k_thread *thread;
	bool ret = false;

	ਤਾਲਾ_SCHED_SPINLOCK {
		thread = z_unpend_ਪਹਿਲਾ_thread_locked(wait_q);
		ਜੇ (thread != ਨਿਰਾ) {
			z_thread_return_ਮੁੱਲ_ਸੈੱਟ_with_ਅੰਕੜੇ(thread,
							    ਅਦਲਾ_ਬਦਲੀ_retval,
							    ਅਦਲਾ_ਬਦਲੀ_ਅੰਕੜੇ);
			z_sched_ready_locked(thread);
			ret = true;
		}
	}

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_KSCHED_H_ */
