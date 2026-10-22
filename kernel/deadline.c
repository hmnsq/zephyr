/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright The Zephyr Project Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief EDF (Earliest Deadline First) scheduling support
 *
 * Implements the public k_thread_deadline_set() and
 * k_thread_absolute_deadline_set() APIs.  The run-queue manipulation
 * required to keep deadline ordering is delegated back to the scheduler
 * via z_sched_prio_deadline_set() so that sched.c keeps full ownership
 * of the run-queue data structures.
 */

#समावेश <zephyr/kernel.h>
#समावेश <kspinlock.h>
#समावेश <ksched.h>
#समावेश <run_q.h>
#समावेश <zephyr/internal/syscall_handler.h>
#समावेश <zephyr/sys/math_extras.h>
#समावेश <zephyr/sys/minmax.h>
#समावेश <zephyr/logging/log.h>

/**
 * @brief Atomically update a thread's prio_deadline in the run queue.
 *
 * If the thread is currently queued the scheduler re-inserts it so that
 * run-queue ordering (including rbtree invariants) is preserved.  May be
 * called from any context; takes and releases the scheduler's spinlock
 * internally.
 *
 * @param thread   Thread whose deadline is being updated.
 * @param deadline New absolute deadline value (raw cycle counter).
 */
व्योम z_sched_prio_deadline_set(काष्ठा k_thread *thread, पूर्णांक deadline)
{
	Z_SCHED_SPINLOCK {
		अगर (z_is_thread_queued(thread)) {
			dequeue_thread(thread);
			thread->base.prio_deadline = deadline;
			queue_thread(thread);
		} अन्यथा {
			thread->base.prio_deadline = deadline;
		}
	}
}

व्योम z_impl_k_thread_absolute_deadline_set(k_tid_t tid, पूर्णांक deadline)
{
	काष्ठा k_thread *thread = tid;

	/* The prio_deadline field changes the sorting order, so it must
	 * not be altered while the thread is in the run queue.  Because
	 * an rbtree will blow up if sorting invariants are violated mid-
	 * update, we delegate the dequeue/update/requeue sequence to
	 * z_sched_prio_deadline_set() which performs it under the
	 * scheduler spinlock.
	 */
	z_sched_prio_deadline_set(thread, deadline);
}

व्योम z_impl_k_thread_deadline_set(k_tid_t tid, पूर्णांक deadline)
{
	/*
	 * Clamp the relative deadline to INT32_MAX / 2 (2^30 cycles) to
	 * satisfy the scheduler comparator's half-modulus invariant.
	 * This leaves a 2^30-cycle margin of tolerance for scheduler
	 * execution skew before priority inversion can occur.
	 */
	deadline = clamp(deadline, 0, INT32_MAX / 2);

	/* Use uint32_t arithmetic to avoid implementation-defined signed conversion. */
	uint32_t newdl = k_cycle_get_32() + (uint32_t)deadline;

	z_impl_k_thread_absolute_deadline_set(tid, (पूर्णांक)newdl);
}

#अगर_घोषित CONFIG_USERSPACE
अटल अंतरभूत व्योम z_vrfy_k_thread_absolute_deadline_set(k_tid_t tid, पूर्णांक deadline)
{
	काष्ठा k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	z_impl_k_thread_absolute_deadline_set((k_tid_t)thread, deadline);
}
#समावेश <zephyr/syscalls/k_thread_absolute_deadline_set_mrsh.c>

अटल अंतरभूत व्योम z_vrfy_k_thread_deadline_set(k_tid_t tid, पूर्णांक deadline)
{
	काष्ठा k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_संदेश(deadline > 0,
				    "invalid thread deadline %d",
				    (पूर्णांक)deadline));

	z_impl_k_thread_deadline_set((k_tid_t)thread, deadline);
}
#समावेश <zephyr/syscalls/k_thread_deadline_set_mrsh.c>
#पूर्ण_अगर /* CONFIG_USERSPACE */
