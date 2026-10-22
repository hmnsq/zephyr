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

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <kspinlock.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <run_q.h>
#ಅನ್ತರ್ಭಾವ <zephyr/internal/syscall_handler.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/math_extras.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/minmax.h>
#ಅನ್ತರ್ಭಾವ <zephyr/logging/log.h>

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
ಶೂನ್ಯ z_sched_prio_deadline_ಗಣ(ರಚನೆ k_thread *thread, ಪೂರ್ಣಾಂಕ deadline)
{
	Z_SCHED_SPINLOCK {
		ಆದರೆ (z_is_thread_queued(thread)) {
			ಸರತಿಯಿಂದ_ತೆಗೆ_thread(thread);
			thread->ತಳ.prio_deadline = deadline;
			ಸರತಿ_thread(thread);
		} ಇಲ್ಲದಿದ್ದರೆ {
			thread->ತಳ.prio_deadline = deadline;
		}
	}
}

ಶೂನ್ಯ z_impl_k_thread_absolute_deadline_ಗಣ(k_tid_t tid, ಪೂರ್ಣಾಂಕ deadline)
{
	ರಚನೆ k_thread *thread = tid;

	/* The prio_deadline field changes the sorting order, so it must
	 * not be altered while the thread is in the run queue.  Because
	 * an rbtree will blow up if sorting invariants are violated mid-
	 * update, we delegate the dequeue/update/requeue sequence to
	 * z_sched_prio_deadline_set() which performs it under the
	 * scheduler spinlock.
	 */
	z_sched_prio_deadline_ಗಣ(thread, deadline);
}

ಶೂನ್ಯ z_impl_k_thread_deadline_ಗಣ(k_tid_t tid, ಪೂರ್ಣಾಂಕ deadline)
{
	/*
	 * Clamp the relative deadline to INT32_MAX / 2 (2^30 cycles) to
	 * satisfy the scheduler comparator's half-modulus invariant.
	 * This leaves a 2^30-cycle margin of tolerance for scheduler
	 * execution skew before priority inversion can occur.
	 */
	deadline = clamp(deadline, 0, INT32_MAX / 2);

	/* Use uint32_t arithmetic to avoid implementation-defined signed conversion. */
	uint32_t newdl = k_ಚಕ್ರ_get_32() + (uint32_t)deadline;

	z_impl_k_thread_absolute_deadline_ಗಣ(tid, (ಪೂರ್ಣಾಂಕ)newdl);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_USERSPACE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_absolute_deadline_ಗಣ(k_tid_t tid, ಪೂರ್ಣಾಂಕ deadline)
{
	ರಚನೆ k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	z_impl_k_thread_absolute_deadline_ಗಣ((k_tid_t)thread, deadline);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_absolute_deadline_set_mrsh.c>

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_vrfy_k_thread_deadline_ಗಣ(k_tid_t tid, ಪೂರ್ಣಾಂಕ deadline)
{
	ರಚನೆ k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_ಸಂದೇಶ(deadline > 0,
				    "invalid thread deadline %d",
				    (ಪೂರ್ಣಾಂಕ)deadline));

	z_impl_k_thread_deadline_ಗಣ((k_tid_t)thread, deadline);
}
#ಅನ್ತರ್ಭಾವ <zephyr/syscalls/k_thread_deadline_set_mrsh.c>
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_USERSPACE */
