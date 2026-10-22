/*
 * Copyright (c) 2019 Intel corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#समावेश <zephyr/kernel.h>
#समावेश <zephyr/spinlock.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/init.h>
#समावेश <ksched.h>
#समावेश <scheduler.h>

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
अटल _wait_q_t futex_wait_q = Z_WAIT_Q_INIT(&futex_wait_q);
अटल काष्ठा k_spinlock futex_lock;

/**
 * Context structure for walking the futex wait queue
 */
काष्ठा futex_walk_तथ्य {
	/**
	 * List head of threads to wake up during post op (NULL if empty)
	 * This is an intermediate value filled by the walk op
	 * Must be initialized to NULL before the walk
	 */
	काष्ठा k_thread *शीर्ष;
	/**
	 * Futex address
	 * This is an input to the walk
	 */
	काष्ठा k_futex *futex;
	/**
	 * Number of threads that have been woken up
	 * This is an output of the walk and must be initialized to zero
	 */
	पूर्णांक woken;
};

अटल व्योम futex_post_walk_op(पूर्णांक status, व्योम *तथ्य)
{
	/*
	 * Note: z_sched_wake_thread_locked() is safe
	 * to call here because this walk_op callback
	 * is invoked with _sched_spinlock held.
	 */
	ARG_UNUSED(status);
	काष्ठा futex_walk_तथ्य *walk_तथ्य = तथ्य;
	काष्ठा k_thread *thread, *अग्र;

	thread = walk_तथ्य->शीर्ष;

	/* Wake up threads if any */
	जबतक (thread != शून्य) {
#अगर_अघोषित CONFIG_WAITQ_SCALABLE
		/*
		 * walk_data->head will only ever be non-NULL for wake-one,
		 * which has no "next" thread to wake by definition.
		 */
		अग्र = शून्य;
#अन्यथा  /* !CONFIG_WAITQ_SCALABLE */
		अग्र = thread->अग्र_wake_link;
#पूर्ण_अगर /* !CONFIG_WAITQ_SCALABLE */

		(व्योम)z_try_abort_thread_timeout(thread);
		arch_thread_return_मूल्य_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_तथ्य->woken++;

		thread = अग्र;
	}
}

अटल पूर्णांक futex_wake_one_walk_op(काष्ठा k_thread *thread, व्योम *तथ्य)
{
	काष्ठा futex_walk_तथ्य *walk_तथ्य = तथ्य;

	/* Ignore threads waiting on different futexes */
	अगर (thread->futex_pointer == walk_तथ्य->futex) {
		/* Find the highest priority thread */
		अगर (walk_तथ्य->शीर्ष == शून्य ||
		    z_is_prio_higher(thread->base.prio, walk_तथ्य->शीर्ष->base.prio)) {
			/*
			 * next_wake_link is initialized to NULL in z_impl_k_futex_wait so
			 * this is always a single thread
			 */
			walk_तथ्य->शीर्ष = thread;
		}
	}

	वापस 0;
}

अटल पूर्णांक futex_wake_all_walk_op(काष्ठा k_thread *thread, व्योम *तथ्य)
{
	काष्ठा futex_walk_तथ्य *walk_तथ्य = तथ्य;

	/* Ignore threads waiting on different futexes */
	अगर (thread->futex_pointer == walk_तथ्य->futex) {
#अगर_अघोषित CONFIG_WAITQ_SCALABLE
		/*
		 * Note: z_sched_wake_thread_locked() is safe
		 * to call here because this walk_op callback
		 * is invoked with _sched_spinlock held.
		 */
		(व्योम)z_try_abort_thread_timeout(thread);
		arch_thread_return_मूल्य_set(thread, 0);
		z_sched_wake_thread_locked(thread);
		walk_तथ्य->woken++;
#अन्यथा  /* !CONFIG_WAITQ_SCALABLE */
		thread->अग्र_wake_link = walk_तथ्य->शीर्ष;
		walk_तथ्य->शीर्ष = thread;
#पूर्ण_अगर /* !CONFIG_WAITQ_SCALABLE */
	}

	वापस 0;
}

पूर्णांक z_impl_k_futex_wake(काष्ठा k_futex *futex, bool wake_all)
{
	k_spinlock_कुंजी_t कुंजी;
	काष्ठा futex_walk_तथ्य walk_तथ्य = {.शीर्ष = शून्य, .futex = futex, .woken = 0};

	कुंजी = k_spin_lock(&futex_lock);

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
			   futex_post_walk_op, &walk_तथ्य);

	अगर (walk_तथ्य.woken == 0) {
		k_spin_unlock(&futex_lock, कुंजी);
	} अन्यथा {
		z_reschedule(&futex_lock, कुंजी);
	}

	वापस walk_तथ्य.woken;
}

अटल अंतरभूत पूर्णांक z_vrfy_k_futex_wake(काष्ठा k_futex *futex, bool wake_all)
{
	अगर (K_SYSCALL_MEMORY_WRITE(futex, माप(काष्ठा k_futex)) != 0) {
		वापस -EACCES;
	}

	वापस z_impl_k_futex_wake(futex, wake_all);
}
#समावेश <zephyr/syscalls/k_futex_wake_mrsh.c>

पूर्णांक z_impl_k_futex_wait(काष्ठा k_futex *futex, पूर्णांक expected, k_timeout_t timeout)
{
	पूर्णांक ret;
	k_spinlock_कुंजी_t कुंजी;

	कुंजी = k_spin_lock(&futex_lock);

	अगर (atomic_get(&futex->val) != (atomic_val_t)expected) {
		k_spin_unlock(&futex_lock, कुंजी);
		वापस -EAGAIN;
	}

	/*
	 * Store futex pointer we are waiting on, so that it can be used to filter
	 * futexes when walking the wait queue
	 */
	_current->futex_pointer = futex;
#अगर_घोषित CONFIG_WAITQ_SCALABLE
	_current->अग्र_wake_link = शून्य;
#पूर्ण_अगर /* CONFIG_WAITQ_SCALABLE */
	ret = z_pend_curr(&futex_lock, कुंजी, &futex_wait_q, timeout);
	अगर (ret == -EAGAIN) {
		ret = -ETIMEDOUT;
	}

	वापस ret;
}

अटल अंतरभूत पूर्णांक z_vrfy_k_futex_wait(काष्ठा k_futex *futex, पूर्णांक expected, k_timeout_t timeout)
{
	अगर (K_SYSCALL_MEMORY_WRITE(futex, माप(काष्ठा k_futex)) != 0) {
		वापस -EACCES;
	}

	वापस z_impl_k_futex_wait(futex, expected, timeout);
}
#समावेश <zephyr/syscalls/k_futex_wait_mrsh.c>
