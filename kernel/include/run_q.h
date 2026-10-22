/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#ضمن <zephyr/kernel.h>
#ضمن <ksched.h>
#ضمن <zephyr/spinlock.h>
#ضمن <wait_q.h>
#ضمن <kthread.h>
#ضمن <priority_q.h>
#ضمن <ipi.h>


ساكن ALWAYS_INLINE فراغ *thread_runq(بنية k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned صحيح cpu = (unsigned صحيح)u32_count_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned صحيح)(ARRAY_SIZE(_kernel.cpus) - 1U));

	ܦܢܐ &_kernel.cpus[cpu].ready_q.runq;
#ܐܚܪܢܐ
	ARG_UNUSED(thread);
	ܦܢܐ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساكن ALWAYS_INLINE فراغ *curr_cpu_runq(فراغ)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ܦܢܐ &arch_curr_cpu()->ready_q.runq;
#ܐܚܪܢܐ
	ܦܢܐ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساكن ALWAYS_INLINE فراغ runq_add(بنية k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

ساكن ALWAYS_INLINE فراغ runq_remove(بنية k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

ساكن ALWAYS_INLINE فراغ runq_yield(فراغ)
{
	_priq_run_yield(curr_cpu_runq());
}

ساكن ALWAYS_INLINE بنية k_thread *runq_best(فراغ)
{
	ܦܢܐ _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ساكن مضمن bool should_queue_thread(بنية k_thread *thread)
{
	ܦܢܐ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ساكن ALWAYS_INLINE فراغ queue_thread(بنية k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ܐܢ (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	ܐܢ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

ساكن ALWAYS_INLINE فراغ dequeue_thread(بنية k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ܐܢ (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
