/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <ksched.h>
#അന്തര്ഭാവ <zephyr/spinlock.h>
#അന്തര്ഭാവ <wait_q.h>
#അന്തര്ഭാവ <kthread.h>
#അന്തര്ഭാവ <priority_q.h>
#അന്തര്ഭാവ <ipi.h>


സ്ഥിതം ALWAYS_INLINE ശൂന്യം *thread_runq(ഘടന k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned പൂർണ്ണസംഖ്യ cpu = (unsigned പൂർണ്ണസംഖ്യ)u32_എണ്ണം_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned പൂർണ്ണസംഖ്യ)(ARRAY_SIZE(_kernel.cpus) - 1U));

	തിരികെ &_kernel.cpus[cpu].ready_q.runq;
#അല്ലെങ്കിൽ
	ARG_UNUSED(thread);
	തിരികെ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം *curr_cpu_runq(ശൂന്യം)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	തിരികെ &arch_curr_cpu()->ready_q.runq;
#അല്ലെങ്കിൽ
	തിരികെ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം runq_add(ഘടന k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം runq_remove(ഘടന k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം runq_yield(ശൂന്യം)
{
	_priq_run_yield(curr_cpu_runq());
}

സ്ഥിതം ALWAYS_INLINE ഘടന k_thread *runq_best(ശൂന്യം)
{
	തിരികെ _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
സ്ഥിതം വരിയിൽ bool should_queue_thread(ഘടന k_thread *thread)
{
	തിരികെ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം queue_thread(ഘടന k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	എങ്കിൽ (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	എങ്കിൽ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം dequeue_thread(ഘടന k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	എങ്കിൽ (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
