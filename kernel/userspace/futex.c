/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/init.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <scheduler.h>

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
స్థిర _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
స్థిర నిర్మాణం k_spinlock futex_తాళం;

/**
 * Context structure for walking the futex wait queue
 */
నిర్మాణం futex_walk_దత్తాంశం {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	నిర్మాణం k_thread *తల;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	నిర్మాణం k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	పూర్ణాంకం woken;
};

స్థిర శూన్య futex_post_walk_op(పూర్ణాంకం స్థితి_2, శూన్య *దత్తాంశం)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_వాడని(స్థితి_2);
	నిర్మాణం futex_walk_దత్తాంశం *walk_దత్తాంశం = దత్తాంశం;
	నిర్మాణం k_thread *thread, *తదుపరి;

	thread = walk_దత్తాంశం->తల;

	/* Wake up threads if any */
	అయితే (thread != ఖాళీ) {
#నిర్వచితం_కాకపోతే CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		తదుపరి = ఖాళీ;
#లేకపోతే  /* !CONFIG_WAITQ_SCALABLE */
		తదుపరి = thread->తదుపరి_wake_link;
#అయితే_ముగింపు /* !CONFIG_WAITQ_SCALABLE */

		(శూన్య)z_try_abort_thread_timeout(thread);
		arch_thread_return_విలువ_సమితి(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_దత్తాంశం->woken++;

		thread = తదుపరి;
	}
}

స్థిర పూర్ణాంకం futex_wake_one_walk_op(నిర్మాణం k_thread *thread, శూన్య *దత్తాంశం)
{
	నిర్మాణం futex_walk_దత్తాంశం *walk_దత్తాంశం = దత్తాంశం;

	/* Ignore threads waiting on different futexes */
	ఒకవేళ (thread->futex_సూచిక == walk_దత్తాంశం->futex) {
		/* Find the highest priority thread */
		ఒకవేళ (walk_దత్తాంశం->తల == ఖాళీ ||
		    z_is_prio_higher(thread->పునాది.prio, walk_దత్తాంశం->తల->పునాది.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_దత్తాంశం->తల = thread;
		}
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం futex_wake_all_walk_op(నిర్మాణం k_thread *thread, శూన్య *దత్తాంశం)
{
	నిర్మాణం futex_walk_దత్తాంశం *walk_దత్తాంశం = దత్తాంశం;

	/* Ignore threads waiting on different futexes */
	ఒకవేళ (thread->futex_సూచిక == walk_దత్తాంశం->futex) {
#నిర్వచితం_కాకపోతే CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(శూన్య)z_try_abort_thread_timeout(thread);
		arch_thread_return_విలువ_సమితి(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_దత్తాంశం->woken++;
#లేకపోతే  /* !CONFIG_WAITQ_SCALABLE */
		thread->తదుపరి_wake_link = walk_దత్తాంశం->తల;
		walk_దత్తాంశం->తల = thread;
#అయితే_ముగింపు /* !CONFIG_WAITQ_SCALABLE */
	}

	తిరిగి 0;
}

పూర్ణాంకం z_impl_k_futex_wake(నిర్మాణం k_futex *futex, bool wake_all)
{
	k_spinlock_కీలకం_t కీలకం;
	నిర్మాణం futex_walk_దత్తాంశం walk_దత్తాంశం = {.తల = ఖాళీ, .futex = futex, .woken = 0};

	కీలకం = k_spin_తాళం(&futex_తాళం);

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
			   futex_post_walk_op, &walk_దత్తాంశం);

	ఒకవేళ (walk_దత్తాంశం.woken == 0) {
		k_spin_unlock(&futex_తాళం, కీలకం);
	} లేకపోతే {
		z_reschedule(&futex_తాళం, కీలకం);
	}

	తిరిగి walk_దత్తాంశం.woken;
}

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_futex_wake(నిర్మాణం k_futex *futex, bool wake_all)
{
	ఒకవేళ (K_SYSCALL_జ్ఞాపకం_WRITE(futex, పరిమాణం(నిర్మాణం k_futex)) != 0) {
		తిరిగి -EACCES;
	}

	తిరిగి z_impl_k_futex_wake(futex, wake_all);
}
#అన్తర్భావ <zephyr/syscalls/k_futex_wake_mrsh.c>

పూర్ణాంకం z_impl_k_futex_wait(నిర్మాణం k_futex *futex, పూర్ణాంకం expected, k_timeout_t timeout)
{
	పూర్ణాంకం ret;
	k_spinlock_కీలకం_t కీలకం;

	కీలకం = k_spin_తాళం(&futex_తాళం);

	ఒకవేళ (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_తాళం, కీలకం);
		తిరిగి -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_సూచిక = futex;
#నిర్వచితమైతే CONFIG_WAITQ_SCALABLE
	_current->తదుపరి_wake_link = ఖాళీ;
#అయితే_ముగింపు /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_తాళం, కీలకం, &futex_wait_q, timeout);
	ఒకవేళ (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	తిరిగి ret;
}

స్థిర అంతఃవరుస పూర్ణాంకం z_vrfy_k_futex_wait(నిర్మాణం k_futex *futex, పూర్ణాంకం expected, k_timeout_t timeout)
{
	ఒకవేళ (K_SYSCALL_జ్ఞాపకం_WRITE(futex, పరిమాణం(నిర్మాణం k_futex)) != 0) {
		తిరిగి -EACCES;
	}

	తిరిగి z_impl_k_futex_wait(futex, expected, timeout);
}
#అన్తర్భావ <zephyr/syscalls/k_futex_wait_mrsh.c>
