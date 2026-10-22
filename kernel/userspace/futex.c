/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/internal/syscall_handler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scheduler.h>

/*
 * Kernel futex implementation
 *
 * We use a single queue for all futex waiters. This is lightweight and
 * efficient for a small number of threads but does not scale well to larger
 * numbers waiting on different futexes, because each wake needs to walk the
 * full queue while holding _sched_spinlock.
 *
 * Potential alternatives for scaling:
 * - Use dedicated queues per futex, which requires a dynamic data structure to
 *   map futex addresses to wait queues.
 * - Create a new data structure to sort by both futex address and priority to
 *   efficiently find the highest priority thread waiting on a given futex.
 * - Keep a small rbtree of active futexes keyed by address, but put the rbnode
 *   in the waiting thread's own TCB instead of a slab pool — the lead waiter
 *   for each futex contributes the node, handed to the next waiter when it
 *   leaves. Per-futex queues, no heap, no fixed table, at the cost of a little
 *   bookkeeping on wait/wake.
 *   https://github.com/zephyrproject-rtos/zephyr/pull/110059#issuecomment-4625886727
 *
 * Further improvements:
 * - Add a futex wait call with priority inheritance, where the futex value is
 *   expected to be the TID of the holder, so that a higher priority thread
 *   waiting for a futex can boost the holders priority.
 */
ਸਥਿਰ _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
ਸਥਿਰ ਬਣਤਰ k_spinlock futex_ਤਾਲਾ;

/**
 * Context structure for walking the futex wait queue
 */
ਬਣਤਰ futex_walk_ਅੰਕੜੇ {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	ਬਣਤਰ k_thread *ਸਿਰ;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	ਬਣਤਰ k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	ਪੂਰਨ woken;
};

ਸਥਿਰ ਖਾਲੀ futex_post_walk_op(ਪੂਰਨ ਸਥਿਤੀ, ਖਾਲੀ *ਅੰਕੜੇ)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_ਅਣਵਰਤਿਆ(ਸਥਿਤੀ);
	ਬਣਤਰ futex_walk_ਅੰਕੜੇ *walk_ਅੰਕੜੇ = ਅੰਕੜੇ;
	ਬਣਤਰ k_thread *thread, *ਅਗਲਾ;

	thread = walk_ਅੰਕੜੇ->ਸਿਰ;

	/* Wake up threads if any */
	ਜਦੋਂ (thread != ਨਿਰਾ) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		ਅਗਲਾ = ਨਿਰਾ;
#ਨਹੀਂਤਾਂ  /* !CONFIG_WAITQ_SCALABLE */
		ਅਗਲਾ = thread->ਅਗਲਾ_wake_link;
#ਜੇ_ਅੰਤ /* !CONFIG_WAITQ_SCALABLE */

		(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
		arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_ਅੰਕੜੇ->woken++;

		thread = ਅਗਲਾ;
	}
}

