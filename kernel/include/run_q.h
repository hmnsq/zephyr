/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#অন্তর্ভাৱ <zephyr/kernel.h>
#অন্তর্ভাৱ <ksched.h>
#অন্তর্ভাৱ <zephyr/spinlock.h>
#অন্তর্ভাৱ <wait_q.h>
#অন্তর্ভাৱ <kthread.h>
#অন্তর্ভাৱ <priority_q.h>
#অন্তর্ভাৱ <ipi.h>


স্থির ALWAYS_INLINE শূন্য *thread_runq(সংরচনা k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned পূর্ণাঙ্ক cpu = (unsigned পূর্ণাঙ্ক)u32_গুনতি_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned পূর্ণাঙ্ক)(ARRAY_SIZE(_kernel.cpus) - 1U));

	ঘূৰাই &_kernel.cpus[cpu].ready_q.runq;
#অন্যথা
	ARG_UNUSED(thread);
	ঘূৰাই &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

স্থির ALWAYS_INLINE শূন্য *curr_cpu_runq(শূন্য)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ঘূৰাই &arch_curr_cpu()->ready_q.runq;
#অন্যথা
	ঘূৰাই &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

স্থির ALWAYS_INLINE শূন্য runq_add(সংরচনা k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

স্থির ALWAYS_INLINE শূন্য runq_remove(সংরচনা k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

স্থির ALWAYS_INLINE শূন্য runq_yield(শূন্য)
{
	_priq_run_yield(curr_cpu_runq());
}

স্থির ALWAYS_INLINE সংরচনা k_thread *runq_best(শূন্য)
{
	ঘূৰাই _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
স্থির অন্তঃস্থ bool should_queue_thread(সংরচনা k_thread *thread)
{
	ঘূৰাই !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

স্থির ALWAYS_INLINE শূন্য queue_thread(সংরচনা k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	যদি (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	যদি (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

স্থির ALWAYS_INLINE শূন্য dequeue_thread(সংরচনা k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	যদি (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
