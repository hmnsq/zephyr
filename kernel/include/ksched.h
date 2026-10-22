/*
 * Copyright (c) 2016-2017 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_KSCHED_H_
#define ZEPHYR_KERNEL_INCLUDE_KSCHED_H_

#અન્તર્ભાવ <zephyr/kernel_structs.h>
#અન્તર્ભાવ <kspinlock.h>
#અન્તર્ભાવ <kernel_internal.h>
#અન્તર્ભાવ <timeout_q.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <zephyr/tracing/tracing.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <priority_q.h>

BUILD_ASSERT(K_LOWEST_APPLICATION_THREAD_PRIO
	     >= K_HIGHEST_APPLICATION_THREAD_PRIO);

#ifdef CONFIG_MULTITHREADING
#define Z_VALID_PRIO(prio, entry_point)				     \
	(((prio) == K_IDLE_PRIO && z_is_idle_thread_entry(entry_point)) || \
	 ((K_LOWEST_APPLICATION_THREAD_PRIO			     \
	   >= K_HIGHEST_APPLICATION_THREAD_PRIO)		     \
	  && (prio) >= K_HIGHEST_APPLICATION_THREAD_PRIO	     \
	  && (prio) <= K_LOWEST_APPLICATION_THREAD_PRIO))

#define Z_ASSERT_VALID_PRIO(prio, entry_point) કરો { \
	__ASSERT(Z_VALID_PRIO((prio), (entry_point)), \
		 "invalid priority (%d); allowed range: %d to %d", \
		 (prio), \
		 K_LOWEST_APPLICATION_THREAD_PRIO, \
		 K_HIGHEST_APPLICATION_THREAD_PRIO); \
	} જ્યારે (false)
#નહીંતર
#define Z_VALID_PRIO(prio, entry_point) ((prio) == -1)
#define Z_ASSERT_VALID_PRIO(prio, entry_point) __ASSERT((prio) == -1, "")
#endif /* CONFIG_MULTITHREADING */

