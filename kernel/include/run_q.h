/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_सततम्_Q_H
#परिभाषय ZEPHYR_KERNEL_INCLUDE_सततम्_Q_H

#अन्तर्भाव <zephyr/kernel.h>
#अन्तर्भाव <ksched.h>
#अन्तर्भाव <zephyr/spinlock.h>
#अन्तर्भाव <wait_q.h>
#अन्तर्भाव <kthread.h>
#अन्तर्भाव <priority_q.h>
#अन्तर्भाव <ipi.h>


स्थिर ALWAYS_INLINE शून्य *thread_runq(संरचना k_thread *thread)
{
#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY
	uint32_t cpu_आवरणम् = thread->आधारः.cpu_आवरणम्;

	__ASSERT(cpu_आवरणम् != 0U, "thread queued with empty CPU mask");

	अचिह्नित पूर्णाङ्क cpu = (अचिह्नित पूर्णाङ्क)u32_गणनम्_trailing_zeros(cpu_आवरणम्);

	__ASSERT(cpu < ARRAY_आकार(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_आकार(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (अचिह्नित पूर्णाङ्क)(ARRAY_आकार(_kernel.cpus) - 1U));

	प्रत्यागम &_kernel.cpus[cpu].ready_q.runq;
#अन्यथा
	ARG_अप्रयुक्तम्(thread);
	प्रत्यागम &_kernel.ready_q.runq;
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

स्थिर ALWAYS_INLINE शून्य *curr_cpu_runq(शून्य)
{
#यदि_परिभाषितम् CONFIG_SCHED_CPU_आवरणम्_PIN_ONLY
	प्रत्यागम &arch_curr_cpu()->ready_q.runq;
#अन्यथा
	प्रत्यागम &_kernel.ready_q.runq;
#यदि_अन्तः /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

स्थिर ALWAYS_INLINE शून्य runq_सङ्कलय(संरचना k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_सततम्_सङ्कलय(thread_runq(thread), thread);
}

स्थिर ALWAYS_INLINE शून्य runq_remove(संरचना k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_सततम्_remove(thread_runq(thread), thread);
}

स्थिर ALWAYS_INLINE शून्य runq_yield(शून्य)
{
	priq_सततम्_yield(curr_cpu_runq());
}

स्थिर ALWAYS_INLINE संरचना k_thread *runq_best(शून्य)
{
	प्रत्यागम priq_सततम्_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
स्थिर अन्तःस्थ bool should_queue_thread(संरचना k_thread *thread)
{
	प्रत्यागम !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

स्थिर ALWAYS_INLINE शून्य queue_thread(संरचना k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	यदि (should_queue_thread(thread)) {
		runq_सङ्कलय(thread);
	}
#यदि_परिभाषितम् CONFIG_SMP
	यदि (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#यदि_अन्तः /* CONFIG_SMP */
}

स्थिर ALWAYS_INLINE शून्य पङ्क्तेः_निष्कासय_thread(संरचना k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	यदि (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
