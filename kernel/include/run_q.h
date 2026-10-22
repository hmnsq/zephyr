/*
 * Copyright (c) Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_ಸತತ_Q_H
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_ಸತತ_Q_H

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <ksched.h>
#ಅನ್ತರ್ಭಾವ <zephyr/spinlock.h>
#ಅನ್ತರ್ಭಾವ <wait_q.h>
#ಅನ್ತರ್ಭಾವ <kthread.h>
#ಅನ್ತರ್ಭಾವ <priority_q.h>
#ಅನ್ತರ್ಭಾವ <ipi.h>


ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ *thread_runq(ರಚನೆ k_thread *thread)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY
	uint32_t cpu_ಮುಖವಾಡ = thread->ತಳ.cpu_ಮುಖವಾಡ;

	__ASSERT(cpu_ಮುಖವಾಡ != 0U, "thread queued with empty CPU mask");

	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ cpu = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)u32_ಎಣಿಕೆ_trailing_zeros(cpu_ಮುಖವಾಡ);

	__ASSERT(cpu < ARRAY_ಗಾತ್ರ(_kernel.cpus),
		 "cpu_mask bit %u exceeds cpu count %zu",
		 cpu, ARRAY_ಗಾತ್ರ(_kernel.cpus));

	/* Clamp so static-analysis tools that cannot see the assert
	 * above do not flag the array access as out-of-bounds.
	 */
	cpu = MIN(cpu, (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)(ARRAY_ಗಾತ್ರ(_kernel.cpus) - 1U));

	ಹಿಂತಿರುಗಿ &_kernel.cpus[cpu].ready_q.runq;
#ಇಲ್ಲದಿದ್ದರೆ
	ARG_ಬಳಸದ(thread);
	ಹಿಂತಿರುಗಿ &_kernel.ready_q.runq;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ *curr_cpu_runq(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_CPU_ಮುಖವಾಡ_PIN_ONLY
	ಹಿಂತಿರುಗಿ &arch_curr_cpu()->ready_q.runq;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ &_kernel.ready_q.runq;
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_CPU_MASK_PIN_ONLY */
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ runq_ಕೂಡಿಸು(ರಚನೆ k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_ಸತತ_ಕೂಡಿಸು(thread_runq(thread), thread);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ runq_remove(ರಚನೆ k_thread *thread)
{
	__ASSERT_NO_MSG(!z_is_idle_thread_object(thread));
	__ASSERT_NO_MSG(!is_thread_dummy(thread));

	priq_ಸತತ_remove(thread_runq(thread), thread);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ runq_yield(ಶೂನ್ಯ)
{
	priq_ಸತತ_yield(curr_cpu_runq());
}

ಸ್ಥಿರ ALWAYS_INLINE ರಚನೆ k_thread *runq_ಉತ್ತಮ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ priq_ಸತತ_ಉತ್ತಮ(curr_cpu_runq());
}

/*
 * _current is never in the run queue until context switch on
 * SMP configurations.
 */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool should_ಸರತಿ_thread(ರಚನೆ k_thread *thread)
{
	ಹಿಂತಿರುಗಿ !IS_ENABLED(CONFIG_SMP) || (thread != _current);
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ ಸರತಿ_thread(ರಚನೆ k_thread *thread)
{
	z_mark_thread_as_queued(thread);
	ಆದರೆ (should_ಸರತಿ_thread(thread)) {
		runq_ಕೂಡಿಸು(thread);
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SMP
	ಆದರೆ (thread == _current) {
		/* add current to end of queue means "yield" */
		_current_cpu->ಅದಲುಬದಲು_ok = true;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SMP */
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ ಸರತಿಯಿಂದ_ತೆಗೆ_thread(ರಚನೆ k_thread *thread)
{
	z_mark_thread_as_not_queued(thread);
	ಆದರೆ (should_ಸರತಿ_thread(thread)) {
		runq_remove(thread);
	}
}

#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_RUN_Q_H_ */
