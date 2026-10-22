/*
 * Copyright (c) 2018 Intel Corporation
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief MetaIRQ scheduling helpers (CONFIG_NUM_METAIRQ_PRIORITIES)
 *
 * MetaIRQ threads are a special class of cooperative threads that run at higher
 * priority than any other thread and must be returned to after preemption.
 * This header encapsulates all of that bookkeeping so that sched.c itself
 * stays focused on run-queue management.
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_METAIRQ_H_
#define ZEPHYR_KERNEL_INCLUDE_METAIRQ_H_

#شامل <zephyr/kernel.h>
#شامل <kthread.h>

BUILD_ASSERT(CONFIG_رقم_COOP_PRIORITIES >= CONFIG_رقم_METAIRQ_PRIORITIES,
	     "You need to provide at least as many CONFIG_NUM_COOP_PRIORITIES as "
	     "CONFIG_NUM_METAIRQ_PRIORITIES as Meta IRQs are just a special class of cooperative "
	     "threads.");

/**
 * @brief Update the per-CPU metairq preemption record.
 *
 * Must be called (with the scheduler spinlock held) whenever a new thread is
 * selected to run.  Records which cooperative thread a metairq preempted so
 * that next_up() can return to it once the metairq finishes.
 *
 * @param thread The thread about to become _current.
 */
ایستا درونخطی تهی update_metairq_preempt(ساختار k_thread *thread)
{
#اگر (CONFIG_رقم_METAIRQ_PRIORITIES > 0)
	اگر (thread_is_metairq(thread) && !thread_is_metairq(_current) &&
	    !thread_is_preemptible(_current)) {
		/* Record new preemption */
		_current_cpu->metairq_preempted = _current;
	} وگرنه اگر (!thread_is_metairq(thread)) {
		/* Returning from existing preemption */
		_current_cpu->metairq_preempted = پوچ;
	}
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
}

/**
 * @brief Recover a preempted cooperative thread after a metairq completes.
 *
 * After the run-queue winner (@p thread) has been chosen, check whether a
 * cooperative thread was preempted by a metairq and should be resumed instead.
 * Returns the adjusted thread pointer (unchanged when metairq logic is
 * disabled or no recovery is needed).
 *
 * @param thread Best candidate chosen from the run queue.
 * @return Thread to run next.
 */
ایستا درونخطی ساختار k_thread *metairq_preempt_recover(ساختار k_thread *thread)
{
#اگر (CONFIG_رقم_METAIRQ_PRIORITIES > 0)
	/* MetaIRQs must always attempt to return back to a cooperative thread
	 * they preempted and not whatever happens to be highest priority now.
	 * The cooperative thread was promised it wouldn't be preempted (by
	 * non-metairq threads)!
	 */
	ساختار k_thread *mirqp = _current_cpu->metairq_preempted;

	اگر (mirqp != پوچ && (thread == پوچ || !thread_is_metairq(thread))) {
		اگر (z_is_thread_ready(mirqp)) {
			بازگشت mirqp;
		}
		_current_cpu->metairq_preempted = پوچ;
	}
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
	بازگشت thread;
}

/**
 * @brief Return true if _current may be re-queued after being context-switched.
 *
 * When a metairq thread preempts a cooperative thread, the preempted thread
 * must not be put back on the run queue until the metairq completes;
 * otherwise a lower-priority thread might steal it.
 *
 * @return true if it is safe to re-queue _current, false otherwise.
 */
ایستا درونخطی bool metairq_current_requeue_allowed(تهی)
{
#اگر (CONFIG_رقم_METAIRQ_PRIORITIES > 0)
	بازگشت _current != _current_cpu->metairq_preempted;
#وگرنه
	بازگشت true;
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
}

/**
 * @brief Clear the metairq preemption record for a thread being halted.
 *
 * If @p thread is the cooperative thread recorded as having been preempted by
 * a metairq, clear that record so that the stale pointer is not used after the
 * thread is suspended or aborted.
 *
 * @param thread Thread that is being halted.
 */
ایستا درونخطی تهی z_metairq_preempted_clear(ساختار k_thread *thread)
{
#اگر (CONFIG_رقم_METAIRQ_PRIORITIES > 0)
	unsigned صحیح cpu_id = 0;

#اگر defined(CONFIG_SMP) && (CONFIG_MP_MAX_رقم_CPUS > 1)
	cpu_id = thread->base.cpu;
#endif
	اگر (_kernel.cpus[cpu_id].metairq_preempted == thread) {
		_kernel.cpus[cpu_id].metairq_preempted = پوچ;
	}
#وگرنه
	ARG_UNUSED(thread);
#endif /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
}

#endif /* ZEPHYR_KERNEL_INCLUDE_METAIRQ_H_ */
