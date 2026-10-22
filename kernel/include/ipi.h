/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ಅನ್ತರ್ಭಾವ <zephyr/kernel.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_ಅಂಕೆ_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


ಶೂನ್ಯ z_sched_ipi(ಶೂನ್ಯ);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
ಶೂನ್ಯ flag_ipi(uint32_t ipi_mask);
ಶೂನ್ಯ signal_pending_ipi(ಶೂನ್ಯ);
atomic_val_t ipi_mask_create(ಸಂರಚನಾ k_thread *thread);
#ಅನ್ಯಥಾ
#define flag_ipi(ipi_mask) ಕುರು { } ಯಾವತ್ (false)
#define signal_pending_ipi() ಕುರು { } ಯಾವತ್ (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ಸಂರಚನಾ k_thread *ipi_idle_reserved_take(ಶೂನ್ಯ);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
ಶೂನ್ಯ ipi_idle_thread_unreserve(ಸಂರಚನಾ k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ಸಂರಚನಾ k_thread *old_thread,
			    ಸಂರಚನಾ k_thread *new_thread);
#ಅನ್ಯಥಾ
#define ipi_idle_reserved_take() ರಿಕ್ತ
#define ipi_idle_thread_unreserve(thread) ಕುರು { \
	ARG_UNUSED(thread); \
} ಯಾವತ್ (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
