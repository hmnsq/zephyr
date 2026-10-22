/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#اگر_تعریف_نشده ZEPHYR_KERNEL_INCLUDE_پیاپی_Q_H
#تعریف_کن ZEPHYR_KERNEL_INCLUDE_پیاپی_Q_H

#ضمن <zephyr/kernel.h>
#ضمن <ksched.h>
#ضمن <zephyr/spinlock.h>
#ضمن <wait_q.h>
#ضمن <kthread.h>
#ضمن <priority_q.h>
#ضمن <ipi.h>


ساكن ALWAYS_INLINE فراغ *thread_runq(بنية k_thread *thread)
{
#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک_PIN_ONLY
	uint32_t cpu_ماسک = thread->پایه.cpu_ماسک;

	__ASSERT(cpu_ماسک != 0U, "thread queued with empty CPU mask");

	بیعلامت صحيح cpu = (بیعلامت صحيح)u32_شمار_trailing_zeros(cpu_ماسک);

	__ASSERT(cpu < ARRAY_اندازه(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_اندازه(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (بیعلامت صحيح)(ARRAY_اندازه(_kernel.cpus) - 1U));

	أرجع &_kernel.cpus[cpu].ready_q.runq;
#وإلا
	ARG_استفادهنشده(thread);
	أرجع &_kernel.ready_q.runq;
#پایان_اگر /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساكن ALWAYS_INLINE فراغ *curr_cpu_runq(فراغ)
{
#اگر_تعریف_شده CONFIG_SCHED_CPU_ماسک_PIN_ONLY
	أرجع &arch_curr_cpu()->ready_q.runq;
#وإلا
	أرجع &_kernel.ready_q.runq;
#پایان_اگر /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساكن ALWAYS_INLINE فراغ runq_جمع_کن(بنية k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_پیاپی_جمع_کن(thread_runq(thread), thread);
}

ساكن ALWAYS_INLINE فراغ runq_remove(بنية k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_پیاپی_remove(thread_runq(thread), thread);
}

ساكن ALWAYS_INLINE فراغ runq_yield(فراغ)
{
	priq_پیاپی_yield(curr_cpu_runq());
}

ساكن ALWAYS_INLINE بنية k_thread *runq_بهترین(فراغ)
{
	أرجع priq_پیاپی_بهترین(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ساكن مضمن bool should_صف_thread(بنية k_thread *thread)
{
	أرجع !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ساكن ALWAYS_INLINE فراغ صف_thread(بنية k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	إذا (should_صف_thread(thread)) {
		runq_جمع_کن(thread);
	}
#اگر_تعریف_شده CONFIG_SMP
	إذا (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->جابجا_ok = true;
	}
#پایان_اگر /* CONFIG_SMP */
}

ساكن ALWAYS_INLINE فراغ خارج_از_صف_thread(بنية k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	إذا (should_صف_thread(thread)) {
		runq_remove(thread);
	}
}

#پایان_اگر /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
