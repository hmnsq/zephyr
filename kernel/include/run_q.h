/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#define ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <priority_q.h>
#अन्तर्भाव <ipi.h>


स्थिर ALWAYS_INLINE शून्य *thread_runq(संरचना k_thread *thread)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	unsigned पूर्णाङ्क cpu = (unsigned पूर्णाङ्क)u32_गणनम्_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_SIZE(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_SIZE(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (unsigned पूर्णाङ्क)(ARRAY_SIZE(_kernel.cpus) - 1U));

	पटिनिवत्त &_kernel.cpus[cpu].ready_q.runq;
#अञ्ञथा
	ARG_UNUSED(thread);
	पटिनिवत्त &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

स्थिर ALWAYS_INLINE शून्य *curr_cpu_runq(शून्य)
{
#ifdef CONFIG_SCHED_CPU_MASK_PIN_ONLY
	पटिनिवत्त &arch_curr_cpu()->ready_q.runq;
#अञ्ञथा
	पटिनिवत्त &_kernel.ready_q.runq;
#endif /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

स्थिर ALWAYS_INLINE शून्य runq_add(संरचना k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

स्थिर ALWAYS_INLINE शून्य runq_remove(संरचना k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

स्थिर ALWAYS_INLINE शून्य runq_yield(शून्य)
{
	_priq_run_yield(curr_cpu_runq());
}

स्थिर ALWAYS_INLINE संरचना k_thread *runq_best(शून्य)
{
	पटिनिवत्त _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
स्थिर अन्तःस्थ bool should_queue_thread(संरचना k_thread *thread)
{
	पटिनिवत्त !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

स्थिर ALWAYS_INLINE शून्य queue_thread(संरचना k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	सचे (should_queue_thread(thread)) {
		runq_add(thread);
	}
#ifdef CONFIG_SMP
	सचे (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#endif /* CONFIG_SMP */
}

स्थिर ALWAYS_INLINE शून्य dequeue_thread(संरचना k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	सचे (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#endif /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
