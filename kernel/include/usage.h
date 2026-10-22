/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#جيڪڏهن_وصف_نه_ٿيل ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#وصف_ڪريو ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#ضمن <zephyr/kernel.h>

#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو


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
فراغ z_sched_usage_stop(فراغ);

فراغ z_sched_usage_شروعات(بنية k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
فراغ z_sched_cpu_usage(uint8_t core_id, بنية k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
فراغ z_sched_thread_usage(بنية k_thread *thread,
			  بنية k_thread_runtime_stats *stats);

ساكن مضمن فراغ z_sched_usage_switch(بنية k_thread *thread)
{
	ARG_اڻ_استعمال_ٿيل(thread);
#جيڪڏهن_وصف_ٿيل CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_شروعات(thread);
#جيڪڏهن_پورو /* CONFIG_SCHED_THREAD_USAGE */
}

#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو


#جيڪڏهن_پورو /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
