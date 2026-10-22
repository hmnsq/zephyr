/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_నిరంతరం_Q_H
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_నిరంతరం_Q_H

#అన్తర్భావ <zephyr/kernel.h>
#అన్తర్భావ <ksched.h>
#అన్తర్భావ <zephyr/spinlock.h>
#అన్తర్భావ <wait_q.h>
#అన్తర్భావ <kthread.h>
#అన్తర్భావ <priority_q.h>
#అన్తర్భావ <ipi.h>


స్థిర ALWAYS_INLINE శూన్య *thread_runq(నిర్మాణం k_thread *thread)
{
#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY
	uint32_t cpu_ముసుగు = thread->పునాది.cpu_ముసుగు;

	__ASSERT(cpu_ముసుగు != 0U, "thread queued with empty CPU mask");

	గుర్తులేని పూర్ణాంకం cpu = (గుర్తులేని పూర్ణాంకం)u32_లెక్క_trailing_zeros(cpu_ముసుగు);

	__ASSERT(cpu < ARRAY_కొలత(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_కొలత(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (గుర్తులేని పూర్ణాంకం)(ARRAY_కొలత(_kernel.cpus) - 1U));

	తిరిగి &_kernel.cpus[cpu].ready_q.runq;
#లేకపోతే
	ARG_వాడని(thread);
	తిరిగి &_kernel.ready_q.runq;
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

స్థిర ALWAYS_INLINE శూన్య *curr_cpu_runq(శూన్య)
{
#నిర్వచితమైతే CONFIG_SCHED_CPU_ముసుగు_PIN_ONLY
	తిరిగి &arch_curr_cpu()->ready_q.runq;
#లేకపోతే
	తిరిగి &_kernel.ready_q.runq;
#అయితే_ముగింపు /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

స్థిర ALWAYS_INLINE శూన్య runq_కూడు(నిర్మాణం k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_నిరంతరం_కూడు(thread_runq(thread), thread);
}

స్థిర ALWAYS_INLINE శూన్య runq_remove(నిర్మాణం k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_నిరంతరం_remove(thread_runq(thread), thread);
}

స్థిర ALWAYS_INLINE శూన్య runq_yield(శూన్య)
{
	priq_నిరంతరం_yield(curr_cpu_runq());
}

స్థిర ALWAYS_INLINE నిర్మాణం k_thread *runq_ఉత్తమ(శూన్య)
{
	తిరిగి priq_నిరంతరం_ఉత్తమ(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
స్థిర అంతఃవరుస bool should_వరుస_thread(నిర్మాణం k_thread *thread)
{
	తిరిగి !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

స్థిర ALWAYS_INLINE శూన్య వరుస_thread(నిర్మాణం k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ఒకవేళ (should_వరుస_thread(thread)) {
		runq_కూడు(thread);
	}
#నిర్వచితమైతే CONFIG_SMP
	ఒకవేళ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->మార్పిడి_ok = true;
	}
#అయితే_ముగింపు /* CONFIG_SMP */
}

స్థిర ALWAYS_INLINE శూన్య వరుస_నుండి_తీయి_thread(నిర్మాణం k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ఒకవేళ (should_వరుస_thread(thread)) {
		runq_remove(thread);
	}
}

#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
