/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <priority_q.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>


ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ *thread_runq(ಸಂರಚನಾ k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned ಪೂರ್ಣಾಙ್ಕ cpu = (unsigned ಪೂರ್ಣಾಙ್ಕ)u32_ಎಣಿಕೆ_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned ಪೂರ್ಣಾಙ್ಕ)(ARRAY_SIZE(_kernel.cpus) - 1U));

	ಪ್ರತ್ಯಾಗಮ &_kernel.cpus[cpu].ready_q.runq;
#ಅನ್ಯಥಾ
	ARG_UNUSED(thread);
	ಪ್ರತ್ಯಾಗಮ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ *curr_cpu_runq(ಶೂನ್ಯ)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ಪ್ರತ್ಯಾಗಮ &arch_curr_cpu()->ready_q.runq;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ runq_add(ಸಂರಚನಾ k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ runq_remove(ಸಂರಚನಾ k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ runq_yield(ಶೂನ್ಯ)
{
	_priq_run_yield(curr_cpu_runq());
}

ಸ್ಥಿರ ALWAYS_INLINE ಸಂರಚನಾ k_thread *runq_best(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool should_queue_thread(ಸಂರಚನಾ k_thread *thread)
{
	ಪ್ರತ್ಯಾಗಮ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ queue_thread(ಸಂರಚನಾ k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ಯದಿ (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	ಯದಿ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ dequeue_thread(ಸಂರಚನಾ k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ಯದಿ (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
