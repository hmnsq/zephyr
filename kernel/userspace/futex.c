/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <zephyr/internal/syscall_handler.h>
#અન્તર્ભાવ <zephyr/init.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <scheduler.h>

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
સ્થિર _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
સ્થિર સંરચના k_spinlock futex_lock;

/**
 * Context structure for walking the futex wait queue
 */
સંરચના futex_walk_data {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	સંરચના k_thread *head;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	સંરચના k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	પૂર્ણાંક woken;
};

સ્થિર શૂન્ય futex_post_walk_op(પૂર્ણાંક status, શૂન્ય *data)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_UNUSED(status);
	સંરચના futex_walk_data *walk_data = data;
	સંરચના k_thread *thread, *next;

	thread = walk_data->head;

	/* Wake up threads if any */
	જ્યારે (thread != ખાલી) {
#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		next = ખાલી;
#નહીંતર  /* !CONFIG_WAITQ_SCALABLE */
		next = thread->next_wake_link;
#endif /* !CONFIG_WAITQ_SCALABLE */

		(શૂન્ય)z_try_abort_thread_timeout(thread);
		arch_thread_return_કિંમત_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_data->woken++;

		thread = next;
	}
}

સ્થિર પૂર્ણાંક futex_wake_one_walk_op(સંરચના k_thread *thread, શૂન્ય *data)
{
	સંરચના futex_walk_data *walk_data = data;

	/* Ignore threads waiting on different futexes */
	જો (thread->futex_pointer == walk_data->futex) {
		/* Find the highest priority thread */
		જો (walk_data->head == ખાલી ||
		    z_is_prio_higher(thread->base.prio, walk_data->head->base.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_data->head = thread;
		}
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક futex_wake_all_walk_op(સંરચના k_thread *thread, શૂન્ય *data)
{
	સંરચના futex_walk_data *walk_data = data;

	/* Ignore threads waiting on different futexes */
	જો (thread->futex_pointer == walk_data->futex) {
#ifndef CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(શૂન્ય)z_try_abort_thread_timeout(thread);
		arch_thread_return_કિંમત_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_data->woken++;
#નહીંતર  /* !CONFIG_WAITQ_SCALABLE */
		thread->next_wake_link = walk_data->head;
		walk_data->head = thread;
#endif /* !CONFIG_WAITQ_SCALABLE */
	}

	પાછા 0;
}

પૂર્ણાંક z_impl_k_futex_wake(સંરચના k_futex *futex, bool wake_all)
{
	k_spinlock_ચાવી_t ચાવી;
	સંરચના futex_walk_data walk_data = {.head = ખાલી, .futex = futex, .woken = 0};

	ચાવી = k_spin_lock(&futex_lock);

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

	જો (walk_data.woken == 0) {
		k_spin_unlock(&futex_lock, ચાવી);
	} નહીંતર {
		z_reschedule(&futex_lock, ચાવી);
	}

	પાછા walk_data.woken;
}

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_futex_wake(સંરચના k_futex *futex, bool wake_all)
{
	જો (K_SYSCALL_MEMORY_WRITE(futex, કદ(સંરચના k_futex)) != 0) {
		પાછા -EACCES;
	}

	પાછા z_impl_k_futex_wake(futex, wake_all);
}
#અન્તર્ભાવ <zephyr/syscalls/k_futex_wake_mrsh.c>

પૂર્ણાંક z_impl_k_futex_wait(સંરચના k_futex *futex, પૂર્ણાંક expected, k_timeout_t timeout)
{
	પૂર્ણાંક ret;
	k_spinlock_ચાવી_t ચાવી;

	ચાવી = k_spin_lock(&futex_lock);

	જો (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_lock, ચાવી);
		પાછા -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_pointer = futex;
#ifdef CONFIG_WAITQ_SCALABLE
	_current->next_wake_link = ખાલી;
#endif /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_lock, ચાવી, &futex_wait_q, timeout);
	જો (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	પાછા ret;
}

સ્થિર અંતઃરેખ પૂર્ણાંક z_vrfy_k_futex_wait(સંરચના k_futex *futex, પૂર્ણાંક expected, k_timeout_t timeout)
{
	જો (K_SYSCALL_MEMORY_WRITE(futex, કદ(સંરચના k_futex)) != 0) {
		પાછા -EACCES;
	}

	પાછા z_impl_k_futex_wait(futex, expected, timeout);
}
#અન્તર્ભાવ <zephyr/syscalls/k_futex_wait_mrsh.c>
