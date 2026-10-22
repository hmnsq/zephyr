/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_لاڳيتو_Q_H
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_لاڳيتو_Q_H

#ضمن <zephyr/kernel.h>
#ضمن <ksched.h>
#ضمن <zephyr/spinlock.h>
#ضمن <wait_q.h>
#ضمن <kthread.h>
#ضمن <priority_q.h>
#ضمن <ipi.h>


ساكن ALWAYS_INLINE فراغ *thread_runq(بنية k_thread *thread)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	uint32_t cpu_نقاب = thread->بنياد.cpu_نقاب;

	__ASSERT(cpu_نقاب != 0U, "thread queued with empty CPU mask");

	بنانشان صحيح cpu = (بنانشان صحيح)u32_ڳڻپ_trailing_zeros(cpu_نقاب);

	__ASSERT(cpu < ARRAY_سائيز(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_سائيز(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (بنانشان صحيح)(ARRAY_سائيز(_kernel.cpus) - 1U));

	أرجع &_kernel.cpus[cpu].ready_q.runq;
#وإلا
	ARG_اڻ_استعمال_ٿيل(thread);
	أرجع &_kernel.ready_q.runq;
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساكن ALWAYS_INLINE فراغ *curr_cpu_runq(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_CPU_نقاب_PIN_ONLY
	أرجع &arch_curr_cpu()->ready_q.runq;
#وإلا
	أرجع &_kernel.ready_q.runq;
#جيڪڏهن_پورو /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ساكن ALWAYS_INLINE فراغ runq_جوڙيو(بنية k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_لاڳيتو_جوڙيو(thread_runq(thread), thread);
}

ساكن ALWAYS_INLINE فراغ runq_remove(بنية k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_لاڳيتو_remove(thread_runq(thread), thread);
}

ساكن ALWAYS_INLINE فراغ runq_yield(فراغ)
{
	priq_لاڳيتو_yield(curr_cpu_runq());
}

ساكن ALWAYS_INLINE بنية k_thread *runq_بهترين(فراغ)
{
	أرجع priq_لاڳيتو_بهترين(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ساكن مضمن bool should_قطار_thread(بنية k_thread *thread)
{
	أرجع !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ساكن ALWAYS_INLINE فراغ قطار_thread(بنية k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	إذا (should_قطار_thread(thread)) {
		runq_جوڙيو(thread);
	}
#جيڪڏهن_وصف_ٿيل CONFIG_SMP
	إذا (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->مٽا_سٽا_ok = true;
	}
#جيڪڏهن_پورو /* CONFIG_SMP */
}

ساكن ALWAYS_INLINE فراغ قطار_مان_ڪڍو_thread(بنية k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	إذا (should_قطار_thread(thread)) {
		runq_remove(thread);
	}
}

#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