#ifdef __cplusplus
બાહ્ય "C" {
#endif

બાહ્ય સંરચના k_thread _thread_dummy;

/**
 * @brief Unpend thread, do not abort its timeout (if it exists).
 */
શૂન્ય z_unpend_thread_no_timeout(સંરચના k_thread *thread);

/**
 * @brief Unpend thread and abort its timeout (if it exists).
 */
શૂન્ય z_unpend_thread(સંરચના k_thread *thread);
સંરચના k_thread *z_unpend1_no_timeout(_wait_q_t *wait_q);
પૂર્ણાંક z_pend_curr(સંરચના k_spinlock *lock, k_spinlock_ચાવી_t ચાવી,
	       _wait_q_t *wait_q, k_timeout_t timeout);
શૂન્ય z_pend_thread(સંરચના k_thread *thread, _wait_q_t *wait_q,
		   k_timeout_t timeout);
શૂન્ય z_reschedule(સંરચના k_spinlock *lock, k_spinlock_ચાવી_t ચાવી);
શૂન્ય z_reschedule_locked(k_spinlock_ચાવી_t ચાવી);
શૂન્ય z_reschedule_irqlock(uint32_t ચાવી);
પૂર્ણાંક z_unpend_all(_wait_q_t *wait_q);
bool z_thread_prio_set(સંરચના k_thread *thread, પૂર્ણાંક prio);
શૂન્ય *z_get_next_switch_handle(શૂન્ય *interrupted);
શૂન્ય z_thread_suspend_current(સંરચના k_thread *thread);

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
સ્થિર ALWAYS_INLINE શૂન્ય *z_sched_next_handle(સંરચના k_thread *curr)
{
#ifndef CONFIG_SMP
	જો (curr == _kernel.ready_q.cache) {
		પાછા ખાલી;
	}
#endif
	પાછા z_get_next_switch_handle(ખાલી);
}

શૂન્ય z_sched_start(સંરચના k_thread *thread);
શૂન્ય z_ready_thread(સંરચના k_thread *thread);
સંરચના k_thread *z_swap_next_thread(શૂન્ય);
શૂન્ય move_current_to_end_of_prio_q(શૂન્ય);

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
શૂન્ય z_thread_halt(સંરચના k_thread *thread, k_spinlock_ચાવી_t ચાવી, bool terminate);

/**
 * @brief Ready a thread while the scheduler spinlock is already held.
 *
 * Equivalent to the internal ready_thread() helper. Callers must hold the
 * scheduler's spinlock.
 *
 * @param thread Thread to make ready.
 */
શૂન્ય z_sched_ready_locked(સંરચના k_thread *thread);

/**
 * @brief Add a thread to a wait queue while the scheduler spinlock is held.
 *
 * Moves the thread out of the run queue and onto the specified wait queue.
 * Callers must hold the scheduler's spinlock.
 *
 * @param thread  Thread to pend.
 * @param wait_q  Wait queue to add the thread to (may be NULL).
 */
શૂન્ય z_sched_add_to_waitq_locked(સંરચના k_thread *thread, _wait_q_t *wait_q);

/**
 * @brief Remove a thread from the run queue (scheduler spinlock must be held).
 *
 * Called by sleep.c to unready the current thread before arming its wakeup
 * timeout.  Callers must already hold the scheduler's spinlock.
 *
 * @param thread Thread to remove from the run queue.
 */
શૂન્ય z_sched_unready_locked(સંરચના k_thread *thread);

/**
 * @brief Yield the current thread's remaining time slice.
 *
 * Moves _current to the end of its priority group in the run queue, updates
 * the scheduler cache, and context-switches away.  Equivalent to the body
 * of k_yield(); exposed so that thread.c can implement k_yield() without
 * depending on sched.c internals.
 */
શૂન્ય z_sched_yield(શૂન્ય);

સ્થિર અંતઃરેખ શૂન્ય z_reschedule_unlocked(શૂન્ય)
{
	(શૂન્ય) z_reschedule_irqlock(arch_irq_lock());
}

સ્થિર અંતઃરેખ bool z_is_under_prio_ceiling(પૂર્ણાંક prio)
{
	પાછા prio >= CONFIG_PRIORITY_CEILING;
}

સ્થિર અંતઃરેખ પૂર્ણાંક z_get_new_prio_with_ceiling(પૂર્ણાંક prio)
{
	પાછા z_is_under_prio_ceiling(prio) ? prio : CONFIG_PRIORITY_CEILING;
}

સ્થિર અંતઃરેખ bool z_is_prio1_higher_than_or_equal_to_prio2(પૂર્ણાંક prio1, પૂર્ણાંક prio2)
{
	પાછા prio1 <= prio2;
}

સ્થિર અંતઃરેખ bool z_is_prio_higher_or_equal(પૂર્ણાંક prio1, પૂર્ણાંક prio2)
{
	પાછા z_is_prio1_higher_than_or_equal_to_prio2(prio1, prio2);
}

સ્થિર અંતઃરેખ bool z_is_prio1_lower_than_or_equal_to_prio2(પૂર્ણાંક prio1, પૂર્ણાંક prio2)
{
	પાછા prio1 >= prio2;
}

સ્થિર અંતઃરેખ bool z_is_prio1_higher_than_prio2(પૂર્ણાંક prio1, પૂર્ણાંક prio2)
{
	પાછા prio1 < prio2;
}

સ્થિર અંતઃરેખ bool z_is_prio_higher(પૂર્ણાંક prio, પૂર્ણાંક test_prio)
{
	પાછા z_is_prio1_higher_than_prio2(prio, test_prio);
}

સ્થિર અંતઃરેખ bool z_is_prio_lower_or_equal(પૂર્ણાંક prio1, પૂર્ણાંક prio2)
{
	પાછા z_is_prio1_lower_than_or_equal_to_prio2(prio1, prio2);
}

સ્થિર અંતઃરેખ bool _is_valid_prio(પૂર્ણાંક prio, k_thread_entry_t entry_point)
{
	જો ((prio == K_IDLE_PRIO) && z_is_idle_thread_entry(entry_point)) {
		પાછા true;
	}

	જો (!z_is_prio_higher_or_equal(prio,
				       K_LOWEST_APPLICATION_THREAD_PRIO)) {
		પાછા false;
	}

	જો (!z_is_prio_lower_or_equal(prio,
				      K_HIGHEST_APPLICATION_THREAD_PRIO)) {
		પાછા false;
	}

	પાછા true;
}

સ્થિર ALWAYS_INLINE _wait_q_t *pended_on_thread(સંરચના k_thread *thread)
{
	__ASSERT_NO_MSG(thread->base.pended_on);

	પાછા thread->base.pended_on;
}


સ્થિર અંતઃરેખ શૂન્ય unpend_thread_no_timeout(સંરચના k_thread *thread)
{
	_priq_wait_remove(&pended_on_thread(thread)->waitq, thread);
	z_mark_thread_as_not_pending(thread);
	thread->base.pended_on = ખાલી;
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
સ્થિર ALWAYS_INLINE સંરચના k_thread *z_unpend_first_thread_locked(_wait_q_t *wait_q)
{
	સંરચના k_thread *thread = _priq_wait_best(&wait_q->waitq);

	જો (unlikely(thread != ખાલી)) {
		unpend_thread_no_timeout(thread);
		/* Abort the thread's timeout. If its handler is in flight on
		 * another CPU it is blocked on the sched lock; the abort flags
		 * it superseded, and z_thread_timeout() bails on that flag when
		 * it finally runs -- so it won't wake the thread from whatever
		 * it has since re-pended on. We don't wait for it here.
		 */
		(શૂન્ય)z_try_abort_thread_timeout(thread);
	}
	પાછા thread;
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
સ્થિર ALWAYS_INLINE bool z_sched_wake(_wait_q_t *wait_q, પૂર્ણાંક swap_retval, શૂન્ય *swap_data)
{
	સંરચના k_thread *thread;
	bool ret = false;

	LOCK_SCHED_SPINLOCK {
		thread = z_unpend_first_thread_locked(wait_q);
		જો (thread != ખાલી) {
			z_thread_return_કિંમત_set_with_data(thread,
							    swap_retval,
							    swap_data);
			z_sched_ready_locked(thread);
			ret = true;
		}
	}

	પાછા ret;
}

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_KERNEL_INCLUDE_KSCHED_H_ */