ਸਥਿਰ ਪੂਰਨ futex_wake_one_walk_op(ਬਣਤਰ k_thread *thread, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਬਣਤਰ futex_walk_ਅੰਕੜੇ *walk_ਅੰਕੜੇ = ਅੰਕੜੇ;

	/* Ignore threads waiting on different futexes */
	ਜੇ (thread->futex_ਸੂਚਕ == walk_ਅੰਕੜੇ->futex) {
		/* Find the highest priority thread */
		ਜੇ (walk_ਅੰਕੜੇ->ਸਿਰ == ਨਿਰਾ ||
		    z_is_prio_higher(thread->ਅਧਾਰ.prio, walk_ਅੰਕੜੇ->ਸਿਰ->ਅਧਾਰ.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_ਅੰਕੜੇ->ਸਿਰ = thread;
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ futex_wake_all_walk_op(ਬਣਤਰ k_thread *thread, ਖਾਲੀ *ਅੰਕੜੇ)
{
	ਬਣਤਰ futex_walk_ਅੰਕੜੇ *walk_ਅੰਕੜੇ = ਅੰਕੜੇ;

	/* Ignore threads waiting on different futexes */
	ਜੇ (thread->futex_ਸੂਚਕ == walk_ਅੰਕੜੇ->futex) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(ਖਾਲੀ)z_try_abort_thread_timeout(thread);
		arch_thread_return_ਮੁੱਲ_ਸੈੱਟ(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_ਅੰਕੜੇ->woken++;
#ਨਹੀਂਤਾਂ  /* !CONFIG_WAITQ_SCALABLE */
		thread->ਅਗਲਾ_wake_link = walk_ਅੰਕੜੇ->ਸਿਰ;
		walk_ਅੰਕੜੇ->ਸਿਰ = thread;
#ਜੇ_ਅੰਤ /* !CONFIG_WAITQ_SCALABLE */
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ z_impl_k_futex_wake(ਬਣਤਰ k_futex *futex, bool wake_all)
{
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;
	ਬਣਤਰ futex_walk_ਅੰਕੜੇ walk_ਅੰਕੜੇ = {.ਸਿਰ = ਨਿਰਾ, .futex = futex, .woken = 0};

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&futex_ਤਾਲਾ);

	/*
	 * Walk through the wait queue to find threads waiting on this futex.
	 *
	 * Wake-all mode operates similarly to events.c: threads are awakened
	 * by futex_wake_all_walk_op when possible, and otherwise added to a
	 * linked list for wake-up by futex_post_walk_op later on.
	 * (NOTE: in the former case, the post-walk op has no effect)
	 *
	 * Wake-one mode operates differently: futex_wake_one_walk_op searches
	 * for the highest-priority thread among those waiting on this futex and
	 * lets futex_post_walk_op perform the wake-up. Note that we cannot use
	 * z_unpend_first_thread because the overall highest priority thread
	 * could be waiting on a different futex.
	 */
	z_sched_waitq_walk(&futex_wait_q,
			   (wake_all) ? futex_wake_all_walk_op : futex_wake_one_walk_op,
			   futex_post_walk_op, &walk_ਅੰਕੜੇ);

	ਜੇ (walk_ਅੰਕੜੇ.woken == 0) {
		k_spin_unlock(&futex_ਤਾਲਾ, ਕੁੰਜੀ);
	} ਨਹੀਂਤਾਂ {
		z_reschedule(&futex_ਤਾਲਾ, ਕੁੰਜੀ);
	}

	ਵਾਪਸ walk_ਅੰਕੜੇ.woken;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_futex_wake(ਬਣਤਰ k_futex *futex, bool wake_all)
{
	ਜੇ (K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(futex, ਆਕਾਰ(ਬਣਤਰ k_futex)) != 0) {
		ਵਾਪਸ -EACCES;
	}

	ਵਾਪਸ z_impl_k_futex_wake(futex, wake_all);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_futex_wake_mrsh.c>

ਪੂਰਨ z_impl_k_futex_wait(ਬਣਤਰ k_futex *futex, ਪੂਰਨ expected, k_timeout_t timeout)
{
	ਪੂਰਨ ret;
	k_spinlock_ਕੁੰਜੀ_t ਕੁੰਜੀ;

	ਕੁੰਜੀ = k_spin_ਤਾਲਾ(&futex_ਤਾਲਾ);

	ਜੇ (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_ਤਾਲਾ, ਕੁੰਜੀ);
		ਵਾਪਸ -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_ਸੂਚਕ = futex;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_WAITQ_SCALABLE
	_current->ਅਗਲਾ_wake_link = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_ਤਾਲਾ, ਕੁੰਜੀ, &futex_wait_q, timeout);
	ਜੇ (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ z_vrfy_k_futex_wait(ਬਣਤਰ k_futex *futex, ਪੂਰਨ expected, k_timeout_t timeout)
{
	ਜੇ (K_SYSCALL_ਸਮ੍ਰਿਤੀ_WRITE(futex, ਆਕਾਰ(ਬਣਤਰ k_futex)) != 0) {
		ਵਾਪਸ -EACCES;
	}

	ਵਾਪਸ z_impl_k_futex_wait(futex, expected, timeout);
}
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/syscalls/k_futex_wait_mrsh.c>
