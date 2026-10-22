/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_KSCHED_H_
#define ZEPHYR_KERNEL_INCLUDE_KSCHED_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel_structs.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <kernel_internal.h>
#ಅನ್ತರ್ಭಾವ <timeout_q.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <zephyr/tracing/tracing.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <priority_q.h>

BUILD_ASSERT(K_LOWEST_APPLICATION_THREAD_PRIO
	     >= K_HIGHEST_APPLICATION_THREAD_PRIO);

#ifdef CONFIG_MULTITHREADING
#define Z_VALID_PRIO(prio, entry_point)				     \
	(((prio) == K_IDLE_PRIO && z_is_idle_thread_entry(entry_point)) || \
	 ((K_LOWEST_APPLICATION_THREAD_PRIO			     \
	   >= K_HIGHEST_APPLICATION_THREAD_PRIO)		     \
	  && (prio) >= K_HIGHEST_APPLICATION_THREAD_PRIO	     \
	  && (prio) <= K_LOWEST_APPLICATION_THREAD_PRIO))

#define Z_ASSERT_VALID_PRIO(prio, entry_point) ಕುರು { \
	__ASSERT(Z_VALID_PRIO((prio), (entry_point)), \
		 "invalid priority (%d); allowed range: %d to %d", \
		 (prio), \
		 K_LOWEST_APPLICATION_THREAD_PRIO, \
		 K_HIGHEST_APPLICATION_THREAD_PRIO); \
	} ಯಾವತ್ (false)
#ಅನ್ಯಥಾ
#define Z_VALID_PRIO(prio, entry_point) ((prio) == -1)
#define Z_ASSERT_VALID_PRIO(prio, entry_point) __ASSERT((prio) == -1, "")
#endif /* CONFIG_MULTITHREADING */

#ifdef __cplusplus
ಬಾಹ್ಯ "C" {
#endif

ಬಾಹ್ಯ ಸಂರಚನಾ k_thread _thread_dummy;

/**
 * @brief Unpend thread, do not abort its timeout (if it exists).
 */
ಶೂನ್ಯ z_unpend_thread_no_timeout(ಸಂರಚನಾ k_thread *thread);

/**
 * @brief Unpend thread and abort its timeout (if it exists).
 */
ಶೂನ್ಯ z_unpend_thread(ಸಂರಚನಾ k_thread *thread);
ಸಂರಚನಾ k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q);
ಪೂರ್ಣಾಙ್ಕ z_pend_curr(ಸಂರಚನಾ k_spinlock *lock, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ,
	       _wait_q_t *wait_q, k_timeout_t timeout);
