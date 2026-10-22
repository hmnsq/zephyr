/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ଅନ୍ତର୍ଭାଵ <zephyr/kernel.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_ଅଙ୍କ_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


ଶୂନ୍ୟ z_sched_ipi(ଶୂନ୍ୟ);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
ଶୂନ୍ୟ flag_ipi(uint32_t ipi_mask);
ଶୂନ୍ୟ signal_pending_ipi(ଶୂନ୍ୟ);
atomic_val_t ipi_mask_create(ସଂରଚନା k_thread *thread);
#ଅନ୍ୟଥା
#define flag_ipi(ipi_mask) କର { } ଯେତେବେଳେ (false)
#define signal_pending_ipi() କର { } ଯେତେବେଳେ (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
ସଂରଚନା k_thread *ipi_idle_reserved_take(ଶୂନ୍ୟ);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
ଶୂନ୍ୟ ipi_idle_thread_unreserve(ସଂରଚନା k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(ସଂରଚନା k_thread *old_thread,
			    ସଂରଚନା k_thread *new_thread);
#ଅନ୍ୟଥା
#define ipi_idle_reserved_take() ଖାଲି
#define ipi_idle_thread_unreserve(thread) କର { \
	ARG_UNUSED(thread); \
} ଯେତେବେଳେ (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
