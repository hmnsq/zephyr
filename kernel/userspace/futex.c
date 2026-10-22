/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#شامل <zephyr/kernel.h>
#شامل <zephyr/spinlock.h>
#شامل <zephyr/internal/syscall_handler.h>
#شامل <zephyr/init.h>
#شامل <ksched.h>
#شامل <scheduler.h>

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
ساکن _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
ساکن ساخت k_spinlock futex_تالا;

/**
 * Context structure for walking the futex wait queue
 */
ساخت futex_walk_ڈیٹا {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	ساخت k_thread *سر;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	ساخت k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	صحیح woken;
};

ساکن خالی futex_post_walk_op(صحیح حالت_2, خالی *ڈیٹا)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_غیر_مستعمل(حالت_2);
	ساخت futex_walk_ڈیٹا *walk_ڈیٹا = ڈیٹا;
	ساخت k_thread *thread, *اگلا;

	thread = walk_ڈیٹا->سر;

	/* Wake up threads if any */
	جب (thread != تہی) {
#اگر_غیر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		اگلا = تہی;
#ورنہ  /* !CONFIG_WAITQ_SCALABLE */
		اگلا = thread->اگلا_wake_link;
#اگر_ختم /* !CONFIG_WAITQ_SCALABLE */

		(خالی)z_try_abort_thread_timeout(thread);
		arch_thread_return_قدر_سیٹ(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_ڈیٹا->woken++;

		thread = اگلا;
	}
}

ساکن صحیح futex_wake_one_walk_op(ساخت k_thread *thread, خالی *ڈیٹا)
{
	ساخت futex_walk_ڈیٹا *walk_ڈیٹا = ڈیٹا;

	/* Ignore threads waiting on different futexes */
	اگر (thread->futex_اشارہ_گر == walk_ڈیٹا->futex) {
		/* Find the highest priority thread */
		اگر (walk_ڈیٹا->سر == تہی ||
		    z_is_prio_higher(thread->بنیاد.prio, walk_ڈیٹا->سر->بنیاد.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_ڈیٹا->سر = thread;
		}
	}

	واپس 0;
}

ساکن صحیح futex_wake_all_walk_op(ساخت k_thread *thread, خالی *ڈیٹا)
{
	ساخت futex_walk_ڈیٹا *walk_ڈیٹا = ڈیٹا;

	/* Ignore threads waiting on different futexes */
	اگر (thread->futex_اشارہ_گر == walk_ڈیٹا->futex) {
#اگر_غیر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(خالی)z_try_abort_thread_timeout(thread);
		arch_thread_return_قدر_سیٹ(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_ڈیٹا->woken++;
#ورنہ  /* !CONFIG_WAITQ_SCALABLE */
		thread->اگلا_wake_link = walk_ڈیٹا->سر;
		walk_ڈیٹا->سر = thread;
#اگر_ختم /* !CONFIG_WAITQ_SCALABLE */
	}

	واپس 0;
}

صحیح z_impl_k_futex_wake(ساخت k_futex *futex, bool wake_all)
{
	k_spinlock_کلید_t کلید;
	ساخت futex_walk_ڈیٹا walk_ڈیٹا = {.سر = تہی, .futex = futex, .woken = 0};

	کلید = k_spin_تالا(&futex_تالا);

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
			   futex_post_walk_op, &walk_ڈیٹا);

	اگر (walk_ڈیٹا.woken == 0) {
		k_spin_unlock(&futex_تالا, کلید);
	} ورنہ {
		z_reschedule(&futex_تالا, کلید);
	}

	واپس walk_ڈیٹا.woken;
}

ساکن اندرونی صحیح z_vrfy_k_futex_wake(ساخت k_futex *futex, bool wake_all)
{
	اگر (K_SYSCALL_حافظہ_WRITE(futex, حجم(ساخت k_futex)) != 0) {
		واپس -EACCES;
	}

	واپس z_impl_k_futex_wake(futex, wake_all);
}
#شامل <zephyr/syscalls/k_futex_wake_mrsh.c>

صحیح z_impl_k_futex_wait(ساخت k_futex *futex, صحیح expected, k_timeout_t timeout)
{
	صحیح ret;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_تالا(&futex_تالا);

	اگر (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_تالا, کلید);
		واپس -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_اشارہ_گر = futex;
#اگر_تعریف_شدہ CONFIG_WAITQ_SCALABLE
	_current->اگلا_wake_link = تہی;
#اگر_ختم /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_تالا, کلید, &futex_wait_q, timeout);
	اگر (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	واپس ret;
}

ساکن اندرونی صحیح z_vrfy_k_futex_wait(ساخت k_futex *futex, صحیح expected, k_timeout_t timeout)
{
	اگر (K_SYSCALL_حافظہ_WRITE(futex, حجم(ساخت k_futex)) != 0) {
		واپس -EACCES;
	}

	واپس z_impl_k_futex_wait(futex, expected, timeout);
}
#شامل <zephyr/syscalls/k_futex_wait_mrsh.c>
