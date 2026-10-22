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

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <kspinlock.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <run_q.h>
#അന്തര്ഭാവ <zephyr/internal/syscall_handler.h>
#അന്തര്ഭാവ <zephyr/sys/math_extras.h>
#അന്തര്ഭാവ <zephyr/sys/minmax.h>
#അന്തര്ഭാവ <zephyr/logging/log.h>

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
ശൂന്യം z_sched_prio_deadline_set(ഘടന k_thread *thread, പൂർണ്ണസംഖ്യ deadline)
{
	Z_SCHED_SPINLOCK {
		എങ്കിൽ (z_is_thread_queued(thread)) {
			dequeue_thread(thread);
			thread->base.prio_deadline = deadline;
			queue_thread(thread);
		} അല്ലെങ്കിൽ {
			thread->base.prio_deadline = deadline;
		}
	}
}

ശൂന്യം z_impl_k_thread_absolute_deadline_set(k_tid_t tid, പൂർണ്ണസംഖ്യ deadline)
{
	ഘടന k_thread *thread = tid;

	/* The prio_deadline field changes the sorting order, so it must
	 * not be altered while the thread is in the run queue.  Because
	 * an rbtree will blow up if sorting invariants are violated mid-
	 * update, we delegate the dequeue/update/requeue sequence to
	 * z_sched_prio_deadline_set() which performs it under the
	 * scheduler spinlock.
	 */
	z_sched_prio_deadline_set(thread, deadline);
}

ശൂന്യം z_impl_k_thread_deadline_set(k_tid_t tid, പൂർണ്ണസംഖ്യ deadline)
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

	z_impl_k_thread_absolute_deadline_set(tid, (പൂർണ്ണസംഖ്യ)newdl);
}

#ifdef CONFIG_USERSPACE
സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_absolute_deadline_set(k_tid_t tid, പൂർണ്ണസംഖ്യ deadline)
{
	ഘടന k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));

	z_impl_k_thread_absolute_deadline_set((k_tid_t)thread, deadline);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_absolute_deadline_set_mrsh.c>

സ്ഥിതം വരിയിൽ ശൂന്യം z_vrfy_k_thread_deadline_set(k_tid_t tid, പൂർണ്ണസംഖ്യ deadline)
{
	ഘടന k_thread *thread = tid;

	K_OOPS(K_SYSCALL_OBJ(thread, K_OBJ_THREAD));
	K_OOPS(K_SYSCALL_VERIFY_സന്ദേശം(deadline > 0,
				    "invalid thread deadline %d",
				    (പൂർണ്ണസംഖ്യ)deadline));

	z_impl_k_thread_deadline_set((k_tid_t)thread, deadline);
}
#അന്തര്ഭാവ <zephyr/syscalls/k_thread_deadline_set_mrsh.c>
#endif /* CONFIG_USERSPACE */
