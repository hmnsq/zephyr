/*
 * Copyright The Zephyr Project Contributors
 * SPDX-License-Identifier: Apache-2.0
 */

#अगर_अघोषित ZEPHYR_KERNEL_INCLUDE_USAGE_H_
#घोषणा ZEPHYR_KERNEL_INCLUDE_USAGE_H_

#समावेश <zephyr/kernel.h>

#अगर_घोषित __cplusplus
बाह्य "C" {
#पूर्ण_अगर


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
व्योम z_sched_usage_stop(व्योम);

व्योम z_sched_usage_start(काष्ठा k_thread *thread);

/**
 * @brief Retrieves CPU cycle usage data for specified core
 */
व्योम z_sched_cpu_usage(uint8_t core_id, काष्ठा k_thread_runtime_stats *stats);

/**
 * @brief Retrieves thread cycle usage data for specified thread
 */
व्योम z_sched_thread_usage(काष्ठा k_thread *thread,
			  काष्ठा k_thread_runtime_stats *stats);

अटल अंतरभूत व्योम z_sched_usage_switch(काष्ठा k_thread *thread)
{
	ARG_UNUSED(thread);
#अगर_घोषित CONFIG_SCHED_THREAD_USAGE
	z_sched_usage_stop();
	z_sched_usage_start(thread);
#पूर्ण_अगर /* CONFIG_SCHED_THREAD_USAGE */
}

#अगर_घोषित __cplusplus
}
#पूर्ण_अगर


#पूर्ण_अगर /* ZEPHYR_KERNEL_INCLUDE_USAGE_H_ */
