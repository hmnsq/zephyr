/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#اگر_غیر_تعریف_شدہ ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#تعریف_کرو ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#شامل <zephyr/kernel.h>

#اگر_تعریف_شدہ __cplusplus
خارجی "C" {
#اگر_ختم


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
خالی z_sched_usage_stop(خالی);

خالی z_sched_usage_آغاز(ساخت k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
خالی z_sched_cpu_usage(uint8_t core_id, ساخت k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
خالی z_sched_thread_usage(ساخت k_thread *thread,
			  ساخت k_thread_runtime_stats *stats);

ساکن اندرونی خالی z_sched_usage_switch(ساخت k_thread *thread)
{
	ARG_غیر_مستعمل(thread);
#اگر_تعریف_شدہ CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_آغاز(thread);
#اگر_ختم /* CONFIG_SCHED_THREAD_USAGE */
}

#اگر_تعریف_شدہ __cplusplus
}
#اگر_ختم


#اگر_ختم /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
