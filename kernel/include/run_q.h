/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_ਲੜੀਵਾਰ_Q_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_ਲੜੀਵਾਰ_Q_H

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ksched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/spinlock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wait_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kthread.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <priority_q.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ipi.h>


ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ *thread_runq(ਬਣਤਰ k_thread *thread)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY
	uint32_t cpu_ਨਕਾਬ = thread->ਅਧਾਰ.cpu_ਨਕਾਬ;

	__ASSERT(cpu_ਨਕਾਬ != 0U, "thread queued with empty CPU mask");

	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ cpu = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)u32_ਗਿਣਤੀ_trailing_zeros(cpu_ਨਕਾਬ);

	__ASSERT(cpu < ARRAY_ਮਾਪ(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_ਮਾਪ(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(ARRAY_ਮਾਪ(_kernel.cpus) - 1U));

	ਵਾਪਸ &_kernel.cpus[cpu].ready_q.runq;
#ਨਹੀਂਤਾਂ
	ARG_ਅਣਵਰਤਿਆ(thread);
	ਵਾਪਸ &_kernel.ready_q.runq;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ *curr_cpu_runq(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_CPU_ਨਕਾਬ_PIN_ONLY
	ਵਾਪਸ &arch_curr_cpu()->ready_q.runq;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ &_kernel.ready_q.runq;
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ runq_ਜੋੜੋ(ਬਣਤਰ k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_ਲੜੀਵਾਰ_ਜੋੜੋ(thread_runq(thread), thread);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ runq_remove(ਬਣਤਰ k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_ਲੜੀਵਾਰ_remove(thread_runq(thread), thread);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ runq_yield(ਖਾਲੀ)
{
	priq_ਲੜੀਵਾਰ_yield(curr_cpu_runq());
}

ਸਥਿਰ ALWAYS_INLINE ਬਣਤਰ k_thread *runq_ਉੱਤਮ(ਖਾਲੀ)
{
	ਵਾਪਸ priq_ਲੜੀਵਾਰ_ਉੱਤਮ(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ਸਥਿਰ ਸਤਰਅੰਦਰ bool should_ਕਤਾਰ_thread(ਬਣਤਰ k_thread *thread)
{
	ਵਾਪਸ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ ਕਤਾਰ_thread(ਬਣਤਰ k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ਜੇ (should_ਕਤਾਰ_thread(thread)) {
		runq_ਜੋੜੋ(thread);
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SMP
	ਜੇ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->ਅਦਲਾ_ਬਦਲੀ_ok = true;
	}
#ਜੇ_ਅੰਤ /* CONFIG_SMP */
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ ਕਤਾਰ_ਤੋਂ_ਕੱਢੋ_thread(ਬਣਤਰ k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ਜੇ (should_ਕਤਾਰ_thread(thread)) {
		runq_remove(thread);
	}
}

#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
