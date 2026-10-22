/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#அந்தர்பாவ <zephyr/kernel.h>
#அந்தர்பாவ <ksched.h>
#அந்தர்பாவ <zephyr/spinlock.h>
#அந்தர்பாவ <wait_q.h>
#அந்தர்பாவ <kthread.h>
#அந்தர்பாவ <priority_q.h>
#அந்தர்பாவ <ipi.h>


நிலையான ALWAYS_INLINE வெறுமை *thread_runq(கட்டமைப்பு k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned முழு cpu = (unsigned முழு)u32_மொத்தம்_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned முழு)(ARRAY_SIZE(_kernel.cpus) - 1U));

	திரும்பு &_kernel.cpus[cpu].ready_q.runq;
#இல்லையெனில்
	ARG_UNUSED(thread);
	திரும்பு &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

நிலையான ALWAYS_INLINE வெறுமை *curr_cpu_runq(வெறுமை)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	திரும்பு &arch_curr_cpu()->ready_q.runq;
#இல்லையெனில்
	திரும்பு &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

நிலையான ALWAYS_INLINE வெறுமை runq_add(கட்டமைப்பு k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

நிலையான ALWAYS_INLINE வெறுமை runq_remove(கட்டமைப்பு k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

நிலையான ALWAYS_INLINE வெறுமை runq_yield(வெறுமை)
{
	_priq_run_yield(curr_cpu_runq());
}

நிலையான ALWAYS_INLINE கட்டமைப்பு k_thread *runq_best(வெறுமை)
{
	திரும்பு _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
நிலையான உள்வரி bool should_queue_thread(கட்டமைப்பு k_thread *thread)
{
	திரும்பு !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

நிலையான ALWAYS_INLINE வெறுமை queue_thread(கட்டமைப்பு k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	எனில் (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	எனில் (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

நிலையான ALWAYS_INLINE வெறுமை dequeue_thread(கட்டமைப்பு k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	எனில் (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
