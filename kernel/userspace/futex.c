/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ضمن <zephyr/kernel.h>
#ضمن <zephyr/spinlock.h>
#ضمن <zephyr/internal/syscall_handler.h>
#ضمن <zephyr/init.h>
#ضمن <ksched.h>
#ضمن <scheduler.h>

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
ساكن _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
ساكن بنية k_spinlock futex_قفل;

/**
 * Context structure for walking the futex wait queue
 */
بنية futex_walk_داده {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	بنية k_thread *سر;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	بنية k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	صحيح woken;
};

ساكن فراغ futex_post_walk_op(صحيح وضعیت, فراغ *داده)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_استفادهنشده(وضعیت);
	بنية futex_walk_داده *walk_داده = داده;
	بنية k_thread *thread, *بعدی;

	thread = walk_داده->سر;

	/* Wake up threads if any */
	طالما (thread != عدم) {
#اگر_تعریف_نشده CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		بعدی = عدم;
#وإلا  /* !CONFIG_WAITQ_SCALABLE */
		بعدی = thread->بعدی_wake_link;
#پایان_اگر /* !CONFIG_WAITQ_SCALABLE */

		(فراغ)z_try_abort_thread_timeout(thread);
		arch_thread_return_مقدار_مجموعه(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_داده->woken++;

		thread = بعدی;
	}
}

ساكن صحيح futex_wake_one_walk_op(بنية k_thread *thread, فراغ *داده)
{
	بنية futex_walk_داده *walk_داده = داده;

	/* Ignore threads waiting on different futexes */
	إذا (thread->futex_اشاره_گر == walk_داده->futex) {
		/* Find the highest priority thread */
		إذا (walk_داده->سر == عدم ||
		    z_is_prio_higher(thread->پایه.prio, walk_داده->سر->پایه.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_داده->سر = thread;
		}
	}

	أرجع 0;
}

ساكن صحيح futex_wake_all_walk_op(بنية k_thread *thread, فراغ *داده)
{
	بنية futex_walk_داده *walk_داده = داده;

	/* Ignore threads waiting on different futexes */
	إذا (thread->futex_اشاره_گر == walk_داده->futex) {
#اگر_تعریف_نشده CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(فراغ)z_try_abort_thread_timeout(thread);
		arch_thread_return_مقدار_مجموعه(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_داده->woken++;
#وإلا  /* !CONFIG_WAITQ_SCALABLE */
		thread->بعدی_wake_link = walk_داده->سر;
		walk_داده->سر = thread;
#پایان_اگر /* !CONFIG_WAITQ_SCALABLE */
	}

	أرجع 0;
}

صحيح z_impl_k_futex_wake(بنية k_futex *futex, bool wake_all)
{
	k_spinlock_کلید_t کلید;
	بنية futex_walk_داده walk_داده = {.سر = عدم, .futex = futex, .woken = 0};

	کلید = k_spin_قفل(&futex_قفل);

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
			   futex_post_walk_op, &walk_داده);

	إذا (walk_داده.woken == 0) {
		k_spin_unlock(&futex_قفل, کلید);
	} وإلا {
		z_reschedule(&futex_قفل, کلید);
	}

	أرجع walk_داده.woken;
}

ساكن مضمن صحيح z_vrfy_k_futex_wake(بنية k_futex *futex, bool wake_all)
{
	إذا (K_SYSCALL_حافظه_WRITE(futex, حجم(بنية k_futex)) != 0) {
		أرجع -EACCES;
	}

	أرجع z_impl_k_futex_wake(futex, wake_all);
}
#ضمن <zephyr/syscalls/k_futex_wake_mrsh.c>

صحيح z_impl_k_futex_wait(بنية k_futex *futex, صحيح expected, k_timeout_t timeout)
{
	صحيح ret;
	k_spinlock_کلید_t کلید;

	کلید = k_spin_قفل(&futex_قفل);

	إذا (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_قفل, کلید);
		أرجع -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_اشاره_گر = futex;
#اگر_تعریف_شده CONFIG_WAITQ_SCALABLE
	_current->بعدی_wake_link = عدم;
#پایان_اگر /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_قفل, کلید, &futex_wait_q, timeout);
	إذا (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	أرجع ret;
}

ساكن مضمن صحيح z_vrfy_k_futex_wait(بنية k_futex *futex, صحيح expected, k_timeout_t timeout)
{
	إذا (K_SYSCALL_حافظه_WRITE(futex, حجم(بنية k_futex)) != 0) {
		أرجع -EACCES;
	}

	أرجع z_impl_k_futex_wait(futex, expected, timeout);
}
#ضمن <zephyr/syscalls/k_futex_wait_mrsh.c>
