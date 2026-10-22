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

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <kspinlock.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <run_q.h>
#অন্তর্ভাৱ <zephyr/internal/syscall_handler.h>
#অন্তর্ভাৱ <zephyr/sys/math_extras.h>
#অন্তর্ভাৱ <zephyr/sys/minmax.h>
#অন্তর্ভাৱ <zephyr/logging/log.h>

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
শূন্য z_sched_prio_deadline_set(সংরচনা k_thread *thread, পূর্ণাঙ্ক deadline)
{
	Z_SCHED_SPINLOCK {
		যদি (z_is_thread_queued(thread)) {
			dequeue_thread(thread);
			thread->base.prio_deadline = deadline;
			queue_thread(thread);
		} অন্যথা {
			thread->base.prio_deadline = deadline;
		}
	}
}

শূন্য z_impl_k_thread_absolute_deadline_set(k_tid_t tid, পূর্ণাঙ্ক deadline)
{
	সংরচনা k_thread *thread = tid;

	/* The prio_deadline field changes the sorting order, so it must
	 * not be altered while the thread is in the run queue.  Because
	 * an rbtree will blow up if sorting invariants are violated mid-
	 * update, we delegate the dequeue/update/requeue sequence to
	 * z_sched_prio_deadline_set() which performs it under the
	 * scheduler spinlock.
	 */
	z_sched_prio_deadline_set(thread, deadline);
}

শূন্য z_impl_k_thread_deadline_set(k_tid_t tid, পূর্ণাঙ্ক deadline)
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

	z_impl_k_thread_absolute_deadline_set(tid, (পূর্ণাঙ্ক)newdl);
}

#ifdef CONFIG_USERSPACE
স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_absolute_deadline_set(k_tid_t tid, পূর্ণাঙ্ক deadline)
{
	সংরচনা k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	z_impl_k_thread_absolute_deadline_set((k_tid_t)thread, deadline);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_absolute_deadline_set_mrsh.c>

স্থির অন্তঃস্থ শূন্য z_vrfy_k_thread_deadline_set(k_tid_t tid, পূর্ণাঙ্ক deadline)
{
	সংরচনা k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_বাৰ্তা(deadline > 0,
				    "invalid thread deadline %d",
				    (পূর্ণাঙ্ক)deadline));

	z_impl_k_thread_deadline_set((k_tid_t)thread, deadline);
}
#অন্তর্ভাৱ <zephyr/syscalls/k_thread_deadline_set_mrsh.c>
#endif /* CONFIG_USERSPACE */
