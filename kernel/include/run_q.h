/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_تسلسل_Q_H
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_تسلسل_Q_H

#شامل <zephyr/kernel.h>
#شامل <ksched.h>
#شامل <zephyr/spinlock.h>
#شامل <wait_q.h>
#شامل <kthread.h>
#شامل <priority_q.h>
#شامل <ipi.h>


ساکن ALWAYS_INLINE خالی *thread_runq(ساخت k_thread *thread)
{
#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	uint32_t cpu_نقاب = thread->بنیاد.cpu_نقاب;

	__ASSERT(cpu_نقاب != 0U, "thread queued with empty CPU mask");

	غیرعلامتی صحیح cpu = (غیرعلامتی صحیح)u32_گنتی_trailing_zeros(cpu_نقاب);

	__ASSERT(cpu < ARRAY_سائز(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_سائز(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (غیرعلامتی صحیح)(ARRAY_سائز(_kernel.cpus) - 1U));

	واپس &_kernel.cpus[cpu].ready_q.runq;
#ورنہ
	ARG_غیر_مستعمل(thread);
	واپس &_kernel.ready_q.runq;
#اگر_ختم /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساکن ALWAYS_INLINE خالی *curr_cpu_runq(خالی)
{
#اگر_تعریف_شدہ CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	واپس &arch_curr_cpu()->ready_q.runq;
#ورنہ
	واپس &_kernel.ready_q.runq;
#اگر_ختم /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساکن ALWAYS_INLINE خالی runq_جوڑو(ساخت k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_تسلسل_جوڑو(thread_runq(thread), thread);
}

ساکن ALWAYS_INLINE خالی runq_remove(ساخت k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_تسلسل_remove(thread_runq(thread), thread);
}

ساکن ALWAYS_INLINE خالی runq_yield(خالی)
{
	priq_تسلسل_yield(curr_cpu_runq());
}

ساکن ALWAYS_INLINE ساخت k_thread *runq_بہترین(خالی)
{
	واپس priq_تسلسل_بہترین(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ساکن اندرونی bool should_قطار_thread(ساخت k_thread *thread)
{
	واپس !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ساکن ALWAYS_INLINE خالی قطار_thread(ساخت k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	اگر (should_قطار_thread(thread)) {
		runq_جوڑو(thread);
	}
#اگر_تعریف_شدہ CONFIG_SMP
	اگر (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->تبادلہ_ok = true;
	}
#اگر_ختم /* CONFIG_SMP */
}

ساکن ALWAYS_INLINE خالی قطار_سے_نکالو_thread(ساخت k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	اگر (should_قطار_thread(thread)) {
		runq_remove(thread);
	}
}

#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
