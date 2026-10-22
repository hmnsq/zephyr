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

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <kspinlock.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <run_q.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/internal/syscall_handler.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/math_extras.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/minmax.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/logging/log.h>

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
ଶୂନ୍ୟ z_sched_prio_deadline_set(ସଂରଚନା k_thread *thread, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadline)
{
	Z_SCHED_SPINLOCK {
		ଯଦି (z_is_thread_queued(thread)) {
			dequeue_thread(thread);
			thread->base.prio_deadline = deadline;
			queue_thread(thread);
		} ଅନ୍ୟଥା {
			thread->base.prio_deadline = deadline;
		}
	}
}

ଶୂନ୍ୟ z_impl_k_thread_absolute_deadline_set(k_tid_t tid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadline)
{
	ସଂରଚନା k_thread *thread = tid;

	/* The prio_deadline field changes the sorting order, so it must
	 * not be altered while the thread is in the run queue.  Because
	 * an rbtree will blow up if sorting invariants are violated mid-
	 * update, we delegate the dequeue/update/requeue sequence to
	 * z_sched_prio_deadline_set() which performs it under the
	 * scheduler spinlock.
	 */
	z_sched_prio_deadline_set(thread, deadline);
}

ଶୂନ୍ୟ z_impl_k_thread_deadline_set(k_tid_t tid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadline)
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

	z_impl_k_thread_absolute_deadline_set(tid, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)newdl);
}

#ifdef CONFIG_USERSPACE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_absolute_deadline_set(k_tid_t tid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadline)
{
	ସଂରଚନା k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	z_impl_k_thread_absolute_deadline_set((k_tid_t)thread, deadline);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_absolute_deadline_set_mrsh.c>

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ z_vrfy_k_thread_deadline_set(k_tid_t tid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadline)
{
	ସଂରଚନା k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_ସନ୍ଦେଶ(deadline > 0,
				    "invalid thread deadline %d",
				    (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)deadline));

	z_impl_k_thread_deadline_set((k_tid_t)thread, deadline);
}
#ଅନ୍ତର୍ଭାଵ <zephyr/syscalls/k_thread_deadline_set_mrsh.c>
#endif /* CONFIG_USERSPACE */
