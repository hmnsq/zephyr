/*
 * Copyright (c) 2024 Intel Corporation
 * SPDX-License-Identifier: Apache-2.0
 */


#ifndef ZEPHYR_KERNEL_INCLUDE_IPI_H_
#define ZEPHYR_KERNEL_INCLUDE_IPI_H_

#ضمن <zephyr/kernel.h>
#ضمن <stdint.h>
#ضمن <zephyr/sys/atomic.h>

#define IPI_ALL_CPUS_MASK  (UINT32_MAX >> (32 - CONFIG_MP_MAX_رقم_CPUS))

#define IPI_CPU_MASK(cpu_id)   \
	(IS_ENABLED(CONFIG_IPI_OPTIMIZE) ? BIT(cpu_id) : IPI_ALL_CPUS_MASK)


فراغ z_sched_ipi(فراغ);

/* defined in ipi.c when CONFIG_SMP=y */
#ifdef CONFIG_SMP
فراغ flag_ipi(uint32_t ipi_mask);
فراغ signal_pending_ipi(فراغ);
atomic_val_t ipi_mask_create(بنية k_thread *thread);
#وإلا
#define flag_ipi(ipi_mask) افعل { } طالما (false)
#define signal_pending_ipi() افعل { } طالما (false)
#endif /* CONFIG_SMP */

#ifdef CONFIG_IPI_OPTIMIZE_IDLE
/*
 * Consume and return the thread covered by the current CPU's reservation.
 * Returns NULL if no reservation exists. Caller must hold _sched_spinlock.
 */
بنية k_thread *ipi_idle_reserved_take(فراغ);

/*
 * Remove a thread's reservation before it leaves the run queue or changes
 * CPU eligibility. Not needed for a temporary dequeue/requeue with unchanged
 * eligibility.
 *
 * Caller must hold _sched_spinlock. Safe if no reservation exists; does not
 * cancel a pending IPI.
 */
فراغ ipi_idle_thread_unreserve(بنية k_thread *thread);

/*
 * Transfer coverage from old_thread to new_thread using an outstanding idle
 * CPU reservation. Returns true if new_thread is covered without another IPI.
 * Otherwise the reservation is canceled and the caller must request a new IPI.
 *
 * Caller must hold _sched_spinlock.
 */
bool ipi_idle_thread_rebind(بنية k_thread *old_thread,
			    بنية k_thread *new_thread);
#وإلا
#define ipi_idle_reserved_take() عدم
#define ipi_idle_thread_unreserve(thread) افعل { \
	ARG_UNUSED(thread); \
} طالما (false)
#define ipi_idle_thread_rebind(old_thread, new_thread) false
#endif /* CONFIG_IPI_OPTIMIZE_IDLE */

#endif /* ZEPHYR_KERNEL_INCLUDE_IPI_H_ */
