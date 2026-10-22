/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#define ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#شامل <zephyr/kernel.h>

#ifdef __cplusplus
بیرونی "C" {
#endif


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
تهی z_sched_usage_stop(تهی);

تهی z_sched_usage_start(ساختار k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
تهی z_sched_cpu_usage(uint8_t core_id, ساختار k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
تهی z_sched_thread_usage(ساختار k_thread *thread,
			  ساختار k_thread_runtime_stats *stats);

ایستا درونخطی تهی z_sched_usage_switch(ساختار k_thread *thread)
{
	ARG_UNUSED(thread);
#ifdef CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_start(thread);
#endif /* CONFIG_SCHED_THREAD_USAGE */
}

#ifdef __cplusplus
}
#endif


#endif /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
