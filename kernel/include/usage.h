/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#यदि_अपरिभाषितम् ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#परिभाषय ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#अन्तर्भाव <zephyr/kernel.h>

#यदि_परिभाषितम् __cplusplus
बाह्य "C" {
#यदि_अन्तः


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
शून्य z_sched_usage_stop(शून्य);

शून्य z_sched_usage_start(संरचना k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
शून्य z_sched_cpu_usage(uint8_t core_id, संरचना k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
शून्य z_sched_thread_usage(संरचना k_thread *thread,
			  संरचना k_thread_runtime_stats *stats);

स्थिर अन्तःस्थ शून्य z_sched_usage_switch(संरचना k_thread *thread)
{
	ARG_अप्रयुक्तम्(thread);
#यदि_परिभाषितम् CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_start(thread);
#यदि_अन्तः /* CONFIG_SCHED_THREAD_USAGE */
}

#यदि_परिभाषितम् __cplusplus
}
#यदि_अन्तः


#यदि_अन्तः /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
