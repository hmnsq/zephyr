/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <zephyr/internal/syscall_handler.h>
#अन्तर्भाव <zephyr/init.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <scheduler.h>

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
स्थिर _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
स्थिर संरचना k_spinlock futex_तालकम्;

/**
 * Context structure for walking the futex wait queue
 */
संरचना futex_walk_दत्त {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	संरचना k_thread *शीर्ष;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	संरचना k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	पूर्णाङ्क woken;
};

स्थिर शून्य futex_post_walk_op(पूर्णाङ्क स्थितिः, शून्य *दत्त)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_अप्रयुक्तम्(स्थितिः);
	संरचना futex_walk_दत्त *walk_दत्त = दत्त;
	संरचना k_thread *thread, *अग्र;

	thread = walk_दत्त->शीर्ष;

	/* Wake up threads if any */
	यावत् (thread != रिक्त) {
#यदि_अपरिभाषितम् CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		अग्र = रिक्त;
#अन्यथा  /* !CONFIG_WAITQ_SCALABLE */
		अग्र = thread->अग्र_wake_link;
#यदि_अन्तः /* !CONFIG_WAITQ_SCALABLE */

		(शून्य)z_try_abort_thread_timeout(thread);
		arch_thread_return_मानम्_समुच्चयः(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_दत्त->woken++;

		thread = अग्र;
	}
}

स्थिर पूर्णाङ्क futex_wake_one_walk_op(संरचना k_thread *thread, शून्य *दत्त)
{
	संरचना futex_walk_दत्त *walk_दत्त = दत्त;

	/* Ignore threads waiting on different futexes */
	यदि (thread->futex_pointer == walk_दत्त->futex) {
		/* Find the highest priority thread */
		यदि (walk_दत्त->शीर्ष == रिक्त ||
		    z_is_prio_higher(thread->आधारः.prio, walk_दत्त->शीर्ष->आधारः.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_दत्त->शीर्ष = thread;
		}
	}

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क futex_wake_all_walk_op(संरचना k_thread *thread, शून्य *दत्त)
{
	संरचना futex_walk_दत्त *walk_दत्त = दत्त;

	/* Ignore threads waiting on different futexes */
	यदि (thread->futex_pointer == walk_दत्त->futex) {
#यदि_अपरिभाषितम् CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(शून्य)z_try_abort_thread_timeout(thread);
		arch_thread_return_मानम्_समुच्चयः(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_दत्त->woken++;
#अन्यथा  /* !CONFIG_WAITQ_SCALABLE */
		thread->अग्र_wake_link = walk_दत्त->शीर्ष;
		walk_दत्त->शीर्ष = thread;
#यदि_अन्तः /* !CONFIG_WAITQ_SCALABLE */
	}

	प्रत्यागम 0;
}

पूर्णाङ्क z_impl_k_futex_wake(संरचना k_futex *futex, bool wake_all)
{
	k_spinlock_कुञ्चिका_t कुञ्चिका;
	संरचना futex_walk_दत्त walk_दत्त = {.शीर्ष = रिक्त, .futex = futex, .woken = 0};

	कुञ्चिका = k_spin_तालकम्(&futex_तालकम्);

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
			   futex_post_walk_op, &walk_दत्त);

	यदि (walk_दत्त.woken == 0) {
		k_spin_unlock(&futex_तालकम्, कुञ्चिका);
	} अन्यथा {
		z_reschedule(&futex_तालकम्, कुञ्चिका);
	}

	प्रत्यागम walk_दत्त.woken;
}

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_futex_wake(संरचना k_futex *futex, bool wake_all)
{
	यदि (K_SYSCALL_स्मृतिः_WRITE(futex, परिमाण(संरचना k_futex)) != 0) {
		प्रत्यागम -EACCES;
	}

	प्रत्यागम z_impl_k_futex_wake(futex, wake_all);
}
#अन्तर्भाव <zephyr/syscalls/k_futex_wake_mrsh.c>

पूर्णाङ्क z_impl_k_futex_wait(संरचना k_futex *futex, पूर्णाङ्क expected, k_timeout_t timeout)
{
	पूर्णाङ्क ret;
	k_spinlock_कुञ्चिका_t कुञ्चिका;

	कुञ्चिका = k_spin_तालकम्(&futex_तालकम्);

	यदि (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_तालकम्, कुञ्चिका);
		प्रत्यागम -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_pointer = futex;
#यदि_परिभाषितम् CONFIG_WAITQ_SCALABLE
	_current->अग्र_wake_link = रिक्त;
#यदि_अन्तः /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_तालकम्, कुञ्चिका, &futex_wait_q, timeout);
	यदि (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	प्रत्यागम ret;
}

स्थिर अन्तःस्थ पूर्णाङ्क z_vrfy_k_futex_wait(संरचना k_futex *futex, पूर्णाङ्क expected, k_timeout_t timeout)
{
	यदि (K_SYSCALL_स्मृतिः_WRITE(futex, परिमाण(संरचना k_futex)) != 0) {
		प्रत्यागम -EACCES;
	}

	प्रत्यागम z_impl_k_futex_wait(futex, expected, timeout);
}
#अन्तर्भाव <zephyr/syscalls/k_futex_wait_mrsh.c>
