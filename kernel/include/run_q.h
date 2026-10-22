/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_

#समावेश <zephyr/kernel.h>
#समावेश <ksched.h>
#समावेश <zephyr/spinlock.h>
#समावेश <wait_q.h>
#समावेश <kthread.h>
#समावेश <priority_q.h>
#समावेश <ipi.h>


अटल ALWAYS_INLINE व्योम *thread_runq(काष्ठा k_thread *thread)
{
#अगर_घोषित CONFIG_SCHED_CPU_MASK_PIN_ONLY
	uint32_t cpu_mask = thread->base.cpu_mask;

	__ASSERT(cpu_mask != 0U, "thread queued with empty CPU mask");

	अचिन्हित पूर्णांक cpu = (अचिन्हित पूर्णांक)u32_गिनती_trailing_zeros(cpu_mask);

	__ASSERT(cpu < ARRAY_आकार(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_आकार(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (अचिन्हित पूर्णांक)(ARRAY_आकार(_kernel.cpus) - 1U));

	वापस &_kernel.cpus[cpu].ready_q.runq;
#अन्यथा
	ARG_UNUSED(thread);
	वापस &_kernel.ready_q.runq;
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

अटल ALWAYS_INLINE व्योम *curr_cpu_runq(व्योम)
{
#अगर_घोषित CONFIG_SCHED_CPU_MASK_PIN_ONLY
	वापस &arch_curr_cpu()->ready_q.runq;
#अन्यथा
	वापस &_kernel.ready_q.runq;
#पूर्ण_अगर /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

अटल ALWAYS_INLINE व्योम runq_add(काष्ठा k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_add(thread_runq(thread), thread);
}

अटल ALWAYS_INLINE व्योम runq_remove(काष्ठा k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	_priq_run_remove(thread_runq(thread), thread);
}

अटल ALWAYS_INLINE व्योम runq_yield(व्योम)
{
	_priq_run_yield(curr_cpu_runq());
}

अटल ALWAYS_INLINE काष्ठा k_thread *runq_best(व्योम)
{
	वापस _priq_run_best(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
अटल अंतरभूत bool should_queue_thread(काष्ठा k_thread *thread)
{
	वापस !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

अटल ALWAYS_INLINE व्योम queue_thread(काष्ठा k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	अगर (should_queue_thread(thread)) {
		runq_add(thread);
	}
#अगर_घोषित CONFIG_SMP
	अगर (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->swap_ok = true;
	}
#पूर्ण_अगर /* CONFIG_SMP */
}

अटल ALWAYS_INLINE व्योम dequeue_thread(काष्ठा k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	अगर (should_queue_thread(thread)) {
		runq_remove(thread);
	}
}

#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