ಶೂನ್ಯ z_pend_thread(ಸಂರಚನಾ k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout);
ಶೂನ್ಯ z_reschedule(ಸಂರಚನಾ k_spinlock *lock, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ);
ಶೂನ್ಯ z_reschedule_locked(k_spinlock_ಕೀಲಿ_t ಕೀಲಿ);
ಶೂನ್ಯ z_reschedule_irqlock(uint32_t ಕೀಲಿ);
ಪೂರ್ಣಾಙ್ಕ z_unpend_all(_wait_q_t *wait_q);
bool z_thread_prio_set(ಸಂರಚನಾ k_thread *thread, ಪೂರ್ಣಾಙ್ಕ prio);
ಶೂನ್ಯ *z_get_next_switch_handle(ಶೂನ್ಯ *interrupted);
ಶೂನ್ಯ z_thread_suspend_current(ಸಂರಚನಾ k_thread *thread);

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
ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ *z_sched_next_handle(ಸಂರಚನಾ k_thread *curr)
{
#ifndef CONFIG_SMP
	ಯದಿ (curr == _kernel.ready_q.cache) {
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
#endif
	ಪ್ರತ್ಯಾಗಮ z_get_next_switch_handle(ರಿಕ್ತ);
}

ಶೂನ್ಯ z_sched_start(ಸಂರಚನಾ k_thread *thread);
ಶೂನ್ಯ z_ready_thread(ಸಂರಚನಾ k_thread *thread);
ಸಂರಚನಾ k_thread *z_swap_next_thread(ಶೂನ್ಯ);
ಶೂನ್ಯ move_current_to_end_of_prio_q(ಶೂನ್ಯ);

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
ಶೂನ್ಯ z_thread_halt(ಸಂರಚನಾ k_thread *thread, k_spinlock_ಕೀಲಿ_t ಕೀಲಿ, bool terminate);

/**
 * @brief Ready a thread while the scheduler spinlock is already held.
 *
 * Equivalent to the internal ready_thread() helper. Callers must hold the
 * scheduler's spinlock.
 *
 * @param thread Thread to make ready.
 */
ಶೂನ್ಯ z_sched_ready_locked(ಸಂರಚನಾ k_thread *thread);

/**
 * @brief Add a thread to a wait queue while the scheduler spinlock is held.
 *
 * Moves the thread out of the run queue and onto the specified wait queue.
 * Callers must hold the scheduler's spinlock.
 *
 * @param thread  Thread to pend.
 * @param wait_q  Wait queue to add the thread to (may be NULL).
 */
ಶೂನ್ಯ z_sched_add_to_waitq_locked(ಸಂರಚನಾ k_thread *thread, _wait_q_t *wait_q);

/**
 * @brief Remove a thread from the run queue (scheduler spinlock must be held).
 *
 * Called by sleep.c to unready the current thread before arming its wakeup
 * timeout.  Callers must already hold the scheduler's spinlock.
 *
 * @param thread Thread to remove from the run queue.
 */
ಶೂನ್ಯ z_sched_unready_locked(ಸಂರಚನಾ k_thread *thread);

/**
 * @brief Yield the current thread's remaining time slice.
 *
 * Moves _current to the end of its priority group in the run queue, updates
 * the scheduler cache, and context-switches away.  Equivalent to the body
 * of k_yield(); exposed so that thread.c can implement k_yield() without
 * depending on sched.c internals.
 */
ಶೂನ್ಯ z_sched_yield(ಶೂನ್ಯ);

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ z_reschedule_unlocked(ಶೂನ್ಯ)
{
	(ಶೂನ್ಯ) z_reschedule_irqlock(arch_irq_lock());
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_under_prio_ceiling(ಪೂರ್ಣಾಙ್ಕ prio)
{
	ಪ್ರತ್ಯಾಗಮ prio >= CONFIG_PRIORITY_CEILING;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ z_get_new_prio_with_ceiling(ಪೂರ್ಣಾಙ್ಕ prio)
{
	ಪ್ರತ್ಯಾಗಮ z_is_under_prio_ceiling(prio) ? prio : CONFIG_PRIORITY_CEILING;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_prio1_higher_than_or_equal_to_prio2(ಪೂರ್ಣಾಙ್ಕ prio1, ಪೂರ್ಣಾಙ್ಕ prio2)
{
	ಪ್ರತ್ಯಾಗಮ prio1 <= prio2;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_prio_higher_or_equal(ಪೂರ್ಣಾಙ್ಕ prio1, ಪೂರ್ಣಾಙ್ಕ prio2)
{
	ಪ್ರತ್ಯಾಗಮ z_is_prio1_higher_than_or_equal_to_prio2(prio1, prio2);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_prio1_lower_than_or_equal_to_prio2(ಪೂರ್ಣಾಙ್ಕ prio1, ಪೂರ್ಣಾಙ್ಕ prio2)
{
	ಪ್ರತ್ಯಾಗಮ prio1 >= prio2;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_prio1_higher_than_prio2(ಪೂರ್ಣಾಙ್ಕ prio1, ಪೂರ್ಣಾಙ್ಕ prio2)
{
	ಪ್ರತ್ಯಾಗಮ prio1 < prio2;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_prio_higher(ಪೂರ್ಣಾಙ್ಕ prio, ಪೂರ್ಣಾಙ್ಕ test_prio)
{
	ಪ್ರತ್ಯಾಗಮ z_is_prio1_higher_than_prio2(prio, test_prio);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool z_is_prio_lower_or_equal(ಪೂರ್ಣಾಙ್ಕ prio1, ಪೂರ್ಣಾಙ್ಕ prio2)
{
	ಪ್ರತ್ಯಾಗಮ z_is_prio1_lower_than_or_equal_to_prio2(prio1, prio2);
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool _is_valid_prio(ಪೂರ್ಣಾಙ್ಕ prio, k_thread_entry_t entry_point)
{
	ಯದಿ ((prio == K_IDLE_PRIO) && z_is_idle_thread_entry(entry_point)) {
		ಪ್ರತ್ಯಾಗಮ true;
	}

	ಯದಿ (!z_is_prio_higher_or_equal(prio,
				       K_LOWEST_APPLICATION_THREAD_PRIO)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಯದಿ (!z_is_prio_lower_or_equal(prio,
				      K_HIGHEST_APPLICATION_THREAD_PRIO)) {
		ಪ್ರತ್ಯಾಗಮ false;
	}

	ಪ್ರತ್ಯಾಗಮ true;
}

ಸ್ಥಿರ ALWAYS_INLINE _wait_q_t *pended_on_thread(ಸಂರಚನಾ k_thread *thread)
{
	__ASSERT_NO_MSG(thread->base.pended_on);

	ಪ್ರತ್ಯಾಗಮ thread->base.pended_on;
}


ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ unpend_thread_no_timeout(ಸಂರಚನಾ k_thread *thread)
{
	_priq_wait_remove(&pended_on_thread(thread)->waitq, thread);
	z_mark_thread_as_not_pending(thread);
	thread->base.pended_on = ರಿಕ್ತ;
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
ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *z_unpend_first_thread_locked(_wait_q_t *wait_q)
{
	ಸಂರಚನಾ k_thread *thread = _priq_wait_best(&wait_q->waitq);

	ಯದಿ (unlikely(thread != ರಿಕ್ತ)) {
		unpend_thread_no_timeout(thread);
		/* Abort the thread's timeout. If its handler is in flight on
		 * another CPU it is blocked on the sched lock; the abort flags
		 * it superseded, and z_thread_timeout() bails on that flag when
		 * it finally runs -- so it won't wake the thread from whatever
		 * it has since re-pended on. We don't wait for it here.
		 */
		(ಶೂನ್ಯ)z_try_abort_thread_timeout(thread);
	}
	ಪ್ರತ್ಯಾಗಮ thread;
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
ಸ್ಥಿರ ALWAYS_INLINE bool z_sched_wake(_wait_q_t *wait_q, ಪೂರ್ಣಾಙ್ಕ swap_retval, ಶೂನ್ಯ *swap_data)
{
	ಸಂರಚನಾ k_thread *thread;
	bool ret = false;

	LOCK_SCHED_SPINLOCK {
		thread = z_unpend_first_thread_locked(wait_q);
		ಯದಿ (thread != ರಿಕ್ತ) {
			z_thread_return_ಮೌಲ್ಯ_set_with_data(thread,
							    swap_retval,
							    swap_data);
			z_sched_ready_locked(thread);
			ret = true;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_KSCHED_H_ */
