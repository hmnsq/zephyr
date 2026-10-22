/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#അന്തര്ഭാവ <zephyr/kernel.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_അക്കം_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


ശൂന്യം z_sched_ipi(ശൂന്യം);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
ശൂന്യം flag_ipi(uint32_t ipi_mask);
ശൂന്യം signal_pending_ipi(ശൂന്യം);
atomic_val_t ipi_mask_create(ഘടന k_thread *thread);
#അല്ലെങ്കിൽ
#define flag_ipi(ipi_mask) ചെയ്യുക { } ആകുമ്പോൾ (false)
#define signal_pending_ipi() ചെയ്യുക { } ആകുമ്പോൾ (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ഘടന k_thread *ipi_idle_reserved_take(ശൂന്യം);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
ശൂന്യം ipi_idle_thread_unreserve(ഘടന k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ഘടന k_thread *old_thread,
			    ഘടന k_thread *new_thread);
#അല്ലെങ്കിൽ
#define ipi_idle_reserved_take() ശൂന്യ
#define ipi_idle_thread_unreserve(thread) ചെയ്യുക { \
	ARG_UNUSED(thread); \
} ആകുമ്പോൾ (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
