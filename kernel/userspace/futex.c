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
স্থির কাঠামো k_spinlock futex_lock;

/**
 * Context structure for walking the futex wait queue
 */
কাঠামো futex_walk_তথ্য {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	কাঠামো k_thread *শীর্ষ;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	কাঠামো k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	পূর্ণসংখ্যা woken;
};

স্থির শূন্য futex_post_walk_op(পূর্ণসংখ্যা status, শূন্য *তথ্য)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_UNUSED(status);
	কাঠামো futex_walk_তথ্য *walk_তথ্য = তথ্য;
	কাঠামো k_thread *thread, *পরবর্তী;

	thread = walk_তথ্য->শীর্ষ;

	/* Wake up threads if any */
	যখন (thread != ফাঁকা) {
#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		পরবর্তী = ফাঁকা;
#অন্যথা  /* !CONFIG_WAITQ_SCALABLE */
		পরবর্তী = thread->পরবর্তী_wake_link;
#endif /* !CONFIG_WAITQ_SCALABLE */

		(শূন্য)z_try_abort_thread_timeout(thread);
		arch_thread_return_মান_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_তথ্য->woken++;

		thread = পরবর্তী;
	}
}

স্থির পূর্ণসংখ্যা futex_wake_one_walk_op(কাঠামো k_thread *thread, শূন্য *তথ্য)
{
	কাঠামো futex_walk_তথ্য *walk_তথ্য = তথ্য;

	/* Ignore threads waiting on different futexes */
	যদি (thread->futex_pointer == walk_তথ্য->futex) {
		/* Find the highest priority thread */
		যদি (walk_তথ্য->শীর্ষ == ফাঁকা ||
		    z_is_prio_higher(thread->base.prio, walk_তথ্য->শীর্ষ->base.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_তথ্য->শীর্ষ = thread;
		}
	}

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা futex_wake_all_walk_op(কাঠামো k_thread *thread, শূন্য *তথ্য)
{
	কাঠামো futex_walk_তথ্য *walk_তথ্য = তথ্য;

	/* Ignore threads waiting on different futexes */
	যদি (thread->futex_pointer == walk_তথ্য->futex) {
#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(শূন্য)z_try_abort_thread_timeout(thread);
		arch_thread_return_মান_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_তথ্য->woken++;
#অন্যথা  /* !CONFIG_WAITQ_SCALABLE */
		thread->পরবর্তী_wake_link = walk_তথ্য->শীর্ষ;
		walk_তথ্য->শীর্ষ = thread;
#endif /* !CONFIG_WAITQ_SCALABLE */
	}

	ফেরত 0;
}

পূর্ণসংখ্যা z_impl_k_futex_wake(কাঠামো k_futex *futex, bool wake_all)
{
	k_spinlock_চাবি_t চাবি;
	কাঠামো futex_walk_তথ্য walk_তথ্য = {.শীর্ষ = ফাঁকা, .futex = futex, .woken = 0};

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
			   futex_post_walk_op, &walk_তথ্য);

	যদি (walk_তথ্য.woken == 0) {
		k_spin_unlock(&futex_lock, চাবি);
	} অন্যথা {
		z_reschedule(&futex_lock, চাবি);
	}

	ফেরত walk_তথ্য.woken;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_futex_wake(কাঠামো k_futex *futex, bool wake_all)
{
	যদি (K_SYSCALL_MEMORY_WRITE(futex, আকার(কাঠামো k_futex)) != 0) {
		ফেরত -EACCES;
	}

	ফেরত z_impl_k_futex_wake(futex, wake_all);
}
#অন্তর্ভাব <zephyr/syscalls/k_futex_wake_mrsh.c>

পূর্ণসংখ্যা z_impl_k_futex_wait(কাঠামো k_futex *futex, পূর্ণসংখ্যা expected, k_timeout_t timeout)
{
	পূর্ণসংখ্যা ret;
	k_spinlock_চাবি_t চাবি;

	চাবি = k_spin_lock(&futex_lock);

	যদি (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_lock, চাবি);
		ফেরত -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_pointer = futex;
#ifdef CONFIG_WAITQ_SCALABLE
	_current->পরবর্তী_wake_link = ফাঁকা;
#endif /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_lock, চাবি, &futex_wait_q, timeout);
	যদি (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	ফেরত ret;
}

স্থির অন্তঃরেখ পূর্ণসংখ্যা z_vrfy_k_futex_wait(কাঠামো k_futex *futex, পূর্ণসংখ্যা expected, k_timeout_t timeout)
{
	যদি (K_SYSCALL_MEMORY_WRITE(futex, আকার(কাঠামো k_futex)) != 0) {
		ফেরত -EACCES;
	}

	ফেরত z_impl_k_futex_wait(futex, expected, timeout);
}
#অন্তর্ভাব <zephyr/syscalls/k_futex_wait_mrsh.c>
