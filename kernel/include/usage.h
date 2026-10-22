/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#ವ್ಯಾಖ್ಯಾನಿಸು ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ


/** @brief Halt thread cycle usage accounting.
 *
 * Halts the accumulation of thread cycle usage and adds the current
 * total to the thread's counter.  Called on context switch.
 *
 * Note that this function is idempotent.  The core kernel code calls
 * it at the end of interrupt handlers (because that is where we have
 * a portable hook) where we are context switching, which will include
 * any cycles spent in the ISR in the per-thread accounting.  But
 * architecture code can also call it earlier out of interrupt entry
 * to improve measurement fidelity.
 *
 * This function assumes local interrupts are masked (so that the
 * current CPU pointer and current thread are safe to modify), but
 * requires no other synchronization.  Architecture layers don't need
 * to do anything more.
 */
ಶೂನ್ಯ z_sched_usage_stop(ಶೂನ್ಯ);

ಶೂನ್ಯ z_sched_usage_ಆರಂಭ(ರಚನೆ k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
ಶೂನ್ಯ z_sched_cpu_usage(uint8_t core_id, ರಚನೆ k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
ಶೂನ್ಯ z_sched_thread_usage(ರಚನೆ k_thread *thread,
			  ರಚನೆ k_thread_runtime_stats *stats);

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ z_sched_usage_switch(ರಚನೆ k_thread *thread)
{
	ARG_ಬಳಸದ(thread);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_ಆರಂಭ(thread);
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SCHED_THREAD_USAGE */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ


#ಆದರೆ_ಅಂತ್ಯ /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
