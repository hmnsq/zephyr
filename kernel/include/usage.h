/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#ਅਨ੍ਤਰ੍ਭਾਵ <zephyr/kernel.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
ਬਾਹਰੀ "C" {
#ਜੇ_ਅੰਤ


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
ਖਾਲੀ z_sched_usage_stop(ਖਾਲੀ);

ਖਾਲੀ z_sched_usage_ਸ਼ੁਰੂ(ਬਣਤਰ k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
ਖਾਲੀ z_sched_cpu_usage(uint8_t core_id, ਬਣਤਰ k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
ਖਾਲੀ z_sched_thread_usage(ਬਣਤਰ k_thread *thread,
			  ਬਣਤਰ k_thread_runtime_stats *stats);

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ z_sched_usage_switch(ਬਣਤਰ k_thread *thread)
{
	ARG_ਅਣਵਰਤਿਆ(thread);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_ਸ਼ੁਰੂ(thread);
#ਜੇ_ਅੰਤ /* CONFIG_SCHED_THREAD_USAGE */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __cplusplus
}
#ਜੇ_ਅੰਤ


#ਜੇ_ਅੰਤ /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
