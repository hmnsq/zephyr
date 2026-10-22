/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#અન્તર્ભાવ <zephyr/kernel.h>
#અન્તર્ભાવ <ksched.h>
#અન્તર્ભાવ <zephyr/spinlock.h>
#અન્તર્ભાવ <wait_q.h>
#અન્તર્ભાવ <kthread.h>
#અન્તર્ભાવ <priority_q.h>
#અન્તર્ભાવ <ipi.h>


સ્થિર ALWAYS_INLINE શૂન્ય *thread_runq(સંરચના k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned પૂર્ણાંક cpu = (unsigned પૂર્ણાંક)u32_ગણતરી_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned પૂર્ણાંક)(ARRAY_SIZE(_kernel.cpus) - 1U));

	પાછા &_kernel.cpus[cpu].ready_q.runq;
#નહીંતર
	ARG_UNUSED(thread);
	પાછા &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

સ્થિર ALWAYS_INLINE શૂન્ય *curr_cpu_runq(શૂન્ય)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	પાછા &arch_curr_cpu()->ready_q.runq;
#નહીંતર
	પાછા &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

સ્થિર ALWAYS_INLINE શૂન્ય runq_add(સંરચના k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

સ્થિર ALWAYS_INLINE શૂન્ય runq_remove(સંરચના k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

સ્થિર ALWAYS_INLINE શૂન્ય runq_yield(શૂન્ય)
{
	_priq_run_yield(curr_cpu_runq());
}

સ્થિર ALWAYS_INLINE સંરચના k_thread *runq_best(શૂન્ય)
{
	પાછા _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
સ્થિર અંતઃરેખ bool should_queue_thread(સંરચના k_thread *thread)
{
	પાછા !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

સ્થિર ALWAYS_INLINE શૂન્ય queue_thread(સંરચના k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	જો (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	જો (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

સ્થિર ALWAYS_INLINE શૂન્ય dequeue_thread(સંરચના k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	જો (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
