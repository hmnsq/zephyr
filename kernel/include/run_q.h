/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <ksched.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/spinlock.h>
#ଅନ୍ତର୍ଭାଵ <wait_q.h>
#ଅନ୍ତର୍ଭାଵ <kthread.h>
#ଅନ୍ତର୍ଭାଵ <priority_q.h>
#ଅନ୍ତର୍ଭାଵ <ipi.h>


ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ *thread_runq(ସଂରଚନା k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)u32_ଗଣନା_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(ARRAY_SIZE(_kernel.cpus) - 1U));

	ଫେରନ୍ତୁ &_kernel.cpus[cpu].ready_q.runq;
#ଅନ୍ୟଥା
	ARG_UNUSED(thread);
	ଫେରନ୍ତୁ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ *curr_cpu_runq(ଶୂନ୍ୟ)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	ଫେରନ୍ତୁ &arch_curr_cpu()->ready_q.runq;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ runq_add(ସଂରଚନା k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ runq_remove(ସଂରଚନା k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ runq_yield(ଶୂନ୍ୟ)
{
	_priq_run_yield(curr_cpu_runq());
}

ସ୍ଥିର ALWAYS_INLINE ସଂରଚନା k_thread *runq_best(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool should_queue_thread(ସଂରଚନା k_thread *thread)
{
	ଫେରନ୍ତୁ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ queue_thread(ସଂରଚନା k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ଯଦି (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	ଯଦି (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ dequeue_thread(ସଂରଚନା k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ଯଦି (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
