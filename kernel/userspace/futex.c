/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#অন্তর্ভাব <zephyr/kernel.h>
#অন্তর্ভাব <zephyr/spinlock.h>
#অন্তর্ভাব <zephyr/internal/syscall_handler.h>
#অন্তর্ভাব <zephyr/init.h>
#অন্তর্ভাব <ksched.h>
#অন্তর্ভাব <scheduler.h>

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
স্থির _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
স্থির সংরচনা k_spinlock futex_lock;

/**
 * Context structure for walking the futex wait queue
 */
সংরচনা futex_walk_data {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	সংরচনা k_thread *head;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	সংরচনা k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	পূর্ণাঙ্ক woken;
};

স্থির শূন্য futex_post_walk_op(পূর্ণাঙ্ক status, শূন্য *data)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_UNUSED(status);
	সংরচনা futex_walk_data *walk_data = data;
	সংরচনা k_thread *thread, *next;

	thread = walk_data->head;

	/* Wake up threads if any */
	যাবত্ (thread != রিক্ত) {
#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		next = রিক্ত;
#অন্যথা  /* !CONFIG_WAITQ_SCALABLE */
		next = thread->next_wake_link;
#endif /* !CONFIG_WAITQ_SCALABLE */

		(শূন্য)z_try_abort_thread_timeout(thread);
		arch_thread_return_মান_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_data->woken++;

		thread = next;
	}
}

স্থির পূর্ণাঙ্ক futex_wake_one_walk_op(সংরচনা k_thread *thread, শূন্য *data)
{
	সংরচনা futex_walk_data *walk_data = data;

	/* Ignore threads waiting on different futexes */
	যদি (thread->futex_pointer == walk_data->futex) {
		/* Find the highest priority thread */
		যদি (walk_data->head == রিক্ত ||
		    z_is_prio_higher(thread->base.prio, walk_data->head->base.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_data->head = thread;
		}
	}

	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক futex_wake_all_walk_op(সংরচনা k_thread *thread, শূন্য *data)
{
	সংরচনা futex_walk_data *walk_data = data;

	/* Ignore threads waiting on different futexes */
	যদি (thread->futex_pointer == walk_data->futex) {
#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(শূন্য)z_try_abort_thread_timeout(thread);
		arch_thread_return_মান_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_data->woken++;
#অন্যথা  /* !CONFIG_WAITQ_SCALABLE */
		thread->next_wake_link = walk_data->head;
		walk_data->head = thread;
#endif /* !CONFIG_WAITQ_SCALABLE */
	}

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক z_impl_k_futex_wake(সংরচনা k_futex *futex, bool wake_all)
{
	k_spinlock_চাবি_t চাবি;
	সংরচনা futex_walk_data walk_data = {.head = রিক্ত, .futex = futex, .woken = 0};

	চাবি = k_spin_lock(&futex_lock);

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
			   futex_post_walk_op, &walk_data);

	যদি (walk_data.woken == 0) {
		k_spin_unlock(&futex_lock, চাবি);
	} অন্যথা {
		z_reschedule(&futex_lock, চাবি);
	}

	প্রত্যাগম walk_data.woken;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_futex_wake(সংরচনা k_futex *futex, bool wake_all)
{
	যদি (K_SYSCALL_MEMORY_WRITE(futex, পরিমাণ(সংরচনা k_futex)) != 0) {
		প্রত্যাগম -EACCES;
	}

	প্রত্যাগম z_impl_k_futex_wake(futex, wake_all);
}
#অন্তর্ভাব <zephyr/syscalls/k_futex_wake_mrsh.c>

পূর্ণাঙ্ক z_impl_k_futex_wait(সংরচনা k_futex *futex, পূর্ণাঙ্ক expected, k_timeout_t timeout)
{
	পূর্ণাঙ্ক ret;
	k_spinlock_চাবি_t চাবি;

	চাবি = k_spin_lock(&futex_lock);

	যদি (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_lock, চাবি);
		প্রত্যাগম -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_pointer = futex;
#ifdef CONFIG_WAITQ_SCALABLE
	_current->next_wake_link = রিক্ত;
#endif /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_lock, চাবি, &futex_wait_q, timeout);
	যদি (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	প্রত্যাগম ret;
}

স্থির অন্তঃস্থ পূর্ণাঙ্ক z_vrfy_k_futex_wait(সংরচনা k_futex *futex, পূর্ণাঙ্ক expected, k_timeout_t timeout)
{
	যদি (K_SYSCALL_MEMORY_WRITE(futex, পরিমাণ(সংরচনা k_futex)) != 0) {
		প্রত্যাগম -EACCES;
	}

	প্রত্যাগম z_impl_k_futex_wait(futex, expected, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_futex_wait_mrsh.c>
