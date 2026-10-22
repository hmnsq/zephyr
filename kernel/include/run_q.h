/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#شامل <zephyr/kernel.h>
#شامل <ksched.h>
#شامل <zephyr/spinlock.h>
#شامل <wait_q.h>
#شامل <kthread.h>
#شامل <priority_q.h>
#شامل <ipi.h>


ایستا ALWAYS_INLINE تهی *thread_runq(ساختار k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned صحیح cpu = (unsigned صحیح)u32_شمار_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned صحیح)(ARRAY_SIZE(_kernel.cpus) - 1U));

	بازگشت &_kernel.cpus[cpu].ready_q.runq;
#وگرنه
	ARG_UNUSED(thread);
	بازگشت &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ایستا ALWAYS_INLINE تهی *curr_cpu_runq(تهی)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	بازگشت &arch_curr_cpu()->ready_q.runq;
#وگرنه
	بازگشت &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ایستا ALWAYS_INLINE تهی runq_add(ساختار k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

ایستا ALWAYS_INLINE تهی runq_remove(ساختار k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

ایستا ALWAYS_INLINE تهی runq_yield(تهی)
{
	_priq_run_yield(curr_cpu_runq());
}

ایستا ALWAYS_INLINE ساختار k_thread *runq_best(تهی)
{
	بازگشت _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ایستا درونخطی bool should_queue_thread(ساختار k_thread *thread)
{
	بازگشت !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ایستا ALWAYS_INLINE تهی queue_thread(ساختار k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	اگر (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	اگر (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

ایستا ALWAYS_INLINE تهی dequeue_thread(ساختار k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	اگر (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
