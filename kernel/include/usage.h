/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#నిర్వచితం_కాకపోతే ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#నిర్వచించు ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#అన్తర్భావ <zephyr/kernel.h>

#నిర్వచితమైతే __cplusplus
బాహ్య "C" {
#అయితే_ముగింపు


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
శూన్య z_sched_usage_stop(శూన్య);

శూన్య z_sched_usage_ప్రారంభం(నిర్మాణం k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
శూన్య z_sched_cpu_usage(uint8_t core_id, నిర్మాణం k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
శూన్య z_sched_thread_usage(నిర్మాణం k_thread *thread,
			  నిర్మాణం k_thread_runtime_stats *stats);

స్థిర అంతఃవరుస శూన్య z_sched_usage_switch(నిర్మాణం k_thread *thread)
{
	ARG_వాడని(thread);
#నిర్వచితమైతే CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_ప్రారంభం(thread);
#అయితే_ముగింపు /* CONFIG_SCHED_THREAD_USAGE */
}

#నిర్వచితమైతే __cplusplus
}
#అయితే_ముగింపు


#అయితే_ముగింపు /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
