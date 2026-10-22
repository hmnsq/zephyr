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

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <kspinlock.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <run_q.h>
#అన్తర్భావ <zephyr/internal/syscall_handler.h>
#అన్తర్భావ <zephyr/sys/math_extras.h>
#అన్తర్భావ <zephyr/sys/minmax.h>
#అన్తర్భావ <zephyr/logging/log.h>

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
శూన్య z_sched_prio_deadline_సమితి(నిర్మాణం k_thread *thread, పూర్ణాంకం deadline)
{
	Z_SCHED_SPINLOCK {
		ఒకవేళ (z_is_thread_queued(thread)) {
			వరుస_నుండి_తీయి_thread(thread);
			thread->పునాది.prio_deadline = deadline;
			వరుస_thread(thread);
		} లేకపోతే {
			thread->పునాది.prio_deadline = deadline;
		}
	}
}

శూన్య z_impl_k_thread_absolute_deadline_సమితి(k_tid_t tid, పూర్ణాంకం deadline)
{
	నిర్మాణం k_thread *thread = tid;

	/* The prio_deadline field changes the sorting order, so it must
	 * not be altered while the thread is in the run queue.  Because
	 * an rbtree will blow up if sorting invariants are violated mid-
	 * update, we delegate the dequeue/update/requeue sequence to
	 * z_sched_prio_deadline_set() which performs it under the
	 * scheduler spinlock.
	 */
	z_sched_prio_deadline_సమితి(thread, deadline);
}

శూన్య z_impl_k_thread_deadline_సమితి(k_tid_t tid, పూర్ణాంకం deadline)
{
	/*
	 * Clamp the relative deadline to INT32_MAX / 2 (2^30 cycles) to
	 * satisfy the scheduler comparator's half-modulus invariant.
	 * This leaves a 2^30-cycle margin of tolerance for scheduler
	 * execution skew before priority inversion can occur.
	 */
	deadline = clamp(deadline, 0, INT32_MAX / 2);

	/* Use uint32_t arithmetic to avoid implementation-defined signed conversion. */
	uint32_t newdl = k_చక్రం_get_32() + (uint32_t)deadline;

	z_impl_k_thread_absolute_deadline_సమితి(tid, (పూర్ణాంకం)newdl);
}

#నిర్వచితమైతే CONFIG_USERSPACE
స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_absolute_deadline_సమితి(k_tid_t tid, పూర్ణాంకం deadline)
{
	నిర్మాణం k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	z_impl_k_thread_absolute_deadline_సమితి((k_tid_t)thread, deadline);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_absolute_deadline_set_mrsh.c>

స్థిర అంతఃవరుస శూన్య z_vrfy_k_thread_deadline_సమితి(k_tid_t tid, పూర్ణాంకం deadline)
{
	నిర్మాణం k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_సందేశం(deadline > 0,
				    "invalid thread deadline %d",
				    (పూర్ణాంకం)deadline));

	z_impl_k_thread_deadline_సమితి((k_tid_t)thread, deadline);
}
#అన్తర్భావ <zephyr/syscalls/k_thread_deadline_set_mrsh.c>
#అయితే_ముగింపు /* CONFIG_USERSPACE */
