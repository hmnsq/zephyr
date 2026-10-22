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

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_METAIRQ_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_METAIRQ_H_

#समावेश <zephyr/kernel.h>
#समावेश <kthread.h>

BUILD_ASSERT(CONFIG_अंक_COOP_PRIORITIES >= CONFIG_अंक_METAIRQ_PRIORITIES,
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
अटल अंतरभूत व्योम update_metairq_preempt(काष्ठा k_thread *thread)
{
#अगर (CONFIG_अंक_METAIRQ_PRIORITIES > 0)
	अगर (thread_is_metairq(thread) && !thread_is_metairq(_current) &&
	    !thread_is_preemptible(_current)) {
		/* Record new preemption */
		_current_cpu->metairq_preempted = _current;
	} अन्यथा अगर (!thread_is_metairq(thread)) {
		/* Returning from existing preemption */
		_current_cpu->metairq_preempted = शून्य;
	}
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
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
अटल अंतरभूत काष्ठा k_thread *metairq_preempt_recover(काष्ठा k_thread *thread)
{
#अगर (CONFIG_अंक_METAIRQ_PRIORITIES > 0)
	/* MetaIRQs must always attempt to return back to a cooperative thread
	 * they preempted and not whatever happens to be highest priority now.
	 * The cooperative thread was promised it wouldn't be preempted (by
	 * non-metairq threads)!
	 */
	काष्ठा k_thread *mirqp = _current_cpu->metairq_preempted;

	अगर (mirqp != शून्य && (thread == शून्य || !thread_is_metairq(thread))) {
		अगर (z_is_thread_ready(mirqp)) {
			वापस mirqp;
		}
		_current_cpu->metairq_preempted = शून्य;
	}
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
	वापस thread;
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
अटल अंतरभूत bool metairq_current_requeue_allowed(व्योम)
{
#अगर (CONFIG_अंक_METAIRQ_PRIORITIES > 0)
	वापस _current != _current_cpu->metairq_preempted;
#अन्यथा
	वापस true;
#पूर्ण_अगर /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
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
अटल अंतरभूत व्योम z_metairq_preempted_clear(काष्ठा k_thread *thread)
{
#अगर (CONFIG_अंक_METAIRQ_PRIORITIES > 0)
	अचिन्हित पूर्णांक cpu_id = 0;

#अगर defined(CONFIG_SMP) && (CONFIG_MP_MAX_अंक_CPUS > 1)
	cpu_id = thread->base.cpu;
#पूर्ण_अगर
	अगर (_kernel.cpus[cpu_id].metairq_preempted == thread) {
		_kernel.cpus[cpu_id].metairq_preempted = शून्य;
	}
#अन्यथा
	ARG_UNUSED(thread);
#पूर्ण_अगर /* CONFIG_NUM_METAIRQ_PRIORITIES > 0 */
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_METAIRQ_H_ */
