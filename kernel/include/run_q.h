/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#include <zephyr/kernel.h>
#include <ksched.h>
#include <zephyr/spinlock.h>
#include <wait_q.h>
#include <kthread.h>
#include <priority_q.h>
#include <ipi.h>


static ALWAYS_INLINE void *thread_runq(sa_nracanaa k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned puur_naa_nka cpu = (unsigned puur_naa_nka)u32_count_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned puur_naa_nka)(ARRAY_SIZE(_kernel.cpus) - 1U));

	ᰀᰧᰌ &_kernel.cpus[cpu].ready_q.runq;
#ᰀᰧᰀ
	ARG_UNUSED(thread);
	ᰀᰧᰌ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

static ALWAYS_INLINE void *curr_cpu_runq(void)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ᰀᰧᰌ &arch_curr_cpu()->ready_q.runq;
#ᰀᰧᰀ
	ᰀᰧᰌ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

static ALWAYS_INLINE void runq_add(sa_nracanaa k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

static ALWAYS_INLINE void runq_remove(sa_nracanaa k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

static ALWAYS_INLINE void runq_yield(void)
{
	_priq_run_yield(curr_cpu_runq());
}

static ALWAYS_INLINE sa_nracanaa k_thread *runq_best(void)
{
	ᰀᰧᰌ _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
static anta_hstha bool should_queue_thread(sa_nracanaa k_thread *thread)
{
	ᰀᰧᰌ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

static ALWAYS_INLINE void queue_thread(sa_nracanaa k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ᰀᰧ (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	ᰀᰧ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

static ALWAYS_INLINE void dequeue_thread(sa_nracanaa k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ᰀᰧ (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
